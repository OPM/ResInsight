/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimEclipseBoundingBoxCase.h"

#include "RiaApplication.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "RiaRegressionTestRunner.h"
#include "RiaResultNames.h"

#include "RicfCommandObject.h"

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseRestartDataAccess.h"
#include "RifInputPropertyLoader.h"
#include "RifReaderBoundingBoxModel.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderOpmCommon.h"
#include "RifReaderOpmCommonActive.h"
#include "RifReaderOpmRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigMainGrid.h"

#include "RimDialogData.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimMockModelSettings.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"

#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafUtils.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <fstream>
#include <string>

CAF_PDM_SOURCE_INIT( RimEclipseBoundingBoxCase, "EclipseBoundingBoxCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseBoundingBoxCase::RimEclipseBoundingBoxCase()
{
    CAF_PDM_InitScriptableObject( "Bounding Box Case", ":/Case48x48.png", "", "Bounding Box Case" );

    //     CAF_PDM_InitFieldNoDefault( &m_unitSystem, "UnitSystem", "Unit System" );
    //     m_unitSystem.registerGetMethod( RimProject::current(), &RimProject::commonUnitSystemForAllCases );
    //     m_unitSystem.uiCapability()->setUiReadOnly( true );

    //     CAF_PDM_InitFieldNoDefault( &m_flowDiagSolutions, "FlowDiagSolutions", "Flow Diagnostics Solutions" );
    //     m_flowDiagSolutions.uiCapability()->setUiTreeChildrenHidden( true );

    //     m_flipXAxis.xmlCapability()->setIOWritable( true );
    //     m_flipYAxis.xmlCapability()->setIOWritable( true );

    //     CAF_PDM_InitFieldNoDefault( &m_sourSimFileName, "SourSimFileName", "SourSim File Name" );
    //     m_sourSimFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    // #ifndef USE_HDF5
    //     m_sourSimFileName.uiCapability()->setUiHidden( true );
    // #endif

    //     CAF_PDM_InitField( &m_mswMergeThreshold, "MswMergeThreshold", std::make_pair( false, 3 ), "MSW Short Well Merge Threshold" );
    //     m_mswMergeThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_minimum, "Minimum", "Minimum" );
    m_minimum.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_maximum, "Maximum", "Maximum" );
    m_maximum.uiCapability()->setUiReadOnly( true );

    m_minimum = cvf::Vec3d( 461275, 5.93004e+06, -1697.72 );
    m_maximum = cvf::Vec3d( 466164, 5.9355e+06, -1556.65 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

void RimEclipseBoundingBoxCase::setBoundingBox( const cvf::BoundingBox& boundingBox )
{
    m_minimum = boundingBox.min();
    m_maximum = boundingBox.max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseBoundingBoxCase::createModel( QString modelName )
{
    cvf::ref<RifReaderBoundingBoxModel> reader    = new RifReaderBoundingBoxModel;
    cvf::ref<RigEclipseCaseData>        reservoir = new RigEclipseCaseData( this );

    // mockFileInterface->setWorldCoordinates( cvf::Vec3d( startX + offsetX, startY + offsetY, startZ + offsetZ ),
    //                                         cvf::Vec3d( startX + widthX + offsetX, startY + widthY + offsetY, startZ + widthZ +
    //                                         offsetZ ) );

    reader->setWorldCoordinates( m_minimum, m_maximum );

    cvf::Vec3st gridPointDimensions( 50, 50, 10 );
    reader->setGridPointDimensions( gridPointDimensions );
    // cvf::Vec3st( mockModelSettings->cellCountX + 1, mockModelSettings->cellCountY + 1, mockModelSettings->cellCountZ + 1 ) );
    // mockFileInterface->setResultInfo( mockModelSettings->resultCount, mockModelSettings->timeStepCount );
    // mockFileInterface->enableWellData( false );

    reader->open( "", reservoir.p() );

    setReservoirData( reservoir.p() );

    return reader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseBoundingBoxCase::openEclipseGridFile()
{
    auto readerInterface = createModel( "" );
    // std::vector<QDateTime> timeSteps;
    // timeSteps.push_back( QDateTime::currentDateTime() );
    // m_timeStepFilter->setTimeStepsFromFile( timeSteps );

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
    results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

    computeCachedData();
    // results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();
    // results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeDepthRelatedResults();
    // results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->computeDepthRelatedResults();
    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimEclipseBoundingBoxCase::~RimEclipseBoundingBoxCase()
// {
//     // Disconnect all comparison views. In debug build on Windows, a crash occurs. The comparison view is also set to zero in the
//     destructor
//     // of Rim3dView()
//     for ( auto v : reservoirViews() )
//     {
//         if ( v ) v->setComparisonView( nullptr );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// QString RimEclipseBoundingBoxCase::locationOnDisc() const
// {
//     QFileInfo fi( gridFileName() );
//     return fi.absolutePath();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::readGridDimensions( std::vector<std::vector<int>>& gridDimensions )
// {
//     RifEclipseOutputFileTools::readGridDimensions( gridFileName(), gridDimensions );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RimFlowDiagSolution* RimEclipseBoundingBoxCase::defaultFlowDiagSolution()
// {
//     if ( !m_flowDiagSolutions.empty() )
//     {
//         return m_flowDiagSolutions[0];
//     }

//     return nullptr;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// std::vector<RimFlowDiagSolution*> RimEclipseBoundingBoxCase::flowDiagSolutions()
// {
//     std::vector<RimFlowDiagSolution*> flowSols;
//     for ( const caf::PdmPointer<RimFlowDiagSolution>& fsol : m_flowDiagSolutions )
//     {
//         flowSols.push_back( fsol.p() );
//     }

//     return flowSols;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RigFlowDiagSolverInterface* RimEclipseBoundingBoxCase::flowDiagSolverInterface()
// {
//     return m_flowDagSolverInterface.p();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RifReaderRftInterface* RimEclipseBoundingBoxCase::rftReader()
// {
//     ensureRftDataIsImported();

//     if ( m_useOpmRftReader ) return m_readerOpmRft.p();

//     return m_readerEclipseRft.p();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// int RimEclipseBoundingBoxCase::mswMergeThreshold() const
// {
//     // This value is used in RigMswCenterLineCalculator::calculateMswWellPipeGeometry

//     if ( m_mswMergeThreshold().first )
//     {
//         return m_mswMergeThreshold().second;
//     }

//     return 4;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::setCaseInfo( const QString& userDescription, const QString& fileName )
// {
//     setCaseUserDescription( userDescription );
//     setGridFileName( fileName );

//     RimProject* proj = RimProject::current();
//     proj->assignCaseIdToCase( this );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::setSourSimFileName( const QString& fileName )
// {
//     m_sourSimFileName = fileName;

//     loadAndUpdateSourSimData();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RimEclipseBoundingBoxCase::hasSourSimFile()
// {
//     return !m_sourSimFileName().path().isEmpty();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
// {
//     uiOrdering.add( &m_caseUserDescription );
//     uiOrdering.add( &m_displayNameOption );
//     uiOrdering.add( &m_caseId );
//     uiOrdering.add( &m_caseFileName );
//     uiOrdering.add( &m_unitSystem );

//     auto group = uiOrdering.addNewGroup( "Case Options" );
//     group->add( &m_activeFormationNames );
//     group->add( &m_flipXAxis );
//     group->add( &m_flipYAxis );
//     group->add( &m_mswMergeThreshold );

//     if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) &&
//          eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->maxTimeStepCount() > 0 )
//     {
//         auto group1 = uiOrdering.addNewGroup( "Time Step Filter" );
//         group1->setCollapsedByDefault();
//         m_timeStepFilter->uiOrdering( uiConfigName, *group1 );
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant&
// newValue )
// {
//     if ( changedField == &m_sourSimFileName )
//     {
//         loadAndUpdateSourSimData();
//     }

//     if ( changedField == &m_mswMergeThreshold )
//     {
//         for ( auto resView : reservoirViews() )
//         {
//             resView->scheduleSimWellGeometryRegen();
//             resView->scheduleCreateDisplayModelAndRedraw();
//         }
//     }

//     return RimEclipseCase::fieldChangedByUi( changedField, oldValue, newValue );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimEclipseBoundingBoxCase::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute*
// attribute )
// {
//     if ( field == &m_sourSimFileName )
//     {
//         auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
//         if ( myAttr )
//         {
//             myAttr->m_fileSelectionFilter = "SourSim (*.sourres)";
//             myAttr->m_defaultPath         = QFileInfo( gridFileName() ).absolutePath();
//         }
//     }
// }
