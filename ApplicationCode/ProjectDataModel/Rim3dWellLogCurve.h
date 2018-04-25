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
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cafPdmFieldCvfColor.h"

#include "cvfObject.h"
#include "cvfVector3.h"

class Riv3dWellLogCurveGeometryGenerator;

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
        VERTICAL_ABOVE,
        VERTICAL_BELOW,
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT
    };
    typedef caf::AppEnum<DrawPlane> DrawPlaneEnum;
public:
    Rim3dWellLogCurve();
    virtual ~Rim3dWellLogCurve();

    void updateCurveIn3dView();

    const QString&  name() const;
    virtual QString resultPropertyString() const = 0;
    DrawPlane       drawPlane() const;
    cvf::Color3f    color() const;
    bool            isShowingCurve() const;

    virtual void    curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const = 0;

    void            setColor(const cvf::Color3f& color);

    double          minCurveValue() const;
    double          maxCurveValue() const;
    void            resetMinMaxValuesAndUpdateUI();
    bool            findClosestPointOnCurve(const cvf::Vec3d& globalIntersection,
                                           cvf::Vec3d*       closestPoint,
                                            double*           measuredDepthAtPoint,
                                            double*           valueAtPoint) const;

    void setGeometryGenerator(Riv3dWellLogCurveGeometryGenerator* generator);
    cvf::ref<Riv3dWellLogCurveGeometryGenerator> geometryGenerator();
    
protected:
    virtual caf::PdmFieldHandle*            objectToggleField() override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual caf::PdmFieldHandle*            userDescriptionField() override;
    void                                    configurationUiOrdering(caf::PdmUiOrdering& uiOrdering);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);    
private:
    void                                    resetMinMaxValues();
protected:
    caf::PdmField<QString>                          m_name;
    caf::PdmField<caf::AppEnum<DrawPlane>>          m_drawPlane;
    caf::PdmField<cvf::Color3f>                     m_color;
    caf::PdmField<double>                           m_minCurveValue;
    caf::PdmField<double>                           m_maxCurveValue;
    double                                          m_minCurveDataValue;
    double                                          m_maxCurveDataValue;
    cvf::ref<Riv3dWellLogCurveGeometryGenerator>    m_geometryGenerator;
private:
    caf::PdmField<bool>                             m_showCurve;

};
