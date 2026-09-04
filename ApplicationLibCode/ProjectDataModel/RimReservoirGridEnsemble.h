/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaPorosityModel.h"
#include "RimNamedObject.h"
#include "RimReservoirGridEnsembleBase.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

#include <set>

class RigActiveCellInfo;
class RigMainGrid;
class RimCaseCollection;
class RimEclipseCase;
class RimEclipseStatisticsCase;
class RimEclipseView;
class RimEclipseViewCollection;
class RimEnsembleFileSet;
class RimFormationNames;
class RimStatisticsContourMap;
class RimWellTargetMapping;

//==================================================================================================
///
/// RimReservoirGridEnsemble - Grid ensemble created from an RimEnsembleFileSet
///
/// This class creates and manages grid cases from an ensemble file set pattern. It detects
/// whether all grids have identical geometry and enables shared grid operations (statistics,
/// shared main grid) when they do.
///
//==================================================================================================
class RimReservoirGridEnsemble : public RimNamedObject, public RimReservoirGridEnsembleBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimReservoirGridEnsemble();

    // Ensemble file set connection
    void                setEnsembleFileSet( RimEnsembleFileSet* ensembleFileSet );
    RimEnsembleFileSet* ensembleFileSet() const;

    // Group ID
    int  groupId() const;
    void setGroupId( int id );

    // Case management
    void                         addCase( RimEclipseCase* reservoir ) override;
    void                         removeCase( RimEclipseCase* reservoir ) override;
    bool                         contains( RimEclipseCase* reservoir ) const;
    std::vector<RimEclipseCase*> cases() const;
    RimEclipseCase*              mainCase() override;
    std::vector<RimEclipseCase*> sourceCases() const override;
    RimEclipseCase*              findByFileName( const QString& gridFileName ) const;

    // Grid detection and shared grid
    RigMainGrid* mainGrid() override;
    RigMainGrid* shareOrAdoptMainGrid( RigMainGrid* candidate ) override;
    void         setupSharedGrid();

    // Open every realization. Needed by the operations that read them all, as realizations can otherwise
    // be opened lazily, see RimReservoirGridEnsembleSumo.
    void ensureAllCasesAreOpen();

    // Deferred loading control
    void loadGridDataFromFiles();
    bool isGridDataLoaded() const;
    void reloadMetaDataIfNeeded();

    // Helper methods
    bool         hasSharedGrid() const;
    GridModeType gridMode() const override;
    QString      ensembleName() const override;

    // Active cells
    RigActiveCellInfo* unionOfActiveCells( RiaDefines::PorosityModelType porosityType ) override;
    void               computeUnionOfActiveCells() override;

    // Formation names
    RimFormationNames* activeFormationNames() const override;

    // Statistics
    RimCaseCollection*        statisticsCaseCollection() const override;
    RimEclipseStatisticsCase* createAndAppendStatisticsCase() override;

    // Views
    void                         addView( RimEclipseView* view );
    RimEclipseView*              addViewForCase( RimEclipseCase* eclipseCase );
    std::vector<RimEclipseView*> allViews() const;
    std::set<RimEclipseCase*>    casesInViews() const override;
    RimEclipseViewCollection*    viewCollection() const override;

    // Well target mapping
    void                               addWellTargetMapping( RimWellTargetMapping* wellTargetMapping );
    std::vector<RimWellTargetMapping*> wellTargetMappings() const;

    // Statistics contour maps
    void                                  addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap ) override;
    std::vector<RimStatisticsContourMap*> statisticsContourMaps() const override;

    // Load and initialization
    void loadDataAndUpdate();
    void createGridCasesFromEnsembleFileSet();

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void createDerivedObjects() override;
    void initAfterRead() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // Drop the existing realization cases and rebuild them from the current source.
    void recreateCaseObjects();

    // Build the realization case objects, without loading the grids. The base implementation derives them
    // from the ensemble file set; a subclass with another source of realizations overrides this.
    virtual void createCaseObjects();

    // Decide whether all realizations have the same grid dimensions. Overridden by a subclass that can
    // answer this without reading the grids.
    virtual bool detectGridDimensionEquality();

    // Establish the grid shared by the ensemble. The base implementation opens every realization eagerly;
    // a subclass where opening a realization is expensive can open only the first and leave the rest to be
    // opened on demand.
    virtual void loadGridsInSharedMode();

    RimCaseCollection* caseCollection() const;

private:
    void onFileSetChanged( const caf::SignalEmitter* emitter );
    void clearActiveCellUnions();
    void clearStatisticsResults();
    void updateMainGridAndActiveCellsForStatisticsCases();
    void updateStatisticsVisibility();

    void loadGridsInIndividualMode();
    void updateGridModeToolTip();

    static bool hasMatchingDimensions( const RigMainGrid* lhs, const RigMainGrid* rhs );

private:
    // File set reference
    caf::PdmPtrField<RimEnsembleFileSet*> m_ensembleFileSet;

    // Formation names (shared across all realizations, only valid in shared grid mode)
    caf::PdmPtrField<RimFormationNames*> m_activeFormationNames;

    // Ensemble id
    caf::PdmField<int> m_groupId;

    // Cases
    caf::PdmChildField<RimCaseCollection*> m_caseCollection;
    caf::PdmChildField<RimCaseCollection*> m_statisticsCaseCollection;
    caf::PdmChildField<RimEclipseCase*>    m_ensembleCase;

    // Grid mode
    caf::PdmField<bool>                       m_autoDetectGridType;
    caf::PdmField<caf::AppEnum<GridModeType>> m_gridMode;
    caf::PdmField<caf::AppEnum<GridModeType>> m_detectedGridMode;

    // Views and mappings
    caf::PdmChildField<RimEclipseViewCollection*>     m_viewCollection;
    caf::PdmChildArrayField<RimWellTargetMapping*>    m_wellTargetMappings;
    caf::PdmChildArrayField<RimStatisticsContourMap*> m_statisticsContourMaps;

    // Shared grid data (for identical grids). Reference counted: with realizations opened lazily this can
    // be the only owner of the grid, e.g. after the realization that loaded it has been deselected.
    cvf::ref<RigMainGrid>       m_mainGrid;
    cvf::ref<RigActiveCellInfo> m_unionOfMatrixActiveCells;
    cvf::ref<RigActiveCellInfo> m_unionOfFractureActiveCells;
};
