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
#include "RiaEclipseUnitTools.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDocument.h"
#include "cvfCollection.h"

#include <vector>

class RigEclipseCaseData;
class RigGridManager;
class RigMainGrid;
class RigWellPath;

class RimTextAnnotation;
class RimReachCircleAnnotation;
class RimPolylinesAnnotation;
class RimSummaryCalculationCollection;
class RimCase;
class RimCommandObject;
class RimCommandObject;
class RimDialogData;
class RimEclipseCase;
class RimGeoMechCase;
class RimIdenticalGridCaseGroup;
class RimMainPlotCollection;
class RimMeasurement;
class RimAdvancedSnapshotExportDefinition; 
class RimObservedData;
class RimOilField;
class RimScriptCollection;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryCaseMainCollection;
class Rim3dView;
class RimGridView;
class RimViewLinker;
class RimViewLinkerCollection;
class RimWellPath;
class RimWellPathImport;
class RimFractureTemplateCollection;
class RimFractureTemplate;
class RimValveTemplateCollection;
class RimValveTemplate;
class RimCompletionTemplateCollection;

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
    ~RimProject(void) override;

    caf::PdmChildArrayField<RimOilField*>               oilFields;
    caf::PdmChildField<RimScriptCollection*>            scriptCollection;
    caf::PdmChildField<RimWellPathImport*>              wellPathImport;
    caf::PdmChildField<RimMainPlotCollection*>          mainPlotCollection;
    caf::PdmChildField<RimViewLinkerCollection*>        viewLinkerCollection;
    caf::PdmChildField<RimSummaryCalculationCollection*>       calculationCollection;
    caf::PdmChildArrayField<RimCommandObject*>          commandObjects;
    
    caf::PdmChildArrayField<RimAdvancedSnapshotExportDefinition*> multiSnapshotDefinitions;

    caf::PdmField<QString>                              mainWindowTreeViewState;
    caf::PdmField<QString>                              mainWindowCurrentModelIndexPath;

    caf::PdmField<QString>                              plotWindowTreeViewState;
    caf::PdmField<QString>                              plotWindowCurrentModelIndexPath;

    void            setScriptDirectories(const QString& scriptDirectories);
    void            initScriptDirectories(const QString& scriptDirectories);

    QString         projectFileVersionString() const;
    bool            isProjectFileVersionEqualOrOlderThan(const QString& otherProjectFileVersion) const;
    void            close();

    void setProjectFileNameAndUpdateDependencies(const QString& projectFileName);

    void            assignCaseIdToCase(RimCase* reservoirCase);
    void            assignIdToCaseGroup(RimIdenticalGridCaseGroup* caseGroup);

    void            allCases(std::vector<RimCase*>& cases) const;

    std::vector<RimSummaryCase*>    allSummaryCases() const;
    std::vector<RimSummaryCaseCollection*> summaryGroups() const;
    RimSummaryCaseMainCollection*   firstSummaryCaseMainCollection() const;

    void            allVisibleViews(std::vector<Rim3dView*>& views) const;
    void            allVisibleGridViews(std::vector<RimGridView*>& views) const;
    void            allNotLinkedViews(std::vector<RimGridView*>& views);

    void            scheduleCreateDisplayModelAndRedrawAllViews();

    void            computeUtmAreaOfInterest();

    void            allOilFields(std::vector<RimOilField*>& allOilFields) const;
    RimOilField*        activeOilField();
    const RimOilField*  activeOilField() const;

    void            actionsBasedOnSelection(QMenu& contextMenu);

    bool            show3DWindow() const;
    bool            showPlotWindow() const;

    bool            subWindowsTiled3DWindow() const;
    bool            subWindowsTiledPlotWindow() const;

    void            setSubWindowsTiledIn3DWindow(bool tiled);
    void            setSubWindowsTiledInPlotWindow(bool tiled);

    void            reloadCompletionTypeResultsInAllViews();
    void            reloadCompletionTypeResultsForEclipseCase(RimEclipseCase* eclipseCase);

    RimDialogData*              dialogData() const;

    std::vector<RimEclipseCase*>    eclipseCases() const;
    RimEclipseCase*                 eclipseCaseFromGridFileName(const QString& gridFileName) const;

    std::vector<QString>            simulationWellNames() const;

    RimWellPath*                    wellPathFromSimWellName(const QString& simWellName, int branchIndex = -1);
    RimWellPath*                    wellPathByName(const QString& wellPathName) const;
    std::vector<RimWellPath*>       allWellPaths() const;
    std::vector<RimTextAnnotation*>         textAnnotations() const;
    std::vector<RimReachCircleAnnotation*>  reachCircleAnnotations() const;
    std::vector<RimPolylinesAnnotation*>     polylineAnnotations() const;

    std::vector<RimGeoMechCase*>    geoMechCases() const;

    std::vector<RimFractureTemplateCollection*> allFractureTemplateCollections() const;
    std::vector<RimFractureTemplate*>           allFractureTemplates() const;

    std::vector<RimValveTemplateCollection*> allValveTemplateCollections() const;
    std::vector<RimValveTemplate*>           allValveTemplates() const;

    RiaEclipseUnitTools::UnitSystemType commonUnitSystemForAllCases() const;
    RimMeasurement*                     measurement() const;

protected:
    // Overridden methods
    void    initAfterRead() override;
    void    setupBeforeSave() override;

    void    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    template <typename T>
    void fieldContentsByType(caf::PdmObjectHandle* object, std::vector<T*>& typedFields);

private:
    caf::PdmField<QString>  m_projectFileVersionString;

    caf::PdmChildField<RimDialogData*>  m_dialogData;


    caf::PdmField<bool>     m_show3DWindow;
    caf::PdmField<bool>     m_showPlotWindow;

    caf::PdmField<bool>     m_subWindowsTiled3DWindow;
    caf::PdmField<bool>     m_subWindowsTiledPlotWindow;

    caf::PdmField<int>                                  nextValidCaseId;          // Unique case ID within a project, used to identify a case from Octave scripts
    caf::PdmField<int>                                  nextValidCaseGroupId;     // Unique case group ID within a project, used to identify a case group from Octave scripts

    caf::PdmChildArrayField<RimEclipseCase*>            casesObsolete; // obsolete
    caf::PdmChildArrayField<RimIdenticalGridCaseGroup*> caseGroupsObsolete; // obsolete
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimProject::fieldContentsByType(caf::PdmObjectHandle* object, std::vector<T*>& typedFields)
{
    if (!object) return;

    std::vector<caf::PdmFieldHandle*> allFieldsInObject;
    object->fields(allFieldsInObject);

    std::vector<caf::PdmObjectHandle*> children;

    for (const auto& field : allFieldsInObject)
    {
        caf::PdmField<T>* typedField = dynamic_cast<caf::PdmField<T>*>(field);
        if (typedField) typedFields.push_back(&typedField->v());

        caf::PdmField< std::vector<T> >* typedFieldInVector = dynamic_cast<caf::PdmField< std::vector<T> >*>(field);
        if (typedFieldInVector)
        {
            for (T& typedFieldFromVector : typedFieldInVector->v())
            {
                typedFields.push_back(&typedFieldFromVector);
            }
        }

        field->childObjects(&children);
    }

    for (const auto& child : children)
    {
        fieldContentsByType(child, typedFields);
    }
}
