import rips

# Connect to ResInsight instance
resInsight = rips.Instance.find()

# Clear any existing MSW grouping pattern to avoid interference with the import
well_path_coll = resInsight.project.well_path_collection()
well_path_coll.set_msw_name_grouping("")

well1 = well_path_coll.import_well_path(
    "f:/scratch/2026-well-path-import/Well-1_Y1.dev"
)
well2 = well_path_coll.import_well_path(
    "f:/scratch/2026-well-path-import/Well-1_Y2.dev"
)

# Create lateral well path from geometry of another well path
well1.append_lateral_from_geometry(well2)
