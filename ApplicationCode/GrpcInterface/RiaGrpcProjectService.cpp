#include "RiaGrpcProjectService.h"

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaSocketTools.h"

#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimGeoMechCase.h"
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
Status RiaGrpcProjectService::CurrentCase(ServerContext* context, const rips::Empty* request, rips::CaseRequest* reply)
{
    RimGridView* view = RiaApplication::instance()->activeGridView();
    if (view)
    {
        RimCase* currentCase = view->ownerCase();
        if (currentCase)
        {
            reply->set_id(currentCase->caseId());
            return Status::OK;
        }
    }
    return Status(grpc::NOT_FOUND, "No current case found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectService::CurrentCaseInfo(ServerContext* context, const rips::Empty* request, rips::CaseInfo* reply)
{
    RimGridView* view = RiaApplication::instance()->activeGridView();
    if (view)
    {
        RimCase* currentCase = view->ownerCase();
        if (currentCase)
        {
            qint64  caseId      = currentCase->caseId();
            qint64  caseGroupId = -1;
            QString caseName, caseType;
            RiaSocketTools::getCaseInfoFromCase(currentCase, caseId, caseName, caseType, caseGroupId);

            reply->set_id(caseId);
            reply->set_group_id(caseGroupId);
            reply->set_name(caseName.toStdString());
            reply->set_type(caseType.toStdString());
            return Status::OK;
        }
    }
    return Status(grpc::NOT_FOUND, "No current case found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcProjectService::CaseInfoFromCase(grpc::ServerContext* context, const rips::CaseRequest* request, rips::CaseInfo* reply)
{
    RimCase* rimCase = findCase(request->id());
    if (rimCase)
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        reply->set_id(caseId);
        reply->set_group_id(caseGroupId);
        reply->set_name(caseName.toStdString());
        reply->set_type(caseType.toStdString());
        return Status::OK;
    }
    return Status(grpc::NOT_FOUND, "No cases found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectService::SelectedCases(ServerContext* context, const rips::Empty* request, rips::CaseInfoArray* reply)
{
    std::vector<RimCase*> cases;
    caf::SelectionManager::instance()->objectsByType(&cases);

    if (cases.empty())
    {
        return Status(grpc::NOT_FOUND, "No cases selected");
    }

    for (RimCase* rimCase : cases)
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        rips::CaseInfo* caseInfo = reply->add_data();
        caseInfo->set_id(caseId);
        caseInfo->set_group_id(caseGroupId);
        caseInfo->set_name(caseName.toStdString());
        caseInfo->set_type(caseType.toStdString());
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcProjectService::AllCaseGroups(grpc::ServerContext* context, const rips::Empty* request, rips::CaseGroups* reply)
{
    RimProject*               proj = RiaApplication::instance()->project();
    RimEclipseCaseCollection* analysisModels =
        (proj && proj->activeOilField()) ? proj->activeOilField()->analysisModels() : nullptr;
    if (analysisModels)
    {
        for (RimIdenticalGridCaseGroup* cg : analysisModels->caseGroups())
        {
            rips::CaseGroup* caseGroup = reply->add_case_groups();
            caseGroup->set_id(cg->groupId());
            caseGroup->set_name(cg->name().toStdString());
        }
        return Status::OK;
    }
    return Status(grpc::NOT_FOUND, "No case groups found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcProjectService::AllCases(grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfoArray* reply)
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    if (cases.empty())
    {
        return Status(grpc::NOT_FOUND, "No cases found");
    }

    for (RimCase* rimCase : cases)
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        rips::CaseInfo* caseInfo = reply->add_data();
        caseInfo->set_id(caseId);
        caseInfo->set_group_id(caseGroupId);
        caseInfo->set_name(caseName.toStdString());
        caseInfo->set_type(caseType.toStdString());
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcProjectService::CasesInGroup(grpc::ServerContext* context, const rips::CaseGroup* request, rips::CaseInfoArray* reply)
{
    RimProject*               proj = RiaApplication::instance()->project();
    RimEclipseCaseCollection* analysisModels =
        (proj && proj->activeOilField()) ? proj->activeOilField()->analysisModels() : nullptr;
    if (analysisModels)
    {
        int                        groupId   = request->id();
        RimIdenticalGridCaseGroup* caseGroup = nullptr;

        for (size_t i = 0; i < analysisModels->caseGroups().size(); i++)
        {
            RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

            if (groupId == cg->groupId())
            {
                caseGroup = cg;
            }
        }

        std::vector<RimCase*> cases;
        if (caseGroup)
        {
            for (size_t i = 0; i < caseGroup->statisticsCaseCollection()->reservoirs.size(); i++)
            {
                cases.push_back(caseGroup->statisticsCaseCollection()->reservoirs[i]);
            }

            for (size_t i = 0; i < caseGroup->caseCollection()->reservoirs.size(); i++)
            {
                cases.push_back(caseGroup->caseCollection()->reservoirs[i]);
            }
        }
        if (!cases.empty())
        {
            for (RimCase* rimCase : cases)
            {
                qint64  caseId      = rimCase->caseId();
                qint64  caseGroupId = -1;
                QString caseName, caseType;
                RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

                rips::CaseInfo* caseInfo = reply->add_data();
                caseInfo->set_id(caseId);
                caseInfo->set_group_id(caseGroupId);
                caseInfo->set_name(caseName.toStdString());
                caseInfo->set_type(caseType.toStdString());
            }
        }
    }
    return Status(grpc::NOT_FOUND, "No cases found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcProjectService::createCallbacks()
{
    typedef RiaGrpcProjectService Self;

    return {
        new RiaGrpcUnaryCallback<Self, Empty, CaseRequest>(this, &Self::CurrentCase, &Self::RequestCurrentCase),
        new RiaGrpcUnaryCallback<Self, Empty, CaseInfo>(this, &Self::CurrentCaseInfo, &Self::RequestCurrentCaseInfo),
        new RiaGrpcUnaryCallback<Self, CaseRequest, CaseInfo>(this, &Self::CaseInfoFromCase, &Self::RequestCaseInfoFromCase),
        new RiaGrpcUnaryCallback<Self, Empty, CaseInfoArray>(this, &Self::SelectedCases, &Self::RequestSelectedCases),
        new RiaGrpcUnaryCallback<Self, Empty, CaseGroups>(this, &Self::AllCaseGroups, &Self::RequestAllCaseGroups),
        new RiaGrpcUnaryCallback<Self, Empty, CaseInfoArray>(this, &Self::AllCases, &Self::RequestAllCases),
        new RiaGrpcUnaryCallback<Self, CaseGroup, CaseInfoArray>(this, &Self::CasesInGroup, &Self::RequestCasesInGroup)};
}

static bool RiaGrpcProjectService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcProjectService>(typeid(RiaGrpcProjectService).hash_code());
