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
#include "cafPdmChildField.h"
#include "cafPdmDocument.h"

#include <vector>

class RigCaseData;
class RigGridManager;
class RigMainGrid;
class RimCase;
class RimCommandObject;
class RimEclipseCase;
class RimIdenticalGridCaseGroup;
class RimViewLinker;
class RimViewLinkerCollection;
class RimMainPlotCollection;
class RimOilField;
class RimScriptCollection;
class RimSummaryCase;
class RimView;
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
    caf::PdmChildArrayField<RimCommandObject*>          commandObjects;
    caf::PdmField<QString>                              treeViewState;
    caf::PdmField<QString>                              currentModelIndexPath;


    void            setScriptDirectories(const QString& scriptDirectories);
    QString         projectFileVersionString() const;
    void            close();

    void            setProjectFileNameAndUpdateDependencies(const QString& fileName);

    void            assignCaseIdToCase(RimCase* reservoirCase);
    void            assignIdToCaseGroup(RimIdenticalGridCaseGroup* caseGroup);

    void            allCases(std::vector<RimCase*>& cases);
    void            allSummaryCases(std::vector<RimSummaryCase*>& sumCases);
    void            allNotLinkedViews(std::vector<RimView*>& views);
    void            allVisibleViews(std::vector<RimView*>& views);

    void            createDisplayModelAndRedrawAllViews(); 

    void            computeUtmAreaOfInterest();

    RimOilField*    activeOilField();

    void            actionsBasedOnSelection(QMenu& contextMenu);

protected:
    // Overridden methods
    void            initScriptDirectories();
    virtual void    initAfterRead();
    virtual void    setupBeforeSave();

    virtual void    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

private:
    void            appendScriptItems(QMenu* menu, RimScriptCollection* scriptCollection);

private:
    caf::PdmField<QString>      m_projectFileVersionString;

    caf::PdmField<int>                                  nextValidCaseId;          // Unique case ID within a project, used to identify a case from Octave scripts
    caf::PdmField<int>                                  nextValidCaseGroupId;     // Unique case group ID within a project, used to identify a case group from Octave scripts

    caf::PdmChildArrayField<RimEclipseCase*>                     casesObsolete; // obsolete
    caf::PdmChildArrayField<RimIdenticalGridCaseGroup*>   caseGroupsObsolete; // obsolete
};
