/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmDocument.h"
//#include "RimScriptCollection.h"
//#include "RimIdenticalGridCaseGroup.h"

class RimCase;
class RigGridManager;
class RimScriptCollection;
class RimIdenticalGridCaseGroup;
class RigMainGrid;
class RigCaseData;

//==================================================================================================
///  
///  
//==================================================================================================
class RimProject : public caf::PdmDocument
{
     CAF_PDM_HEADER_INIT;

public:
    RimProject(void);
    virtual ~RimProject(void);

    caf::PdmPointersField<RimCase*>                     reservoirs;
    caf::PdmPointersField<RimIdenticalGridCaseGroup*>   caseGroups;
    caf::PdmField<RimScriptCollection*>                 scriptCollection;
    caf::PdmField<QString>                              treeViewState;
    caf::PdmField<QString>                              currentModelIndexPath;
    caf::PdmField<int>                                  nextValidCaseId;          // Unique case ID within a project, used to identify a case from Octave scripts

    void            setScriptDirectories(const QString& scriptDirectories);
    QString         projectFileVersionString() const;
    void            close();

    RimIdenticalGridCaseGroup* 
                    createIdenticalCaseGroupFromMainCase(RimCase* mainCase);
    void            insertCaseInCaseGroup(RimIdenticalGridCaseGroup* caseGroup, RimCase* rimReservoir);
    void            moveEclipseCaseIntoCaseGroup(RimCase* rimReservoir);
    void            removeCaseFromAllGroups(RimCase* rimReservoir);

    void            setProjectFileNameAndUpdateDependencies(const QString& fileName);

    void            assignCaseIdToCase(RimCase* reservoirCase);
    
private:
    RigMainGrid*    registerCaseInGridCollection(RigCaseData* rigEclipseCase);
    void            allCases(std::vector<RimCase*>& cases);

protected:
    // Overridden methods
    virtual void    initAfterRead();
    virtual void    setupBeforeSave();

private:
    caf::PdmField<QString>      m_projectFileVersionString;
    cvf::ref<RigGridManager>    m_gridCollection;
};
