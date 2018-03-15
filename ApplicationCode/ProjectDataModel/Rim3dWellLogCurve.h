/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RimCase.h"

class RimGeoMechResultDefinition;
class RimEclipseResultDefinition;

//==================================================================================================
///
///
//==================================================================================================
class Rim3dWellLogCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum DrawPlane
    {
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT,
        VERTICAL_ABOVE,
        VERTICAL_BELOW,
        CAMERA_ALIGNED_SIDE1,
        CAMERA_ALIGNED_SIDE2
    };

    enum DrawStyle
    {
        LINE,
        FILLED
    };

    enum ColoringStyle
    {
        SINGLE_COLOR,
        CURVE_VALUE,
        OTHER_RESULT
    };

public:
    Rim3dWellLogCurve();
    virtual ~Rim3dWellLogCurve();

    void updateCurveIn3dView();

    void setPropertiesFromView(Rim3dView* view);

    DrawPlane drawPlane() const;
    bool toggleState() const;

    virtual void curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const;

protected:
    virtual caf::PdmFieldHandle*            objectToggleField() override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual caf::PdmFieldHandle*            userDescriptionField() override;

    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void initAfterRead() override;

    void appearanceUiOrdering(caf::PdmUiOrdering& uiOrdering);

protected:
    caf::PdmField<QString>                          m_name;

private:
    caf::PdmField<bool>                             m_showCurve;

    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;

    caf::PdmField<caf::AppEnum<DrawPlane>> m_drawPlane;
    caf::PdmField<caf::AppEnum<DrawStyle>> m_drawStyle;
    caf::PdmField<caf::AppEnum<ColoringStyle>> m_coloringStyle;
};
