#include "RiaGrpcGridGeometryExtractionService.h"

// #include "VectorDefines.pb.h"

#include "qstring.h"

#include "Commands/RicImportGeneralDataFeature.h"
#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"
#include "RifReaderSettings.h"
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
#include "RivGridPartMgr.h"
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

    // Initialize pointer
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

    // Retrieve grid surface vertices from eclipse view
    status = initializeMainGridPartManagerFromEclipseView();
    if ( status.error_code() != grpc::StatusCode::OK )
    {
        return status;
    }

    auto* gridSurfaceVertices = m_mainGridPartManager->getOrCreateSurfaceVertices();
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
    const auto sourceCellIndicesArray = m_mainGridPartManager->getSurfaceQuadToCellIndicesArray();
    if ( sourceCellIndicesArray.empty() )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No source cell indices array found" );
    }
    for ( const auto& sourceCellIndex : sourceCellIndicesArray )
    {
        response->add_sourcecellindicesarr( static_cast<google::protobuf::uint32>( sourceCellIndex ) );
    }

    // Set grid dimensions
    const auto            countI         = m_eclipseView->mainGrid()->cellCountI();
    const auto            countJ         = m_eclipseView->mainGrid()->cellCountJ();
    const auto            countK         = m_eclipseView->mainGrid()->cellCountK();
    rips::Vec3i*          dimensions     = new rips::Vec3i;
    rips::GridDimensions* gridDimensions = new rips::GridDimensions;
    dimensions->set_i( countI );
    dimensions->set_j( countJ );
    dimensions->set_k( countK );
    gridDimensions->set_allocated_dimensions( dimensions );
    response->set_allocated_griddimensions( gridDimensions );

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
    return grpc::Status( grpc::StatusCode::UNIMPLEMENTED, "Not implemented" );
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
    m_application         = nullptr;
    m_eclipseCase         = nullptr;
    m_eclipseView         = nullptr;
    m_mainGridPartManager = nullptr;
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
grpc::Status RiaGrpcGridGeometryExtractionService::initializeMainGridPartManagerFromEclipseView()
{
    if ( m_eclipseView == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No initialized eclipse view found" );
    }

    auto* viewPartManager = m_eclipseView->reservoirGridPartManager();
    if ( viewPartManager == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No view part manager found for eclipse view" );
    }

    // Ensure static geometry parts created prior to getting part manager
    viewPartManager->ensureStaticGeometryPartsCreated( RivCellSetEnum::RANGE_FILTERED );

    const size_t timeStepIndex            = 0;
    auto         rangeFilteredPartManager = viewPartManager->rangeFilteredReservoirPartManager( timeStepIndex );

    if ( rangeFilteredPartManager == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No range filtered part manager found for eclipse view" );
    }

    auto* mainGridPartManager = rangeFilteredPartManager->mainGridPartManager();
    if ( mainGridPartManager == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND,
                             "No main grid part manager found for range filtered part manager" );
    }
    m_mainGridPartManager = mainGridPartManager;

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
        // For empty grid file name, use mock model
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

    return grpc::Status::OK;
}

static bool RiaGrpcGridGeometryExtractionService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridGeometryExtractionService>(
        typeid( RiaGrpcGridGeometryExtractionService ).hash_code() );
