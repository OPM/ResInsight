import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot

# The generated data is written to a .gitignore'd folder 'completion_export_output'.
# The reference data is stored in 'completion_export_reference'.
# If changes are made to the export functionality that are intended to change the output,
# the reference data must be updated accordingly.


def normalize_content(text):
    """Normalize content by removing header until '-- Grid Model:' line and excluding file path from compare."""
    lines = text.split("\n")

    # Find the line containing "-- Grid Model:" and skip it and the following line
    start_index = 0
    for i, line in enumerate(lines):
        if "-- Grid Model:" in line:
            start_index = i + 2  # Skip "-- Grid Model:" and the line after it
            break

    if start_index >= len(lines):
        return ""

    normalized_lines = lines[start_index:]
    return "\n".join(normalized_lines)


def compare_exported_files_with_reference(
    export_folder, reference_folder, file_type_description
):
    """Compare all exported files with their corresponding reference files.

    Args:
        export_folder: Path to folder containing exported files
        reference_folder: Path to folder containing reference files
        file_type_description: Description for error messages (e.g., "unified export", "split by well")
    """
    exported_files = [
        f
        for f in os.listdir(export_folder)
        if os.path.isfile(os.path.join(export_folder, f))
    ]

    for filename in exported_files:
        export_file_path = os.path.join(export_folder, filename)
        reference_file_path = os.path.join(reference_folder, filename)
        compare_with_reference(
            export_file_path,
            reference_file_path,
            f"{file_type_description} file '{filename}'",
        )


def compare_with_reference(
    export_file_path, reference_file_path, file_description="file"
):
    """Compare an export file with reference data

    Args:
        export_file_path: Path to the exported file
        reference_file_path: Path to the reference file
        file_description: Description of the file for error messages

    Raises:
        AssertionError: If files don't match or don't exist
    """
    assert os.path.exists(export_file_path), (
        f"Export {file_description} does not exist: {export_file_path}"
    )

    with open(export_file_path, "r") as f:
        content = f.read()

    if os.path.exists(reference_file_path):
        with open(reference_file_path, "r") as ref_f:
            ref_content = ref_f.read()

        normalized_content = normalize_content(content)
        normalized_ref_content = normalize_content(ref_content)

        assert normalized_content == normalized_ref_content, (
            f"Export {file_description} differs from reference data. "
            f"Export length: {len(normalized_content)}, Reference length: {len(normalized_ref_content)}"
        )


def test_export_completion_files_unified(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    project_path = case_root_path + "/well_completions_export.rsp"
    project = rips_instance.project.open(path=project_path)

    export_folder = os.path.abspath(
        case_root_path + "/completion_export_output/unified_export"
    )
    os.makedirs(export_folder, exist_ok=True)

    try:
        rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

        case = project.cases()[0]

        well_names_to_use = ["Well-1"]

        case.export_well_path_completions(
            time_step=0,  # Use time step 0 instead of 1
            well_path_names=well_names_to_use,
            file_split="UNIFIED_FILE",
            include_perforations=True,
            include_fishbones=True,
            export_welspec=True,
            export_comments=False,
        )

        # Compare exported files with reference data
        reference_folder = (
            case_root_path + "/completion_export_reference/unified_export"
        )
        compare_exported_files_with_reference(
            export_folder, reference_folder, "unified export"
        )

    finally:
        pass  # Keep exported files in the output directory


def test_export_completion_files_split_by_well(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    project_path = case_root_path + "/well_completions_export.rsp"
    project = rips_instance.project.open(path=project_path)

    export_folder = os.path.abspath(
        case_root_path + "/completion_export_output/split_by_well"
    )
    os.makedirs(export_folder, exist_ok=True)

    try:
        rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

        case = project.cases()[0]

        # Use Well-1 for all tests
        test_wells = ["Well-1"]

        case.export_well_path_completions(
            time_step=0,  # Use time step 0 instead of 1
            well_path_names=test_wells,
            file_split="SPLIT_ON_WELL",
            include_perforations=True,
            include_fishbones=False,  # This matches the reference generation script
            export_comments=False,
        )

        # Compare exported files with reference data
        reference_folder = case_root_path + "/completion_export_reference/split_by_well"
        compare_exported_files_with_reference(
            export_folder, reference_folder, "split by well"
        )

    finally:
        pass  # Keep exported files in the output directory


def test_export_completion_well_entering_lgr_twice(rips_instance, initialize_test):
    """A well going into the LGR, out of it, and into it again must give one
    WELSPECL and one COMPDATL keyword, with rows for both perforations."""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case = rips_instance.project.load_case(
        path=case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert len(case.grids()) == 2

    rips_instance.project.import_well_paths(
        [case_root_path + "/wellpath_lgr_twice.dev"]
    )
    well_path = rips_instance.project.well_path_by_name("LgrWell")
    assert well_path is not None

    # The well path leaves the LGR between MD 300 and MD 500
    well_path.append_perforation_interval(
        start_md=100.0, end_md=290.0, diameter=0.2, skin_factor=0.0
    )
    well_path.append_perforation_interval(
        start_md=560.0, end_md=740.0, diameter=0.2, skin_factor=0.0
    )

    export_folder = os.path.abspath(
        case_root_path + "/completion_export_output/lgr_twice"
    )
    os.makedirs(export_folder, exist_ok=True)
    for old_file in os.listdir(export_folder):
        os.remove(os.path.join(export_folder, old_file))

    rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)
    case.export_well_path_completions(
        time_step=0,
        well_path_names=["LgrWell"],
        file_split="UNIFIED_FILE",
        include_perforations=True,
        include_fishbones=False,
        export_welspec=True,
        export_comments=False,
    )

    lgr_files = [
        f for f in os.listdir(export_folder) if f.endswith("_LGR") and "_MSW_" not in f
    ]
    assert len(lgr_files) == 1

    with open(os.path.join(export_folder, lgr_files[0]), "r") as f:
        lines = [line.strip() for line in f.read().split("\n")]

    assert lines.count("WELSPECL") == 1
    assert lines.count("COMPDATL") == 1

    # Both perforations give connections in the LGR, on both sides of the part outside the LGR
    compdatl_start = lines.index("COMPDATL")
    compdatl_end = lines.index("/", compdatl_start)
    rows = [
        line
        for line in lines[compdatl_start:compdatl_end]
        if line.startswith("LgrWell")
    ]
    assert len(rows) > 2
    assert all(row.split()[1] == "LGR1" for row in rows)


def test_export_completion_dual_porosity_model(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/dualporo-testcase"
    project_path = case_root_path + "/Well-completions-mail.rsp"
    project = rips_instance.project.open(path=project_path)

    export_folder = os.path.abspath(case_root_path + "/completion_export_output")
    os.makedirs(export_folder, exist_ok=True)

    try:
        rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

        case = project.cases()[0]

        # Use Well-1 for all tests
        test_wells = ["PH", "PV"]

        case.export_well_path_completions(
            time_step=0,  # Use time step 0 instead of 1
            well_path_names=test_wells,
            file_split="SPLIT_ON_WELL",
            include_perforations=True,
            include_fishbones=False,  # This matches the reference generation script
            export_comments=False,
        )

        # Compare exported files with reference data
        reference_folder = case_root_path + "/completion_export_reference"
        compare_exported_files_with_reference(
            export_folder,
            reference_folder,
            "dual porosity completion data from fracture cells",
        )

    finally:
        pass  # Keep exported files in the output directory
