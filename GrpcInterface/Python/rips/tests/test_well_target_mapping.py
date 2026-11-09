import glob
import math
import os
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot
import rips

CASE_PATH = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
OTHER_CASE_PATH = dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID"
GRID_ENSEMBLE_GLOB = dataroot.PATH + "/grid_ensemble/realization-*/iter-1/*.EGRID"


def _cluster_ids(case, time_step):
    values = case.active_cell_property(
        rips.PropertyType.GENERATED, "WELL_TARGET_NUM", time_step
    )
    return [int(v) for v in values if math.isfinite(v) and v > 0]


def test_add_well_target_mapping(rips_instance, initialize_test):
    case = rips_instance.project.load_case(CASE_PATH)
    time_step_count = len(case.time_steps())

    wtm = case.add_well_target_mapping(name="Python Mapping", max_num_targets=3)
    assert wtm is not None
    assert wtm.name == "Python Mapping"
    assert wtm.time_step == time_step_count - 1
    assert wtm.max_num_targets == 3

    mappings = case.data_analytics_collection().well_target_mappings()
    assert len(mappings) == 1

    cluster_ids = _cluster_ids(case, wtm.time_step)
    assert len(cluster_ids) > 0
    assert max(cluster_ids) <= 3

    statistics = wtm.cluster_statistics()
    assert len(statistics) == max(cluster_ids)
    for stat in statistics:
        assert stat.num_cells == cluster_ids.count(stat.cluster_id)
        assert stat.total_porv_soil > 0.0

    largest = max(statistics, key=lambda s: s.num_cells)
    assert largest.pressure > 0.0
    assert largest.permeability > 0.0


def test_edit_and_compute(rips_instance, initialize_test):
    case = rips_instance.project.load_case(CASE_PATH)
    wtm = case.add_well_target_mapping(max_num_targets=5)
    assert len(wtm.cluster_statistics()) > 1

    wtm.max_num_targets = 1
    wtm.update()
    wtm.compute()

    assert len(wtm.cluster_statistics()) == 1
    assert set(_cluster_ids(case, wtm.time_step)) == {1}


def test_grow_data_filter(rips_instance, initialize_test):
    case = rips_instance.project.load_case(CASE_PATH)
    combined = case.data_filter_collection().add_combined_filter(name="K1")
    combined.add_range_filter(name="K-slice 1", start_k=1, cell_count_k=1)

    wtm = case.add_well_target_mapping(grow_data_filter=combined, avoid_faults=True)
    assert wtm.grow_data_filter is not None
    assert wtm.avoid_faults

    values = case.active_cell_property(
        rips.PropertyType.GENERATED, "WELL_TARGET_NUM", wtm.time_step
    )
    cells = case.cell_info_for_active_cells()
    target_k = {
        cell.local_ijk.k for cell, v in zip(cells, values) if math.isfinite(v) and v > 0
    }
    assert target_k == {0}


def test_filter_from_other_case_fails(rips_instance, initialize_test):
    case = rips_instance.project.load_case(CASE_PATH)
    other_case = rips_instance.project.load_case(OTHER_CASE_PATH)
    other_filter = other_case.data_filter_collection().add_combined_filter(name="Other")

    with pytest.raises(rips.RipsError):
        case.add_well_target_mapping(grow_data_filter=other_filter)

    assert len(case.data_analytics_collection().well_target_mappings()) == 0


def test_invalid_input_fails(rips_instance, initialize_test):
    case = rips_instance.project.load_case(CASE_PATH)

    with pytest.raises(rips.RipsError):
        case.add_well_target_mapping(time_step=1000)

    with pytest.raises(rips.RipsError):
        case.add_well_target_mapping(volumes_type="NOT_A_VOLUMES_TYPE")

    assert len(case.data_analytics_collection().well_target_mappings()) == 0

    wtm = case.add_well_target_mapping()
    wtm.time_step = 1000
    wtm.update()
    with pytest.raises(rips.RipsError):
        wtm.compute()


def test_reservoir_grid_ensemble(rips_instance, initialize_test):
    file_names = sorted(glob.glob(GRID_ENSEMBLE_GLOB))
    assert len(file_names) > 1

    ensemble = rips_instance.project.create_reservoir_grid_ensemble(
        file_names=file_names
    )
    assert ensemble is not None

    wtm = ensemble.add_well_target_mapping()
    assert wtm is not None
    assert len(ensemble.well_target_mappings()) == 1
    assert len(wtm.cluster_statistics()) == 0

    statistics_case = wtm.ensemble_statistics_case()
    assert statistics_case is not None

    values = statistics_case.active_cell_property(
        rips.PropertyType.GENERATED, "TOTAL_PORV_SOIL_P10", 0
    )
    assert any(math.isfinite(v) for v in values)

    realization = ensemble.descendants(rips.EclipseCase)[0]
    with pytest.raises(rips.RipsError):
        realization.add_well_target_mapping()
