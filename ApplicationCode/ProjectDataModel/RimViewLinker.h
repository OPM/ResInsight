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
class RimGridView;
class RimCellRangeFilter;
class RimPropertyFilter;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewLinker : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewLinker();
    ~RimViewLinker() override;
    
    bool                                    isActive() const;

    void                                    setMasterView(RimGridView* view);
    RimGridView*                            masterView() const;

    void                                    addDependentView(RimGridView* view);
    void                                    updateDependentViews();
    void                                    removeViewController(RimViewController* viewController);

    void                                    updateOverrides();

    void                                    updateCamera(RimGridView* sourceView);
    void                                    updateTimeStep(RimGridView* sourceView, int timeStep);
    void                                    updateScaleZ(RimGridView* sourceView, double scaleZ);

    void                                    updateCellResult();

    void                                    updateRangeFilters(RimCellRangeFilter* changedRangeFilter);
    void                                    applyRangeFilterCollectionByUserChoice();

    void                                    updatePropertyFilters(RimPropertyFilter* changedPropertyFilter);

    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType);
    void                                    scheduleCreateDisplayModelAndRedrawForDependentViews();

    void                                    allViews(std::vector<RimGridView*>& views) const;

    void                                    updateUiNameAndIcon();

    void                                    addViewControllers(caf::PdmUiTreeOrdering& uiTreeOrdering) const;

    static void                             applyIconEnabledState(caf::PdmObject* obj, const QIcon& icon, bool disable);
    static void                             findNameAndIconFromView(QString* name, QIcon* icon, RimGridView* view);

    void                                    updateCursorPosition(const RimGridView* sourceView, const cvf::Vec3d& domainCoord);

protected:
    caf::PdmFieldHandle*                    userDescriptionField() override  { return &m_name; }
    void                                    initAfterRead() override;

private:
    static QString                          displayNameForView(RimGridView* view);

    void                                    allViewsForCameraSync(const RimGridView* source, std::vector<RimGridView*>& views) const;
    
    void                                    removeOverrides();

private:
    caf::PdmChildArrayField<RimViewController*>   m_viewControllers;
    caf::PdmPtrField<RimGridView*>                m_masterView;
    caf::PdmField<QString>                        m_name;
    QIcon                                         m_originalIcon;
};
