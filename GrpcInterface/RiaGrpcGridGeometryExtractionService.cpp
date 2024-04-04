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
#include "RigNNCData.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
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
class ElapsedTimeCount
{
public:
    ElapsedTimeCount()
        : m_start( std::chrono::high_resolution_clock::now() ){};
    ~ElapsedTimeCount() = default;

    long long elapsedMsCount() const
    {
        const auto end     = std::chrono::high_resolution_clock::now();
        auto       elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - m_start );
        return elapsed.count();
    }

private:
    const std::chrono::high_resolution_clock::time_point m_start;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcGridGeometryExtractionService::RiaGrpcGridGeometryExtractionService()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::GetGridSurface( grpc::ServerContext*               context,
                                                                   const rips::GetGridSurfaceRequest* request,
                                                                   rips::GetGridSurfaceResponse*      response )
{
    m_elapsedTimeInfo.reset();
    auto totalTimeCount = ElapsedTimeCount();

    // Initialize application and eclipse case
    grpc::Status status = initializeApplicationAndEclipseCaseFromAbsoluteFilePath( request->gridfilename() );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    // Ensure view in eclipse case
    status = ensureViewInEclipseCase( *m_eclipseCase );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }
    auto* eclipseView = dynamic_cast<RimEclipseView*>( m_eclipseCase->views().front() );
    if ( eclipseView == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No eclipse view found" );
    }
    eclipseView->setShowInactiveCells( true );
    eclipseView->faultCollection()->setActive( false ); // TODO: Check if this is correct??

    // Apply ijk-filtering - assuming 0-indexing from gRPC
    if ( request->has_ijkindexfilter() )
    {
        status = applyIJKCellFilterToEclipseView( request->ijkindexfilter(), eclipseView );
        if ( status.error_code() != grpc::StatusCode::OK )
        {
            return status;
        }
    }

    // Configure eclipse view
    // - Ensure static geometry parts created for grid part manager in view
    // - To include inactive cells and activate requested filter for view
    auto createGridGeometryPartsTimeCount = ElapsedTimeCount();
    eclipseView->createGridGeometryParts();
    m_elapsedTimeInfo.elapsedTimePerEventMs["CreateGridGeometryParts"] =
        static_cast<std::uint32_t>( createGridGeometryPartsTimeCount.elapsedMsCount() );

    // Set visibility
    const bool useOpenMP                    = false;
    auto       surfaceGridGeometryGenerator = cvf::StructGridGeometryGenerator( eclipseView->mainGrid(), useOpenMP );
    auto       faultGridGeometryGenerator   = cvf::StructGridGeometryGenerator( eclipseView->mainGrid(), useOpenMP );
    status = initializeGridGeometryGeneratorWithEclipseViewCellVisibility( surfaceGridGeometryGenerator,
                                                                           faultGridGeometryGenerator,
                                                                           eclipseView );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    // Create grid surface vertices
    auto  createVerticesTimeCount = ElapsedTimeCount();
    auto* gridSurfaceVertices     = surfaceGridGeometryGenerator.getOrCreateVertices();
    if ( gridSurfaceVertices == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No grid surface vertices found" );
    }
    m_elapsedTimeInfo.elapsedTimePerEventMs["CreateGridSurfaceVertices"] =
        static_cast<std::uint32_t>( createVerticesTimeCount.elapsedMsCount() );

    // Create grid fault vertices
    auto  createFaultVerticesTimeCount = ElapsedTimeCount();
    auto* gridFaultVertices            = faultGridGeometryGenerator.getOrCreateVertices();
    if ( gridFaultVertices == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No grid fault vertices found" );
    }

    m_elapsedTimeInfo.elapsedTimePerEventMs["CreateGridFaultVertices"] =
        static_cast<std::uint32_t>( createFaultVerticesTimeCount.elapsedMsCount() );

    // Retrieve the UTM offset
    const auto mainGridModelOffset = m_eclipseCase->mainGrid()->displayModelOffset();

    // Vertex welding
    // - Low welding distance, as the goal is to weld duplicate vertices
    // - Welding is done for surface and fault vertices separately
    const double weldingDistance       = 1e-3;
    const double weldingCellSize       = 4.0 * weldingDistance;
    auto         weldVerticesTimeCount = ElapsedTimeCount();

    // Weld surface vertices
    cvf::VertexWelder surfaceVertexWelder;
    const cvf::uint   numSurfaceWelderBuckets = static_cast<cvf::uint>( gridSurfaceVertices->size() );
    surfaceVertexWelder.initialize( weldingDistance, weldingCellSize, numSurfaceWelderBuckets );
    const auto& surfaceVertexIndices = weldVertices( surfaceVertexWelder, *gridSurfaceVertices );

    // Weld fault vertices
    cvf::VertexWelder faultVertexWelder;
    const cvf::uint   numFaultWelderBuckets = static_cast<cvf::uint>( gridFaultVertices->size() );
    faultVertexWelder.initialize( weldingDistance, weldingCellSize, numFaultWelderBuckets );
    const auto& faultVertexIndices = weldVertices( faultVertexWelder, *gridFaultVertices );
    m_elapsedTimeInfo.elapsedTimePerEventMs["WeldVertices"] =
        static_cast<std::uint32_t>( weldVerticesTimeCount.elapsedMsCount() );

    // Fill response
    auto       fillResponseTimeCount = ElapsedTimeCount();
    const auto zAxisOffset           = mainGridModelOffset.z();
    for ( cvf::uint i = 0; i < surfaceVertexWelder.vertexCount(); ++i )
    {
        const auto& vertex = surfaceVertexWelder.vertex( i );
        response->add_vertexarray( vertex.x() );
        response->add_vertexarray( vertex.y() );
        response->add_vertexarray( vertex.z() + zAxisOffset );
    }
    for ( const auto& vertexIndex : surfaceVertexIndices )
    {
        response->add_quadindicesarr( static_cast<google::protobuf::uint32>( vertexIndex ) );
    }

    const cvf::uint vertexIndexOffset = static_cast<cvf::uint>( surfaceVertexWelder.vertexCount() );
    for ( cvf::uint i = 0; i < faultVertexWelder.vertexCount(); ++i )
    {
        const auto& vertex = faultVertexWelder.vertex( i );
        response->add_vertexarray( vertex.x() );
        response->add_vertexarray( vertex.y() );
        response->add_vertexarray( vertex.z() + zAxisOffset );
    }
    for ( const auto& vertexIndex : faultVertexIndices )
    {
        response->add_quadindicesarr( static_cast<google::protobuf::uint32>( vertexIndex + vertexIndexOffset ) );
    }

    // Origin is the UTM offset
    rips::Vec2d* originUtmXy = new rips::Vec2d;
    originUtmXy->set_x( mainGridModelOffset.x() );
    originUtmXy->set_y( mainGridModelOffset.y() );
    response->set_allocated_originutmxy( originUtmXy );

    // Source cell indices from main grid part manager
    std::vector<size_t> sourceCellIndicesArray = std::vector<size_t>();
    if ( surfaceGridGeometryGenerator.quadToCellFaceMapper() != nullptr )
    {
        for ( const auto& elm : surfaceGridGeometryGenerator.quadToCellFaceMapper()->quadToCellIndicesArray() )
        {
            sourceCellIndicesArray.push_back( elm );
        }
    }
    if ( faultGridGeometryGenerator.quadToCellFaceMapper() != nullptr )
    {
        for ( const auto& elm : faultGridGeometryGenerator.quadToCellFaceMapper()->quadToCellIndicesArray() )
        {
            sourceCellIndicesArray.push_back( elm );
        }
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
    rips::Vec3i* dimensions = new rips::Vec3i;
    dimensions->set_i( m_eclipseCase->mainGrid()->cellCountI() );
    dimensions->set_j( m_eclipseCase->mainGrid()->cellCountJ() );
    dimensions->set_k( m_eclipseCase->mainGrid()->cellCountK() );
    response->set_allocated_griddimensions( dimensions );

    m_elapsedTimeInfo.elapsedTimePerEventMs["FillResponse"] =
        static_cast<std::uint32_t>( fillResponseTimeCount.elapsedMsCount() );

    // Clear existing view
    tearDownExistingViewsInEclipseCase();
    eclipseView = nullptr;
    delete eclipseView;

    // Fill elapsed time info
    m_elapsedTimeInfo.totalTimeElapsedMs = static_cast<std::uint32_t>( totalTimeCount.elapsedMsCount() );

    // Add elapsed time info to response
    rips::TimeElapsedInfo* elapsedTimeInfo = new rips::TimeElapsedInfo;
    elapsedTimeInfo->set_totaltimeelapsedms( m_elapsedTimeInfo.totalTimeElapsedMs );
    for ( const auto& event : m_elapsedTimeInfo.elapsedTimePerEventMs )
    {
        const auto& message                                                  = event.first;
        const auto& timeElapsed                                              = event.second;
        ( *elapsedTimeInfo->mutable_namedeventsandtimeelapsedms() )[message] = timeElapsed;
    }
    response->set_allocated_timeelapsedinfo( elapsedTimeInfo );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::CutAlongPolyline( grpc::ServerContext*                 context,
                                                                     const rips::CutAlongPolylineRequest* request,
                                                                     rips::CutAlongPolylineResponse*      response )
{
    m_elapsedTimeInfo.reset();
    auto totalTimeCount = ElapsedTimeCount();

    // Initialize pointers
    grpc::Status status = initializeApplicationAndEclipseCaseFromAbsoluteFilePath( request->gridfilename() );
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    // Validate requested polyline
    auto& fencePolyline = request->fencepolylineutmxy();
    if ( fencePolyline.size() < 2 )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "Invalid fence polyline - require two or more points" );
    }

    // Convert polyline to vector of cvf::Vec3d
    std::vector<cvf::Vec2d> polylineUtmXy;
    for ( int i = 0; i < fencePolyline.size(); i += 2 )
    {
        const double xValue = fencePolyline.Get( i );
        const double yValue = fencePolyline.Get( i + 1 );
        polylineUtmXy.push_back( cvf::Vec2d( xValue, yValue ) );
    }

    RigActiveCellInfo* activeCellInfo    = nullptr; // No active cell info for grid
    const bool         showInactiveCells = true;
    auto               eclipseIntersectionGrid =
        RivEclipseIntersectionGrid( m_eclipseCase->mainGrid(), activeCellInfo, showInactiveCells );

    auto polylineIntersectionGenerator =
        RivPolylineIntersectionGeometryGenerator( polylineUtmXy, &eclipseIntersectionGrid );

    // Use all grid cells for intersection geometry
    cvf::UByteArray visibleCells = cvf::UByteArray( m_eclipseCase->mainGrid()->cellCount() );
    visibleCells.setAll( 1 );

    // Generate intersection
    auto generateIntersectionTimeCount = ElapsedTimeCount();
    polylineIntersectionGenerator.generateIntersectionGeometry( &visibleCells );
    if ( !polylineIntersectionGenerator.isAnyGeometryPresent() )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "No intersection geometry present" );
    }
    m_elapsedTimeInfo.elapsedTimePerEventMs["GenerateIntersection"] =
        static_cast<std::uint32_t>( generateIntersectionTimeCount.elapsedMsCount() );

    // Add fence mesh sections to response
    auto        fillResponseTimeCount    = ElapsedTimeCount();
    const auto& polylineSegmentsMeshData = polylineIntersectionGenerator.polylineSegmentsMeshData();
    for ( const auto& segment : polylineSegmentsMeshData )
    {
        auto* fenceMeshSection = response->add_fencemeshsections();

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

    // Add grid dimensions
    rips::Vec3i* dimensions = new rips::Vec3i;
    dimensions->set_i( m_eclipseCase->mainGrid()->cellCountI() );
    dimensions->set_j( m_eclipseCase->mainGrid()->cellCountJ() );
    dimensions->set_k( m_eclipseCase->mainGrid()->cellCountK() );
    response->set_allocated_griddimensions( dimensions );

    m_elapsedTimeInfo.elapsedTimePerEventMs["FillResponse"] =
        static_cast<std::uint32_t>( fillResponseTimeCount.elapsedMsCount() );

    // Clear existing view
    tearDownExistingViewsInEclipseCase();

    // Fill elapsed time info
    m_elapsedTimeInfo.totalTimeElapsedMs = static_cast<std::uint32_t>( totalTimeCount.elapsedMsCount() );

    // Add elapsed time info to response
    rips::TimeElapsedInfo* elapsedTimeInfo = new rips::TimeElapsedInfo;
    elapsedTimeInfo->set_totaltimeelapsedms( m_elapsedTimeInfo.totalTimeElapsedMs );
    for ( const auto& event : m_elapsedTimeInfo.elapsedTimePerEventMs )
    {
        const auto& message                                                  = event.first;
        const auto& timeElapsed                                              = event.second;
        ( *elapsedTimeInfo->mutable_namedeventsandtimeelapsedms() )[message] = timeElapsed;
    }
    response->set_allocated_timeelapsedinfo( elapsedTimeInfo );

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
/// Weld vertices and return array of indices of welded vertices
//--------------------------------------------------------------------------------------------------
std::vector<cvf::uint> RiaGrpcGridGeometryExtractionService::weldVertices( cvf::VertexWelder&     rWelder,
                                                                           const cvf::Vec3fArray& vertices )
{
    std::vector<cvf::uint> vertexIndices;
    for ( const auto& vertex : vertices )
    {
        bool       wasWelded   = false;
        const auto welderIndex = rWelder.weldVertex( vertex, &wasWelded );
        vertexIndices.push_back( welderIndex );
    }
    return vertexIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGridGeometryExtractionService::tearDownExistingViewsInEclipseCase()
{
    if ( m_eclipseCase == nullptr )
    {
        return;
    }

    std::vector<RimEclipseView*> eclipseViews = m_eclipseCase->reservoirViews.childrenByType();
    for ( const auto& view : eclipseViews )
    {
        m_eclipseCase->reservoirViews.removeChild( view );
        delete view;
    }
    m_eclipseCase->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::ensureViewInEclipseCase( RimEclipseCase& eclipseCase )
{
    // Ensure maximum one reservoir view for case
    if ( eclipseCase.views().size() > 1 )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "More than one view found for eclipse case" );
    }

    if ( eclipseCase.views().empty() )
    {
        auto createAndAddViewTimeCount = ElapsedTimeCount();
        eclipseCase.createAndAddReservoirView();
        m_elapsedTimeInfo.elapsedTimePerEventMs["CreateAndAddView"] =
            static_cast<std::uint32_t>( createAndAddViewTimeCount.elapsedMsCount() );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcGridGeometryExtractionService::resetInternalPointers()
{
    m_application = nullptr;
    m_eclipseCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Apply ijk-filtering - assuming 0-indexing from gRPC
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::applyIJKCellFilterToEclipseView( const rips::IJKIndexFilter& filter,
                                                                                    RimEclipseView*             view )
{
    if ( view == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "Uninitialized eclipse view provided" );
    }

    // Find the selected Cell Filter Collection
    RimCellFilterCollection* filtColl = view->cellFilterCollection();
    if ( filtColl == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No cell filter collection found for eclipse view" );
    }

    // Clear potentially existing filter
    const auto existingFilters = filtColl->filters();
    for ( const auto& filter : existingFilters )
    {
        filtColl->removeFilter( filter );
    }

    // Add range filter object
    const int sliceDirection = -1;
    const int gridIndex      = 0;
    RimCellRangeFilter* cellRangeFilter = filtColl->addNewCellRangeFilter( view->eclipseCase(), gridIndex, sliceDirection );

    // Eclipse indexing, first index is 1
    // Dimension filter is disabled if both min and max is -1 for respective dimension. I.e. all cells for
    // respective dimension are included in the filter.
    const bool isIDimensionFilterDisabled = filter.imin() == -1 && filter.imax() == -1;
    int        startIndexI                = filter.imin() + 1;
    int        cellCountI                 = filter.imax() - filter.imin() + 1;
    if ( isIDimensionFilterDisabled )
    {
        startIndexI = 1;
        cellCountI  = view->mainGrid()->cellCountI();
    }

    const bool isJDimensionFilterDisabled = filter.jmin() == -1 && filter.jmax() == -1;
    int        startIndexJ                = filter.jmin() + 1;
    int        cellCountJ                 = filter.jmax() - filter.jmin() + 1;
    if ( isJDimensionFilterDisabled )
    {
        startIndexJ = 1;
        cellCountJ  = view->mainGrid()->cellCountJ();
    }

    const bool isKDimensionFilterDisabled = filter.kmin() == -1 && filter.kmax() == -1;
    int        startIndexK                = filter.kmin() + 1;
    int        cellCountK                 = filter.kmax() - filter.kmin() + 1;
    if ( isKDimensionFilterDisabled )
    {
        startIndexK = 1;
        cellCountK  = view->mainGrid()->cellCountK();
    }

    // Apply ijk-filter values to range filter object
    cellRangeFilter->startIndexI = startIndexI;
    cellRangeFilter->startIndexJ = startIndexJ;
    cellRangeFilter->startIndexK = startIndexK;
    cellRangeFilter->cellCountI  = cellCountI;
    cellRangeFilter->cellCountJ  = cellCountJ;
    cellRangeFilter->cellCountK  = cellCountK;

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::initializeGridGeometryGeneratorWithEclipseViewCellVisibility(
    cvf::StructGridGeometryGenerator& surfaceGeometryGenerator,
    cvf::StructGridGeometryGenerator& faultGeometryGenerator,
    RimEclipseView*                   view )
{
    if ( view == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "Uninitialized eclipse view provided" );
    }

    auto* mainGrid = view->mainGrid();
    if ( mainGrid == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No main grid found for eclipse view" );
    }

    // Cell visibilities
    const int firstTimeStep    = 0;
    auto*     cellVisibilities = new cvf::UByteArray( mainGrid->cellCount() );
    view->calculateCurrentTotalCellVisibility( cellVisibilities, firstTimeStep ); // TODO: Check if this is correct way
                                                                                  // to get cell visibilities

    // Face visibility filter
    m_surfaceFaceVisibilityFilter =
        std::make_unique<RigGridCellFaceVisibilityFilter>( RigGridCellFaceVisibilityFilter( mainGrid ) );
    surfaceGeometryGenerator.setCellVisibility( cellVisibilities ); // Ownership transferred
    surfaceGeometryGenerator.addFaceVisibilityFilter( m_surfaceFaceVisibilityFilter.get() );

    m_faultFaceVisibilityFilter =
        std::make_unique<RigGridCellFaultFaceVisibilityFilter>( RigGridCellFaultFaceVisibilityFilter( mainGrid ) );
    faultGeometryGenerator.setCellVisibility( cellVisibilities ); // Ownership transferred
    faultGeometryGenerator.addFaceVisibilityFilter( m_faultFaceVisibilityFilter.get() );

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
    readerSettings               = RifReaderSettings::createGridOnlyReaderSettings();
    readerSettings->importFaults = true;

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
grpc::Status
    RiaGrpcGridGeometryExtractionService::initializeApplicationAndEclipseCaseFromAbsoluteFilePath( const std::string filePath )
{
    if ( filePath.empty() )
    {
        return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "Empty file path" );
    }

    // Get ResInsight instance and open grid
    m_application = RiaApplication::instance();
    if ( m_application == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "ResInsight instance not found" );
    }

    // Open new grid if file name is different from file name of existing case
    if ( m_eclipseCase == nullptr || m_eclipseCase->gridFileName() != QString::fromStdString( filePath ) )
    {
        // Close potential existing projects
        m_application->closeProject();

        if ( filePath == "MOCKED_TEST_GRID" )
        {
            // For testing, use mock model
            m_application->createMockModel();
        }
        else
        {
            // Load case from file name
            auto loadCaseFromFileNameTimeCount = ElapsedTimeCount();
            auto status                        = loadGridGeometryFromAbsoluteFilePath( filePath );
            m_elapsedTimeInfo.elapsedTimePerEventMs["LoadGridFromFilePath"] =
                static_cast<std::uint32_t>( loadCaseFromFileNameTimeCount.elapsedMsCount() );
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
        if ( eclipseCases.size() > 1 )
        {
            return grpc::Status( grpc::StatusCode::INVALID_ARGUMENT, "More than one grid case found for project" );
        }
        m_eclipseCase = eclipseCases.front();

        // Set nncData for main grid
        // TODO: Do not perform if calling intersection endpoint only
        if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() &&
             m_eclipseCase->eclipseCaseData()->mainGrid()->nncData() )
        {
            auto       createNncDataForGrid = ElapsedTimeCount();
            const bool includeInactiveCells = true;
            m_eclipseCase->eclipseCaseData()
                ->mainGrid()
                ->nncData()
                ->setSourceDataForProcessing( m_eclipseCase->eclipseCaseData()->mainGrid(),
                                              m_eclipseCase->eclipseCaseData()->activeCellInfo(
                                                  RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                              includeInactiveCells );
            m_elapsedTimeInfo.elapsedTimePerEventMs["CreateNncDataForGrid"] =
                static_cast<std::uint32_t>( createNncDataForGrid.elapsedMsCount() );
        }
    }

    if ( m_eclipseCase == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No eclipse case found" );
    }

    return grpc::Status::OK;
}

static bool RiaGrpcGridGeometryExtractionService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridGeometryExtractionService>(
        typeid( RiaGrpcGridGeometryExtractionService ).hash_code() );
