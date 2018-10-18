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

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <QObject>
#include <QPointer>

class RicBoxManipulatorEventHandler;
class RiuViewer;
class RivIntersectionBoxPartMgr;

namespace cvf {
    class ModelBasicList;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIntersectionBox : public QObject, public caf::PdmObject
{
    Q_OBJECT;

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
    ~RimIntersectionBox() override;

    // Fields
    caf::PdmField<QString>          name;
    caf::PdmField<bool>             isActive;
    caf::PdmField<bool>             showInactiveCells;
    
    cvf::Mat4d                      boxOrigin() const;
    cvf::Vec3d                      boxSize()   const;
    SinglePlaneState                singlePlaneState() const; 

    bool                            show3dManipulator() const;
    RivIntersectionBoxPartMgr*      intersectionBoxPartMgr();
    void                            appendManipulatorPartsToModel(cvf::ModelBasicList* model);

    void                            setToDefaultSizeBox();
    void                            setToDefaultSizeSlice(SinglePlaneState plane, const cvf::Vec3d& position);

protected:
    caf::PdmFieldHandle*    userDescriptionField() override;
    caf::PdmFieldHandle*    objectToggleField() override;
                                            
    void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;
    void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                    initAfterRead() override;

protected slots:
    void                            slotScheduleRedraw();
    void                            slotUpdateGeometry(const cvf::Vec3d& origin, const cvf::Vec3d& size);

private:
    friend class RimIntersectionCollection;
    void                            updateBoxManipulatorGeometry();
    
    void                            rebuildGeometryAndScheduleCreateDisplayModel();
    void                            updateVisibility();
    void                            clampSinglePlaneValues();
    void                            switchSingelPlaneState();
    
    void                            setFromOriginAndSize(const cvf::Vec3d& origin, const cvf::Vec3d& size);

    cvf::BoundingBox                currentCellBoundingBox();

    RiuViewer*                      viewer();

private:
    caf::PdmField<caf::AppEnum< SinglePlaneState > > m_singlePlaneState;
    
    caf::PdmField<double>           m_minXCoord;
    caf::PdmField<double>           m_minYCoord;
    caf::PdmField<double>           m_minDepth;

    caf::PdmField<double>           m_maxXCoord;
    caf::PdmField<double>           m_maxYCoord;
    caf::PdmField<double>           m_maxDepth;

    caf::PdmField<double>           m_xySliderStepSize;
    caf::PdmField<double>           m_depthSliderStepSize;

    caf::PdmField<bool>             m_show3DManipulator;



    cvf::ref<RivIntersectionBoxPartMgr>     m_intersectionBoxPartMgr;
    QPointer<RicBoxManipulatorEventHandler> m_boxManipulator;
};
