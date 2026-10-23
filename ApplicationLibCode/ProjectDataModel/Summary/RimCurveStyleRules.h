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

#include "RimCurveStyleRule.h"
#include "RimPlotCurveAppearance.h"
#include "RimSavedCurveAppearance.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfColor3.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

class RimColorLegend;
class RimEnsembleCurveSet;
class RimPlotCurve;
class RimSummaryPlot;
class RifEclipseSummaryAddress;

//==================================================================================================
/// Project-stored list of per-item style rules for the global curve appearance.
//==================================================================================================
class RimCurveStyleRules : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCurveStyleRules();

    std::vector<RimCurveStyleRule*> rules() const;
    RimCurveStyleRule*              findOrCreateRule( RimCurveStyleRule::Dimension dimension, const QString& name );

    // Custom rules for the dimensions selected for color or symbol, ordered so that later entries win: by dimension
    std::vector<const RimCurveStyleRule*> rulesFor( const RifEclipseSummaryAddress& address, const std::string& caseName ) const;

    RimCurveStyleSettings* settings() const;

    // Re-applies the global appearance to all plots using it
    void rulesChanged();

    // Stores custom settings edited for the selected item, and re-applies the global appearance
    void ruleChanged( RimCurveStyleRule* rule );

    // Whether the global appearance assigns the color or symbol of a curve with the given address and case
    bool controlsColor( const RifEclipseSummaryAddress& address, const std::string& caseName ) const;
    bool controlsSymbol( const RifEclipseSummaryAddress& address, const std::string& caseName ) const;

    // Index used to pick the palette color and symbol for a case or well name. Indices are kept in the project so that
    // existing items keep their appearance when new cases or wells are added.
    // Names not yet registered get the indices they would get if registered now.
    void                   registerItemNames( RimCurveStyleRule::Dimension dimension, const QStringList& names );
    std::map<QString, int> itemIndices( RimCurveStyleRule::Dimension dimension ) const;

    // Color from the project color legend used for auto colors, if one is selected
    std::optional<cvf::Color3f> autoLegendColor( int index ) const;

    // Configures a field as a label with a link that shows the global curve appearance
    static void addGlobalAppearanceLink( caf::PdmUiOrdering& uiOrdering, const QString& text );

    // The global appearance applies to all summary plots in the project
    bool useGlobalAppearance() const;
    void setUseGlobalAppearance( bool enable );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

public:
    enum class AppearanceType
    {
        COLOR,
        SYMBOL
    };

private:
    void               assignedAppearance( const QString&                                              name,
                                           int                                                         index,
                                           const std::map<QString, RimCurveStyleRule::PlotAppearance>& plotAppearances,
                                           std::optional<cvf::Color3f>&                                color,
                                           std::optional<RiuPlotCurveSymbol::PointSymbolEnum>&         symbol ) const;
    RimCurveStyleRule* findRule( RimCurveStyleRule::Dimension dimension, const QString& name ) const;
    QString            currentItemName() const;
    bool               updateSelectedItem();

    // The dimension controlling the selected appearance type, as stored in the global settings
    caf::AppEnum<RimCurveStyleRule::Dimension> currentDimension() const;
    void                                       setCurrentDimension( const caf::AppEnum<RimCurveStyleRule::Dimension>& dimension );

    caf::PdmField<std::vector<QString>>*       itemOrder( RimCurveStyleRule::Dimension dimension );
    const caf::PdmField<std::vector<QString>>* itemOrder( RimCurveStyleRule::Dimension dimension ) const;

    void applyGlobalAppearanceMode();
    void saveAppearance( RimSummaryPlot* plot );
    void restoreAppearances();

    // The curve appearance before the global appearance was enabled, restored when disabled
    caf::PdmChildArrayField<RimSavedCurveAppearance*> m_savedAppearances;
    caf::PdmField<bool>                               m_useGlobalAppearance;

    caf::PdmChildField<RimCurveStyleSettings*>                          m_settings;
    caf::PdmChildArrayField<RimCurveStyleRule*>                         m_rules;
    caf::PdmField<std::vector<QString>>                                 m_caseOrder;
    caf::PdmField<std::vector<QString>>                                 m_wellOrder;
    caf::PdmField<caf::AppEnum<AppearanceType>>                         m_appearanceType;
    caf::PdmProxyValueField<caf::AppEnum<RimCurveStyleRule::Dimension>> m_currentType;
    caf::PdmPtrField<RimColorLegend*>                                   m_autoColorLegend;
    caf::PdmField<int>                                                  m_symbolLegend;
    caf::PdmField<int>                                                  m_currentItemIndex;

    // Edited in the dock window for the selected item. Not stored, custom settings are copied to a rule in m_rules.
    caf::PdmChildField<RimCurveStyleRule*> m_selectedItem;
};
