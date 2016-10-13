from ert.analysis.analysis_module import AnalysisModule
from ert.analysis.enums.analysis_module_options_enum import AnalysisModuleOptionsEnum
from ert.enkf import RealizationStateEnum, EnkfVarType
from ert.job_queue import WorkflowRunner
from ert.util import BoolVector, StringList
from ert_gui import ERT
from ert_gui.ertwidgets import showWaitCursorWhileWaiting


def getRealizationCount():
    return ERT.ert.getEnsembleSize()


def getAllCases():
    """ @rtype: list[str] """
    fs = ERT.ert.getEnkfFsManager().getCurrentFileSystem()
    case_list = ERT.ert.getEnkfFsManager().getCaseList()
    return [str(case) for case in case_list if not ERT.ert.getEnkfFsManager().isCaseHidden(case)]


def caseExists(case_name):
    """ @rtype: bool """
    return str(case_name) in getAllCases()


def caseIsInitialized(case_name):
    """ @rtype: bool """
    return ERT.ert.getEnkfFsManager().isCaseInitialized(case_name)


def getAllInitializedCases():
    """ @rtype: list[str] """
    return [case for case in getAllCases() if caseIsInitialized(case)]


def getCurrentCaseName():
    """ @rtype: str """
    return str(ERT.ert.getEnkfFsManager().getCurrentFileSystem().getCaseName())


def getHistoryLength():
    """ @rtype: int """
    return ERT.ert.getHistoryLength()


@showWaitCursorWhileWaiting
def selectOrCreateNewCase(case_name):
    if getCurrentCaseName() != case_name:
        fs = ERT.ert.getEnkfFsManager().getFileSystem(case_name)
        ERT.ert.getEnkfFsManager().switchFileSystem(fs)
        ERT.emitErtChange()


def caseHasDataAndIsNotRunning(case):
    """ @rtype: bool """
    case_has_data = False
    state_map = ERT.ert.getEnkfFsManager().getStateMapForCase(case)

    for state in state_map:
        if state == RealizationStateEnum.STATE_HAS_DATA:
            case_has_data = True
            break

    return case_has_data and not caseIsRunning(case)


def getAllCasesWithDataAndNotRunning():
    """ @rtype: list[str] """
    return [case for case in getAllCases() if caseHasDataAndIsNotRunning(case)]


def caseIsRunning(case):
    """ @rtype: bool """
    return ERT.ert.getEnkfFsManager().isCaseRunning(case)


def getAllCasesNotRunning():
    """ @rtype: list[str] """
    return [case for case in getAllCases() if not caseIsRunning(case)]


def getCaseRealizationStates(case_name):
    """ @rtype: list[ert.enkf.enums.RealizationStateEnum] """
    state_map = ERT.ert.getEnkfFsManager().getStateMapForCase(case_name)
    return [state for state in state_map]


@showWaitCursorWhileWaiting
def initializeCurrentCaseFromScratch(parameters, members):
    selected_parameters = StringList(parameters)
    for member in members:
        member = int(member.strip())
        ERT.ert.getEnkfFsManager().initializeFromScratch(selected_parameters, member, member)

    ERT.emitErtChange()


@showWaitCursorWhileWaiting
def initializeCurrentCaseFromExisting(source_case, target_case, source_report_step, parameters, members):
    if caseExists(source_case) and caseIsInitialized(source_case) and caseExists(target_case):
        total_member_count = getRealizationCount()

        member_mask = BoolVector.createFromList(total_member_count, members)
        selected_parameters = StringList(parameters)

        ERT.ert.getEnkfFsManager().customInitializeCurrentFromExistingCase(source_case, source_report_step, member_mask,
                                                                           selected_parameters)

        ERT.emitErtChange()


def getParameterList():
    """ @rtype: list[str] """
    return [str(p) for p in ERT.ert.ensembleConfig().getKeylistFromVarType(EnkfVarType.PARAMETER)]


def getRunPath():
    """ @rtype: str """
    return ERT.ert.getModelConfig().getRunpathAsString()


def getNumberOfIterations():
    """ @rtype: int """
    return ERT.ert.analysisConfig().getAnalysisIterConfig().getNumIterations()


def setNumberOfIterations(iteration_count):
    """ @type iteration_count: int """
    if iteration_count != getNumberOfIterations():
        ERT.ert.analysisConfig().getAnalysisIterConfig().setNumIterations(iteration_count)
        ERT.emitErtChange()


def getWorkflowNames():
    """ @rtype: list[str] """
    return sorted(ERT.ert.getWorkflowList().getWorkflowNames(), key=str.lower)


def createWorkflowRunner(workflow_name):
    """ @rtype: WorkflowRunner """
    workflow_list = ERT.ert.getWorkflowList()

    workflow = workflow_list[workflow_name]
    context = workflow_list.getContext()
    return WorkflowRunner(workflow, ERT.ert, context)


def getAnalysisModules(iterable=False):
    """ @rtype: list[ert.analysis.AnalysisModule]"""
    module_names = ERT.ert.analysisConfig().getModuleList()

    modules = []
    for module_name in module_names:
        module = ERT.ert.analysisConfig().getModule(module_name)
        module_is_iterable = module.checkOption(AnalysisModuleOptionsEnum.ANALYSIS_ITERABLE)

        if iterable == module_is_iterable:
            modules.append(module)

    return sorted(modules, key=AnalysisModule.getName)

def getAnalysisModuleNames(iterable=False):
    """ @rtype: list[str] """
    modules = getAnalysisModules(iterable)
    return [module.getName() for module in modules]


def getCurrentAnalysisModuleName():
    """ @rtype: str """
    return ERT.ert.analysisConfig().activeModuleName()


