#include "RiaGrpcProjectInfoService.h"

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

#include "CaseInfo.grpc.pb.h"

using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcProjectInfoService::CurrentCase(ServerContext* context, const rips::Empty* request, rips::Case* reply)
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
Status RiaGrpcProjectInfoService::CurrentCaseInfo(ServerContext* context, const rips::Empty* request, rips::CaseInfo* reply)
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
    RiaGrpcProjectInfoService::CaseInfoFromCase(grpc::ServerContext* context, const rips::Case* request, rips::CaseInfo* reply)
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
Status RiaGrpcProjectInfoService::SelectedCases(ServerContext* context, const rips::Empty* request, rips::CaseInfos* reply)
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

        rips::CaseInfo* caseInfo = reply->add_case_info();
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
    RiaGrpcProjectInfoService::AllCaseGroups(grpc::ServerContext* context, const rips::Empty* request, rips::CaseGroups* reply)
{
    RimProject*               proj = RiaApplication::instance()->project();
    RimEclipseCaseCollection* analysisModels =
        (proj && proj->activeOilField()) ? proj->activeOilField()->analysisModels() : nullptr;
    if (analysisModels)
    {
        for (RimIdenticalGridCaseGroup* cg : analysisModels->caseGroups())
        {
            rips::CaseGroup* caseGroup = reply->add_case_group();
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
grpc::Status RiaGrpcProjectInfoService::AllCases(grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfos* reply)
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

        rips::CaseInfo* caseInfo = reply->add_case_info();
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
    RiaGrpcProjectInfoService::CasesInGroup(grpc::ServerContext* context, const rips::CaseGroup* request, rips::CaseInfos* reply)
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

                rips::CaseInfo* caseInfo = reply->add_case_info();
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
std::vector<RiaAbstractGrpcCallback*> RiaGrpcProjectInfoService::createCallbacks()
{
    typedef RiaGrpcProjectInfoService Self;

    return {
        new RiaGrpcCallback<Self, Empty, Case>(this, &Self::CurrentCase, &Self::RequestCurrentCase),
        new RiaGrpcCallback<Self, Empty, CaseInfo>(this, &Self::CurrentCaseInfo, &Self::RequestCurrentCaseInfo),
        new RiaGrpcCallback<Self, Case, CaseInfo>(this, &Self::CaseInfoFromCase, &Self::RequestCaseInfoFromCase),
        new RiaGrpcCallback<Self, Empty, CaseInfos>(this, &Self::SelectedCases, &Self::RequestSelectedCases),
        new RiaGrpcCallback<Self, Empty, CaseGroups>(this, &Self::AllCaseGroups, &Self::RequestAllCaseGroups),
        new RiaGrpcCallback<Self, Empty, CaseInfos>(this, &Self::AllCases, &Self::RequestAllCases),
        new RiaGrpcCallback<Self, CaseGroup, CaseInfos>(this, &Self::CasesInGroup, &Self::RequestCasesInGroup)};
}

static bool RiaGrpcProjectInfoService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcProjectInfoService>(typeid(RiaGrpcProjectInfoService).hash_code());
