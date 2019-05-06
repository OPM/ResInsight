#include "RiaGrpcProjectInfoService.h"
#include "RiaGrpcServerCallData.h"

#include "CaseInfo.grpc.pb.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectInfoServiceImpl::GetCurrentCase(ServerContext* context, const ResInsight::Empty* request, ResInsight::Case* reply)
{
    reply->set_id(1);
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcServerCallMethod*> RiaGrpcProjectInfoServiceImpl::createCallbacks(ServerCompletionQueue* cq)
{
    std::vector<RiaGrpcServerCallMethod*> callbacks;
    callbacks.push_back(new RiaGrpcServerCallData<RiaGrpcProjectInfoServiceImpl, ResInsight::Empty, ResInsight::Case>(
        this,
        cq,
        "GetCurrentCase",
        &RiaGrpcProjectInfoServiceImpl::GetCurrentCase,
        &RiaGrpcProjectInfoServiceImpl::RequestGetCurrentCase));
    return callbacks;
}


