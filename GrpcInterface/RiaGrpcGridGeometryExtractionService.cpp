#include "RiaGrpcGridGeometryExtractionService.h"

#include "qstring.h"

#include "cvfStructGridGeometryGenerator.h"

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"

#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "cvfArray.h"
#include "cvfDrawableGeo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridGeometryExtractionService::GetGridSurface( grpc::ServerContext*               context,
                                                                   const rips::GetGridSurfaceRequest* request,
                                                                   rips::GetGridSurfaceResponse*      response )
{
    // Get resinsight instance and open grid
    RiaApplication* applicationInstance = RiaApplication::instance();

    if ( applicationInstance == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "ResInsight instance not found" );
    }

    applicationInstance->createMockModel();

    RimProject* project = applicationInstance->project();
    if ( project == nullptr )
    {
        return grpc::Status( grpc::StatusCode::NOT_FOUND, "No project found" );
    }

    // 1)
    {
        // 1. RimEclipseView
        // 2. RivReservoirViewPartMgr using reservoirGridPartManager()
        // 3. reservoirPartManager(RivCellSetEnum::RANGE_FILTERED, timeStepIndex); , timeStepIndex = 0  (make method
        // public?)
        // 4. ... ?
    }

    // 2)
    {
        // Temporary code using mainGrid
        auto eclipseCases = project->eclipseCases();
        if ( eclipseCases.empty() )
        {
            return grpc::Status( grpc::StatusCode::NOT_FOUND, "No grid cases found" );
        }
        auto* eclipseCase = eclipseCases.front();
        auto* mainGrid    = eclipseCase->eclipseCaseData()->mainGrid();

        // Get RigFemPartGrid object ?
        cvf::StructGridGeometryGenerator gridGeometryGenerator( mainGrid, false );

        cvf::UByteArray* cellVisibilities = new cvf::UByteArray( mainGrid->cellCount() );
        cellVisibilities->setAll( 1 );
        auto* faceVisibilityFilter = new RigGridCellFaceVisibilityFilter( mainGrid );

        gridGeometryGenerator.setCellVisibility( cellVisibilities );
        gridGeometryGenerator.addFaceVisibilityFilter( faceVisibilityFilter );

        auto gridSurfaceVertices = gridGeometryGenerator.getOrCreateVertices();
        if ( gridSurfaceVertices.p() == nullptr )
        {
            return grpc::Status( grpc::StatusCode::NOT_FOUND, "No grid vertices found" );
        }

        // Set vertex_array and quadindicesarr response
        const auto* verticesArray = gridSurfaceVertices.p();
        for ( int i = 0; i < verticesArray->size(); ++i )
        {
            const auto& vertex = verticesArray->get( i );
            response->add_vertexarray( vertex.x() );
            response->add_vertexarray( vertex.y() );
            response->add_vertexarray( vertex.z() );

            response->add_quadindicesarr( i );
        }
    }

    // TODO: Add:
    // - sourceCellIndicesArr
    // - gridDimensions
    // - originUtm

    applicationInstance->closeProject();

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

static bool RiaGrpcGridGeometryExtractionService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridGeometryExtractionService>(
        typeid( RiaGrpcGridGeometryExtractionService ).hash_code() );
