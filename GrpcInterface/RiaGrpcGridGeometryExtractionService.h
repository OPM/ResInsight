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
#pragma once

#include "GridGeometryExtraction.grpc.pb.h"
#include "RiaApplication.h"
#include "RiaGrpcServiceInterface.h"
#include "RigGridBase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RivGridPartMgr.h"

#include "cvfArray.h"
#include "cvfStructGridGeometryGenerator.h"

#include <grpcpp/grpcpp.h>

namespace rips
{
class GetGridSurfaceRequest;
class GetGridSurfaceResponse;
class CutAlongPolylineRequest;
class CutAlongPolylineResponse;
} // namespace rips

class RiaGrpcCallbackInterface;

//==================================================================================================
//
// gRPC-service answering requests about grid geometry extraction
//
//==================================================================================================
class RiaGrpcGridGeometryExtractionService final : public rips::GridGeometryExtraction::AsyncService,
                                                   public RiaGrpcServiceInterface
{
public:
    RiaGrpcGridGeometryExtractionService();

    grpc::Status GetGridSurface( grpc::ServerContext*               context,
                                 const rips::GetGridSurfaceRequest* request,
                                 rips::GetGridSurfaceResponse*      response ) override;

    grpc::Status CutAlongPolyline( grpc::ServerContext*                 context,
                                   const rips::CutAlongPolylineRequest* request,
                                   rips::CutAlongPolylineResponse*      response ) override;

public:
    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;

private:
    void resetInternalPointers();
    void tearDownExistingViewsInEclipseCase();

    grpc::Status ensureViewInEclipseCase( RimEclipseCase& eclipseCase );
    grpc::Status loadGridGeometryFromAbsoluteFilePath( const std::string filePath );
    grpc::Status applyIJKCellFilterToEclipseView( const rips::IJKIndexFilter& filter, RimEclipseView* view );
    grpc::Status initializeApplicationAndEclipseCaseFromAbsoluteFilePath( const std::string filePath );

    grpc::Status initializeGridGeometryGeneratorWithEclipseViewCellVisibility( cvf::StructGridGeometryGenerator& generator,
                                                                               RimEclipseView* view );

    RiaApplication*                                  m_application          = nullptr;
    RimEclipseCase*                                  m_eclipseCase          = nullptr;
    std::unique_ptr<RigGridCellFaceVisibilityFilter> m_faceVisibilityFilter = nullptr;

    struct ElapsedTimeInfo
    {
        std::uint32_t                        totalTimeElapsedMs = 0; // Total time elapsed for entire request
        std::map<std::string, std::uint32_t> elapsedTimePerEventMs; // Time elapsed for each custom named event

        void reset()
        {
            totalTimeElapsedMs = 0;
            elapsedTimePerEventMs.clear();
        }
    };

    ElapsedTimeInfo m_elapsedTimeInfo;
};
