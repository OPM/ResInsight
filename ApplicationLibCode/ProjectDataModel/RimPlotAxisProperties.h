/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaDefines.h"
#include "RimPlotAxisPropertiesInterface.h"

#include "RiuPlotAxis.h"

#include "cafAppEnum.h"
#include "cafFontTools.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RimPlotAxisAnnotation;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotAxisProperties : public RimPlotAxisPropertiesInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum NumberFormatType
    {
        NUMBER_FORMAT_AUTO,
        NUMBER_FORMAT_DECIMAL,
        NUMBER_FORMAT_SCIENTIFIC
    };

public:
    caf::Signal<bool>                                             logarithmicChanged;
    caf::Signal<RimPlotAxisProperties*, RiuPlotAxis, RiuPlotAxis> axisPositionChanged;

public:
    RimPlotAxisProperties();

    void setAlwaysRequired( bool enable );

    void                  setEnableTitleTextSettings( bool enable );
    void                  enableRangeSettings( bool enable );
    void                  setNameForUnusedAxis();
    void                  setNameAndAxis( const QString& name, RiaDefines::PlotAxis axis, int axisIndex = 0 );
    AxisTitlePositionType titlePosition() const override;

    int titleFontSize() const override;
    int valuesFontSize() const override;

    const QString& name() const override;
    RiuPlotAxis    plotAxisType() const override;
    bool           useAutoTitle() const;

    void setShowDescription( bool enable );
    bool showDescription() const;

    void setShowAcronym( bool enable );
    bool showAcronym() const;

    void setShowUnitText( bool enable );
    bool showUnitText() const;

    bool isAutoZoom() const override;
    void setAutoZoom( bool enableAutoZoom ) override;
    bool isAxisInverted() const override;
    void setAxisInverted( bool inverted );
    bool showNumbers() const;
    void setShowNumbers( bool enable );

    void setVisible( bool visible );
    void computeAndSetScaleFactor();

    bool isDeletable() const override;

    std::vector<RimPlotAxisAnnotation*> annotations() const override;
    void                                appendAnnotation( RimPlotAxisAnnotation* annotation ) override;
    void                                removeAllAnnotations() override;

    caf::PdmField<QString> customTitle;

    caf::PdmField<caf::AppEnum<NumberFormatType>> numberFormat;
    caf::PdmField<int>                            numberOfDecimals;
    caf::PdmField<double>                         scaleFactor;

    bool isLogarithmicScaleEnabled() const override;
    bool isActive() const override;

    void showAnnotationObjectsInProjectTree();

    double visibleRangeMin() const override;
    double visibleRangeMax() const override;

    void setVisibleRangeMin( double value ) override;
    void setVisibleRangeMax( double value ) override;

    LegendTickmarkCount majorTickmarkCount() const override;
    void                setMajorTickmarkCount( LegendTickmarkCount count ) override;

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    void                     updateOptionSensitivity();
    caf::FontTools::FontSize plotFontSize() const;

private:
    caf::PdmField<bool> m_isActive;

    caf::PdmField<bool> isAutoTitle;
    caf::PdmField<bool> m_displayShortName;
    caf::PdmField<bool> m_displayLongName;
    caf::PdmField<bool> m_displayUnitText;
    caf::PdmField<bool> m_isAutoZoom;
    caf::PdmField<bool> m_isAxisInverted;
    caf::PdmField<bool> m_showNumbers;

    caf::PdmField<double> m_visibleRangeMin;
    caf::PdmField<double> m_visibleRangeMax;

    caf::PdmField<QString>                            m_name;
    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis;
    caf::PdmField<int>                                m_plotAxisIndex;
    caf::PdmField<LegendTickmarkCountEnum>            m_majorTickmarkCount;

    caf::PdmField<bool> m_isLogarithmicScaleEnabled;

    bool m_enableTitleTextSettings;
    bool m_isRangeSettingsEnabled;
    bool m_isAlwaysRequired;

    caf::PdmField<caf::FontTools::RelativeSizeEnum>    m_titleFontSize;
    caf::PdmField<caf::AppEnum<AxisTitlePositionType>> m_titlePositionEnum;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>    m_valuesFontSize;
    caf::PdmChildArrayField<RimPlotAxisAnnotation*>    m_annotations;
};
