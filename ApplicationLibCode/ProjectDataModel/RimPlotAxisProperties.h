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
class RimPlotAxisProperties : public caf::PdmObject, public RimPlotAxisPropertiesInterface
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
    caf::Signal<>     settingsChanged;
    caf::Signal<bool> logarithmicChanged;

public:
    RimPlotAxisProperties();

    void                  setEnableTitleTextSettings( bool enable );
    void                  enableRangeSettings( bool enable );
    void                  setNameAndAxis( const QString& name, RiaDefines::PlotAxis axis );
    AxisTitlePositionType titlePosition() const override;

    int titleFontSize() const override;
    int valuesFontSize() const override;

    QString              name() const;
    RiaDefines::PlotAxis plotAxisType() const override;
    bool                 useAutoTitle() const;
    bool                 showDescription() const;
    bool                 showAcronym() const;
    bool                 showUnitText() const;
    bool                 isAutoZoom() const;
    void                 setAutoZoom( bool enableAutoZoom );
    bool                 isAxisInverted() const;
    void                 setAxisInverted( bool inverted );

    std::vector<RimPlotAxisAnnotation*> annotations() const override;
    void                                appendAnnotation( RimPlotAxisAnnotation* annotation ) override;
    void                                removeAllAnnotations() override;

    caf::PdmField<QString> customTitle;

    caf::PdmField<double> visibleRangeMin;
    caf::PdmField<double> visibleRangeMax;

    caf::PdmField<caf::AppEnum<NumberFormatType>> numberFormat;
    caf::PdmField<int>                            numberOfDecimals;
    caf::PdmField<double>                         scaleFactor;
    caf::PdmField<bool>                           isLogarithmicScaleEnabled;

    bool isActive() const;

    void setInvertedAxis( bool enable );
    void showAnnotationObjectsInProjectTree();

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

    caf::PdmField<QString> m_name;
    RiaDefines::PlotAxis   m_axis;

    bool m_enableTitleTextSettings;
    bool m_isRangeSettingsEnabled;

    caf::PdmField<caf::FontTools::RelativeSizeEnum>    m_titleFontSize;
    caf::PdmField<caf::AppEnum<AxisTitlePositionType>> m_titlePositionEnum;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>    m_valuesFontSize;
    caf::PdmChildArrayField<RimPlotAxisAnnotation*>    m_annotations;
};
