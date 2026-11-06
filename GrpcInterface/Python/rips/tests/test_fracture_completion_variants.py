"""
Run the fracture completion export script in TestModels/homogenous-grid for all fracture templates and
orientation variants, and compare the exported completion and MSW files with the stored reference files.
"""

import math
import os
import re
import sys

TEST_MODEL_DIR = os.path.abspath(
    os.path.join(
        os.path.dirname(__file__),
        "..",
        "..",
        "..",
        "..",
        "TestModels",
        "homogenous-grid",
    )
)
EXPECTED_DIR = os.path.join(TEST_MODEL_DIR, "expected")
RELATIVE_TOLERANCE = 1e-6

# Absolute file paths, e.g. the grid model and the StimPlan fracture file
PATH_PATTERN = re.compile(r"(?:[A-Za-z]:)?[\\/]\S*[\\/](\S+)")
# Table separators like "------" whose width depends on column widths
SEPARATOR_PATTERN = re.compile(r"^[-=]+$")

sys.path.insert(0, TEST_MODEL_DIR)
import export_fracture_completions  # noqa: E402


def expected_files():
    files = []
    for root, _, names in os.walk(EXPECTED_DIR):
        for name in names:
            if name.endswith(".sch"):
                files.append(os.path.relpath(os.path.join(root, name), EXPECTED_DIR))
    return sorted(files)


def normalized_lines(file_path):
    lines = []
    with open(file_path) as f:
        for line in f:
            if line.startswith("-- Exported from ResInsight"):
                continue
            # Compare file names only, and ignore column alignment that depends on path lengths
            tokens = [
                "--" if SEPARATOR_PATTERN.match(token) else token
                for token in PATH_PATTERN.sub(r"\1", line).split()
            ]
            if tokens:
                lines.append(tokens)
    return lines


def tokens_equal(actual, expected):
    if actual == expected:
        return True
    try:
        return math.isclose(float(actual), float(expected), rel_tol=RELATIVE_TOLERANCE)
    except ValueError:
        return False


def assert_files_equal(actual_path, expected_path, relative_path):
    actual = normalized_lines(actual_path)
    expected = normalized_lines(expected_path)

    assert len(actual) == len(expected), "Line count differs in " + relative_path
    for line_index, (actual_tokens, expected_tokens) in enumerate(
        zip(actual, expected)
    ):
        assert len(actual_tokens) == len(expected_tokens) and all(
            tokens_equal(a, e) for a, e in zip(actual_tokens, expected_tokens)
        ), "{} differs at line {}:\n  actual:   {}\n  expected: {}".format(
            relative_path,
            line_index,
            " ".join(actual_tokens),
            " ".join(expected_tokens),
        )


def test_fracture_completion_variants(rips_instance, initialize_test, tmp_path):
    relative_paths = expected_files()
    # One completion file and one MSW file per template and variant
    assert len(relative_paths) == 2 * len(export_fracture_completions.TEMPLATES) * len(
        export_fracture_completions.VARIANTS
    )
    assert any(path.endswith("_MSW.sch") for path in relative_paths)

    export_fracture_completions.export_variants(
        rips_instance, str(tmp_path), export_fracture_completions.VARIANTS
    )

    for relative_path in relative_paths:
        actual_path = os.path.join(tmp_path, relative_path)
        assert os.path.exists(actual_path), "Missing export " + relative_path
        assert_files_equal(
            actual_path, os.path.join(EXPECTED_DIR, relative_path), relative_path
        )
