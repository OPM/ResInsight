import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot
import rips


def _open_case(rips_instance):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    return project.cases()[0]


def _create_well_path(rips_instance):
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "perforation_filter_well"
    well_path.update()

    geometry = well_path.well_path_geometry()
    geometry.append_well_target([1000.0, 2000.0, 0.0])
    geometry.append_well_target([1000.0, 2000.0, -1000.0])
    geometry.append_well_target([1000.0, 2000.0, -3000.0])
    geometry.append_well_target([1000.0, 2000.0, -3700.0])
    geometry.update()

    return well_path


def _refreshed_perforations(well_path):
    return well_path.completions().perforations().perforations()


def test_add_filter_to_perforation(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(name="My Filter", combine_mode="AND")

    well_path = _create_well_path(rips_instance)
    perf = well_path.append_perforation_interval(3300, 3350, 0.2, 0.76)

    assert perf.cell_filter() is None

    perf.add_filter(combined)

    refreshed = _refreshed_perforations(well_path)
    assert len(refreshed) == 1
    assert refreshed[0].cell_filter() is not None
    assert refreshed[0].cell_filter().name == "My Filter"


def test_add_filter_replaces_existing(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    first = data_filters.add_combined_filter(name="First", combine_mode="AND")
    second = data_filters.add_combined_filter(name="Second", combine_mode="OR")

    well_path = _create_well_path(rips_instance)
    perf = well_path.append_perforation_interval(3300, 3350, 0.2, 0.76)

    perf.add_filter(first)
    perf.add_filter(second)

    refreshed = _refreshed_perforations(well_path)[0]
    filter_object = refreshed.cell_filter()
    assert filter_object is not None
    assert filter_object.name == "Second"


def test_two_perforations_share_one_filter(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(name="Shared", combine_mode="AND")

    well_path = _create_well_path(rips_instance)
    perf1 = well_path.append_perforation_interval(3300, 3350, 0.2, 0.76)
    perf2 = well_path.append_perforation_interval(3500, 3600, 0.2, 0.76)

    perf1.add_filter(combined)
    perf2.add_filter(combined)

    refreshed = _refreshed_perforations(well_path)
    assert len(refreshed) == 2
    assert refreshed[0].cell_filter().name == "Shared"
    assert refreshed[1].cell_filter().name == "Shared"
