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

#include "RivCellSetEnum.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfVector3.h"

namespace cvf
{
    class BoundingBox;
}

class RimViewController;
class RiuViewer;
class Rim3dView;
class RimCellRangeFilter;

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
    
    bool                                    isActive() const;

    void                                    setMasterView(Rim3dView* view);
    Rim3dView*                                masterView() const;

    void                                    addDependentView(Rim3dView* view);
    void                                    updateDependentViews();
    void                                    removeViewController(RimViewController* viewController);

    void                                    updateOverrides();

    void                                    updateCamera(Rim3dView* sourceView);
    void                                    updateTimeStep(Rim3dView* sourceView, int timeStep);
    void                                    updateScaleZ(Rim3dView* sourceView, double scaleZ);

    void                                    updateCellResult();

    void                                    updateRangeFilters(RimCellRangeFilter* changedRangeFilter);
    void                                    applyRangeFilterCollectionByUserChoice();

    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType);
    void                                    scheduleCreateDisplayModelAndRedrawForDependentViews();

    void                                    allViews(std::vector<Rim3dView*>& views) const;

    void                                    updateUiNameAndIcon();

    void                                    addViewControllers(caf::PdmUiTreeOrdering& uiTreeOrdering) const;

    static void                             applyIconEnabledState(caf::PdmObject* obj, const QIcon& icon, bool disable);
    static void                             findNameAndIconFromView(QString* name, QIcon* icon, Rim3dView* view);

    void                                    updateCursorPosition(const Rim3dView* sourceView, const cvf::Vec3d& domainCoord);

public:
    static QString                          displayNameForView(Rim3dView* view);

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual void                            initAfterRead();

private:
    void                                    allViewsForCameraSync(const Rim3dView* source, std::vector<Rim3dView*>& views) const;
    
    void                                    removeOverrides();

private:
    caf::PdmChildArrayField<RimViewController*>   m_viewControllers;
    caf::PdmPtrField<Rim3dView*>                    m_masterView;
    caf::PdmField<QString>                        m_name;
    QIcon                                         m_originalIcon;
};
