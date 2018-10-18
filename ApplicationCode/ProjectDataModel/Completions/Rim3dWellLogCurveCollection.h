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
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfVector3.h"

class Rim3dWellLogCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dWellLogCurveCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    Rim3dWellLogCurveCollection();
    ~Rim3dWellLogCurveCollection() override;

    bool has3dWellLogCurves() const;
    void add3dWellLogCurve(Rim3dWellLogCurve* curve);
    void remove3dWellLogCurve(Rim3dWellLogCurve* curve);

    bool isShowingPlot() const;
    bool isShowingGrid() const;
    bool isShowingBackground() const;

    float         planeWidthScaling() const;

    std::vector<Rim3dWellLogCurve*> vectorOf3dWellLogCurves() const;
    void                            redrawAffectedViewsAndEditors();
    Rim3dWellLogCurve*              checkForCurveIntersection(const cvf::Vec3d& globalIntersection,
                                                              cvf::Vec3d*       closestPoint,
                                                              double*           measuredDepthAtPoint,
                                                              double*           valueAtPoint);

private:
    void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                 defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
private:
    caf::PdmField<bool>                         m_showPlot;
    caf::PdmField<float>                        m_planeWidthScaling;

    caf::PdmField<bool>                         m_showGrid;
    caf::PdmField<bool>                         m_showBackground;


    caf::PdmChildArrayField<Rim3dWellLogCurve*> m_3dWellLogCurves;
};
