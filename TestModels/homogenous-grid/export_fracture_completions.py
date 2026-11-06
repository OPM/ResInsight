"""
Create a horizontal well path through the center of HOMOGENEOUS_20X20X20.GRDECL, add a fracture in the
center of the model and export completions with a 15 m perforation interval for the fracture
orientations along well path, transverse to well path, and azimuth with the fracture plane along,
transverse and at 45 degrees to the well path. An additional transverse azimuth variant is exported
without user-defined perforation length.

All variants are exported for two fracture templates: the default elliptical fracture template, and a
StimPlan fracture template (UNIFORM_STIMPLAN_FRACTURE.xml) with uniform properties in all cells.

Usage:
    python export_fracture_completions.py [export_folder]

Uses a running ResInsight instance if present, otherwise launches a dedicated instance
(RESINSIGHT_EXECUTABLE must be set). The project is closed before each variant, which is then written
to <export_folder>/<template>/<variant>, containing the completion export, the multi-segment well (MSW)
export and the project file.
"""

import os
import sys

import rips

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
GRID_FILE = os.path.join(SCRIPT_DIR, "HOMOGENEOUS_20X20X20.GRDECL")

# Rectangular fracture with half length 100 m and height 75 m centered at the well depth.
# Conductivity 1000 md-m and width 0.01 m in all cells, matching the default ellipse template
STIMPLAN_FILE = os.path.join(SCRIPT_DIR, "UNIFORM_STIMPLAN_FRACTURE.xml")

TEMPLATES = ["ellipse", "stimplan"]

# Grid is 20x20x20 cells of 100 x 100 x 10 m, origin at (0, 0), top at 2000 m depth
CENTER_X = 1050.0
CENTER_Y = 1050.0
CENTER_DEPTH = 2105.0

WELL_START_X = -1000.0
WELL_END_X = 3000.0
FRACTURE_MD = CENTER_X - WELL_START_X

PERFORATION_LENGTH = 15.0

# The well runs along the x-axis (well azimuth 90 deg). The fracture azimuth is the strike of the fracture plane
WELL_AZIMUTH = 90.0

# (label, template orientation, fracture azimuth angle, use user-defined perforation length)
VARIANTS = [
    ("along", "Longitudinal", None, True),
    ("transverse", "Transverse", None, True),
    ("azimuth_along", "Azimuth", WELL_AZIMUTH, True),
    ("azimuth_transverse", "Azimuth", WELL_AZIMUTH - 90.0, True),
    ("azimuth_between", "Azimuth", WELL_AZIMUTH - 45.0, True),
    ("azimuth_transverse_no_perf", "Azimuth", WELL_AZIMUTH - 90.0, False),
]


def main(variants=VARIANTS, default_export_folder="export"):
    export_root = (
        sys.argv[1]
        if len(sys.argv) > 1
        else os.path.join(SCRIPT_DIR, default_export_folder)
    )

    # Use a running instance if present, otherwise launch a dedicated instance that is closed when done
    try:
        resinsight = rips.Instance.find()
    except rips.RipsError:
        resinsight = None
    launched = resinsight is None
    if launched:
        resinsight = rips.Instance.launch(console=True)
    if resinsight is None:
        raise RuntimeError("Could not connect to or launch ResInsight")

    try:
        export_variants(resinsight, export_root, variants)
    finally:
        if launched:
            resinsight.exit()


def export_variants(resinsight, export_root, variants):
    for template_kind in TEMPLATES:
        for label, orientation, azimuth_angle, use_perforation_length in variants:
            variant_folder = os.path.join(export_root, template_kind, label)
            export_variant(
                resinsight,
                template_kind,
                label,
                orientation,
                azimuth_angle,
                use_perforation_length,
                variant_folder,
            )
            print(
                "Exported {} {} fracture to {}".format(
                    template_kind, label, variant_folder
                )
            )


def export_variant(
    resinsight,
    template_kind,
    label,
    orientation,
    azimuth_angle,
    use_perforation_length,
    variant_folder,
):
    # Start each variant from an empty project
    resinsight.project.close()
    project = resinsight.project

    case = project.load_case(path=GRID_FILE)

    # Horizontal well path along the x-axis through the center of the model. z is elevation (negative depth)
    well_path = project.well_path_collection().import_well_path_from_points(
        name="FracWell",
        coordinates=[
            [WELL_START_X, CENTER_Y, -CENTER_DEPTH],
            [WELL_END_X, CENTER_Y, -CENTER_DEPTH],
        ],
    )

    fracture_template = create_fracture_template(project, template_kind)

    fracture_template.orientation = orientation
    fracture_template.non_darcy_flow_type = "Computed"
    fracture_template.perforation_length = PERFORATION_LENGTH
    fracture_template.user_defined_perforation_length = use_perforation_length
    if azimuth_angle is not None:
        fracture_template.azimuth_angle = azimuth_angle
    fracture_template.update()

    if template_kind == "stimplan":
        fracture = well_path.add_fracture(
            measured_depth=FRACTURE_MD, stim_plan_fracture_template=fracture_template
        )
    else:
        # Without a template argument, the fracture is assigned the first template matching the unit
        # system, which is the default ellipse template
        fracture = well_path.add_fracture(measured_depth=FRACTURE_MD)
    if fracture is None:
        raise RuntimeError("Could not add fracture to well path")

    os.makedirs(variant_folder, exist_ok=True)

    create_well_path_intersection_view(case, well_path)

    case.export_completions(
        well_paths=[well_path],
        time_step=0,
        export_folder=variant_folder,
        file_split=rips.CompletionExportSplit.UNIFIED_FILE,
        custom_file_name="fracture_{}.sch".format(label),
        include_msw=True,
    )

    project.save(path=os.path.join(variant_folder, "fracture_{}.rsp".format(label)))


def create_fracture_template(project, template_kind):
    if template_kind == "stimplan":
        template_collection = project.descendants(rips.FractureTemplateCollection)[0]
        template = template_collection.append_fracture_template(file_path=STIMPLAN_FILE)
        if template is None:
            raise RuntimeError("Could not import StimPlan template " + STIMPLAN_FILE)
        return template

    # A new project has a default elliptical fracture template
    templates = [
        t
        for t in project.descendants(rips.FractureTemplate)
        if "Ellipse" in t.user_description
    ]
    if not templates:
        raise RuntimeError("No elliptical fracture template found in project")
    return templates[0]


def create_well_path_intersection_view(case, well_path):
    view = case.create_view()
    view.mesh_mode = rips.MeshMode.NO_MESH
    view.surface_mode = rips.SurfaceMode.NO_SURFACE
    view.update()

    intersection_coll = view.descendants(rips.IntersectionCollection)[0]
    intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    intersection.name = well_path.name
    intersection.type = "CS_WELL_PATH"
    intersection.well_path = well_path
    intersection.update()

    return view


if __name__ == "__main__":
    main()
