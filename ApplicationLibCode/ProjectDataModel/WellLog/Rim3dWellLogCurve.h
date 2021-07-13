/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "RimNameConfig.h"

class Riv3dWellLogCurveGeometryGenerator;
class Rim3dView;

//==================================================================================================
///
///
//==================================================================================================
class Rim3dWellLogCurve : public caf::PdmObject, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum DrawPlane
    {
        VERTICAL_ABOVE,
        VERTICAL_CENTER,
        VERTICAL_BELOW,
        HORIZONTAL_LEFT,
        HORIZONTAL_CENTER,
        HORIZONTAL_RIGHT
    };
    typedef caf::AppEnum<DrawPlane> DrawPlaneEnum;

public:
    Rim3dWellLogCurve();
    ~Rim3dWellLogCurve() override;

    void updateCurveIn3dView();

    virtual QString name() const                 = 0;
    virtual QString resultPropertyString() const = 0;

    DrawPlane     drawPlane() const;
    static double drawPlaneAngle( DrawPlane drawPlane );

    cvf::Color3f color() const;
    bool         isShowingCurve() const;
    virtual bool isShowingTimeDependentResult() const { return isShowingCurve(); }
    virtual bool showInView( const Rim3dView* gridView ) const { return isShowingCurve(); }
    virtual bool followAnimationTimeStep() const { return false; }
    virtual void curveValuesAndMds( std::vector<double>* values, std::vector<double>* measuredDepthValues ) const = 0;
    virtual void curveValuesAndMdsAtTimeStep( std::vector<double>* values,
                                              std::vector<double>* measuredDepthValues,
                                              int                  timeStep ) const;
    virtual std::pair<double, double> findCurveValueRange();

    void setColor( const cvf::Color3f& color );

    float minCurveUIValue() const;
    float maxCurveUIValue() const;
    void  resetMinMaxValues();
    bool  findClosestPointOnCurve( const cvf::Vec3d& globalIntersection,
                                   cvf::Vec3d*       closestPoint,
                                   double*           measuredDepthAtPoint,
                                   double*           valueAtPoint ) const;

    void                                         setGeometryGenerator( Riv3dWellLogCurveGeometryGenerator* generator );
    cvf::ref<Riv3dWellLogCurveGeometryGenerator> geometryGenerator();

protected:
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 configurationUiOrdering( caf::PdmUiOrdering& uiOrdering );
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;
    void                 initAfterRead() override;

protected:
    caf::PdmField<DrawPlaneEnum>                 m_drawPlane;
    caf::PdmField<cvf::Color3f>                  m_color;
    caf::PdmField<float>                         m_minCurveUIValue;
    caf::PdmField<float>                         m_maxCurveUIValue;
    float                                        m_minCurveDataValue;
    float                                        m_maxCurveDataValue;
    cvf::ref<Riv3dWellLogCurveGeometryGenerator> m_geometryGenerator;

private:
    caf::PdmField<bool> m_showCurve;
};
