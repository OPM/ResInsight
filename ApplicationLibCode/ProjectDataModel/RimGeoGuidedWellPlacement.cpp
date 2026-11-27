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

#include "RimGeoGuidedWellPlacement.h"

#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RifCaseRealizationParametersReader.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigGeoGuidedWellPlacementTools.h"
#include "Well/RigWellPath.h"

#include "Cloud/RimSumoDataSource.h"
#include "RimCellFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimModeledWellPath.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"
#include "RimRoffCaseSumo.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafProgressInfo.h"

#include "cvfBoundingBox.h"

#include <QRegularExpression>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimGeoGuidedWellPlacement, "RimGeoGuidedWellPlacement" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoGuidedWellPlacement::RimGeoGuidedWellPlacement()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Geo-Guided Well Placement",
                                                    ":/EditableWell.png",
                                                    "",
                                                    "",
                                                    "GeoGuidedWellPlacement",
                                                    "Creates one modeled well path per realization, where the well targets are moved "
                                                    "vertically into the zone defined by a cell filter. Call compute() after changing "
                                                    "fields." );

    setName( "Geo-Guided Well Placement" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_sourceWellPath, "SourceWellPath", "Source Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_cellFilter, "CellFilter", "Cell Filter", "", "The zone the well targets are moved into", "" );

    CAF_PDM_InitScriptableField( &m_fraction, "Fraction", 0.5, "Fraction", "", "0 is the bottom of the zone, 1 is the top", "" );
    m_fraction.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_fraction.uiCapability()->setAttribute( caf::PdmUiDoubleSliderEditor::Keys::MINIMUM, 0.0 );
    m_fraction.uiCapability()->setAttribute( caf::PdmUiDoubleSliderEditor::Keys::MAXIMUM, 1.0 );

    CAF_PDM_InitScriptableFieldNoDefault( &m_perforationFilter,
                                          "PerforationFilter",
                                          "Perforation Filter",
                                          "",
                                          "Cell filter of the perforation interval covering the generated wells",
                                          "" );

    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "Time step used to evaluate dynamic cell filters", "" );

    CAF_PDM_InitScriptableField( &m_resampleSpacing,
                                 "ResampleSpacing",
                                 50.0,
                                 "Target Spacing [m]",
                                 "",
                                 "MD spacing of the well targets created from a source well that is not a modeled well",
                                 "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_generatedWellPaths, "GeneratedWellPaths", "Generated Well Paths" );
    m_generatedWellPaths.uiCapability()->setUiReadOnly( true );
    m_generatedWellPaths.uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoGuidedWellPlacement::~RimGeoGuidedWellPlacement()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoGuidedWellPlacement::setDefaults()
{
    if ( auto wellPathCollection = RimWellPathCollection::instance() )
    {
        auto wellPaths = wellPathCollection->allWellPaths();
        auto it = std::find_if( wellPaths.begin(), wellPaths.end(), []( RimWellPath* w ) { return dynamic_cast<RimModeledWellPath*>( w ); } );
        if ( it != wellPaths.end() )
            m_sourceWellPath = *it;
        else if ( !wellPaths.empty() )
            m_sourceWellPath = wellPaths.front();
    }

    if ( auto collection = dataFilterCollection(); collection && !collection->filters().empty() )
    {
        m_cellFilter = collection->filters().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimGeoGuidedWellPlacement::generatedWellPaths() const
{
    return m_generatedWellPaths.ptrReferencedObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimGeoGuidedWellPlacement::compute()
{
    if ( auto valid = validateInput(); !valid ) return valid;

    auto gridEnsemble = ensemble();
    gridEnsemble->ensureAllCasesAreOpen();

    deleteGeneratedWellPaths();

    auto wellPathCollection = RimWellPathCollection::instance();

    QStringList  errors;
    const auto   cases       = gridEnsemble->cases();
    const size_t colorOffset = wellPathCollection->modelledWellPathCount();

    caf::ProgressInfo progress( cases.size(), "Geo-guided well placement" );
    for ( size_t i = 0; i < cases.size(); ++i )
    {
        auto eclipseCase = cases[i];
        progress.incrementProgress();
        if ( !eclipseCase ) continue;

        auto wellPath = createWellPathForRealization( eclipseCase );
        if ( !wellPath )
        {
            errors.push_back( QString( "%1: %2" ).arg( eclipseCase->caseUserDescription(), wellPath.error() ) );
            continue;
        }

        int realization = realizationNumber( eclipseCase );
        if ( realization < 0 ) realization = static_cast<int>( i );

        auto newWell = wellPath.value();
        newWell->setName( generatedWellName( m_sourceWellPath()->name(), iterationName( eclipseCase ), realization ) );
        newWell->setWellPathColor( RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( colorOffset + m_generatedWellPaths.size() ) );

        wellPathCollection->addWellPath( newWell );
        newWell->createWellPathGeometry();
        addPerforationInterval( newWell );

        m_generatedWellPaths.push_back( newWell );
    }

    wellPathCollection->uiCapability()->updateConnectedEditors();
    updateConnectedEditors();
    if ( auto project = RimProject::current() ) project->scheduleCreateDisplayModelAndRedrawAllViews();

    if ( !errors.isEmpty() )
    {
        return std::unexpected( QString( "Geo-guided well placement failed for %1 of %2 realizations:\n%3" )
                                    .arg( errors.size() )
                                    .arg( cases.size() )
                                    .arg( errors.join( "\n" ) ) );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoGuidedWellPlacement::generatedWellName( const QString& sourceWellName, const QString& iterationName, int realization )
{
    if ( iterationName.isEmpty() ) return QString( "%1_R%2" ).arg( sourceWellName ).arg( realization );

    return QString( "%1_%2_R%3" ).arg( sourceWellName, iterationName ).arg( realization );
}

//--------------------------------------------------------------------------------------------------
/// Ensemble iteration, e.g. "iter-0" or "pred", from the Sumo ensemble or the file path of the realization
//--------------------------------------------------------------------------------------------------
QString RimGeoGuidedWellPlacement::iterationName( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return {};

    if ( auto sumoCase = dynamic_cast<RimRoffCaseSumo*>( eclipseCase ) )
    {
        return sumoCase->dataSource() ? sumoCase->dataSource()->ensembleName() : QString();
    }

    static const QRegularExpression pattern( "[/\\\\]((?:iter|pred)[^/\\\\]*)[/\\\\]", QRegularExpression::CaseInsensitiveOption );

    auto match = pattern.match( eclipseCase->gridFileName() );
    if ( match.hasMatch() ) return match.captured( 1 );

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Realization number of the case, or -1 if not known
//--------------------------------------------------------------------------------------------------
int RimGeoGuidedWellPlacement::realizationNumber( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return -1;

    if ( auto sumoCase = dynamic_cast<RimRoffCaseSumo*>( eclipseCase ) ) return sumoCase->realization();

    return RifCaseRealizationParametersFileLocator::realizationNumber( eclipseCase->gridFileName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimGeoGuidedWellPlacement::validateInput() const
{
    if ( !ensemble() ) return std::unexpected( "Geo-guided well placement must belong to a reservoir grid ensemble" );
    if ( !m_sourceWellPath() ) return std::unexpected( "No source well path selected" );
    if ( !m_sourceWellPath()->wellPathGeometry() ) return std::unexpected( "The source well path has no geometry" );
    if ( !m_cellFilter() ) return std::unexpected( "No cell filter selected" );
    if ( !isFilterInEnsemble( m_cellFilter() ) )
        return std::unexpected( "The cell filter must be a data filter of the grid ensemble owning the well placement" );
    if ( m_perforationFilter() && !isFilterInEnsemble( m_perforationFilter() ) )
        return std::unexpected( "The perforation filter must be a data filter of the grid ensemble owning the well placement" );
    if ( m_fraction() < 0.0 || m_fraction() > 1.0 ) return std::unexpected( "The fraction must be in the range [0, 1]" );
    if ( m_timeStep() < 0 ) return std::unexpected( "The time step must be zero or positive" );
    if ( !dynamic_cast<RimModeledWellPath*>( m_sourceWellPath() ) && m_resampleSpacing() <= 0.0 )
        return std::unexpected( "The target spacing must be positive" );
    if ( !RimWellPathCollection::instance() ) return std::unexpected( "No well path collection" );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoGuidedWellPlacement::deleteGeneratedWellPaths()
{
    auto wellPathCollection = RimWellPathCollection::instance();
    auto wellPaths          = m_generatedWellPaths.ptrReferencedObjectsByType();
    m_generatedWellPaths.clearWithoutDelete();

    for ( auto wellPath : wellPaths )
    {
        if ( !wellPath || wellPath == m_sourceWellPath() ) continue;

        if ( wellPathCollection ) wellPathCollection->removeWellPath( wellPath );
        delete wellPath;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimModeledWellPath*, QString> RimGeoGuidedWellPlacement::createWellPathForRealization( RimEclipseCase* eclipseCase )
{
    const RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData || !caseData->mainGrid() ) return std::unexpected( "Grid is not loaded" );

    const size_t timeStepCount = static_cast<size_t>( std::max( eclipseCase->timeStepStrings().size(), qsizetype( 1 ) ) );
    if ( static_cast<size_t>( m_timeStep() ) >= timeStepCount )
        return std::unexpected( QString( "Time step %1 is out of range" ).arg( m_timeStep() ) );

    auto visibility = m_cellFilter()->computeReservoirCellVisibility( eclipseCase, m_timeStep() );
    if ( visibility.isNull() ) return std::unexpected( "Could not evaluate the cell filter" );

    std::unique_ptr<RimModeledWellPath> wellPath( createWellPathFromSource() );
    if ( !wellPath ) return std::unexpected( "Could not create well targets from the source well path" );

    auto             geometryDef = wellPath->geometryDefinition();
    const auto       anchor      = geometryDef->anchorPointXyz();
    cvf::BoundingBox gridBox     = caseData->mainGrid()->boundingBox();

    size_t adjustedCount = 0;
    auto   targets       = geometryDef->activeWellTargets( false );
    for ( size_t i = 0; i < targets.size(); ++i )
    {
        cvf::Vec3d point = targets[i]->targetPointXYZ() + anchor;
        if ( !gridBox.contains( point ) ) continue;

        auto z = RigGeoGuidedWellPlacementTools::computeAdjustedZ( caseData, *visibility, point, m_fraction() );
        if ( !z ) return std::unexpected( QString( "Well target %1: %2" ).arg( i + 1 ).arg( z.error() ) );

        point.z() = z.value();
        targets[i]->setPointXYZ( point - anchor );
        adjustedCount++;
    }

    if ( adjustedCount == 0 ) return std::unexpected( "No well targets are located inside the grid" );

    return wellPath.release();
}

//--------------------------------------------------------------------------------------------------
/// Create a modeled well path with the targets of the source well. A modeled source well is copied,
/// while targets are created by resampling the geometry of other wells.
//--------------------------------------------------------------------------------------------------
RimModeledWellPath* RimGeoGuidedWellPlacement::createWellPathFromSource() const
{
    auto newWell = new RimModeledWellPath();
    newWell->setUnitSystem( m_sourceWellPath()->unitSystem() );

    auto geometryDef = newWell->geometryDefinition();

    if ( auto modeledSource = dynamic_cast<RimModeledWellPath*>( m_sourceWellPath() ) )
    {
        auto sourceDef = modeledSource->geometryDefinition();
        geometryDef->readObjectFromXmlString( sourceDef->writeObjectToXmlString(), caf::PdmDefaultObjectFactory::instance() );
        geometryDef->xmlCapability()->initAfterReadRecursively();

        // The generated well is standalone, so keep the absolute location of a lateral
        geometryDef->setIsAttachedToParentWell( false );
        geometryDef->enableReferencePointFromTopLevelWell( false );
        geometryDef->setReferencePointXyz( sourceDef->anchorPointXyz() );
        geometryDef->enableTargetPointPicking( false );

        if ( geometryDef->activeWellTargets( false ).size() < 2 )
        {
            delete newWell;
            return nullptr;
        }
        return newWell;
    }

    const RigWellPath* sourceGeometry = m_sourceWellPath()->wellPathGeometry();
    const auto         mds            = sourceGeometry->uniqueMeasuredDepths();
    if ( mds.size() < 2 )
    {
        delete newWell;
        return nullptr;
    }

    const double startMd = mds.front();
    const double endMd   = mds.back();

    std::vector<cvf::Vec3d> points;
    for ( double md = startMd; md < endMd; md += m_resampleSpacing() )
    {
        points.push_back( sourceGeometry->interpolatedPointAlongWellPath( md ) );
    }
    points.push_back( sourceGeometry->interpolatedPointAlongWellPath( endMd ) );

    if ( points.size() < 2 )
    {
        delete newWell;
        return nullptr;
    }

    geometryDef->setUseAutoGeneratedTargetAtSeaLevel( false );
    geometryDef->setMdAtFirstTarget( startMd );
    geometryDef->createAndInsertTargets( points );

    return newWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoGuidedWellPlacement::addPerforationInterval( RimModeledWellPath* wellPath ) const
{
    if ( !wellPath->wellPathGeometry() || !wellPath->perforationIntervalCollection() ) return;

    auto perforation = new RimPerforationInterval;
    perforation->setStartAndEndMD( wellPath->uniqueStartMD(), wellPath->uniqueEndMD() );
    perforation->setUnitSystemSpecificDefaults();
    perforation->setCellFilter( m_perforationFilter() );

    wellPath->perforationIntervalCollection()->appendPerforation( perforation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsemble* RimGeoGuidedWellPlacement::ensemble() const
{
    return firstAncestorOrThisOfType<RimReservoirGridEnsemble>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterCollection* RimGeoGuidedWellPlacement::dataFilterCollection() const
{
    if ( auto gridEnsemble = ensemble() ) return gridEnsemble->dataFilterCollection();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoGuidedWellPlacement::isFilterInEnsemble( RimCellFilter* filter ) const
{
    auto collection = dataFilterCollection();
    if ( !collection ) return false;

    auto filters = collection->filters();
    return std::find( filters.begin(), filters.end(), filter ) != filters.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoGuidedWellPlacement::onComputeButtonClicked()
{
    if ( auto result = compute(); !result )
    {
        RiaLogging::error( result.error().toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoGuidedWellPlacement::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.addNewButton( "Compute", [this]() { onComputeButtonClicked(); } );

    auto inputGroup = uiOrdering.addNewGroup( "Input" );
    inputGroup->add( &m_sourceWellPath );
    if ( m_sourceWellPath() && !dynamic_cast<RimModeledWellPath*>( m_sourceWellPath() ) )
    {
        inputGroup->add( &m_resampleSpacing );
    }
    inputGroup->add( &m_cellFilter );
    inputGroup->add( &m_fraction );
    inputGroup->add( &m_timeStep );

    auto completionGroup = uiOrdering.addNewGroup( "Completion" );
    completionGroup->add( &m_perforationFilter );

    auto resultGroup = uiOrdering.addNewGroup( "Result" );
    resultGroup->add( &m_generatedWellPaths );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoGuidedWellPlacement::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_sourceWellPath )
    {
        auto generated = generatedWellPaths();
        if ( auto wellPathCollection = RimWellPathCollection::instance() )
        {
            for ( auto wellPath : wellPathCollection->allWellPaths() )
            {
                if ( std::find( generated.begin(), generated.end(), wellPath ) != generated.end() ) continue;
                options.push_back( caf::PdmOptionItemInfo( wellPath->name(), wellPath ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_cellFilter || fieldNeedingOptions == &m_perforationFilter )
    {
        if ( fieldNeedingOptions == &m_perforationFilter )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
        if ( auto collection = dataFilterCollection() )
        {
            for ( auto filter : collection->filters() )
            {
                options.push_back( caf::PdmOptionItemInfo( filter->name(), filter ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( auto gridEnsemble = ensemble(); gridEnsemble && gridEnsemble->mainCase() )
        {
            RimTools::timeStepsForCase( gridEnsemble->mainCase(), &options );
        }
    }

    return options;
}
