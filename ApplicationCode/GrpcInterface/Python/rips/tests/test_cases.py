import sys
import os
import pytest 

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_Launch(rips_instance, initialize_test):
    assert(rips_instance is not None)

def test_EmptyProject(rips_instance, initialize_test):
    cases = rips_instance.project.cases()
    assert(len(cases) is 0)

def test_OneCase(rips_instance, initialize_test):
    case = rips_instance.project.load_case(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    assert(case.name == "TEST10K_FLT_LGR_NNC")
    assert(case.id == 0)
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)

def test_MultipleCases(rips_instance, initialize_test):
    case_paths = []
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    
    case_names = []
    for case_path in case_paths:
        case_name = os.path.splitext(os.path.basename(case_path))[0]
        case_names.append(case_name)
        rips_instance.project.load_case(path=case_path)

    cases = rips_instance.project.cases()
    assert(len(cases) == len(case_names))
    for i, case_name in enumerate(case_names):
        assert(case_name == cases[i].name)

def test_10k(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert(case.grid_count() == 2)
    cell_count_info = case.cell_count()
    assert(cell_count_info.active_cell_count == 11125)
    assert(cell_count_info.reservoir_cell_count == 316224)
    time_steps = case.time_steps()
    assert(len(time_steps) == 9)
    days_since_start = case.days_since_start()
    assert(len(days_since_start) == 9)

def test_PdmObject(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert(case.id == 0)
    assert(case.address() is not 0)
    assert(case.class_keyword() == "EclipseCase")
    case_id = case.get_value('CaseId')
    assert(case_id == case.id)

@pytest.mark.skipif(sys.platform.startswith('linux'), reason="Brugge is currently exceptionally slow on Linux")
def test_brugge_0010(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert(case.grid_count() == 1)
    cellCountInfo = case.cell_count()
    assert(cellCountInfo.active_cell_count == 43374)
    assert(cellCountInfo.reservoir_cell_count == 60048)
    time_steps = case.time_steps()
    assert(len(time_steps) == 11)
    days_since_start = case.days_since_start()
    assert(len(days_since_start) == 11)

@pytest.mark.skipif(sys.platform.startswith('linux'), reason="Brugge is currently exceptionally slow on Linux")
def test_replaceCase(rips_instance, initialize_test):
    project = rips_instance.project.open(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp")
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = project.case(id=0)
    assert(case is not None)
    assert(case.name == "TEST10K_FLT_LGR_NNC")
    assert(case.id == 0)
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)

    rips_instance.commands.replace_case(new_grid_file=case_path, case_id=case.id)
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)
    case = project.case(id=0)
    assert(case.name == "Real0--BRUGGE_0000.EGRID")
    assert(case.id == 0)
