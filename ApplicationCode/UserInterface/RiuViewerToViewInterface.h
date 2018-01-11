/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfColor3.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include <QString>

class RimCase;
class RimViewLinker;
struct RimMdiWindowGeometry;
class RimViewController;

namespace caf
{
    class PdmObjectHandle;
    class DisplayCoordTransform;
}


class RiuViewerToViewInterface
{
public:
    virtual caf::PdmObjectHandle* implementingPdmObject() = 0;

    virtual void handleMdiWindowClosed() = 0;
    virtual void setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry) = 0;

    virtual void setCameraPosition(const cvf::Mat4d & cameraPosition) = 0;
    virtual void setCameraPointOfInterest(const cvf::Vec3d& cameraPointOfInterest) = 0;
    
    virtual cvf::Color3f backgroundColor() const = 0;
    virtual void selectOverlayInfoConfig() = 0;

    virtual RimViewLinker* assosiatedViewLinker() const = 0;
    virtual RimViewController* viewController() const = 0;

    virtual QString  timeStepName( int  ) const = 0;
    virtual cvf::ref<caf::DisplayCoordTransform>  displayCoordTransform() const = 0;

    virtual void setCurrentTimeStepAndUpdate(int frameIndex) = 0;
    virtual void updateCurrentTimeStepAndRedraw() = 0;
    virtual void endAnimation() = 0;
};
