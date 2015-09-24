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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfObject.h"

class RimView;
class RimEclipseView;
class RimGeoMechView;
class RimViewLinker;
class RigCaseToCaseCellMapper;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewController : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewController(void);
    virtual ~RimViewController(void);

    caf::PdmField<bool>         isActive;
    caf::PdmField<QString>      name;

    RimView*                    managedView();
    void                        setManagedView(RimView* view);
    RimView*                    masterView();
    RimViewLinker*              ownerViewLinker();

    const RigCaseToCaseCellMapper* cellMapper();

    // Linked (both ways) properties
    caf::PdmField<bool>         syncCamera;
    caf::PdmField<bool>         syncTimeStep;

    // Overridden properties
    caf::PdmField<bool>         syncCellResult;
    bool                        syncVisibleCells();
    caf::PdmField<bool>         syncRangeFilters;
    caf::PdmField<bool>         syncPropertyFilters;

    void                        configureOverrides();
    void                        updateOptionSensitivity();
    void                        removeOverrides();

    void                        updateUiIconFromActiveState();
    void                        updateDisplayNameAndIcon();

protected:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                            initAfterRead();
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &name; }
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual caf::PdmFieldHandle*            objectToggleField()     { return &isActive; }

private:
    void            doSyncCamera();
    void            doSyncTimeStep();
    void            doSyncCellResult();

    bool            isVisibleCellsSyncPossible();

    RimEclipseView* managedEclipseView();
    RimGeoMechView* managedGeoView();

    caf::PdmPtrField<RimView*>  m_managedView;
    QIcon                       m_originalIcon;
    cvf::ref<RigCaseToCaseCellMapper> m_caseToCaseCellMapper;
    caf::PdmField<bool>         m_syncVisibleCells;

};
