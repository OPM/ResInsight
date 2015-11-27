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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

class RimEclipseWell;
class RimEclipseWellCollection;
class RimWellPath;
class RivCrossSectionPartMgr;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimCrossSection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum CrossSectionEnum
    {
        CS_WELL_PATH,
        CS_SIMULATION_WELL,
        CS_USER_DEFINED
    };

    enum CrossSectionDirEnum
    {
        CS_VERTICAL,
        CS_HORIZONTAL
    };

public:
    RimCrossSection();

    caf::PdmField<QString>                               name;
    caf::PdmField<bool>                                  isActive;

    caf::PdmField< caf::AppEnum< CrossSectionEnum > >    type;
    caf::PdmField< caf::AppEnum< CrossSectionDirEnum > > direction;

    caf::PdmPtrField<RimWellPath*>                       wellPath;
    caf::PdmPtrField<RimEclipseWell*>                    simulationWell;

    std::vector< std::vector <cvf::Vec3d> >              polyLines() const;
    RivCrossSectionPartMgr*                              crossSectionPartMgr();

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField();
    virtual caf::PdmFieldHandle*            objectToggleField();
                                            
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
                                            
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
                                            
private:                                    
    caf::PdmField<int>                      m_branchIndex;
    caf::PdmField<double>                   m_extentLength;
                                            
    RimEclipseWellCollection*               simulationWellCollection();
    void                                    updateWellCenterline() const;
    void                                    updateWellExtentDefaultValue();
    void                                    addExtents(std::vector<cvf::Vec3d> &polyLine) const;
    void                                    clipToReservoir(std::vector<cvf::Vec3d> &polyLine) const;
    void                                    updateName();
private:                                    
    cvf::ref<RivCrossSectionPartMgr>        m_crossSectionPartMgr;
    
    mutable 
    std::vector< std::vector <cvf::Vec3d> > m_wellBranchCenterlines;
};
