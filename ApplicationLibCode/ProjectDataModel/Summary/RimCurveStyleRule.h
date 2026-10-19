/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RimCurveStyleSettings.h"
#include "RimPlotCurveAppearance.h"

#include "RiuPlotCurveSymbol.h"

#include "cafAppEnum.h"
#include "cafIconProvider.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"

#include "cvfColor3.h"

#include <QString>
#include <QStringList>

#include <map>
#include <optional>

class RimPlotCurve;

//==================================================================================================
/// Project-stored curve appearance for one specific summary case, ensemble or well. The values are
/// edited in the curve appearance dock window.
//==================================================================================================
class RimCurveStyleRule : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using Dimension = RimCurveStyleSettings::Dimension;

    enum class Mode
    {
        AUTO,
        CUSTOM
    };

    RimCurveStyleRule();

    Dimension dimension() const;
    QString   itemName() const;

    void setDimension( Dimension dimension );
    void setItemName( const QString& itemName );
    void setCustom( bool isCustom );

    // Copies the mode and appearance, not the item the rule applies to
    void copySettingsFrom( const RimCurveStyleRule& other );

    // Applies the rule to the curve if it is custom. Color and symbol are only applied when requested
    void applyToCurve( RimPlotCurve* curve, bool applyColor, bool applySymbol ) const;

    static QStringList availableNames( Dimension dimension );

    // Caches availableNames() while in scope, as finding them can require scanning all summary addresses
    class AvailableNamesCache
    {
    public:
        AvailableNamesCache();
        ~AvailableNamesCache();
        AvailableNamesCache( const AvailableNamesCache& )            = delete;
        AvailableNamesCache& operator=( const AvailableNamesCache& ) = delete;

    private:
        bool m_isOwner;
    };

    struct PlotAppearance
    {
        std::optional<cvf::Color3f>                        color;
        std::optional<RiuPlotCurveSymbol::PointSymbolEnum> symbol;
    };

    // The appearance of the first curve for each item in plots using the global appearance
    static std::map<QString, PlotAppearance> appearancesFromPlots( Dimension dimension );

    static caf::IconProvider swatchIcon( const cvf::Color3f& color );
    static caf::IconProvider paletteIcon();
    static caf::IconProvider symbolIcon();
    static caf::IconProvider dimensionIcon( Dimension dimension );

    // Dimension icon followed by color and/or symbol swatches, when given. The symbol is drawn with the color if present.
    static caf::IconProvider dimensionColorIcon( Dimension                                                 dimension,
                                                 const std::optional<cvf::Color3f>&                        color,
                                                 const std::optional<RiuPlotCurveSymbol::PointSymbolEnum>& symbol = std::nullopt );
    bool                     isCustom() const;

    // The custom symbol, if the rule is custom
    std::optional<RiuPlotCurveSymbol::PointSymbolEnum> overriddenSymbol() const;

    // Adds the mode, color and symbol fields, to be edited for the object selected in the collection
    void appendFieldsToUiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering );

    // The properties currently assigned automatically by the global appearance, shown while the item is Auto
    void setAutoAssigned( const std::optional<cvf::Color3f>& color, const std::optional<RiuPlotCurveSymbol::PointSymbolEnum>& symbol );
    void updateAppearanceFromCurves();

    // The custom color, if the rule is custom
    std::optional<cvf::Color3f> overriddenColor() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;

private:
    void copyAppearanceFromCurves();
    void connectAppearanceSignal();
    void onAppearanceChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<caf::AppEnum<Dimension>> m_dimension;
    caf::PdmField<QString>                 m_itemName;
    caf::PdmField<caf::AppEnum<Mode>>      m_mode;

    // Color, symbol, line and fill settings that can be overridden
    caf::PdmChildField<RimPlotCurveAppearance*> m_curveAppearance;

    std::optional<cvf::Color3f>                        m_autoColor;
    std::optional<RiuPlotCurveSymbol::PointSymbolEnum> m_autoSymbol;
};