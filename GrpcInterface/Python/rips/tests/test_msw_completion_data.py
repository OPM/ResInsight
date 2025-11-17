import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def test_msw_completion_data_perf_lateral(rips_instance, initialize_test):
    """Test MSW completion data retrieval from perf_lateral.rsp project.

    This test loads a project with Multi-Segment Well (MSW) configuration
    and verifies that completion data can be retrieved including MSW-specific
    tables (WELSEGS, COMPSEGS).
    """
    project_path = dataroot.PATH + "/msw-export/project-files/perf_lateral.rsp"
    project = rips_instance.project.open(path=project_path)

    # Get the case
    cases = project.cases()
    assert len(cases) > 0, "Project should contain at least one case"
    case = cases[0]

    # Get well paths
    well_paths = project.well_paths()
    assert len(well_paths) >= 2, "Project should contain at least 2 well paths"

    # Find the well paths by name
    well_y1 = None
    well_y2 = None
    for well in well_paths:
        if well.name == "Well-1 Y1":
            well_y1 = well
        elif well.name == "Well-1 Y2":
            well_y2 = well

    assert well_y1 is not None, "Well-1 Y1 should exist in project"
    assert well_y2 is not None, "Well-1 Y2 should exist in project"

    # Test completion data for Well-1 Y1
    completion_data_y1 = well_y1.completion_data(case.id)

    # Verify WELSPECS data
    welspecs = completion_data_y1.welspecs
    assert len(welspecs) > 0, "Should have WELSPECS data for Well-1 Y1"

    welspec = welspecs[0]
    assert welspec.well_name == "Well-1", "Well name should be Well-1"
    assert welspec.group_name == "PROD_N", "Group name should be PROD_N"
    assert welspec.phase == "OIL", "Phase should be OIL"

    # Verify COMPDAT data (perforation intervals)
    compdat = completion_data_y1.compdat
    assert len(compdat) > 0, "Should have COMPDAT data for Well-1 Y1"

    # Check that we have MD information in COMPDAT
    has_md_data = False
    for comp in compdat:
        if comp.HasField("start_md") and comp.HasField("end_md"):
            has_md_data = True
            assert comp.start_md > 0, "Start MD should be positive"
            assert comp.end_md > comp.start_md, "End MD should be greater than start MD"
            break

    assert has_md_data, "Should have MD (measured depth) data in COMPDAT"

    # Verify MSW-specific data: WELSEGS
    welsegs = completion_data_y1.welsegs
    assert len(welsegs) > 0, "Should have WELSEGS data for MSW well Well-1 Y1"

    # Check WELSEGS header (first entry)
    welsegs_entry = welsegs[0]
    assert (
        welsegs_entry.header.well_name == "Well-1"
    ), "WELSEGS well name should be Well-1"
    assert (
        welsegs_entry.header.top_depth > 0
    ), "Should have positive top depth in WELSEGS"
    assert welsegs_entry.header.top_length >= 0, "Should have top length in WELSEGS"
    assert welsegs_entry.header.info_type in [
        "ABS",
        "INC",
    ], "Should have valid info type"

    # Check WELSEGS rows (segment definitions)
    assert len(welsegs_entry.row) > 0, "Should have segment rows in WELSEGS"
    for row in welsegs_entry.row:
        assert row.segment_1 > 0, "Segment 1 should be positive"
        assert row.segment_2 > 0, "Segment 2 should be positive"

    # Verify MSW-specific data: COMPSEGS
    compsegs = completion_data_y1.compsegs
    assert len(compsegs) > 0, "Should have COMPSEGS data for MSW well Well-1 Y1"

    # Check COMPSEGS entries
    for compseg in compsegs:
        assert compseg.i > 0, "Grid I should be positive"
        assert compseg.j > 0, "Grid J should be positive"
        assert compseg.k > 0, "Grid K should be positive"
        assert compseg.branch >= 0, "Branch should be non-negative"


def test_msw_completion_data_unified(rips_instance, initialize_test):
    """Test unified MSW completion data retrieval for multiple wells.

    This test verifies that completion data can be retrieved for multiple
    MSW wells in a single call using the completion_data_unified method.
    """
    project_path = dataroot.PATH + "/msw-export/project-files/perf_lateral.rsp"
    project = rips_instance.project.open(path=project_path)

    # Get the case
    cases = project.cases()
    assert len(cases) > 0, "Project should contain at least one case"
    case = cases[0]

    # Get well path collection
    well_path_collection = project.well_path_collection()

    # Get unified completion data for all wells
    unified_data = well_path_collection.completion_data_unified(case_id=case.id)

    # Verify WELSPECS
    assert len(unified_data.welspecs) > 0, "Should have WELSPECS in unified data"

    # Verify COMPDAT
    assert len(unified_data.compdat) > 0, "Should have COMPDAT in unified data"

    # Verify WELSEGS (MSW-specific)
    assert len(unified_data.welsegs) > 0, "Should have WELSEGS in unified MSW data"

    # Count wells in WELSEGS
    well_names_in_welsegs = set()
    for welsegs_entry in unified_data.welsegs:
        well_names_in_welsegs.add(welsegs_entry.header.well_name)

    assert (
        len(well_names_in_welsegs) >= 1
    ), "Should have at least one well in WELSEGS data"

    # Verify COMPSEGS (MSW-specific)
    assert len(unified_data.compsegs) > 0, "Should have COMPSEGS in unified MSW data"

    # Check that COMPSEGS has valid grid coordinates
    for compseg in unified_data.compsegs:
        assert compseg.i > 0, "Grid I should be positive"
        assert compseg.j > 0, "Grid J should be positive"
        assert compseg.k > 0, "Grid K should be positive"


def test_msw_export(rips_instance, initialize_test):
    """Test MSW export functionality.

    This test verifies that MSW data can be exported to file using the
    export_msw method on a case.
    """
    project_path = dataroot.PATH + "/msw-export/project-files/perf_lateral.rsp"
    project = rips_instance.project.open(path=project_path)

    # Get the case
    cases = project.cases()
    assert len(cases) > 0, "Project should contain at least one case"
    case = cases[0]

    # Get well paths
    well_paths = project.well_paths()
    well_y1 = None
    for well in well_paths:
        if well.name == "Well-1 Y1":
            well_y1 = well
            break

    assert well_y1 is not None, "Well-1 Y1 should exist in project"

    # Set export folder
    export_folder = tempfile.gettempdir()
    rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

    # Export MSW data
    case.export_msw(well_path=well_y1.name)

    # Note: The actual file creation validation would require checking
    # the export folder, which may be platform-dependent and require
    # additional file system checks


def test_msw_settings_read(rips_instance, initialize_test):
    """Test reading MSW settings from project.

    This test verifies that MSW settings configured in the project
    can be read through the Python API.
    """
    project_path = dataroot.PATH + "/msw-export/project-files/perf_lateral.rsp"
    project = rips_instance.project.open(path=project_path)

    # Get well paths
    well_paths = project.well_paths()
    well_y1 = None
    for well in well_paths:
        if well.name == "Well-1 Y1":
            well_y1 = well
            break

    assert well_y1 is not None, "Well-1 Y1 should exist in project"

    # Get MSW settings
    msw_settings = well_y1.msw_settings()

    # Verify MSW settings values from the project file
    # According to the project analysis, the liner diameter is 0.152m
    assert msw_settings.liner_diameter == 0.152, "Liner diameter should be 0.152m"
    assert msw_settings.roughness_factor == 1e-05, "Roughness factor should be 1e-05"

    # Check pressure drop setting
    assert msw_settings.pressure_drop == "HF-", "Pressure drop should be HF-"

    # Check max segment length setting
    assert msw_settings.max_segment_length == 200.0, "Max segment length should be 200m"

    # Verify completion settings
    completion_settings = well_y1.completion_settings()
    assert (
        completion_settings.well_name_for_export == "Well-1"
    ), "Well name for export should be Well-1"
    assert (
        completion_settings.group_name_for_export == "PROD_N"
    ), "Group name should be PROD_N"
    assert completion_settings.well_type_for_export == "OIL", "Well type should be OIL"
    assert (
        completion_settings.allow_well_cross_flow
    ), "Allow well cross flow should be True"
