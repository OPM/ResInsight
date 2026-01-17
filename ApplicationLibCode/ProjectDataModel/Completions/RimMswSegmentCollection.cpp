/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimMswSegmentCollection.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaOpmParserTools.h"

#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimMswSegment.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathCompletions.h"

#include "RiuFileDialogTools.h"
#include "RiuTools.h"

#include "cafPdmUiButton.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <algorithm>
#include <map>
#include <set>

//==================================================================================================
/// Dialog for mapping well path laterals to imported MSW branch numbers
//==================================================================================================
class RiuMswBranchMappingDialog : public QDialog
{
public:
    RiuMswBranchMappingDialog( QWidget* parent, const std::vector<RimWellPath*> wellPaths, const std::set<int>& availableBranches )
        : QDialog( parent, RiuTools::defaultDialogFlags() )
    {
        setWindowTitle( "Assign Branch Numbers to Well Paths" );

        auto* mainLayout = new QVBoxLayout( this );

        auto* infoLabel = new QLabel( "Assign imported branch numbers to each well path:" );
        mainLayout->addWidget( infoLabel );

        auto* gridLayout = new QGridLayout();
        gridLayout->addWidget( new QLabel( "Well Path" ), 0, 0 );
        gridLayout->addWidget( new QLabel( "Branch Number" ), 0, 1 );

        int row = 1;
        for ( auto* wellPath : wellPaths )
        {
            auto* nameLabel = new QLabel( wellPath->name() );
            auto* comboBox  = new QComboBox();

            for ( int branch : availableBranches )
            {
                comboBox->addItem( QString::number( branch ), branch );
            }

            // Default selection: try to match row index to branch number
            int defaultIndex = std::min( row - 1, comboBox->count() - 1 );
            comboBox->setCurrentIndex( defaultIndex );

            gridLayout->addWidget( nameLabel, row, 0 );
            gridLayout->addWidget( comboBox, row, 1 );

            m_wellPathCombos.push_back( { wellPath, comboBox } );
            row++;
        }

        mainLayout->addLayout( gridLayout );

        auto* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
        connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
        mainLayout->addWidget( buttonBox );

        setLayout( mainLayout );
    }

    std::map<RimWellPath*, int> branchMapping() const
    {
        std::map<RimWellPath*, int> mapping;
        for ( const auto& [wellPath, comboBox] : m_wellPathCombos )
        {
            mapping[wellPath] = comboBox->currentData().toInt();
        }
        return mapping;
    }

private:
    std::vector<std::pair<RimWellPath*, QComboBox*>> m_wellPathCombos;
};

CAF_PDM_SOURCE_INIT( RimMswSegmentCollection, "RimMswSegmentCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswSegmentCollection::RimMswSegmentCollection()
{
    CAF_PDM_InitObject( "MSW Segments", ":/WellCollection.png" );

    CAF_PDM_InitFieldNoDefault( &m_segments, "Segments", "Segments" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );

    // m_isChecked field is defined in the base class RimCheckableNamedObject. Update its UI properties here.
    m_isChecked.uiCapability()->setUiHidden( false );
    m_isChecked.uiCapability()->setUiName( "Show MSW Segments" );

    setName( "MSW Segments" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswSegmentCollection::hasSegments() const
{
    return !m_segments.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimMswSegment*> RimMswSegmentCollection::segments() const
{
    std::vector<const RimMswSegment*> result;
    for ( const auto& seg : m_segments )
    {
        result.push_back( seg );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::appendSegment( RimMswSegment* segment )
{
    m_segments.push_back( segment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::clearSegments()
{
    m_segments.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::populateFromWelsegsData( std::vector<WelsegsRow> welsegsData, double wellTotalDepth )
{
    clearSegments();

    if ( welsegsData.empty() ) return;

    // Sort segments by measured depth (length)
    std::sort( welsegsData.begin(), welsegsData.end(), []( const WelsegsRow& a, const WelsegsRow& b ) { return a.length < b.length; } );

    // Create segments with computed start/end MD
    for ( size_t i = 0; i < welsegsData.size(); ++i )
    {
        const auto& row = welsegsData[i];

        double startMD = row.length;
        double endMD   = ( i + 1 < welsegsData.size() ) ? welsegsData[i + 1].length : wellTotalDepth;

        // Skip if start >= end (invalid interval)
        if ( startMD >= endMD ) continue;

        double diameter = row.diameter.value_or( 0.1 ); // Default diameter if not specified

        auto* segment = new RimMswSegment();
        segment->setSegmentData( row.segment1, row.branch, startMD, endMD, diameter );
        appendSegment( segment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegmentCollection::referenceDiameter() const
{
    if ( m_segments.empty() ) return 0.1; // Default diameter

    // Use the first segment's diameter as reference
    double firstDiameter = m_segments[0]->diameter();
    if ( firstDiameter > 0.0 ) return firstDiameter;

    return 0.1; // Default if no diameter specified
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimMswSegmentCollection::eclipseCase() const
{
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::updateSegments()
{
    // Clear existing segments before creating new ones
    clearSegments();

    auto scheduleRedraw = []()
    {
        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    };

    auto* wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath )
    {
        RiaLogging::error( "Unable to update MSW segments: no well path found." );
        scheduleRedraw();
        return;
    }

    if ( !m_eclipseCase() )
    {
        RiaLogging::error( "Unable to update MSW segments: no Eclipse case selected." );
        scheduleRedraw();
        return;
    }

    constexpr int timeStep        = 0;
    auto          tableDataResult = RicWellPathExportMswTableData::extractSingleWellMswData( m_eclipseCase(), wellPath, timeStep );

    if ( !tableDataResult.has_value() )
    {
        RiaLogging::error( QString::fromStdString( tableDataResult.error() ) );
        scheduleRedraw();
        return;
    }

    const RigMswTableData& tableData = tableDataResult.value();

    // Group segments by source well name
    std::map<std::string, std::vector<WelsegsRow>> segmentsByWell;
    for ( const auto& row : tableData.welsegsData() )
    {
        segmentsByWell[row.sourceWellName].push_back( row );
    }

    // Update MSW segments for each well path
    for ( const auto& [sourceWellName, segments] : segmentsByWell )
    {
        RimWellPath* targetWellPath = RimProject::current()->wellPathByName( QString::fromStdString( sourceWellName ) );
        if ( !targetWellPath )
        {
            RiaLogging::warning(
                QString( "Unable to find well path '%1' for MSW segment update." ).arg( QString::fromStdString( sourceWellName ) ) );
            continue;
        }

        if ( !targetWellPath->completions() || !targetWellPath->completions()->mswSegmentCollection() )
        {
            continue;
        }

        double wellTotalDepth = 0.0;
        if ( auto* geom = targetWellPath->wellPathGeometry() )
        {
            auto mds = geom->uniqueMeasuredDepths();
            if ( !mds.empty() ) wellTotalDepth = mds.back();
        }

        auto* segCollection = targetWellPath->completions()->mswSegmentCollection();
        segCollection->populateFromWelsegsData( segments, wellTotalDepth );
        segCollection->updateConnectedEditors();
    }

    scheduleRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_isChecked );
    auto group = uiOrdering.addNewGroup( "Segments from grid model" );
    group->add( &m_eclipseCase );
    auto updateButton = group->addNewButton( "Update Segments", [this]() { updateSegments(); } );
    updateButton->setAlignment( Qt::AlignRight );
    auto importButton = uiOrdering.addNewButton( "Import from File...", [this]() { importFromFile(); } );
    importButton->setAlignment( Qt::AlignRight );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::importFromFile()
{
    auto* wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath )
    {
        RiaLogging::error( "Unable to import MSW segments: no well path found." );
        return;
    }

    RiaApplication* app          = RiaApplication::instance();
    QString         defaultDir   = app->lastUsedDialogDirectory( "WELSEGS_IMPORT" );
    QString         filterText   = "Eclipse Data Files (*.data *.DATA);;Schedule Files (*.sch *.SCH);;All Files (*.*)";
    QString         selectedFile = RiuFileDialogTools::getOpenFileName( nullptr, "Import WELSEGS", defaultDir, filterText );

    if ( selectedFile.isEmpty() ) return;

    app->setLastUsedDialogDirectory( "WELSEGS_IMPORT", QFileInfo( selectedFile ).absolutePath() );

    auto welsegsData = RiaOpmParserTools::extractWelsegs( selectedFile.toStdString() );
    if ( welsegsData.empty() )
    {
        RiaLogging::warning( QString( "No WELSEGS data found in file: %1" ).arg( selectedFile ) );
        return;
    }

    // Get top-level well path to access all laterals
    RimWellPath* topLevelWell = wellPath->topLevelWellPath();
    if ( !topLevelWell ) topLevelWell = wellPath;

    // Use the well name from completion settings for matching with WELSEGS data
    QString wellName = topLevelWell->completionSettings()->wellNameForExport();

    std::vector<WelsegsRow> matchingSegments;
    for ( const auto& [name, segments] : welsegsData )
    {
        if ( QString::fromStdString( name ) == wellName )
        {
            matchingSegments = segments;
            break;
        }
    }

    if ( matchingSegments.empty() )
    {
        QString availableWells;
        for ( const auto& [name, segments] : welsegsData )
        {
            if ( !availableWells.isEmpty() ) availableWells += ", ";
            availableWells += QString::fromStdString( name );
        }
        RiaLogging::warning( QString( "No WELSEGS data found for well '%1'. Available wells: %2" ).arg( wellName ).arg( availableWells ) );
        return;
    }

    // Extract unique branch numbers from imported segments
    std::set<int> availableBranches;
    for ( const auto& segment : matchingSegments )
    {
        availableBranches.insert( segment.branch );
    }

    // Collect all well paths: main well + laterals
    std::vector<RimWellPath*> allWellPaths;
    allWellPaths.push_back( topLevelWell );
    for ( auto* lateral : topLevelWell->wellPathLaterals() )
    {
        allWellPaths.push_back( lateral );
    }

    // Show branch mapping dialog if there are multiple branches or laterals
    std::map<RimWellPath*, int> branchMapping;
    if ( allWellPaths.size() > 1 || availableBranches.size() > 1 )
    {
        RiuMswBranchMappingDialog dialog( nullptr, allWellPaths, availableBranches );
        if ( dialog.exec() != QDialog::Accepted )
        {
            return;
        }
        branchMapping = dialog.branchMapping();
    }
    else
    {
        // Single well path and single branch - auto-assign
        if ( !availableBranches.empty() )
        {
            branchMapping[topLevelWell] = *availableBranches.begin();
        }
    }

    // Import segments for each well path based on branch mapping
    for ( const auto& [targetWellPath, branchNumber] : branchMapping )
    {
        // Filter segments by branch number
        std::vector<WelsegsRow> branchSegments;
        for ( const auto& segment : matchingSegments )
        {
            if ( segment.branch == branchNumber )
            {
                branchSegments.push_back( segment );
            }
        }

        if ( branchSegments.empty() ) continue;

        // Get well total depth for this well path
        double wellTotalDepth = 0.0;
        if ( auto* geom = targetWellPath->wellPathGeometry() )
        {
            auto mds = geom->uniqueMeasuredDepths();
            if ( !mds.empty() ) wellTotalDepth = mds.back();
        }

        // Get MSW segment collection for this well path
        if ( !targetWellPath->completions() || !targetWellPath->completions()->mswSegmentCollection() )
        {
            continue;
        }

        auto* segCollection = targetWellPath->completions()->mswSegmentCollection();
        segCollection->clearSegments();
        segCollection->populateFromWelsegsData( branchSegments, wellTotalDepth );
        segCollection->updateConnectedEditors();

        RiaLogging::info( QString( "Imported %1 WELSEGS segments (branch %2) for well '%3'" )
                              .arg( branchSegments.size() )
                              .arg( branchNumber )
                              .arg( targetWellPath->name() ) );
    }

    if ( RimProject* project = RimProject::current() )
    {
        project->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMswSegmentCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        auto cases = RimEclipseCaseTools::eclipseCases();
        for ( auto* c : cases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegmentCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isChecked )
    {
        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}
