/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

namespace cvf
{
    class Part;
    class ModelBasicList;
    class Transform;
}

class RimWell;

class RivWellHeadPartMgr : public cvf::Object
{
public:
    RivWellHeadPartMgr(RimReservoirView* reservoirView, RimWell* well);
    ~RivWellHeadPartMgr();

    void setScaleTransform(cvf::Transform * scaleTransform) { m_scaleTransform = scaleTransform;}

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);


private:
    void buildWellHeadParts(size_t frameIndex);


private:
    caf::PdmPointer<RimReservoirView>   m_rimReservoirView;
    caf::PdmPointer<RimWell>            m_rimWell;
    
    cvf::ref<cvf::Transform>            m_scaleTransform; 
    cvf::Collection< cvf::Part >        m_wellHeadParts;
};
