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

#include "RimcEclipseView.h"

#include "ExportCommands/RicEclipseCellResultToFileImpl.h"

#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultDistance.h"
#include "RimFaultDistanceCollection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafUtils.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseView, RimcEclipseView_addFaultDistance, "add_fault_distance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseView_addFaultDistance::RimcEclipseView_addFaultDistance( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Fault Distance", "", "", "Create a FAULTDIST cell result for a chosen subset of faults" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "Name", "Name (default FAULTDIST<n> if empty)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_faults, "Faults", "Faults to include (empty = all)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseView_addFaultDistance::execute()
{
    auto* eclipseView = self<RimEclipseView>();
    if ( !eclipseView ) return std::unexpected( QString( "No view" ) );

    auto* distanceCollection = eclipseView->faultDistanceCollection();
    if ( !distanceCollection ) return std::unexpected( QString( "No fault distance results collection" ) );

    auto* newResult = distanceCollection->addResult();
    if ( !newResult ) return std::unexpected( QString( "Failed to create fault distance result" ) );

    if ( !m_resultName().isEmpty() ) newResult->setResultName( m_resultName() );

    std::vector<RimFaultInView*> selected = m_faults.ptrReferencedObjectsByType();
    if ( selected.empty() && eclipseView->faultCollection() )
    {
        // Default to all faults, but leave the ResInsight-generated faults unticked.
        selected = eclipseView->faultCollection()->faults();
        std::erase_if( selected, []( RimFaultInView* fault ) { return fault && fault->isGeneratedFault(); } );
    }

    newResult->setSelectedFaults( selected );

    // When created from Python, always trigger the calculation (the UI uses an explicit Generate button instead).
    newResult->compute();

    eclipseView->updateConnectedEditors();

    return newResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcEclipseView_addFaultDistance::classKeywordReturnedType() const
{
    return RimFaultDistance::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseView, RimEclipseView_exportCurrentProperty, "exportCurrentProperty" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView_exportCurrentProperty::RimEclipseView_exportCurrentProperty( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Current Property", "", "", "Export the cell result currently shown in the view to a GRDECL style text file" );

    CAF_PDM_InitScriptableField( &m_exportFile, "ExportFile", QString(), "Export File", "", "", "Full path of the file to write" );
    CAF_PDM_InitScriptableField( &m_undefinedValue, "UndefinedValue", 0.0, "Undefined Value", "", "", "Value written for undefined cells" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseView_exportCurrentProperty::setExportFile( const QString& exportFile )
{
    m_exportFile = exportFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseView_exportCurrentProperty::setUndefinedValue( double undefinedValue )
{
    m_undefinedValue = undefinedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseView_exportCurrentProperty::execute()
{
    auto* view = self<RimEclipseView>();
    if ( !view ) return std::unexpected( "No view is available." );

    if ( m_exportFile().isEmpty() ) return std::unexpected( "No export file specified." );

    RimEclipseCase* eclipseCase = view->eclipseCase();
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return std::unexpected( "The view has no case data." );

    const int mainGridIndex = 0;

    cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                                                       mainGridIndex,
                                                                                                       view->currentTimeStep(),
                                                                                                       view->cellResult() );

    const QString propertyName = view->cellResult()->resultVariableUiShortName();

    if ( resultAccessor.isNull() )
    {
        return std::unexpected( QString( "Could not find property '%1' at time step %2 in view '%3'" )
                                    .arg( propertyName )
                                    .arg( view->currentTimeStep() )
                                    .arg( view->name() ) );
    }

    const bool writeEchoKeywords = false;
    QString    errorMessage;
    if ( !RicEclipseCellResultToFileImpl::writeResultToTextFile( m_exportFile(),
                                                                 eclipseCase->eclipseCaseData(),
                                                                 resultAccessor.p(),
                                                                 propertyName,
                                                                 m_undefinedValue(),
                                                                 "exportProperty",
                                                                 writeEchoKeywords,
                                                                 &errorMessage ) )
    {
        return std::unexpected( errorMessage );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseView_exportCurrentProperty::defaultFileBaseName( const RimEclipseView* view )
{
    if ( !view || !view->eclipseCase() ) return {};

    const QString propertyName = view->cellResult()->resultVariableUiShortName();
    const QString fileName =
        QString( "%1-%2-T%3-%4" ).arg( view->eclipseCase()->caseUserDescription() ).arg( view->name() ).arg( view->currentTimeStep() ).arg( propertyName );

    return caf::Utils::makeValidFileBasename( fileName );
}
