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

#include "RivCellSetEnum.h"

class RimGridView;
class RimEclipseView;
class RimGeoMechView;
class RimViewLinker;
class RigCaseToCaseCellMapper;
class RimCellRangeFilter;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewController : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewController();
    virtual ~RimViewController();

    bool                                    isActive() const;

    RimGridView*                            managedView() const;
    void                                    setManagedView(RimGridView* view);

    RimGridView*                            masterView() const;
    RimViewLinker*                          ownerViewLinker() const;

    const RigCaseToCaseCellMapper*          cellMapper();
    
    bool                                    isCameraLinked() const;
    bool                                    showCursor() const;
    bool                                    isTimeStepLinked() const;

    bool                                    isResultColorControlled() const;
    bool                                    isLegendDefinitionsControlled() const;
    bool                                    isRangeFiltersControlled() const;
    
    bool                                    isVisibleCellsOveridden() const;
    bool                                    isPropertyFilterOveridden() const;

    void                                    scheduleCreateDisplayModelAndRedrawForDependentView() const;
    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType) const;
    void                                    updateOverrides();
    void                                    updateOptionSensitivity();
    void                                    removeOverrides();
    void                                    updateDisplayNameAndIcon();

    void                                    updateRangeFilterOverrides(RimCellRangeFilter* changedRangeFilter);
    void                                    applyRangeFilterCollectionByUserChoice();


protected:  // Pdm overridden methods
    void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    caf::PdmFieldHandle*                    userDescriptionField() override  { return &m_name; }
    caf::PdmFieldHandle*                    objectToggleField() override     { return &m_isActive; }

private:
    void                                    updateCameraLink();
    void                                    updateTimeStepLink();
    void                                    updateResultColorsControl();
    void                                    updateLegendDefinitions();

    bool                                    isMasterAndDepViewDifferentType() const;
    bool                                    isRangeFilterControlPossible() const;
    bool                                    isPropertyFilterControlPossible() const;
    bool                                    isRangeFilterMappingApliccable() const;

    RimEclipseView*                         managedEclipseView() const;
    RimGeoMechView*                         managedGeoView() const;
    
    static void                             removeOverrides(RimGridView* view);
    static bool                             askUserToRestoreOriginalRangeFilterCollection(const QString& viewName);

private:
    caf::PdmField<QString>                  m_name;
    caf::PdmPtrField<RimGridView*>          m_managedView;

    caf::PdmField<bool>                     m_isActive;
    caf::PdmField<bool>                     m_syncCamera;
    caf::PdmField<bool>                     m_showCursor;
    caf::PdmField<bool>                     m_syncTimeStep;


    // Overridden properties
    caf::PdmField<bool>                     m_syncCellResult;
    caf::PdmField<bool>                     m_syncLegendDefinitions;
    
    caf::PdmField<bool>                     m_syncRangeFilters;
    caf::PdmField<bool>                     m_syncVisibleCells;
    caf::PdmField<bool>                     m_syncPropertyFilters;

    QIcon                                   m_originalIcon;
    cvf::ref<RigCaseToCaseCellMapper>       m_caseToCaseCellMapper;
};
