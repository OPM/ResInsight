import sys
import os
import pytest
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_loadProject(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    case = project.cases()[0]
    assert case is not None
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert case.id == 0
    cases = rips_instance.project.cases()
    assert len(cases) == 1


def test_well_log_plots(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    plots = project.plots()
    well_log_plots = []
    for plot in plots:
        if isinstance(plot, rips.WellLogPlot):
            assert plot.depth_type == "MEASURED_DEPTH"
            assert plot.depth_type == rips.DepthType.MEASURED_DEPTH
            well_log_plots.append(plot)
    assert len(well_log_plots) == 2

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        for well_log_plot in well_log_plots:
            well_log_plot.depth_type = rips.DepthType.TRUE_VERTICAL_DEPTH_RKB
            well_log_plot.update()
            assert well_log_plot.depth_type == "TRUE_VERTICAL_DEPTH_RKB"
            if rips_instance.is_gui():
                well_log_plot.export_snapshot(tmpdirname)
            well_log_plot.export_data_as_las(tmpdirname)
        files = os.listdir(tmpdirname)
        print(files)
        if rips_instance.is_gui():
            assert len(files) == 4
        else:
            assert len(files) == 2

    plots2 = project.plots()
    for plot2 in plots2:
        if isinstance(plot2, rips.WellLogPlot):
            assert plot2.depth_type == "TRUE_VERTICAL_DEPTH_RKB"


def test_loadGridCaseGroup(rips_instance, initialize_test):
    case_paths = []
    case_paths.append(dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
    case_paths.append(
        dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID"
    )
    grid_case_group = rips_instance.project.create_grid_case_group(
        case_paths=case_paths
    )
    assert grid_case_group is not None and grid_case_group.group_id == 0

    project = rips_instance.project
    groups = project.grid_case_groups()
    assert len(groups) == 1
    looked_up = project.grid_case_group(grid_case_group.group_id)
    assert looked_up is not None
    assert looked_up.group_id == grid_case_group.group_id
    assert project.grid_case_group(9999) is None


def test_save_project_round_trip(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    project = rips_instance.project
    project.load_case(case_path)
    assert len(project.cases()) == 1

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        project_file = os.path.join(tmpdirname, "round_trip.rsp")
        project.save(project_file)
        assert os.path.exists(project_file)

        project.close()
        assert len(rips_instance.project.cases()) == 0

        reopened = rips_instance.project.open(project_file)
        cases = reopened.cases()
        assert len(cases) == 1
        assert cases[0].name == "TEST10K_FLT_LGR_NNC"


def test_views_and_view_lookup(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)
    case.create_view()

    project = rips_instance.project
    views = project.views()
    assert len(views) >= 1

    first_view = views[0]
    looked_up = project.view(first_view.id)
    assert looked_up is not None
    assert looked_up.id == first_view.id
    assert project.view(999999) is None


def test_well_path_by_name(rips_instance, initialize_test):
    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)

    project = rips_instance.project
    well_path = project.well_path_by_name("Well Path A")
    assert well_path is not None
    assert well_path.name == "Well Path A"
    assert project.well_path_by_name("Nonexistent Well Path") is None


def test_plot_lookup(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    plots = project.plots()
    assert len(plots) >= 1

    first_plot = plots[0]
    looked_up = project.plot(first_plot.id)
    assert looked_up is not None
    assert looked_up.id == first_plot.id
    assert project.plot(999999) is None


def test_export_well_paths(rips_instance, initialize_test):
    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        rips_instance.set_export_folder(export_type="WELLPATHS", path=tmpdirname)
        rips_instance.project.export_well_paths(
            well_paths="Well Path A", md_step_size=10.0
        )
        exported = os.listdir(tmpdirname)
        assert any(name.endswith(".dev") for name in exported)


def test_scale_fracture_template_and_set_containment(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/small-completion-export-fractures.rsp"
    )
    template = project.descendants(rips.FractureTemplate)[0]
    assert template.height_scale_factor == 1.0
    # The half-length scale factor uses the legacy keyword "WidthScaleFactor".
    assert template.width_scale_factor == 1.0
    assert template.d_factor_scale_factor == 1.0
    assert template.conductivity_factor == 1.0

    project.scale_fracture_template(
        template_id=0, half_length=2.0, height=3.0, d_factor=4.0, conductivity=5.0
    )

    after_scale = project.descendants(rips.FractureTemplate)[0]
    assert after_scale.width_scale_factor == 2.0
    assert after_scale.height_scale_factor == 3.0
    assert after_scale.d_factor_scale_factor == 4.0
    assert after_scale.conductivity_factor == 5.0

    project.set_fracture_containment(template_id=0, top_layer=5, base_layer=10)

    # RimFractureContainment fields are not scriptable, so verify the change
    # made it through by saving the project and inspecting the .rsp XML.
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        out_path = os.path.join(tmpdirname, "containment.rsp")
        project.save(out_path)
        with open(out_path) as rsp:
            content = rsp.read()
        assert "<TopKLayer>5</TopKLayer>" in content
        assert "<BaseKLayer>10</BaseKLayer>" in content


def test_summary_cases(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    project = rips_instance.project
    assert project.summary_cases() == []

    summary_case = project.import_summary_case(case_path)
    cases = project.summary_cases()
    assert len(cases) == 1
    assert cases[0].id == summary_case.id


def test_import_formation_names(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)

    rips_instance.project.import_formation_names(
        formation_files=dataroot.PATH + "/20Layers.lyr"
    )

    available = case.available_properties(rips.PropertyType.FORMATION_NAMES)
    assert "Active Formation Names" in available


_MINIMAL_LAS = """~Version Information
 VERS.                 2.0 : CWLS log ASCII Standard - VERSION 2.0
 WRAP.                 NO  : One line per depth step
~Well Information
#MNEM.UNIT       Value      Description
#---------    -----------   ---------------
 STRT.M       1000.0       : Start Depth
 STOP.M       1002.0       : Stop Depth
 STEP.M          1.0       : Step
 NULL.        -999.25      : Null Value
 WELL.        RIPS_TEST    : WELL
~Curve Information
 DEPT.M               : Depth
 GR.GAPI              : Gamma Ray
~ASCII
 1000.0   50.0
 1001.0   55.0
 1002.0   60.0
"""


def test_import_well_log_files(rips_instance, initialize_test):
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        las_path = os.path.join(tmpdirname, "rips_test.las")
        with open(las_path, "w") as las_file:
            las_file.write(_MINIMAL_LAS)

        well_path_names = rips_instance.project.import_well_log_files(
            well_log_folder=tmpdirname
        )
        assert "RIPS_TEST" in well_path_names


def test_exportSnapshots(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.set_export_folder(export_type="SNAPSHOTS", path=tmpdirname)
        rips_instance.project.export_snapshots(width=640, height=480)
        print(os.listdir(tmpdirname))
        #        assert(len(os.listdir(tmpdirname)) > 0)
        for fileName in os.listdir(tmpdirname):
            assert os.path.splitext(fileName)[1] == ".png"
