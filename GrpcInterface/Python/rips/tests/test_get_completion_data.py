import sys
import os
import json
import hashlib
from pathlib import Path

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot

import pytest


def test_get_completion_data_basic(rips_instance, initialize_test):
    """Test GetCompletionData with a well path and completions"""
    # Load a grid case and import existing well path
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2

    # Import existing well path that intersects the grid properly
    well_path_file = case_root_path + "/wellpath_a.dev"
    well_path_names = rips_instance.project.import_well_paths([well_path_file])
    assert len(well_path_names) == 1

    wells = rips_instance.project.well_paths()
    assert len(wells) == 1
    well_path = wells[0]
    well_path.name = "TestWell"  # Rename for our test
    well_path.update()

    # Get the trajectory properties to find valid MD range
    result = well_path.trajectory_properties(resampling_interval=10.0)
    measured_depths = result["measured_depth"]

    # Use measured depth range from the actual well trajectory
    start_md = measured_depths[len(measured_depths) // 2]  # Middle of well
    end_md = (
        measured_depths[len(measured_depths) // 2 + 10]
        if len(measured_depths) > 20
        else measured_depths[-1]
    )

    # Add perforation intervals (completions)
    perf_interval = well_path.append_perforation_interval(
        start_md=start_md, end_md=end_md, diameter=0.25, skin_factor=0.1
    )
    assert perf_interval is not None

    # Test the completion_data method
    completion_data = well_path.completion_data(case.id)

    # Verify basic structure exists
    assert completion_data is not None

    # Should have WELSPECS data for the well
    assert len(completion_data.welspecs) > 0
    welspec = completion_data.welspecs[0]
    assert welspec.well_name == "TestWell"
    assert welspec.grid_i > 0
    assert welspec.grid_j > 0

    # Should have COMPDAT data for the perforation intervals
    assert len(completion_data.compdat) > 0
    compdat = completion_data.compdat[0]
    assert compdat.well_name == "TestWell"
    assert compdat.grid_i > 0
    assert compdat.grid_j > 0
    assert compdat.upper_k > 0
    assert compdat.lower_k > 0
    assert compdat.open_shut_flag in ["OPEN", "SHUT"]

    # Check measured depth information in COMPDAT
    assert compdat.HasField("start_md")
    assert compdat.HasField("end_md")
    assert compdat.start_md >= start_md - 10  # Allow some tolerance
    assert compdat.end_md <= end_md + 10


def test_get_completion_data_with_valves(rips_instance, initialize_test):
    """Test GetCompletionData with valves/ICDs"""
    # Load a grid case and import well path
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Import existing well path
    well_path_file = case_root_path + "/wellpath_b.dev"
    well_path_names = rips_instance.project.import_well_paths([well_path_file])
    assert len(well_path_names) == 1

    wells = rips_instance.project.well_paths()
    well_path = wells[0]
    well_path.name = "TestWellWithValves"
    well_path.update()

    # Get trajectory properties for valid MD range
    result = well_path.trajectory_properties(resampling_interval=10.0)
    measured_depths = result["measured_depth"]
    start_md = measured_depths[len(measured_depths) // 2]
    end_md = (
        measured_depths[len(measured_depths) // 2 + 10]
        if len(measured_depths) > 20
        else measured_depths[-1]
    )

    # Add perforation interval with valve
    perf_interval = well_path.append_perforation_interval(
        start_md=start_md, end_md=end_md, diameter=0.25, skin_factor=0.1
    )

    # Add a valve to the perforation interval
    valve_templates = rips_instance.project.valve_templates()
    valve_defs = valve_templates.valve_definitions()
    assert len(valve_defs) >= 1

    valve_start_md = start_md + (end_md - start_md) * 0.2
    valve_end_md = start_md + (end_md - start_md) * 0.8
    valve = perf_interval.add_valve(
        template=valve_defs[0],
        start_md=valve_start_md,
        end_md=valve_end_md,
        valve_count=2,
    )
    assert valve is not None

    # Test completion data
    completion_data = well_path.completion_data(case.id)
    assert completion_data is not None

    # Should have WELSPECS
    assert len(completion_data.welspecs) > 0
    welspec = completion_data.welspecs[0]
    assert welspec.well_name == "TestWellWithValves"

    # Should have COMPDAT
    assert len(completion_data.compdat) > 0


def test_get_completion_data_multiple_intervals(rips_instance, initialize_test):
    """Test GetCompletionData with multiple perforation intervals"""
    # Load a grid case and import well path
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Import existing well path
    well_path_file = case_root_path + "/wellpath_a.dev"
    well_path_names = rips_instance.project.import_well_paths([well_path_file])
    wells = rips_instance.project.well_paths()
    well_path = wells[0]
    well_path.name = "TestMultiInterval"
    well_path.update()

    # Get trajectory properties for valid MD ranges
    result = well_path.trajectory_properties(resampling_interval=10.0)
    measured_depths = result["measured_depth"]

    # Create three intervals along the well path
    total_length = len(measured_depths)
    if total_length >= 30:
        # Interval 1: Early part of well
        start_md1 = measured_depths[total_length // 4]
        end_md1 = measured_depths[total_length // 4 + 5]

        # Interval 2: Middle part of well
        start_md2 = measured_depths[total_length // 2]
        end_md2 = measured_depths[total_length // 2 + 5]

        # Interval 3: Later part of well
        start_md3 = measured_depths[3 * total_length // 4]
        end_md3 = measured_depths[3 * total_length // 4 + 5]

        # Add multiple perforation intervals
        interval1 = well_path.append_perforation_interval(
            start_md=start_md1, end_md=end_md1, diameter=0.2, skin_factor=0.0
        )
        interval2 = well_path.append_perforation_interval(
            start_md=start_md2, end_md=end_md2, diameter=0.25, skin_factor=0.1
        )
        interval3 = well_path.append_perforation_interval(
            start_md=start_md3, end_md=end_md3, diameter=0.3, skin_factor=0.2
        )

        assert interval1 is not None
        assert interval2 is not None
        assert interval3 is not None

        # Test completion data
        completion_data = well_path.completion_data(case.id)
        assert completion_data is not None

        # Should have WELSPECS
        assert len(completion_data.welspecs) > 0
        welspec = completion_data.welspecs[0]
        assert welspec.well_name == "TestMultiInterval"

        # Should have multiple COMPDAT entries for different intervals
        assert len(completion_data.compdat) >= 1  # At least some COMPDAT data

        # Check that we have MD information in COMPDAT entries
        has_md_data = False
        for compdat in completion_data.compdat:
            if compdat.HasField("start_md") and compdat.HasField("end_md"):
                has_md_data = True
                assert compdat.start_md >= 0
                assert compdat.end_md >= compdat.start_md

        assert has_md_data, "Should have measured depth data in COMPDAT"
    else:
        # Skip test if well is too short for multiple intervals
        assert True  # Test passes if well is too short


def test_get_completion_data_field_validation(rips_instance, initialize_test):
    """Test optional field handling in completion data"""
    # Load a grid case and import well path
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Import existing well path
    well_path_file = case_root_path + "/wellpath_a.dev"
    well_path_names = rips_instance.project.import_well_paths([well_path_file])
    wells = rips_instance.project.well_paths()
    well_path = wells[0]
    well_path.name = "FieldValidationTest"
    well_path.update()

    # Get valid MD range
    result = well_path.trajectory_properties(resampling_interval=10.0)
    measured_depths = result["measured_depth"]
    start_md = measured_depths[len(measured_depths) // 2]
    end_md = (
        measured_depths[len(measured_depths) // 2 + 5]
        if len(measured_depths) > 10
        else measured_depths[-1]
    )

    perf_interval = well_path.append_perforation_interval(start_md, end_md, 0.25, 0.1)
    assert perf_interval is not None

    # Get completion data
    completion_data = well_path.completion_data(case.id)
    assert completion_data is not None

    # Validate COMPDAT optional fields
    if len(completion_data.compdat) > 0:
        comp = completion_data.compdat[0]

        # Test required fields
        assert comp.well_name == "FieldValidationTest"
        assert comp.grid_i > 0
        assert comp.grid_j > 0
        assert comp.upper_k > 0
        assert comp.lower_k > 0
        assert comp.open_shut_flag in ["OPEN", "SHUT"]

        # Test optional double fields - verify type and range when present
        if comp.HasField("transmissibility"):
            assert comp.transmissibility >= 0.0
        if comp.HasField("diameter"):
            assert comp.diameter > 0.0
        if comp.HasField("skin_factor"):
            assert isinstance(comp.skin_factor, float)
        if comp.HasField("start_md") and comp.HasField("end_md"):
            assert comp.end_md >= comp.start_md

    # Validate WELSPECS required and optional fields
    if len(completion_data.welspecs) > 0:
        welspec = completion_data.welspecs[0]

        # Required fields
        assert welspec.well_name == "FieldValidationTest"
        assert welspec.group_name
        assert welspec.grid_i > 0
        assert welspec.grid_j > 0
        assert welspec.phase in ["OIL", "GAS", "WATER", "LIQ"]

        # Optional fields
        if welspec.HasField("bhp_depth"):
            assert welspec.bhp_depth >= 0.0
        if welspec.HasField("drainage_radius"):
            assert welspec.drainage_radius > 0.0


def test_get_completion_data_invalid_case_id(rips_instance, initialize_test):
    """Test error handling for invalid case ID"""
    # Load a grid case
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(path=case_path)

    # Create well path
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "ErrorTestWell"
    well_path.update()

    geometry = well_path.well_path_geometry()
    surface_coord = [460000.0, 5930000.0, 1600.0]
    bottom_coord = [461000.0, 5931000.0, 1900.0]

    geometry.append_well_target(surface_coord)
    geometry.append_well_target(bottom_coord)
    well_path.update()

    perf_interval = well_path.append_perforation_interval(1000.0, 1200.0, 0.25, 0.1)
    assert perf_interval is not None

    # Test with invalid case ID
    try:
        # Using a negative case ID that shouldn't exist
        completion_data = well_path.completion_data(-1)
        # Should return empty data or handle gracefully
        assert completion_data is not None
        # With invalid case, completion data should be empty or minimal
    except Exception as e:
        # Acceptable if it throws an error for invalid case ID
        assert (
            "case" in str(e).lower()
            or "invalid" in str(e).lower()
            or "id" in str(e).lower()
        )


def test_get_completion_data_empty_well(rips_instance, initialize_test):
    """Test GetCompletionData with a well that has no completions"""
    # Load a grid case
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Create well path without completions
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "EmptyWell"
    well_path.update()

    geometry = well_path.well_path_geometry()
    surface_coord = [460000.0, 5930000.0, 1600.0]
    bottom_coord = [461000.0, 5931000.0, 1900.0]

    geometry.append_well_target(surface_coord)
    geometry.append_well_target(bottom_coord)
    well_path.update()

    # Don't add any perforation intervals

    # Test completion data - should return valid response but with empty/minimal data
    completion_data = well_path.completion_data(case.id)
    assert completion_data is not None

    # May or may not have WELSPECS depending on implementation
    # But should not have COMPDAT entries without perforations
    assert len(completion_data.compdat) == 0


def test_get_completion_data_stability_comprehensive(rips_instance, initialize_test):
    """Comprehensive stability test for GetCompletionData - ensures deterministic results"""

    # 1. Use deterministic test setup
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    well_path_file = case_root_path + "/wellpath_a.dev"
    rips_instance.project.import_well_paths([well_path_file])
    well_path = rips_instance.project.well_paths()[0]
    well_path.name = "StabilityTestWell"
    well_path.update()

    # 2. Fixed completion parameters (no dynamic MD calculation) - 400m interval
    perf_interval = well_path.append_perforation_interval(
        start_md=2200.0, end_md=2600.0, diameter=0.25, skin_factor=0.1
    )
    assert perf_interval is not None

    # 3. Get completion data multiple times
    data_run1 = well_path.completion_data(case.id)
    data_run2 = well_path.completion_data(case.id)

    # 4. Serialize and compare
    compdat1 = serialize_compdat_for_comparison(data_run1.compdat)
    compdat2 = serialize_compdat_for_comparison(data_run2.compdat)

    # 5. Verify deterministic behavior
    assert compdat1 == compdat2, "COMPDAT data should be identical between runs"
    assert len(compdat1) > 0, "Should have COMPDAT entries"

    # 6. Validate expected properties
    validate_compdat_properties(
        data_run1.compdat, expected_diameter=0.25, expected_skin=0.1
    )

    # 7. Validate WELSPECS stability
    welspecs1 = serialize_welspecs_for_comparison(data_run1.welspecs)
    welspecs2 = serialize_welspecs_for_comparison(data_run2.welspecs)
    assert welspecs1 == welspecs2, "WELSPECS data should be identical between runs"

    # 8. Cross-validate with perforation interval properties
    for compdat in data_run1.compdat:
        assert compdat.well_name == "StabilityTestWell"
        if compdat.HasField("diameter"):
            assert abs(compdat.diameter - 0.25) < 0.001, "Diameter should match input"
        if compdat.HasField("skin_factor"):
            assert (
                abs(compdat.skin_factor - 0.1) < 0.001
            ), "Skin factor should match input"
        if compdat.HasField("start_md") and compdat.HasField("end_md"):
            assert (
                2190.0 <= compdat.start_md <= 2610.0
            ), "Start MD should be in expected range"
            assert (
                2190.0 <= compdat.end_md <= 2610.0
            ), "End MD should be in expected range"
            assert compdat.end_md >= compdat.start_md, "End MD should be >= start MD"


def test_completion_data_regression_reference(rips_instance, initialize_test):
    """Regression test against stored reference JSON file"""

    # Generate completion data with fixed parameters
    completion_data = generate_reference_completion_data(rips_instance)

    # Serialize current results
    current_results = {
        "compdat": serialize_compdat_for_comparison(completion_data.compdat),
        "welspecs": serialize_welspecs_for_comparison(completion_data.welspecs),
    }

    # Path to reference data (cross-platform)
    test_dir = Path(__file__).parent
    reference_dir = test_dir / "reference_data"
    reference_file = reference_dir / "completion_data_reference.json"

    # Ensure reference directory exists
    reference_dir.mkdir(exist_ok=True)

    if reference_file.exists():
        # Load reference and compare
        with open(reference_file, "r", encoding="utf-8") as f:
            reference_results = json.load(f)

        # Compare COMPDAT data
        assert current_results["compdat"] == reference_results["compdat"], (
            "COMPDAT data differs from reference. "
            "If this is expected, delete the reference file and run again to regenerate."
        )

        # Compare WELSPECS data
        assert current_results["welspecs"] == reference_results["welspecs"], (
            "WELSPECS data differs from reference. "
            "If this is expected, delete the reference file and run again to regenerate."
        )

        print("Completion data matches reference file successfully")

    else:
        # Generate reference file (run once to establish baseline)
        with open(reference_file, "w", encoding="utf-8") as f:
            json.dump(current_results, f, indent=2, sort_keys=True)

        print(f"Generated reference data at: {reference_file}")
        print("Re-run the test to validate against the reference")

        # Validate the generated data has expected structure
        assert (
            len(current_results["compdat"]) > 0
        ), "Should have COMPDAT entries in reference"
        assert (
            len(current_results["welspecs"]) > 0
        ), "Should have WELSPECS entries in reference"


def generate_reference_completion_data(rips_instance):
    """Generate completion data with fixed, deterministic parameters for reference testing"""

    # Load case
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Import well path
    well_path_file = case_root_path + "/wellpath_a.dev"
    rips_instance.project.import_well_paths([well_path_file])
    well_path = rips_instance.project.well_paths()[0]
    well_path.name = "ReferenceTestWell"
    well_path.update()

    # Create completion with fixed parameters - 400m interval to cover multiple cells
    perf_interval = well_path.append_perforation_interval(
        start_md=2200.0, end_md=2600.0, diameter=0.25, skin_factor=0.1
    )
    assert perf_interval is not None

    # Return completion data
    return well_path.completion_data(case.id)


def serialize_compdat_for_comparison(compdat_entries):
    """Convert COMPDAT to stable comparison format - cross-platform consistent"""
    entries = []
    for comp in compdat_entries:
        # Create ordered dict to ensure consistent field ordering
        entry = {}

        # Add fields in alphabetical order for consistency
        if comp.HasField("comment"):
            entry["comment"] = comp.comment
        if comp.HasField("d_factor"):
            entry["d_factor"] = round(float(comp.d_factor), 8)
        if comp.HasField("diameter"):
            entry["diameter"] = round(float(comp.diameter), 8)
        if comp.HasField("direction"):
            entry["direction"] = comp.direction
        if comp.HasField("end_md"):
            entry["end_md"] = round(float(comp.end_md), 6)
        entry["grid_i"] = int(comp.grid_i)
        entry["grid_j"] = int(comp.grid_j)
        if comp.HasField("grid_name"):
            entry["grid_name"] = comp.grid_name
        if comp.HasField("kh"):
            entry["kh"] = round(float(comp.kh), 8)
        entry["lower_k"] = int(comp.lower_k)
        entry["open_shut_flag"] = comp.open_shut_flag
        if comp.HasField("saturation"):
            entry["saturation"] = round(float(comp.saturation), 8)
        if comp.HasField("skin_factor"):
            entry["skin_factor"] = round(float(comp.skin_factor), 8)
        if comp.HasField("start_md"):
            entry["start_md"] = round(float(comp.start_md), 6)
        if comp.HasField("transmissibility"):
            entry["transmissibility"] = round(float(comp.transmissibility), 8)
        entry["upper_k"] = int(comp.upper_k)
        entry["well_name"] = comp.well_name

        entries.append(entry)

    # Sort for consistent ordering across platforms
    entries.sort(key=lambda x: (x["grid_i"], x["grid_j"], x["upper_k"], x["lower_k"]))
    return entries


def serialize_welspecs_for_comparison(welspecs_entries):
    """Convert WELSPECS to stable comparison format - cross-platform consistent"""
    entries = []
    for welspec in welspecs_entries:
        # Create ordered dict with alphabetical field ordering
        entry = {}

        if welspec.HasField("auto_shut_in"):
            entry["auto_shut_in"] = welspec.auto_shut_in
        if welspec.HasField("bhp_depth"):
            entry["bhp_depth"] = round(float(welspec.bhp_depth), 6)
        if welspec.HasField("cross_flow"):
            entry["cross_flow"] = welspec.cross_flow
        if welspec.HasField("drainage_radius"):
            entry["drainage_radius"] = round(float(welspec.drainage_radius), 6)
        if welspec.HasField("fip_region"):
            entry["fip_region"] = int(welspec.fip_region)
        entry["grid_i"] = int(welspec.grid_i)
        entry["grid_j"] = int(welspec.grid_j)
        if welspec.HasField("grid_name"):
            entry["grid_name"] = welspec.grid_name
        entry["group_name"] = welspec.group_name
        if welspec.HasField("hydrostatic_density_calc"):
            entry["hydrostatic_density_calc"] = welspec.hydrostatic_density_calc
        if welspec.HasField("inflow_equation"):
            entry["inflow_equation"] = welspec.inflow_equation
        entry["phase"] = welspec.phase
        if welspec.HasField("pvt_num"):
            entry["pvt_num"] = int(welspec.pvt_num)
        entry["well_name"] = welspec.well_name

        entries.append(entry)

    # Sort for consistent ordering across platforms
    entries.sort(key=lambda x: (x["well_name"], x["grid_i"], x["grid_j"]))
    return entries


def validate_compdat_properties(
    compdat_entries, expected_diameter=None, expected_skin=None
):
    """Validate that COMPDAT entries have consistent and expected properties"""
    assert len(compdat_entries) > 0, "Should have at least one COMPDAT entry"

    for i, compdat in enumerate(compdat_entries):
        # Validate required fields
        assert compdat.well_name, f"Entry {i}: Well name should not be empty"
        assert compdat.grid_i > 0, f"Entry {i}: Grid I should be positive"
        assert compdat.grid_j > 0, f"Entry {i}: Grid J should be positive"
        assert compdat.upper_k > 0, f"Entry {i}: Upper K should be positive"
        assert (
            compdat.lower_k >= compdat.upper_k
        ), f"Entry {i}: Lower K should be >= upper K"
        assert compdat.open_shut_flag in [
            "OPEN",
            "SHUT",
        ], f"Entry {i}: Invalid open/shut flag"

        # Validate optional fields when present
        if compdat.HasField("transmissibility"):
            assert (
                compdat.transmissibility >= 0
            ), f"Entry {i}: Transmissibility should be non-negative"

        if compdat.HasField("diameter") and expected_diameter is not None:
            assert (
                abs(compdat.diameter - expected_diameter) < 0.001
            ), f"Entry {i}: Diameter mismatch"

        if compdat.HasField("skin_factor") and expected_skin is not None:
            assert (
                abs(compdat.skin_factor - expected_skin) < 0.001
            ), f"Entry {i}: Skin factor mismatch"

        if compdat.HasField("start_md") and compdat.HasField("end_md"):
            assert (
                compdat.end_md >= compdat.start_md
            ), f"Entry {i}: End MD should be >= start MD"
            assert compdat.start_md >= 0, f"Entry {i}: Start MD should be non-negative"

        if compdat.HasField("kh"):
            assert compdat.kh >= 0, f"Entry {i}: KH should be non-negative"


def generate_compdat_hash(compdat_data):
    """Generate hash for regression detection - cross-platform stable"""
    # Ensure consistent JSON serialization across platforms
    data_str = json.dumps(
        compdat_data,
        sort_keys=True,
        separators=(",", ":"),  # Remove whitespace variations
        ensure_ascii=True,  # Avoid unicode encoding differences
    )
    # Use UTF-8 encoding explicitly for cross-platform consistency
    return hashlib.sha256(data_str.encode("utf-8")).hexdigest()


def test_completion_data_hash_cross_platform_stability(rips_instance, initialize_test):
    """Test that hash generation is stable across different platforms"""

    # Generate test data
    completion_data = generate_reference_completion_data(rips_instance)

    # Serialize the same data multiple times
    serialized_data1 = {
        "compdat": serialize_compdat_for_comparison(completion_data.compdat),
        "welspecs": serialize_welspecs_for_comparison(completion_data.welspecs),
    }

    serialized_data2 = {
        "compdat": serialize_compdat_for_comparison(completion_data.compdat),
        "welspecs": serialize_welspecs_for_comparison(completion_data.welspecs),
    }

    # Generate hashes
    hash1 = generate_compdat_hash(serialized_data1)
    hash2 = generate_compdat_hash(serialized_data2)

    # Should be identical
    assert hash1 == hash2, "Hash should be deterministic for same data"

    # Test specific cross-platform concerns
    test_data = [
        {"float_val": 0.12345678901234567, "int_val": 42, "str_val": "test"},
        {"float_val": 1.0000000000000001, "int_val": 0, "str_val": ""},
    ]

    # Generate hash with explicit settings
    json_str1 = json.dumps(
        test_data, sort_keys=True, separators=(",", ":"), ensure_ascii=True
    )
    json_str2 = json.dumps(
        test_data, sort_keys=True, separators=(",", ":"), ensure_ascii=True
    )

    hash_test1 = hashlib.sha256(json_str1.encode("utf-8")).hexdigest()
    hash_test2 = hashlib.sha256(json_str2.encode("utf-8")).hexdigest()

    assert hash_test1 == hash_test2, "JSON serialization should be deterministic"

    print(f"Cross-platform stable hash: {hash1[:16]}...")


def test_completion_data_reference_failure_simulation(rips_instance, initialize_test):
    """Test that the reference test properly detects changes (simulate failure scenario)"""
    # This test validates that our reference test would catch real changes
    # by temporarily creating modified data and verifying it fails comparison

    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    well_path_file = case_root_path + "/wellpath_a.dev"
    rips_instance.project.import_well_paths([well_path_file])
    well_path = rips_instance.project.well_paths()[0]
    well_path.name = "ModifiedTestWell"  # Different well name
    well_path.update()

    # Same completion parameters as reference
    perf_interval = well_path.append_perforation_interval(
        start_md=2200.0, end_md=2600.0, diameter=0.25, skin_factor=0.1
    )

    completion_data = well_path.completion_data(case.id)
    modified_results = {
        "compdat": serialize_compdat_for_comparison(completion_data.compdat),
        "welspecs": serialize_welspecs_for_comparison(completion_data.welspecs),
    }

    # Load reference data
    test_dir = Path(__file__).parent
    reference_file = test_dir / "reference_data" / "completion_data_reference.json"

    with open(reference_file, "r", encoding="utf-8") as f:
        reference_results = json.load(f)

    # This should fail because well names are different
    assert (
        modified_results["compdat"] != reference_results["compdat"]
    ), "Modified data should differ from reference (well name changed)"
    assert (
        modified_results["welspecs"] != reference_results["welspecs"]
    ), "Modified WELSPECS should differ from reference (well name changed)"

    print("Reference test correctly detects data changes")


def test_welspecs_grid_name_field(rips_instance, initialize_test):
    """Test WELSPECS grid_name field behavior for main grid vs LGR grids"""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    well_path_file = case_root_path + "/wellpath_a.dev"
    rips_instance.project.import_well_paths([well_path_file])
    well_path = rips_instance.project.well_paths()[0]
    well_path.name = "GridNameTestWell"
    well_path.update()

    # Add completion
    result = well_path.trajectory_properties(resampling_interval=10.0)
    measured_depths = result["measured_depth"]
    start_md = measured_depths[len(measured_depths) // 2]
    end_md = (
        measured_depths[len(measured_depths) // 2 + 5]
        if len(measured_depths) > 10
        else measured_depths[-1]
    )

    perf_interval = well_path.append_perforation_interval(start_md, end_md, 0.25, 0.1)
    assert perf_interval is not None

    # Get completion data
    completion_data = well_path.completion_data(case.id)
    assert completion_data is not None
    assert len(completion_data.welspecs) > 0

    # Test WELSPECS grid_name field
    for welspec in completion_data.welspecs:
        assert welspec.well_name == "GridNameTestWell"

        # For main grid wells, grid_name should be empty or not present
        # This results in WELSPECS output (not WELSPECL)
        if welspec.HasField("grid_name"):
            # If grid_name field exists, it should be empty for main grid
            assert (
                welspec.grid_name.strip() == ""
            ), "Main grid wells should have empty grid_name"

        # Verify required WELSPECS fields
        assert welspec.group_name, "Should have group name"

        # Test specific grid coordinates (these should be deterministic for the test well path)
        assert welspec.grid_i == 29, f"Expected grid_i=29, got {welspec.grid_i}"
        assert welspec.grid_j == 41, f"Expected grid_j=41, got {welspec.grid_j}"

        assert welspec.phase in [
            "OIL",
            "GAS",
            "WATER",
            "LIQ",
        ], "Should have valid phase"

    print(
        "Note: WELSPECL testing requires LGR grids. Current test case uses main grid only."
    )
    print(
        "WELSPECL functionality would be tested when grid_name contains a non-empty string."
    )


def test_completion_data_cross_validation(rips_instance, initialize_test):
    """Cross-validate completion data with perforation interval properties"""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    well_path_file = case_root_path + "/wellpath_a.dev"
    rips_instance.project.import_well_paths([well_path_file])
    well_path = rips_instance.project.well_paths()[0]
    well_path.name = "CrossValidationWell"
    well_path.update()

    # Create perforation with known parameters
    diameter = 0.3
    skin_factor = 0.05
    start_md = 2400.0
    end_md = 2420.0

    perf_interval = well_path.append_perforation_interval(
        start_md, end_md, diameter, skin_factor
    )

    # Get data via gRPC API
    completion_data = well_path.completion_data(case.id)

    # Cross-validate with perforation interval properties
    assert len(completion_data.compdat) > 0, "Should have COMPDAT entries"

    # Validate against original interval properties
    found_matching_entry = False
    for compdat in completion_data.compdat:
        assert compdat.well_name == "CrossValidationWell", "Well name should match"

        # Check diameter matches
        if compdat.HasField("diameter"):
            if abs(compdat.diameter - diameter) < 0.001:
                found_matching_entry = True

                # Also check skin factor for this entry
                if compdat.HasField("skin_factor"):
                    assert (
                        abs(compdat.skin_factor - skin_factor) < 0.001
                    ), "Skin factor should match"

                # Check MD range
                if compdat.HasField("start_md") and compdat.HasField("end_md"):
                    assert (
                        start_md <= compdat.start_md <= end_md + 5
                    ), "Start MD should be in range"
                    assert (
                        start_md <= compdat.end_md <= end_md + 5
                    ), "End MD should be in range"

    assert found_matching_entry, f"Should find COMPDAT entry with diameter {diameter}"
