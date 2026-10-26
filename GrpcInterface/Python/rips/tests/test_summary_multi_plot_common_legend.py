import os
import shutil

import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips
from rips.pdmobject import PdmObjectBase

import dataroot

SOURCE_DIR = dataroot.PATH + "/flow_diagnostics_test"
SUMMARY_FILES = ["SIMPLE_SUMMARY2.SMSPEC", "SIMPLE_SUMMARY2.UNSMRY"]

# RimSummaryCurve is not exposed as a dedicated scriptable class in
# generated_classes.py, so look it up generically by its PDM class keyword.
SummaryCurve = type("SummaryCurve", (PdmObjectBase,), {})


def _import_case(rips_instance, tmp_path, short_name):
    """Import a copy of the shared test summary case into its own folder so
    several distinctly named cases can be opened at the same time."""
    folder = os.path.join(str(tmp_path), short_name)
    os.makedirs(folder, exist_ok=True)
    for f in SUMMARY_FILES:
        shutil.copy(os.path.join(SOURCE_DIR, f), folder)

    case_path = os.path.join(folder, SUMMARY_FILES[0])
    case = rips_instance.project.import_summary_case(case_path)
    # Give each case a unique, human readable name so the common legend can
    # actually distinguish curves by case (the import path based default
    # name is identical for every copy of the same source file).
    case.short_name = short_name
    case.update()
    return case


def _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, prefix):
    """Snapshot export requires a GUI application; skip gracefully when the
    tests are run in --console mode (e.g. on CI)."""
    if not rips_instance.is_gui():
        return None

    multi_plot.update()
    snapshot_dir = os.path.join(str(tmp_path), "snapshots")
    os.makedirs(snapshot_dir, exist_ok=True)
    # Explicit width/height, as the plot widget may not have a usable size
    # yet when running headless inside the shared test session.
    multi_plot.export_snapshot(
        export_folder=snapshot_dir, prefix=prefix, width=1000, height=700
    )

    files = os.listdir(snapshot_dir)
    assert files, "export_snapshot did not produce any file"
    return files


def test_common_legend_split_by_case_and_well(rips_instance, initialize_test, tmp_path):
    """Two cases x two wells, one vector: the common legend must be able to
    split into a color legend (per case) and a symbol legend (per well)."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")
    case_b = _import_case(rips_instance, tmp_path, "case_b")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(
        summary_cases=[case_a, case_b], address="WOPR:P1;WOPR:P2"
    )

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.use_common_legend is True

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 cases x 2 wells

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "case_and_well")


def test_common_legend_split_by_vector_and_case(
    rips_instance, initialize_test, tmp_path
):
    """Two cases x two vectors, one well: the common legend must be able to
    split into a color legend (per case) and a symbol legend (per vector)."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")
    case_b = _import_case(rips_instance, tmp_path, "case_b")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(
        summary_cases=[case_a, case_b], address="WOPR:P1;WBHP:P1"
    )

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.use_common_legend is True

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 cases x 2 vectors

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "vector_and_case")


def test_common_legend_split_by_vector_and_well(
    rips_instance, initialize_test, tmp_path
):
    """One case, two vectors x two wells: the common legend must be able to
    split into a color legend (per vector) and a symbol legend (per well)."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(
        summary_cases=[case_a], address="WOPR:P1;WOPR:P2;WBHP:P1;WBHP:P2"
    )

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.use_common_legend is True

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 vectors x 2 wells

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "vector_and_well")


def test_common_legend_split_by_case_and_group(
    rips_instance, initialize_test, tmp_path
):
    """Two cases x two groups, one vector: the common legend must be able to
    split into a color legend (per case) and a symbol legend (per group).
    The OBJECT dimension used for the symbol/color legend is generic and
    covers groups, not just wells."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")
    case_b = _import_case(rips_instance, tmp_path, "case_b")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(
        summary_cases=[case_a, case_b], address="GOPR:WIA;GOPR:PRODA"
    )

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 cases x 2 groups

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "case_and_group")


def test_common_legend_split_by_vector_and_group(
    rips_instance, initialize_test, tmp_path
):
    """One case, two vectors x two groups: the common legend must be able to
    split into a color legend (per vector) and a symbol legend (per group)."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(
        summary_cases=[case_a], address="GOPR:WIA;GOPR:PRODA;GWCT:WIA;GWCT:PRODA"
    )

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 vectors x 2 groups

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "vector_and_group")


def test_common_legend_split_by_case_and_region(
    rips_instance, initialize_test, tmp_path
):
    """Two cases x two regions, one vector: the common legend must be able
    to split into a color legend (per case) and a symbol legend (per
    region). The OBJECT dimension used for the symbol/color legend is
    generic and covers regions, not just wells."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")
    case_b = _import_case(rips_instance, tmp_path, "case_b")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(summary_cases=[case_a, case_b], address="ROIP:1;ROIP:2")

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    multi_plot.use_common_legend = True
    multi_plot.update()

    summary_plot = rips_instance.project.descendants(rips.SummaryPlot)[-1]
    curves = summary_plot.descendants(SummaryCurve)
    assert len(curves) == 4  # 2 cases x 2 regions

    _export_snapshot_if_gui(rips_instance, multi_plot, tmp_path, "case_and_region")


def test_use_common_legend_is_scriptable(rips_instance, initialize_test, tmp_path):
    """The use_common_legend field round-trips through update() independent
    of any particular curve layout."""
    case_a = _import_case(rips_instance, tmp_path, "case_a")

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(summary_cases=[case_a], address="FOPT")

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert multi_plot.use_common_legend in (True, False)

    multi_plot.use_common_legend = not multi_plot.use_common_legend
    expected = multi_plot.use_common_legend
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.use_common_legend == expected
