/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cvfBoundingBox.h"

class RivIntersectionBoxPartMgr;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIntersectionBox : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum SinglePlaneState
    {
        PLANE_STATE_NONE,
        PLANE_STATE_X,
        PLANE_STATE_Y,
        PLANE_STATE_Z
    };

public:
    RimIntersectionBox();
    ~RimIntersectionBox();

    // Fields
    caf::PdmField<QString>          name;
    caf::PdmField<bool>             isActive;

    cvf::Mat4d                      boxOrigin() const;
    cvf::Vec3d                      boxSize()   const;

    RivIntersectionBoxPartMgr*      intersectionBoxPartMgr();

    void                            initialize();
    void                            setModelBoundingBox(cvf::BoundingBox& boundingBox);

    void                            setXSlice(double xValue);
    void                            setYSlice(double yValue);
    void                            setZSlice(double zValue);

    void updateLabelsFromBoundingBox();

protected:
    virtual caf::PdmFieldHandle*    userDescriptionField() override;
    virtual caf::PdmFieldHandle*    objectToggleField() override;
                                            
    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                    initAfterRead() override;

private:
    void                            rebuildGeometryAndScheduleCreateDisplayModel();
    void                            updateVisibility();
    void                            clampSinglePlaneValues();

private:
    caf::PdmField<caf::AppEnum< SinglePlaneState > > singlePlaneState;
    
    caf::PdmField<double>           minXCoord;
    caf::PdmField<double>           minYCoord;
    caf::PdmField<double>           minZCoord;

    caf::PdmField<double>           maxXCoord;
    caf::PdmField<double>           maxYCoord;
    caf::PdmField<double>           maxZCoord;


    cvf::BoundingBox                        m_boundingBox;

    cvf::ref<RivIntersectionBoxPartMgr>     m_intersectionBoxPartMgr;
};
