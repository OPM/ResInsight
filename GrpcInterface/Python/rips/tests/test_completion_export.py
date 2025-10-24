import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

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
    assert os.path.exists(
        export_file_path
    ), f"Export {file_description} does not exist: {export_file_path}"

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
