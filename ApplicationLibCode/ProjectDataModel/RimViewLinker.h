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

#include "cvfVector3.h"

namespace caf
{
class IconProvider;
}

namespace cvf
{
class BoundingBox;
}

class RimViewController;
class RiuViewer;
class Rim3dView;
class RimCellFilter;
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

    bool isActive() const;

    void       setMasterView( Rim3dView* view );
    Rim3dView* masterView() const;
    void       addDependentView( Rim3dView* view );
    bool       isFirstViewDependentOnSecondView( const Rim3dView* firstView, const Rim3dView* secondView ) const;
    void       updateDependentViews();
    void       removeViewController( RimViewController* viewController );
    Rim3dView* firstControlledView();

    void updateOverrides();
    void updateWindowTitles();
    void updateDuplicatedPropertyFilters();

    void updateCamera( Rim3dView* sourceView );
    void updateTimeStep( Rim3dView* sourceView, int timeStep );
    void updateScaleZ( Rim3dView* sourceView, double scaleZ );

    void updateCellResult();

    void updateCellFilters( const RimCellFilter* changedFilter );
    void applyCellFilterCollectionByUserChoice();

    void updatePropertyFilters( RimPropertyFilter* changedPropertyFilter );

    void scheduleGeometryRegenForDepViews( RivCellSetEnum geometryType );
    void scheduleCreateDisplayModelAndRedrawForDependentViews();

    std::vector<Rim3dView*> allViews() const;

    void updateUiNameAndIcon();

    void addViewControllers( caf::PdmUiTreeOrdering& uiTreeOrdering ) const;

    static void findNameAndIconFromView( QString* name, caf::IconProvider* icon, Rim3dView* view );

    void updateCursorPosition( const Rim3dView* sourceView, const cvf::Vec3d& domainCoord );

protected:
    caf::PdmFieldHandle* userDescriptionField() override { return &m_name; }

    void initAfterRead() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    static QString displayNameForView( Rim3dView* view );

    void allViewsForCameraSync( const Rim3dView* source, std::vector<Rim3dView*>& views ) const;

    void removeOverrides();
    void updateScaleWidgetVisibility();

private:
    caf::PdmChildArrayField<RimViewController*> m_viewControllers;
    caf::PdmPtrField<Rim3dView*>                m_masterView;
    caf::PdmField<QString>                      m_name;
};
