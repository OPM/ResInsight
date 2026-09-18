import sys
import os
import pytest
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import rips
import dataroot


def test_create_lgr_well(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert len(case.grids()) == 2

    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)

    time_step = 5
    well_path_names = ["Well Path A"]
    case.create_lgr_for_completion(
        time_step,
        well_path_names,
        refinement_i=2,
        refinement_j=3,
        refinement_k=1,
        split_type="LGR_PER_WELL",
    )
    assert len(case.grids()) == 3


def test_create_and_export_lgr_for_completions(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert len(case.grids()) == 2

    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)
    well_path = rips_instance.project.well_path_by_name("Well Path A")
    assert well_path is not None

    case.create_lgr_for_completions(
        well_paths=[well_path],
        time_step=5,
        refinement_i=2,
        refinement_j=3,
        refinement_k=1,
        split_type=rips.LgrSplitType.LGR_PER_WELL,
    )
    assert len(case.grids()) == 3

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        case.export_lgr_for_completions(
            well_paths=[well_path],
            time_step=5,
            export_folder=tmpdirname,
            refinement_i=2,
            refinement_j=2,
            refinement_k=2,
            split_type=rips.LgrSplitType.LGR_PER_WELL,
        )
        files = os.listdir(tmpdirname)
        assert len(files) == 1
        with open(os.path.join(tmpdirname, files[0])) as f:
            assert "CARFIN" in f.read()

    with pytest.raises(rips.RipsError):
        case.create_lgr_for_completions(well_paths=[], time_step=0)
    with pytest.raises(rips.RipsError):
        case.export_lgr_for_completions(well_paths=[well_path], time_step=0)


def test_create_multiple_fractures(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/small-completion-export-fractures.rsp"
    )
    case = project.cases()[0]
    template = project.descendants(rips.FractureTemplate)[0]
    well_path = project.well_paths()[0]

    case.create_multiple_fractures(
        well_paths=[well_path],
        fracture_template=template,
        min_dist_from_well_td=100.0,
        max_fractures_per_well=5,
        top_layer=1,
        base_layer=30,
        spacing=100.0,
        action=rips.MultipleFracturesAction.REPLACE_FRACTURES,
    )

    fracture_instances = project.descendants(rips.WellPathFracture)
    assert len(fracture_instances) > 0


def test_create_multiple_fractures_object_method(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/small-completion-export-fractures.rsp"
    )
    case = project.cases()[0]
    template = project.descendants(rips.FractureTemplate)[0]
    well_paths = project.well_paths()
    assert len(well_paths) > 0

    before = len(project.descendants(rips.WellPathFracture))

    case.create_multiple_fractures(
        well_paths=well_paths[:1],
        fracture_template=template,
        min_dist_from_well_td=100.0,
        max_fractures_per_well=3,
        spacing=100.0,
        action=rips.MultipleFracturesAction.APPEND_FRACTURES,
    )
    after_append = len(project.descendants(rips.WellPathFracture))
    assert after_append > before

    case.create_multiple_fractures(
        well_paths=well_paths[:1],
        fracture_template=template,
        max_fractures_per_well=1,
        spacing=100.0,
        action=rips.MultipleFracturesAction.REPLACE_FRACTURES,
    )
    after_replace = len(project.descendants(rips.WellPathFracture))
    assert after_replace < after_append

    with pytest.raises(rips.RipsError):
        case.create_multiple_fractures(well_paths=well_paths[:1])
    with pytest.raises(rips.RipsError):
        case.create_multiple_fractures(fracture_template=template)
