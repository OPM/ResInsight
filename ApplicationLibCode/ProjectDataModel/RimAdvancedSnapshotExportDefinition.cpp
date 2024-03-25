/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimAdvancedSnapshotExportDefinition.h"

#include "RiaOptionItemFactory.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigReservoirGridTools.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafPdmPointer.h"
#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT( RimAdvancedSnapshotExportDefinition, "MultiSnapshotDefinition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAdvancedSnapshotExportDefinition::RimAdvancedSnapshotExportDefinition()
{
    // CAF_PDM_InitObject("MultiSnapshotDefinition", ":/Well.svg");
    CAF_PDM_InitObject( "MultiSnapshotDefinition" );

    CAF_PDM_InitField( &isActive, "IsActive", true, "Active" );

    CAF_PDM_InitFieldNoDefault( &view, "View", "View" );

    CAF_PDM_InitFieldNoDefault( &eclipseResultType, "EclipseResultType", "Result Type" );
    CAF_PDM_InitFieldNoDefault( &m_selectedEclipseResult, "SelectedEclipseResults", "Properties" );

    CAF_PDM_InitField( &timeStepStart, "TimeStepStart", 0, "Start Time" );
    CAF_PDM_InitField( &timeStepEnd, "TimeStepEnd", 0, "End Time" );

    CAF_PDM_InitField( &sliceDirection,
                       "SnapShotDirection",
                       caf::AppEnum<RiaDefines::GridCaseAxis>( RiaDefines::GridCaseAxis::UNDEFINED_AXIS ),
                       "Range Filter Slice" );

    CAF_PDM_InitField( &startSliceIndex, "RangeFilterStart", 1, "Range Start" );
    CAF_PDM_InitField( &endSliceIndex, "RangeFilterEnd", 1, "Range End" );

    CAF_PDM_InitFieldNoDefault( &additionalCases, "AdditionalCases", "Cases" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAdvancedSnapshotExportDefinition::~RimAdvancedSnapshotExportDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAdvancedSnapshotExportDefinition::setSelectedEclipseResults( const QString& result )
{
    m_selectedEclipseResult = result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimAdvancedSnapshotExportDefinition::selectedEclipseResults() const
{
    // The interface here can return a vector of selected results. The user interface in the table is not working well for multi-select of
    // strings, so we only allow one result to be selected at a time.

    return { m_selectedEclipseResult() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAdvancedSnapshotExportDefinition::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &view )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        std::vector<Rim3dView*> views;

        RimProject*           proj  = RimProject::current();
        std::vector<RimCase*> cases = proj->allGridCases();
        for ( RimCase* rimCase : cases )
        {
            for ( Rim3dView* rimView : rimCase->views() )
            {
                views.push_back( rimView );
            }
        }

        for ( Rim3dView* rim3dView : views )
        {
            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( rim3dView, &options );
        }
    }
    else if ( fieldNeedingOptions == &eclipseResultType )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ResultCatType>( RiaDefines::ResultCatType::DYNAMIC_NATIVE ).uiText(),
                                                   RiaDefines::ResultCatType::DYNAMIC_NATIVE ) );
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ResultCatType>( RiaDefines::ResultCatType::STATIC_NATIVE ).uiText(),
                                                   RiaDefines::ResultCatType::STATIC_NATIVE ) );
    }
    else if ( fieldNeedingOptions == &m_selectedEclipseResult )
    {
        auto* rimEclipseView = dynamic_cast<RimEclipseView*>( view() );
        if ( rimEclipseView )
        {
            QStringList varList;
            varList = rimEclipseView->currentGridCellResults()->resultNames( eclipseResultType() );

            options = toOptionList( varList );
        }
    }
    else if ( fieldNeedingOptions == &timeStepEnd )
    {
        getTimeStepStrings( options );
    }
    else if ( fieldNeedingOptions == &timeStepStart )
    {
        getTimeStepStrings( options );
    }
    else if ( fieldNeedingOptions == &additionalCases )
    {
        RimTools::caseOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAdvancedSnapshotExportDefinition::getTimeStepStrings( QList<caf::PdmOptionItemInfo>& options )
{
    if ( !view() ) return;

    QStringList timeSteps;

    timeSteps = view->ownerCase()->timeStepStrings();

    for ( int i = 0; i < timeSteps.size(); i++ )
    {
        options.push_back( caf::PdmOptionItemInfo( timeSteps[i], i ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAdvancedSnapshotExportDefinition::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                            const QVariant&            oldValue,
                                                            const QVariant&            newValue )
{
    if ( changedField == &eclipseResultType )
    {
        m_selectedEclipseResult.v().clear();
    }
    else if ( changedField == &sliceDirection )
    {
        const cvf::StructGridInterface* mainGrid    = nullptr;
        const RigActiveCellInfo*        actCellInfo = nullptr;

        if ( view() )
        {
            actCellInfo = RigReservoirGridTools::activeCellInfo( view() );

            auto rimCase = view()->ownerCase();
            if ( rimCase )
            {
                mainGrid = RigReservoirGridTools::mainGrid( rimCase );
            }
        }

        if ( mainGrid && actCellInfo )
        {
            auto [min, max] = actCellInfo->ijkBoundingBox();

            // Adjust to Eclipse indexing
            min.x() = min.x() + 1;
            min.y() = min.y() + 1;
            min.z() = min.z() + 1;

            max.x() = max.x() + 1;
            max.y() = max.y() + 1;
            max.z() = max.z() + 1;

            int maxInt = 0;
            int minInt = 0;

            if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_I )
            {
                maxInt = static_cast<int>( max.x() );
                minInt = static_cast<int>( min.x() );
            }
            else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_J )
            {
                maxInt = static_cast<int>( max.y() );
                minInt = static_cast<int>( min.y() );
            }
            else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
            {
                maxInt = static_cast<int>( max.z() );
                minInt = static_cast<int>( min.z() );
            }

            startSliceIndex = minInt;
            endSliceIndex   = maxInt;
        }

        startSliceIndex.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAdvancedSnapshotExportDefinition::toOptionList( const QStringList& varList )
{
    QList<caf::PdmOptionItemInfo> optionList;
    int                           i;
    for ( i = 0; i < varList.size(); ++i )
    {
        optionList.push_back( caf::PdmOptionItemInfo( varList[i], varList[i] ) );
    }
    return optionList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAdvancedSnapshotExportDefinition::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !isActive() )
    {
        view.uiCapability()->setUiReadOnly( true );
        eclipseResultType.uiCapability()->setUiReadOnly( true );
        m_selectedEclipseResult.uiCapability()->setUiReadOnly( true );
        timeStepStart.uiCapability()->setUiReadOnly( true );
        timeStepEnd.uiCapability()->setUiReadOnly( true );
        sliceDirection.uiCapability()->setUiReadOnly( true );
        startSliceIndex.uiCapability()->setUiReadOnly( true );
        endSliceIndex.uiCapability()->setUiReadOnly( true );
        additionalCases.uiCapability()->setUiReadOnly( true );
    }
    else
    {
        view.uiCapability()->setUiReadOnly( false );

        if ( !view() )
        {
            eclipseResultType.uiCapability()->setUiReadOnly( true );
            m_selectedEclipseResult.uiCapability()->setUiReadOnly( true );
            timeStepStart.uiCapability()->setUiReadOnly( true );
            timeStepEnd.uiCapability()->setUiReadOnly( true );
            sliceDirection.uiCapability()->setUiReadOnly( true );
            startSliceIndex.uiCapability()->setUiReadOnly( true );
            endSliceIndex.uiCapability()->setUiReadOnly( true );
            additionalCases.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            eclipseResultType.uiCapability()->setUiReadOnly( false );
            m_selectedEclipseResult.uiCapability()->setUiReadOnly( false );
            timeStepStart.uiCapability()->setUiReadOnly( false );
            timeStepEnd.uiCapability()->setUiReadOnly( false );
            sliceDirection.uiCapability()->setUiReadOnly( false );

            additionalCases.uiCapability()->setUiReadOnly( false );

            bool rangeReadOnly = false;
            if ( sliceDirection() == RiaDefines::GridCaseAxis::UNDEFINED_AXIS )
            {
                rangeReadOnly = true;
            }

            startSliceIndex.uiCapability()->setUiReadOnly( rangeReadOnly );
            endSliceIndex.uiCapability()->setUiReadOnly( rangeReadOnly );
        }
    }
}
