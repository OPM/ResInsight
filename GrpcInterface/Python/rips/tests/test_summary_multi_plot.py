import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot
import rips


def _create_summary_multi_plot(rips_instance):
    """Import a summary case and create one summary plot, returning the
    newly created RimSummaryMultiPlot wrapper."""
    case_path = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(case_path)
    assert summary_case.id == 1

    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(summary_cases=[summary_case], address="FOPT")

    multi_plots = rips_instance.project.descendants(rips.MultiSummaryPlot)
    assert len(multi_plots) >= 1
    return multi_plots[-1]


def test_summary_multi_plot_layout_is_scriptable(rips_instance, initialize_test):
    """The column/row count fields on a summary multi-plot are exposed over
    gRPC and round-trip through update()."""
    multi_plot = _create_summary_multi_plot(rips_instance)

    # Fields are readable (default seeded from the Multi Plot Defaults
    # preference; the exact default is environment dependent).
    assert multi_plot.number_of_columns in (1, 2, 3, 4)
    assert multi_plot.rows_per_page in (1, 2, 3, 4)

    # Set, update, and verify the new values are persisted server-side.
    multi_plot.number_of_columns = 3
    multi_plot.rows_per_page = 2
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.number_of_columns == 3
    assert refetched.rows_per_page == 2


def test_summary_multi_plot_layout_override(rips_instance, initialize_test):
    """Forcing a 1x1 layout works regardless of the preference default —
    this is what a headless export uses to get deterministic output."""
    multi_plot = _create_summary_multi_plot(rips_instance)

    multi_plot.number_of_columns = 1
    multi_plot.rows_per_page = 1
    multi_plot.update()

    refetched = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]
    assert refetched.number_of_columns == 1
    assert refetched.rows_per_page == 1
