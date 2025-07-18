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
#include "RiaPlotDefines.h"

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
class RimSummaryCalculation;
class RimCase;
class RimDialogData;
class RimEclipseCase;
class RimGeoMechCase;
class RimIdenticalGridCaseGroup;
class RimMainPlotCollection;
class RimMeasurement;
class RimAdvancedSnapshotExportDefinition;
class RimObservedSummaryData;
class RimOilField;
class RimColorLegendCollection;
class RimScriptCollection;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryCaseMainCollection;
class Rim3dView;
class RimGridView;
class RimPlotWindow;
class RimViewLinker;
class RimViewLinkerCollection;
class RimViewWindow;
class RimWellPath;
class RimFractureTemplateCollection;
class RimFractureTemplate;
class RimValveTemplateCollection;
class RimValveTemplate;
class RimCompletionTemplateCollection;
class RimPlotTemplateFolderItem;
class RimGridCalculationCollection;
class RimQuickAccessCollection;
class RimAutomationSettings;
class RimJobCollection;
class RimEnsembleFileSetCollection;

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
    RimProject();
    ~RimProject() override;
    static RimProject* current();

    caf::PdmChildArrayField<RimOilField*>                oilFields;
    caf::PdmChildField<RimColorLegendCollection*>        colorLegendCollection;
    caf::PdmChildField<RimScriptCollection*>             scriptCollection;
    caf::PdmChildField<RimViewLinkerCollection*>         viewLinkerCollection;
    caf::PdmChildField<RimSummaryCalculationCollection*> calculationCollection;
    caf::PdmChildField<RimGridCalculationCollection*>    gridCalculationCollection;

    RimMainPlotCollection* mainPlotCollection() const;

    caf::PdmChildArrayField<RimAdvancedSnapshotExportDefinition*> multiSnapshotDefinitions;

    caf::PdmField<QString> mainWindowTreeViewStates;
    caf::PdmField<QString> mainWindowCurrentModelIndexPaths;

    caf::PdmField<QString> plotWindowTreeViewStates;
    caf::PdmField<QString> plotWindowCurrentModelIndexPaths;

    bool writeProjectFile();

    void setScriptDirectories( const QString& scriptDirectories, int maxFolderDepth );
    void setPlotTemplateFolders( const QStringList& plotTemplateFolders );

    QString projectFileVersionString() const;
    bool    isProjectFileVersionEqualOrOlderThan( const QString& otherProjectFileVersion ) const;
    void    close();

    void setProjectFileNameAndUpdateDependencies( const QString& projectFileName );

    void assignCaseIdToCase( RimCase* reservoirCase );
    void assignIdToCaseGroup( RimIdenticalGridCaseGroup* caseGroup );
    void assignViewIdToView( Rim3dView* view );
    void assignPlotIdToPlotWindow( RimPlotWindow* plotWindow );
    void assignCaseIdToSummaryCase( RimSummaryCase* summaryCase );
    void assignIdToEnsemble( RimSummaryEnsemble* summaryCaseCollection );

    [[nodiscard]] std::vector<RimCase*> allGridCases() const;

    std::vector<RimSummaryCase*>     allSummaryCases() const;
    std::vector<RimSummaryEnsemble*> summaryEnsembles() const;
    RimSummaryCaseMainCollection*    firstSummaryCaseMainCollection() const;

    [[nodiscard]] std::vector<Rim3dView*>   allViews() const;
    [[nodiscard]] std::vector<Rim3dView*>   allVisibleViews() const;
    [[nodiscard]] std::vector<RimGridView*> allVisibleGridViews() const;
    [[nodiscard]] std::vector<Rim3dView*>   allNotLinkedViews() const;

    void scheduleCreateDisplayModelAndRedrawAllViews();

    [[nodiscard]] std::vector<RimOilField*> allOilFields() const;
    RimOilField*                            activeOilField();
    const RimOilField*                      activeOilField() const;

    void actionsBasedOnSelection( QMenu& contextMenu );

    bool show3DWindow() const;
    bool showPlotWindow() const;
    bool showPlotWindowOnTop() const;

    RiaDefines::WindowTileMode subWindowsTileMode3DWindow() const;
    RiaDefines::WindowTileMode subWindowsTileModePlotWindow() const;
    void                       setSubWindowsTileMode3DWindow( RiaDefines::WindowTileMode tileMode );
    void                       setSubWindowsTileModePlotWindow( RiaDefines::WindowTileMode tileMode );

    void reloadCompletionTypeResultsInAllViews();
    void reloadCompletionTypeResultsForEclipseCase( RimEclipseCase* eclipseCase );

    RimDialogData* dialogData() const;

    std::vector<RimEclipseCase*> eclipseCases() const;
    RimEclipseCase*              eclipseCaseFromGridFileName( const QString& gridFileName ) const;
    RimEclipseCase*              eclipseCaseFromCaseId( const int caseId ) const;

    std::vector<QString> simulationWellNames() const;

    RimWellPath*                           wellPathFromSimWellName( const QString& simWellName, int branchIndex = -1 );
    RimWellPath*                           wellPathByName( const QString& wellPathName ) const;
    std::vector<RimWellPath*>              allWellPaths() const;
    std::vector<RimTextAnnotation*>        textAnnotations() const;
    std::vector<RimReachCircleAnnotation*> reachCircleAnnotations() const;

    std::vector<RimGeoMechCase*> geoMechCases() const;

    std::vector<RimFractureTemplateCollection*> allFractureTemplateCollections() const;
    std::vector<RimFractureTemplate*>           allFractureTemplates() const;

    std::vector<RimValveTemplateCollection*> allValveTemplateCollections() const;
    std::vector<RimValveTemplate*>           allValveTemplates() const;

    caf::AppEnum<RiaDefines::EclipseUnitSystem> commonUnitSystemForAllCases() const;
    RimMeasurement*                             measurement() const;

    RimPlotTemplateFolderItem* rootPlotTemplateItem() const;

    std::vector<caf::FilePath*> allFilePaths() const;

    void updatesAfterProjectFileIsRead();

    RimQuickAccessCollection*     pinnedFieldCollection() const;
    RimAutomationSettings*        automationSettings() const;
    RimEnsembleFileSetCollection* ensembleFileSetCollection() const;
    RimJobCollection*             jobCollection() const;

protected:
    void                              initAfterRead() override;
    void                              setupBeforeSave() override;
    std::vector<caf::PdmFieldHandle*> fieldsForExport() const override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmChildField<RimMainPlotCollection*>        m_mainPlotCollection;
    caf::PdmChildField<RimQuickAccessCollection*>     m_pinnedFieldCollection;
    caf::PdmChildField<RimAutomationSettings*>        m_automationSettings;
    caf::PdmChildField<RimEnsembleFileSetCollection*> m_ensembleFileSetCollection;
    caf::PdmChildField<RimJobCollection*>             m_jobCollection;

    caf::PdmField<QString> m_globalPathList;
    caf::PdmField<QString> m_projectFileVersionString;

    caf::PdmChildField<RimDialogData*>             m_dialogData;
    caf::PdmChildField<RimPlotTemplateFolderItem*> m_plotTemplateTopFolder;

    caf::PdmField<bool> m_show3DWindow;
    caf::PdmField<bool> m_showPlotWindow;
    caf::PdmField<bool> m_showPlotWindowOnTopOf3DWindow;

    caf::PdmField<bool> m_subWindowsTiled3DWindow_OBSOLETE;
    caf::PdmField<bool> m_subWindowsTiledPlotWindow_OBSOLETE;

    caf::PdmField<caf::AppEnum<RiaDefines::WindowTileMode>> m_subWindowsTileMode3DWindow;
    caf::PdmField<caf::AppEnum<RiaDefines::WindowTileMode>> m_subWindowsTileModePlotWindow;

    int m_nextValidCaseId;
    int m_nextValidCaseGroupId;
    int m_nextValidViewId;
    int m_nextValidPlotId;
    int m_nextValidSummaryCaseId;
    int m_nextValidEnsembleId;

    caf::PdmChildArrayField<RimEclipseCase*>            casesObsolete; // obsolete
    caf::PdmChildArrayField<RimIdenticalGridCaseGroup*> caseGroupsObsolete; // obsolete
};
