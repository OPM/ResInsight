/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafPdmObject.h"

#include "cafPdmField.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"


class RimEllipseFractureTemplate;
class RigFracture;
class RivWellFracturePartMgr;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFracture : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimFracture(void);
    virtual ~RimFracture(void);

    caf::PdmField<double>           azimuth;

    virtual cvf::Vec3d              centerPointForFracture() = 0;
    cvf::Mat4f                      transformMatrix(); 

    virtual RimEllipseFractureTemplate*  attachedFractureDefinition() = 0;
    const RigFracture*                  attachedRigFracture() const;

    RivWellFracturePartMgr*         fracturePartManager();

    bool                            hasValidGeometry() const;
    void                            computeGeometry();
    void                            setRecomputeGeometryFlag();
    
    const std::vector<cvf::uint>&   triangleIndices() const;
    const std::vector<cvf::Vec3f>&  nodeCoords() const;


    virtual std::vector<size_t>     getPotentiallyFracturedCells();
    void                                            computeTransmissibility();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

private:
    bool                            isRecomputeGeometryFlagSet();

    //Functions for area calculations - should these be in separate class
    bool planeCellIntersection(size_t cellindex, std::vector<std::vector<cvf::Vec3d> > & polygons);
    void calculateFracturePlaneCellPolygonOverlap(std::vector<std::vector<cvf::Vec3d> > planeCellPolygons,
        std::vector<cvf::Vec3f> fracturePolygon, double & area);


private:
    cvf::ref<RigFracture>   m_rigFracture;
    bool                    m_recomputeGeometry;

    cvf::ref<RivWellFracturePartMgr> m_rivFracture;
};
