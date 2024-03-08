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
//////////////////////////////////////////////////////////////////////////////////

#include "RiaGrpcGridGeometryExtractionService.h"

#include "qstring.h"

#include "Commands/RicImportGeneralDataFeature.h"
#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"
#include "RifReaderSettings.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RivEclipseIntersectionGrid.h"
#include "RivGridPartMgr.h"
#include "RivPolylineIntersectionGeometryGenerator.h"
#include "RivReservoirPartMgr.h"
#include "RivReservoirViewPartMgr.h"

#include "cafSelectionManagerTools.h"

#include "cvfArray.h"
#include "cvfDrawableGeo.h"
#include "cvfStructGridGeometryGenerator.h"

#include "qfileinfo.h"
#include "qstring.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::GetGridSurface( grpc::ServerContext*               context,
                                                                   const rips::GetGridSurfaceRequest* request,
                                                                   rips::GetGridSurfaceResponse*      response )
{
    // Reset all pointers
    resetInternalPointers();

    // Initialize pointers
    grpc::Status status = initializeApplicationAndEclipseCaseAndEclipseViewFromAbsoluteFilePath( request->gridfilename() );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    // Apply ijk-filtering - assuming 0-indexing from gRPC
    if ( request->has_ijkindexfilter() )
    {
        status = applyIJKCellFilterToEclipseCase( request->ijkindexfilter() );
        if ( status.error_code() != grpc::StatusCode::OK )
        {
            return status;
        }
    }

    // Ensure static geometry parts created for grid part manager in view
    m_eclipseView->createGridGeometryParts();

    // Initialize grid geometry generator
    const bool useOpenMP             = false;
    auto*      gridGeometryGenerator = new cvf::StructGridGeometryGenerator( m_eclipseView->mainGrid(), useOpenMP );
    status = initializeGridGeometryGeneratorWithEclipseViewCellVisibility( gridGeometryGenerator );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    auto* gridSurfaceVertices = gridGeometryGenerator->getOrCreateVertices();
    if ( gridSurfaceVertices == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No grid vertices found" );
    }

    // Set vertex_array and quadindicesarr response
    for ( int i = 0; i < gridSurfaceVertices->size(); ++i )
    {
        const auto& vertex = gridSurfaceVertices->get( i );
        response->add_vertexarray( vertex.x() );
        response->add_vertexarray( vertex.y() );
        response->add_vertexarray( vertex.z() );

        response->add_quadindicesarr( i );
    }

    // Origin in utm is the offset
    rips::Vec3d* modelOffset         = new rips::Vec3d;
    const auto   mainGridModelOffset = m_eclipseView->mainGrid()->displayModelOffset();
    modelOffset->set_x( mainGridModelOffset.x() );
    modelOffset->set_y( mainGridModelOffset.y() );
    modelOffset->set_z( mainGridModelOffset.z() );
    response->set_allocated_originutm( modelOffset );

    // Source cell indices from main grid part manager
    std::vector<size_t> sourceCellIndicesArray = std::vector<size_t>();
    if ( gridGeometryGenerator->quadToCellFaceMapper() != nullptr )
    {
        sourceCellIndicesArray = gridGeometryGenerator->quadToCellFaceMapper()->quadToCellIndicesArray();
    }
    if ( sourceCellIndicesArray.empty() )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No source cell indices array found" );
    }
    for ( const auto& sourceCellIndex : sourceCellIndicesArray )
    {
        response->add_sourcecellindicesarr( static_cast<google::protobuf::uint32>( sourceCellIndex ) );
    }

    // Set grid dimensions
    const auto   countI     = m_eclipseView->mainGrid()->cellCountI();
    const auto   countJ     = m_eclipseView->mainGrid()->cellCountJ();
    const auto   countK     = m_eclipseView->mainGrid()->cellCountK();
    rips::Vec3i* dimensions = new rips::Vec3i;
    dimensions->set_i( countI );
    dimensions->set_j( countJ );
    dimensions->set_k( countK );
    response->set_allocated_griddimensions( dimensions );

    // Close project and return
    if ( m_application != nullptr )
    {
        m_application->closeProject();
    }
    resetInternalPointers();

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::CutAlongPolyline( grpc::ServerContext*                 context,
                                                                     const rips::CutAlongPolylineRequest* request,
                                                                     rips::CutAlongPolylineResponse*      response )
{
    // Reset all pointers
    resetInternalPointers();

    // Initialize pointers
    grpc::Status status = initializeApplicationAndEclipseCaseAndEclipseViewFromAbsoluteFilePath( request->gridfilename() );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    // Ensure static geometry parts created for grid part manager in view
    m_eclipseView->createGridGeometryParts();

    auto& fencePolyline = request->fencepolylineutmxy();
    if ( fencePolyline.size() < 2 )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "Invalid fence polyline" );
    }

    // Convert polyline to vector of cvf::Vec3d
    std::vector<cvf::Vec2d> polylineUtmXy;
    for ( int i = 0; i < fencePolyline.size(); i += 2 )
    {
        const double xValue = fencePolyline.Get( i );
        const double yValue = fencePolyline.Get( i + 1 );
        polylineUtmXy.push_back( cvf::Vec2d( xValue, yValue ) );
    }

    RigActiveCellInfo*          activeCellInfo    = nullptr; // No active cell info for grid
    const bool                  showInactiveCells = true;
    RivEclipseIntersectionGrid* eclipseIntersectionGrid =
        new RivEclipseIntersectionGrid( m_eclipseView->mainGrid(), activeCellInfo, showInactiveCells );

    auto* polylineIntersectionGenerator =
        new RivPolylineIntersectionGeometryGenerator( polylineUtmXy, eclipseIntersectionGrid );

    // Handle cell visibilities
    const int        firstTimeStep = 0;
    cvf::UByteArray* visibleCells  = new cvf::UByteArray( m_eclipseView->mainGrid()->cellCount() );
    m_eclipseView->calculateCurrentTotalCellVisibility( visibleCells, firstTimeStep );

    // Loop to count number of visible cells
    int numVisibleCells = 0;
    for ( size_t i = 0; i < visibleCells->size(); ++i )
    {
        if ( ( *visibleCells )[i] != 0 )
        {
            ++numVisibleCells;
        }
    }

    // Generate intersection
    polylineIntersectionGenerator->generateIntersectionGeometry( visibleCells );
    if ( !polylineIntersectionGenerator->isAnyGeometryPresent() )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "No intersection geometry present" );
    }

    // Add fence mesh sections
    const auto& polylineSegmentsMeshData = polylineIntersectionGenerator->polylineSegmentsMeshData();
    for ( const auto& segment : polylineSegmentsMeshData )
    {
        auto* fenceMeshSection = response->add_fecemeshsections();

        // Set start UTM (x,y)
        rips::Vec2d* startUtmXY = new rips::Vec2d;
        startUtmXY->set_x( segment.startUtmXY.x() );
        startUtmXY->set_y( segment.startUtmXY.y() );
        fenceMeshSection->set_allocated_startutmxy( startUtmXY );

        // Set end UTM (x,y)
        rips::Vec2d* endUtmXY = new rips::Vec2d;
        endUtmXY->set_x( segment.endUtmXY.x() );
        endUtmXY->set_y( segment.endUtmXY.y() );
        fenceMeshSection->set_allocated_endutmxy( endUtmXY );

        // Fill the vertext array with coordinates
        for ( const auto& coord : segment.vertexArrayUZ )
        {
            fenceMeshSection->add_vertexarrayuz( coord );
        }

        // Fill vertices per polygon array
        for ( const auto& verticesPerPolygon : segment.verticesPerPolygon )
        {
            fenceMeshSection->add_verticesperpolygonarr( static_cast<google::protobuf::uint32>( verticesPerPolygon ) );
        }

        // Fill polygon indices array
        for ( const auto& polygonIndex : segment.polygonIndices )
        {
            fenceMeshSection->add_polyindicesarr( static_cast<google::protobuf::uint32>( polygonIndex ) );
        }

        // Fill the source cell indices array
        for ( const auto& sourceCellIndex : segment.polygonToCellIndexMap )
        {
            fenceMeshSection->add_sourcecellindicesarr( static_cast<google::protobuf::uint32>( sourceCellIndex ) );
        }
    }

    // Add temporary test response
    {
        rips::PolylineTestResponse* polylineTestResponse = new rips::PolylineTestResponse;

        // Polygon vertices
        const auto& polygonVertices = polylineIntersectionGenerator->polygonVxes();
        if ( polygonVertices->size() == 0 )
        {
            return grpc::Status( grpc::StatusCode::NOT_FOUND, "No polygon vertices found for polyline" );
        }
        for ( int i = 0; i < polygonVertices->size(); ++i )
        {
            const auto& vertex = polygonVertices->get( i );
            polylineTestResponse->add_polygonvertexarray( vertex.x() );
            polylineTestResponse->add_polygonvertexarray( vertex.y() );
            polylineTestResponse->add_polygonvertexarray( vertex.z() );
        }

        // Vertices per polygon
        const auto& verticesPerPolygon = polylineIntersectionGenerator->vertiesPerPolygon();
        for ( const auto& elm : verticesPerPolygon )
        {
            polylineTestResponse->add_verticesperpolygonarr( static_cast<google::protobuf::uint32>( elm ) );
        }

        // Polygon to cell indices
        const auto& polygonCellIndices = polylineIntersectionGenerator->polygonToCellIndex();
        for ( const auto& elm : polygonCellIndices )
        {
            polylineTestResponse->add_sourcecellindicesarr( static_cast<google::protobuf::uint32>( elm ) );
        }

        response->set_allocated_polylinetestresponse( polylineTestResponse );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcGridGeometryExtractionService::createCallbacks()
{
    typedef RiaGrpcGridGeometryExtractionService Self;

    return { new RiaGrpcUnaryCallback<Self, rips::GetGridSurfaceRequest, rips::GetGridSurfaceResponse>( this,
                                                                                                        &Self::GetGridSurface,
                                                                                                        &Self::RequestGetGridSurface ),
             new RiaGrpcUnaryCallback<Self, rips::CutAlongPolylineRequest, rips::CutAlongPolylineResponse>( this,
                                                                                                            &Self::CutAlongPolyline,
                                                                                                            &Self::RequestCutAlongPolyline ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGridGeometryExtractionService::resetInternalPointers()
{
    m_application = nullptr;
    m_eclipseCase = nullptr;
    m_eclipseView = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Apply ijk-filtering - assuming 0-indexing from gRPC
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::applyIJKCellFilterToEclipseCase( const rips::IJKIndexFilter& filter )
{
    if ( m_eclipseCase == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No initialized eclipse case found" );
    }
    if ( m_eclipseView == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No initialized eclipse view found" );
    }

    // Find the selected Cell Filter Collection
    RimCellFilterCollection* filtColl = m_eclipseView->cellFilterCollection();
    if ( filtColl == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No cell filter collection found for eclipse view" );
    }

    // Add range filter object
    const int           sliceDirection  = -1;
    const int           gridIndex       = 0;
    RimCellRangeFilter* cellRangeFilter = filtColl->addNewCellRangeFilter( m_eclipseCase, gridIndex, sliceDirection );

    // Apply ijk-filter values to range filter object
    cellRangeFilter->startIndexI = filter.imin() + 1; // Eclipse indexing, first index is 1
    cellRangeFilter->startIndexJ = filter.jmin() + 1; // Eclipse indexing, first index is 1
    cellRangeFilter->startIndexK = filter.kmin() + 1; // Eclipse indexing, first index is 1
    cellRangeFilter->cellCountI  = filter.imax() - filter.imin() + 1;
    cellRangeFilter->cellCountJ  = filter.jmax() - filter.jmin() + 1;
    cellRangeFilter->cellCountK  = filter.kmax() - filter.kmin() + 1;

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::initializeGridGeometryGeneratorWithEclipseViewCellVisibility(
    cvf::StructGridGeometryGenerator* generator )
{
    if ( m_eclipseView == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No initialized eclipse view found" );
    }

    auto* mainGrid = m_eclipseCase->mainGrid();
    if ( mainGrid == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No main grid found for eclipse view" );
    }

    // Cell visibilities
    const int firstTimeStep    = 0;
    auto*     cellVisibilities = new cvf::UByteArray( mainGrid->cellCount() );
    m_eclipseView->calculateCurrentTotalCellVisibility( cellVisibilities, firstTimeStep );

    // Face visibility filter
    auto* faceVisibilityFilter = new RigGridCellFaceVisibilityFilter( mainGrid );

    generator->setCellVisibility( cellVisibilities );
    generator->addFaceVisibilityFilter( faceVisibilityFilter );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::loadGridGeometryFromAbsoluteFilePath( const std::string filePath )
{
    QString   absolutePath = QString::fromStdString( filePath );
    QFileInfo projectPathInfo( absolutePath );

    std::shared_ptr<RifReaderSettings> readerSettings;
    readerSettings = RifReaderSettings::createGridOnlyReaderSettings();

    // TODO: Set true or false?
    bool createPlot       = true;
    bool createView       = true;
    auto fileOpenMetaData = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList{ absolutePath },
                                                                                        createPlot,
                                                                                        createView,
                                                                                        readerSettings );

    if ( fileOpenMetaData.createdCaseIds.empty() )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND,
                             QString( "loadCase: Unable to load case from %1" ).arg( absolutePath ).toStdString() );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
/// Helper function to get application instance and load eclipse case from given file path
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::initializeApplicationAndEclipseCaseAndEclipseViewFromAbsoluteFilePath(
    const std::string filePath )
{
    // Get ResInsight instance and open grid
    m_application = RiaApplication::instance();

    if ( m_application == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "ResInsight instance not found" );
    }

    // Ensure existing project is closed
    m_application->closeProject();

    if ( filePath.empty() )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "Empty file path" );
    }
    if ( filePath == "MOCKED_TEST_GRID" )
    {
        // For testing, use mock model
        m_application->createMockModel();
    }
    else
    {
        // Load case from file name
        auto status = loadGridGeometryFromAbsoluteFilePath( filePath );
        if ( status.error_code() != grpc::StatusCode::OK ) return status;
    }

    RimProject* project = m_application->project();
    if ( project == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No project found" );
    }

    auto eclipseCases = project->eclipseCases();
    if ( eclipseCases.empty() || eclipseCases.front() == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No eclipse case found for project" );
    }
    m_eclipseCase = eclipseCases.front();

    if ( m_eclipseCase->views().empty() || m_eclipseCase->views().front() == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No eclipse view found for eclipse case" );
    }
    m_eclipseView = dynamic_cast<RimEclipseView*>( m_eclipseCase->views().front() );
    m_eclipseView->setShowInactiveCells( true );

    return grpc::Status::OK;
}

static bool RiaGrpcGridGeometryExtractionService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridGeometryExtractionService>(
        typeid( RiaGrpcGridGeometryExtractionService ).hash_code() );
