import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot

import pytest


def test_10k(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
        case_root_path + "/wellpath_b.dev",
    ]
    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 2
    wells = rips_instance.project.well_paths()
    assert len(wells) == 2
    assert wells[0].name == "Well Path A"
    assert wells[1].name == "Well Path B"


def test_10k_import_well(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
        case_root_path + "/wellpath_b.dev",
    ]

    well_a = rips_instance.project.well_path_collection().import_well_path(
        well_path_files[0]
    )
    assert well_a.name == "Well Path A"

    well_b = rips_instance.project.well_path_collection().import_well_path(
        well_path_files[1]
    )
    assert well_b.name == "Well Path B"

    wells = rips_instance.project.well_paths()
    assert len(wells) == 2
    assert wells[0].name == "Well Path A"
    assert wells[1].name == "Well Path B"


def test_10k_intersection(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    view = case.create_view()
    view.set_time_step(1)

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 1
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    # Add a curve intersection based on the well path
    intersection_coll = rips_instance.project.descendants(rips.IntersectionCollection)[
        0
    ]
    well_path_intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    well_path_intersection.type = "CS_WELL_PATH"
    well_path_intersection.well_path = well_path
    well_path_intersection.update()

    # Three coords per triangle
    geometry = well_path_intersection.geometry()
    coord_count = len(geometry.x_coords)
    assert coord_count == 13254

    # One value per triangle
    geometry_result_values = well_path_intersection.geometry_result()
    result_count = len(geometry_result_values.values)
    assert result_count == 4418

    # Three coords per triangle, one result value per triangle
    assert (result_count * 3) == coord_count


def test_empty_well_intersection(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    view = case.create_view()
    view.set_time_step(1)

    # Add a curve intersection based on the well path
    intersection_coll = rips_instance.project.descendants(rips.IntersectionCollection)[
        0
    ]

    well_path_intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    well_path_intersection.type = "CS_WELL_PATH"
    well_path_intersection.well_path = None
    well_path_intersection.update()

    # Test with empty geometry.
    with pytest.raises(rips.RipsError):
        well_path_intersection.geometry()

    with pytest.raises(rips.RipsError):
        well_path_intersection.geometry_result()


def test_10k_intersection_trajectory_properties(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 1
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    result = well_path.trajectory_properties(resampling_interval=10.0)
    assert result

    assert "coordinate_x" in result
    assert "coordinate_y" in result
    assert "coordinate_z" in result
    assert "measured_depth" in result
    assert "azimuth" in result
    assert "inclination" in result
    assert "dogleg" in result


def test_10k_intersection_add_well_perforation_interval_with_valves(
    rips_instance, initialize_test
):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 1
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    result = well_path.trajectory_properties(resampling_interval=10.0)

    measured_depths = result["measured_depth"]

    start_md = measured_depths[len(measured_depths) - 3]
    end_md = measured_depths[len(measured_depths) - 2]
    diameter = 0.25
    skin_factor = 0.1

    perf_interval = well_path.append_perforation_interval(
        start_md, end_md, diameter, skin_factor
    )

    assert perf_interval.start_measured_depth == start_md
    assert perf_interval.end_measured_depth == end_md
    assert perf_interval.diameter == diameter
    assert perf_interval.skin_factor == skin_factor

    valve_templates = rips_instance.project.valve_templates()
    assert len(valve_templates.valve_definitions()) == 3

    assert len(perf_interval.valves()) == 0

    valve_start_md = start_md + 1
    valve_end_md = end_md - 1
    valve_count = 1
    valve = perf_interval.add_valve(
        template=valve_templates.valve_definitions()[0],
        start_md=valve_start_md,
        end_md=valve_end_md,
        valve_count=valve_count,
    )

    assert valve
    assert valve.name == "1 AICD: 2451 - 2459"

    assert len(perf_interval.valves()) == 1


def test_10k_intersection_add_well_perforation_interval_with_invalid_valves(
    rips_instance, initialize_test
):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 1
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    result = well_path.trajectory_properties(resampling_interval=10.0)

    measured_depths = result["measured_depth"]

    start_md = measured_depths[len(measured_depths) - 3]
    end_md = measured_depths[len(measured_depths) - 2]
    diameter = 0.25
    skin_factor = 0.1

    perf_interval = well_path.append_perforation_interval(
        start_md, end_md, diameter, skin_factor
    )

    valve_templates = rips_instance.project.valve_templates()

    with pytest.raises(rips.RipsError):
        # end_md < start_md
        perf_interval.add_valve(
            template=valve_templates.valve_definitions()[0],
            start_md=1000,
            end_md=800,
            valve_count=1,
        )

    with pytest.raises(rips.RipsError):
        # zero valves
        perf_interval.add_valve(
            template=valve_templates.valve_definitions()[0],
            start_md=400,
            end_md=800,
            valve_count=0,
        )


def test_valve_template_creation(rips_instance, initialize_test):
    """Test creating valve templates through Python API"""
    valve_templates = rips_instance.project.valve_templates()

    # Check initial state - should have 3 default templates
    initial_count = len(valve_templates.valve_definitions())
    assert initial_count == 3

    # Test creating ICD template with default values
    icd_template = valve_templates.add_template(completion_type="ICD")
    assert icd_template is not None
    assert icd_template.orifice_diameter == 8.0  # Default value
    assert icd_template.flow_coefficient == 0.7  # Default value
    assert len(valve_templates.valve_definitions()) == initial_count + 1

    # Test creating ICV template with custom values
    icv_template = valve_templates.add_template(
        completion_type="ICV",
        orifice_diameter=10.5,
        flow_coefficient=0.9,
        user_label="Custom ICV Template",
    )
    assert icv_template is not None
    assert icv_template.orifice_diameter == 10.5
    assert icv_template.flow_coefficient == 0.9
    assert "Custom ICV Template" in icv_template.name
    assert len(valve_templates.valve_definitions()) == initial_count + 2

    # Test creating AICD template
    aicd_template = valve_templates.add_template(
        completion_type="AICD",
        orifice_diameter=7.3,
        flow_coefficient=0.2,
        user_label="Test AICD",
    )
    assert aicd_template is not None
    assert aicd_template.orifice_diameter == 7.3
    assert aicd_template.flow_coefficient == 0.2
    assert "Test AICD" in aicd_template.name
    assert len(valve_templates.valve_definitions()) == initial_count + 3

    # Test that the new templates can be used in valves
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(path=case_path)
    well_path_files = [case_root_path + "/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    # Add perforation with our new template
    perf_interval = well_path.append_perforation_interval(2450, 2460, 0.25, 0.1)
    valve = perf_interval.add_valve(
        template=icd_template,
        start_md=2451,
        end_md=2459,
        valve_count=1,
    )
    assert valve is not None
    assert len(perf_interval.valves()) == 1


def test_valve_template_invalid_completion_type(rips_instance, initialize_test):
    """Test error handling for invalid completion types"""
    valve_templates = rips_instance.project.valve_templates()

    # Test invalid completion type
    with pytest.raises(rips.RipsError):
        valve_templates.add_template(completion_type="INVALID_TYPE")
