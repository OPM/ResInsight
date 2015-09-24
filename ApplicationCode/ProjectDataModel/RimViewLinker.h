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

#include "RimDefines.h"

#include "RivCellSetEnum.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

namespace cvf
{
    class BoundingBox;
}

class RimViewController;
class RiuViewer;
class RimView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewLinker : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewLinker(void);
    virtual ~RimViewLinker(void);
    
    bool                                    isActive();

    void                                    setMasterView(RimView* view);
    RimView*                                masterView();

    void                                    addDependentView(RimView* view);
    void                                    updateDependentViews();

    void                                    updateCamera(RimView* sourceView);
    void                                    updateTimeStep(RimView* sourceView, int timeStep);
    void                                    updateScaleZ(RimView* sourceView, double scaleZ);

    void                                    updateCellResult();

    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType);
    void                                    scheduleCreateDisplayModelAndRedrawForDependentViews();

    void                                    allViews(std::vector<RimView*>& views);

    void                                    updateUiNameAndIcon();

    void                                    addViewControllers(caf::PdmUiTreeOrdering& uiTreeOrdering);

    static void                             applyIconEnabledState(caf::PdmObject* obj, const QIcon& icon, bool disable);
    static void                             findNameAndIconFromView(QString* name, QIcon* icon, RimView* view);

public:
    static QString                          displayNameForView(RimView* view);

    // Move to RimView and make nonstatic
    static RimViewController*               viewLinkForView(const RimView* view);
    static RimViewLinker*                   viewLinkerIfMainView(RimView* view);
    static RimViewLinker*                   viewLinkerForMainOrControlledView(RimView* view);

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual void                            initAfterRead();

private:
    void                                    allViewsForCameraSync(RimView* source, std::vector<RimView*>& views);
    
    void                                    updateOverrides();
    void                                    removeOverrides();

    static bool                             isBoundingBoxesOverlappingOrClose(const cvf::BoundingBox& sourceBB, const cvf::BoundingBox& destBB);

private:
    caf::PdmChildArrayField<RimViewController*>   viewLinks;
    caf::PdmPtrField<RimView*>                    m_masterView;
    caf::PdmField<QString>                        m_name;
    QIcon                                         m_originalIcon;
};
