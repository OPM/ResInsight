#include "RiaGrpcProjectService.h"

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaSocketTools.h"

#include "GeoMech/RimGeoMechCase.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimGridView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "cafSelectionManager.h"

#include "Case.grpc.pb.h"

using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectService::GetCurrentCase( ServerContext* context, const rips::Empty* request, rips::CaseRequest* reply )
{
    int scriptCaseId = RiaApplication::instance()->currentScriptCaseId();
    if ( scriptCaseId != -1 )
    {
        reply->set_id( scriptCaseId );
        return Status::OK;
    }
    else
    {
        RimGridView* view = RiaApplication::instance()->activeGridView();
        if ( view )
        {
            RimCase* currentCase = view->ownerCase();
            if ( currentCase )
            {
                reply->set_id( currentCase->caseId() );
                return Status::OK;
            }
        }
    }

    return Status( grpc::NOT_FOUND, "No current case found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectService::GetSelectedCases( ServerContext* context, const rips::Empty* request, rips::CaseInfoArray* reply )
{
    std::vector<RimCase*> cases;
    caf::SelectionManager::instance()->objectsByType( &cases );

    if ( cases.empty() )
    {
        return Status( grpc::NOT_FOUND, "No cases selected" );
    }

    for ( RimCase* rimCase : cases )
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase( rimCase, caseId, caseName, caseType, caseGroupId );

        rips::CaseInfo* caseInfo = reply->add_data();
        caseInfo->set_id( caseId );
        caseInfo->set_group_id( caseGroupId );
        caseInfo->set_name( caseName.toStdString() );
        caseInfo->set_type( caseType.toStdString() );
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcProjectService::GetAllCaseGroups( grpc::ServerContext* context,
                                                      const rips::Empty*   request,
                                                      rips::CaseGroups*    reply )
{
    RimProject*               proj = RimProject::current();
    RimEclipseCaseCollection* analysisModels =
        ( proj && proj->activeOilField() ) ? proj->activeOilField()->analysisModels() : nullptr;
    if ( analysisModels )
    {
        for ( RimIdenticalGridCaseGroup* cg : analysisModels->caseGroups() )
        {
            rips::CaseGroup* caseGroup = reply->add_case_groups();
            caseGroup->set_id( cg->groupId() );
            caseGroup->set_name( cg->name().toStdString() );
        }
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "No case groups found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcProjectService::GetAllCases( grpc::ServerContext* context,
                                                 const rips::Empty*   request,
                                                 rips::CaseInfoArray* reply )
{
    std::vector<RimCase*> cases;
    RimProject::current()->allCases( cases );

    if ( cases.empty() )
    {
        return Status( grpc::NOT_FOUND, "No cases found" );
    }

    for ( RimCase* rimCase : cases )
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase( rimCase, caseId, caseName, caseType, caseGroupId );

        rips::CaseInfo* caseInfo = reply->add_data();
        caseInfo->set_id( caseId );
        caseInfo->set_group_id( caseGroupId );
        caseInfo->set_name( caseName.toStdString() );
        caseInfo->set_type( caseType.toStdString() );
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcProjectService::GetCasesInGroup( grpc::ServerContext*   context,
                                                     const rips::CaseGroup* request,
                                                     rips::CaseInfoArray*   reply )
{
    RimProject*               proj = RimProject::current();
    RimEclipseCaseCollection* analysisModels =
        ( proj && proj->activeOilField() ) ? proj->activeOilField()->analysisModels() : nullptr;
    if ( analysisModels )
    {
        int                        groupId   = request->id();
        RimIdenticalGridCaseGroup* caseGroup = nullptr;

        for ( size_t i = 0; i < analysisModels->caseGroups().size(); i++ )
        {
            RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

            if ( groupId == cg->groupId() )
            {
                caseGroup = cg;
            }
        }

        std::vector<RimCase*> cases;
        if ( caseGroup )
        {
            for ( size_t i = 0; i < caseGroup->statisticsCaseCollection()->reservoirs.size(); i++ )
            {
                cases.push_back( caseGroup->statisticsCaseCollection()->reservoirs[i] );
            }

            for ( size_t i = 0; i < caseGroup->caseCollection()->reservoirs.size(); i++ )
            {
                cases.push_back( caseGroup->caseCollection()->reservoirs[i] );
            }
        }
        if ( !cases.empty() )
        {
            for ( RimCase* rimCase : cases )
            {
                qint64  caseId      = rimCase->caseId();
                qint64  caseGroupId = -1;
                QString caseName, caseType;
                RiaSocketTools::getCaseInfoFromCase( rimCase, caseId, caseName, caseType, caseGroupId );

                rips::CaseInfo* caseInfo = reply->add_data();
                caseInfo->set_id( caseId );
                caseInfo->set_group_id( caseGroupId );
                caseInfo->set_name( caseName.toStdString() );
                caseInfo->set_type( caseType.toStdString() );
            }
        }
    }
    return Status( grpc::NOT_FOUND, "No cases found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcProjectService::GetPdmObject( grpc::ServerContext* context, const rips::Empty* request, rips::PdmObject* reply )
{
    RimProject* project = RimProject::current();
    if ( project )
    {
        copyPdmObjectFromCafToRips( project, reply );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcProjectService::createCallbacks()
{
    typedef RiaGrpcProjectService Self;

    return { new RiaGrpcUnaryCallback<Self, Empty, CaseRequest>( this, &Self::GetCurrentCase, &Self::RequestGetCurrentCase ),
             new RiaGrpcUnaryCallback<Self, Empty, CaseInfoArray>( this,
                                                                   &Self::GetSelectedCases,
                                                                   &Self::RequestGetSelectedCases ),
             new RiaGrpcUnaryCallback<Self, Empty, CaseGroups>( this, &Self::GetAllCaseGroups, &Self::RequestGetAllCaseGroups ),
             new RiaGrpcUnaryCallback<Self, Empty, CaseInfoArray>( this, &Self::GetAllCases, &Self::RequestGetAllCases ),
             new RiaGrpcUnaryCallback<Self, CaseGroup, CaseInfoArray>( this,
                                                                       &Self::GetCasesInGroup,
                                                                       &Self::RequestGetCasesInGroup ),
             new RiaGrpcUnaryCallback<Self, Empty, PdmObject>( this, &Self::GetPdmObject, &Self::RequestGetPdmObject ) };
}

static bool RiaGrpcProjectService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcProjectService>(
    typeid( RiaGrpcProjectService ).hash_code() );
