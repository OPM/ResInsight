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
    caf::PdmField< bool >           showInactiveCells;

    cvf::Mat4d                      boxOrigin() const;
    cvf::Vec3d                      boxSize()   const;
    SinglePlaneState                singlePlaneState() const; 

    RivIntersectionBoxPartMgr*      intersectionBoxPartMgr();

    void                            setToDefaultSizeBox();
    void                            setToDefaultSizeSlice(SinglePlaneState plane, const cvf::Vec3d& position);

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
    void                            updateLabelsFromBoundingBox();
    void                            clampSinglePlaneValues();
    void                            switchSingelPlaneState();

private:
    caf::PdmField<caf::AppEnum< SinglePlaneState > > m_singlePlaneState;
    
    caf::PdmField<double>           m_minXCoord;
    caf::PdmField<double>           m_minYCoord;
    caf::PdmField<double>           m_minDepth;

    caf::PdmField<double>           m_maxXCoord;
    caf::PdmField<double>           m_maxYCoord;
    caf::PdmField<double>           m_maxDepth;


    cvf::BoundingBox                        currentCellBoundingBox();

    cvf::ref<RivIntersectionBoxPartMgr>     m_intersectionBoxPartMgr;
};
