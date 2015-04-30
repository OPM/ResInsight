/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"

class RiuViewer;
class Rim3dOverlayInfoConfig;

#define CAF_PDM_ABSTRACT_SOURCE_INIT(ClassName, keyword) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_HEADER_INIT_and_or_CAF_PDM_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false;} \
    QString ClassName::classKeywordStatic() { assert(PdmObject::isValidXmlElementName(keyword)); return keyword;   } 

//==================================================================================================
///  
///  
//==================================================================================================
class RimView : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimView(void);
    virtual ~RimView(void);

    // 3D Viewer
    RiuViewer*                          viewer();

    caf::PdmField<QString>              name;
    caf::PdmField<double>               scaleZ;

    caf::PdmField<bool>                 showWindow;
    caf::PdmField<cvf::Mat4d>           cameraPosition;
    caf::PdmField< cvf::Color3f >       backgroundColor;

    caf::PdmField<int>                  maximumFrameRate;
    caf::PdmField<bool>                 animationMode;

    // Animation
    int                                 currentTimeStep()    { return m_currentTimeStep;}
    virtual void                        setCurrentTimeStep(int frameIdx) = 0;
    virtual void                        updateCurrentTimeStepAndRedraw() = 0;
    virtual void                        endAnimation() = 0;

    void                                scheduleCreateDisplayModelAndRedraw();
    virtual void                        createDisplayModelAndRedraw() = 0;

public:
    virtual caf::PdmFieldHandle*        objectToggleField()     { return &showWindow; }
    virtual caf::PdmFieldHandle*        userDescriptionField()  { return &name; }
protected:
    //void                                updateViewerWidget();
    virtual void                        resetLegendsInViewer() = 0;
    void                                updateViewerWidget();
    virtual void                        updateViewerWidgetWindowTitle() = 0;
    QPointer<RiuViewer>                     m_viewer;
    caf::PdmField<int>                      m_currentTimeStep;
    caf::PdmField<Rim3dOverlayInfoConfig*>              overlayInfoConfig;

};



