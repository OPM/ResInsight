import os
import shutil
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot

SOURCE_DIR = dataroot.PATH + "/flow_diagnostics_test"
SUMMARY_FILES = ["SIMPLE_SUMMARY2.SMSPEC", "SIMPLE_SUMMARY2.UNSMRY"]
REALIZATION_COUNT = 3


def _create_ensemble_files(root_dir, name):
    """Copy the summary files into realization-N/iter-0 folders"""
    smspec_files = []
    for real in range(REALIZATION_COUNT):
        folder = os.path.join(str(root_dir), name, f"realization-{real}", "iter-0")
        os.makedirs(folder)
        for file_name in SUMMARY_FILES:
            shutil.copy(os.path.join(SOURCE_DIR, file_name), folder)
        smspec_files.append(os.path.abspath(os.path.join(folder, SUMMARY_FILES[0])))
    return smspec_files


def _import_ensemble(rips_instance, root_dir, name):
    files = _create_ensemble_files(root_dir, name)
    return rips_instance.project.import_summary_ensemble(
        file_names=files, ensemble_name=name
    )


def test_import_ensemble_and_list_cases(rips_instance, initialize_test, tmp_path):
    ensemble = _import_ensemble(rips_instance, tmp_path, "ens_a")

    assert ensemble.is_ensemble
    assert ensemble.summary_collection_name == "ens_a"
    assert len(ensemble.summary_cases()) == REALIZATION_COUNT


def test_vector_values(rips_instance, initialize_test, tmp_path):
    ensemble = _import_ensemble(rips_instance, tmp_path, "ens_a")

    values = ensemble.vector_values("FOPT", "NONE")
    assert len(values) == REALIZATION_COUNT
    for data in values.values():
        assert len(data["values"]) == 60
        assert len(data["time_steps"]) == 60


def test_set_vector_after_project_reload(rips_instance, initialize_test, tmp_path):
    _import_ensemble(rips_instance, tmp_path, "ens_a")
    project_path = str(tmp_path / "ensemble.rsp")
    rips_instance.project.save(project_path)
    rips_instance.project.close()
    rips_instance.project.open(project_path)

    ensemble = rips_instance.project.summary_ensembles()[0]
    summary_case = ensemble.summary_cases()[0]
    count = len(summary_case.available_time_steps().values)

    # Realization addresses are created lazily after a project is loaded
    summary_case.set_summary_values("FWCT_N", "", [0.5] * count)
    with pytest.raises(rips.RipsError):
        summary_case.set_summary_values("FOPT", "", [0.5] * count)


def test_multiple_ensembles_in_one_subplot(rips_instance, initialize_test, tmp_path):
    ensemble_a = _import_ensemble(rips_instance, tmp_path, "ens_a")
    ensemble_b = _import_ensemble(rips_instance, tmp_path, "ens_b")

    collection = rips_instance.project.summary_plot_collection()
    plot = collection.new_summary_plot(
        ensembles=[ensemble_a, ensemble_b], address="FOPT"
    )
    assert plot is not None

    curve_sets = plot.descendants(rips.EnsembleCurveSet)
    assert len(curve_sets) == 2
    assert len(rips_instance.project.descendants(rips.MultiSummaryPlot)) == 1


def test_append_subplot(rips_instance, initialize_test, tmp_path):
    ensemble = _import_ensemble(rips_instance, tmp_path, "ens_a")

    collection = rips_instance.project.summary_plot_collection()
    collection.new_summary_plot(ensemble=ensemble, address="FOPT")

    multi_plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[0]
    assert len(multi_plot.descendants(rips.SummaryPlot)) == 1

    new_plot = multi_plot.append_subplot(ensembles=[ensemble], address="FWPT")
    assert new_plot is not None
    assert len(rips_instance.project.descendants(rips.MultiSummaryPlot)) == 1
    assert len(multi_plot.descendants(rips.SummaryPlot)) == 2

    with pytest.raises(Exception):
        multi_plot.append_subplot()


def test_new_summary_multi_plot(rips_instance, initialize_test, tmp_path):
    ensemble = _import_ensemble(rips_instance, tmp_path, "ens_a")

    collection = rips_instance.project.summary_plot_collection()
    multi_plot = collection.new_summary_multi_plot(
        ensembles=[ensemble], addresses=["FOPT", "FWPT"]
    )
    assert multi_plot is not None
    assert len(rips_instance.project.descendants(rips.MultiSummaryPlot)) == 1
    assert len(multi_plot.descendants(rips.SummaryPlot)) == 2

    with pytest.raises(Exception):
        collection.new_summary_multi_plot(ensembles=[ensemble])


def test_project_summary_ensembles(rips_instance, initialize_test, tmp_path):
    assert rips_instance.project.summary_ensembles() == []

    ensemble = _import_ensemble(rips_instance, tmp_path, "ens_a")
    ensembles = rips_instance.project.summary_ensembles()
    assert len(ensembles) == 1
    assert ensembles[0].summary_collection_name == ensemble.summary_collection_name


def test_project_summary_plot_collection(rips_instance, initialize_test):
    collection = rips_instance.project.summary_plot_collection()
    assert isinstance(collection, rips.SummaryPlotCollection)
