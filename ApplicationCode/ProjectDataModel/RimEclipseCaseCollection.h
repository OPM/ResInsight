/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

class RigEclipseCaseData;
class RigGridManager;
class RigMainGrid;
class RimEclipseCase;
class RimIdenticalGridCaseGroup;
class RimWellPathCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseCaseCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimEclipseCaseCollection(void);
    virtual ~RimEclipseCaseCollection(void);

    caf::PdmChildArrayField<RimEclipseCase*>                     cases;
    caf::PdmChildArrayField<RimIdenticalGridCaseGroup*>   caseGroups;

    void                                                close();

    RimIdenticalGridCaseGroup*                          createIdenticalCaseGroupFromMainCase(RimEclipseCase* mainCase);
    void                                                insertCaseInCaseGroup(RimIdenticalGridCaseGroup* caseGroup, RimEclipseCase* rimReservoir);
    void                                                removeCaseFromAllGroups(RimEclipseCase* rimReservoir);

    void                                                recomputeStatisticsForAllCaseGroups();

private:
    RigMainGrid*                                        registerCaseInGridCollection(RigEclipseCaseData* rigEclipseCase);
    cvf::ref<RigGridManager>                            m_gridCollection;
};
