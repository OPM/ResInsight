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

#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDocument.h"
#include "cvfCollection.h"

#include <vector>

class RigEclipseCaseData;
class RigGridManager;
class RigMainGrid;
class RigWellPath;

class RimSummaryCalculationCollection;
class RimCase;
class RimCommandObject;
class RimDialogData;
class RimEclipseCase;
class RimGeoMechCase;
class RimIdenticalGridCaseGroup;
class RimMainPlotCollection;
class RimMultiSnapshotDefinition; 
class RimObservedData;
class RimOilField;
class RimScriptCollection;
class RimSummaryCase;
class RimView;
class RimViewLinker;
class RimViewLinkerCollection;
class RimWellPath;
class RimWellPathImport;

namespace caf
{
    class PdmUiTreeOrdering;
}


class QAction;
class QMenu;

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

    caf::PdmChildArrayField<RimOilField*>               oilFields;
    caf::PdmChildField<RimScriptCollection*>            scriptCollection;
    caf::PdmChildField<RimWellPathImport*>              wellPathImport;
    caf::PdmChildField<RimMainPlotCollection*>          mainPlotCollection;
    caf::PdmChildField<RimViewLinkerCollection*>        viewLinkerCollection;
    caf::PdmChildField<RimSummaryCalculationCollection*>       calculationCollection;
    caf::PdmChildArrayField<RimCommandObject*>          commandObjects;
    
    caf::PdmChildArrayField<RimMultiSnapshotDefinition*> multiSnapshotDefinitions;

    caf::PdmField<QString>                              mainWindowTreeViewState;
    caf::PdmField<QString>                              mainWindowCurrentModelIndexPath;

    caf::PdmField<QString>                              plotWindowTreeViewState;
    caf::PdmField<QString>                              plotWindowCurrentModelIndexPath;

    void            setScriptDirectories(const QString& scriptDirectories);
    QString         projectFileVersionString() const;
    void            close();

    void            setProjectFileNameAndUpdateDependencies(const QString& fileName);

    void            assignCaseIdToCase(RimCase* reservoirCase);
    void            assignIdToCaseGroup(RimIdenticalGridCaseGroup* caseGroup);

    void            allCases(std::vector<RimCase*>& cases);

    std::vector<RimSummaryCase*>    allSummaryCases() const;
    
    void            allNotLinkedViews(std::vector<RimView*>& views);
    void            allVisibleViews(std::vector<RimView*>& views);

    void            createDisplayModelAndRedrawAllViews(); 

    void            computeUtmAreaOfInterest();

    void                allOilFields(std::vector<RimOilField*>& oilFields);
    RimOilField*        activeOilField();
    const RimOilField*  activeOilField() const;

    void            actionsBasedOnSelection(QMenu& contextMenu);

    bool            show3DWindow() const;
    bool            showPlotWindow() const;

    void            reloadCompletionTypeResultsInAllViews();
    void            reloadCompletionTypeResultsForEclipseCase(RimEclipseCase* eclipseCase);

    RimDialogData*              dialogData() const;

    std::vector<RimEclipseCase*>    eclipseCases() const;
    std::vector<QString>            simulationWellNames() const;

    RimWellPath*                    wellPathFromSimWellName(const QString& simWellName, int branchIndex = -1);
    RimWellPath*                    wellPathByName(const QString& wellPathName) const;
    std::vector<RimWellPath*>       allWellPaths() const;

    std::vector<RimGeoMechCase*>    geoMechCases() const;

protected:
    // Overridden methods
    void            initScriptDirectories();
    virtual void    initAfterRead();
    virtual void    setupBeforeSave();

    virtual void    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

private:
    caf::PdmField<QString>  m_projectFileVersionString;

    caf::PdmChildField<RimDialogData*>  m_dialogData;


    caf::PdmField<bool>     m_show3DWindow;
    caf::PdmField<bool>     m_showPlotWindow;

    caf::PdmField<int>                                  nextValidCaseId;          // Unique case ID within a project, used to identify a case from Octave scripts
    caf::PdmField<int>                                  nextValidCaseGroupId;     // Unique case group ID within a project, used to identify a case group from Octave scripts

    caf::PdmChildArrayField<RimEclipseCase*>            casesObsolete; // obsolete
    caf::PdmChildArrayField<RimIdenticalGridCaseGroup*> caseGroupsObsolete; // obsolete
};
