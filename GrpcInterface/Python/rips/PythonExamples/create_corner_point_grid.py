######################################################################
# This script creates a corner point grid from a Eclipse coord, zcorn
# and actnum configuration.
######################################################################
import rips
from xtgeo.grid3d._egrid import EGrid
from xtgeo.io._file import FileFormat
import numpy as np

grid_filepath = "/home/resinsight/testdata/01_drogon_ahm/realization-0/iter-0/eclipse/model/DROGON-0.EGRID"

name = "DROGON-0 from python"

grid = EGrid.from_file(grid_filepath, fileformat=FileFormat.EGRID)
print("Grid: ", grid)
print("Grid type: ", type(grid))
print("coord: ", grid.coord.shape, grid.coord.dtype)
print("zcorn:", grid.zcorn.shape, grid.zcorn.dtype)
print("actnum: ", grid.actnum.shape, grid.actnum.dtype)

resinsight = rips.Instance.find()

project = resinsight.project

coord = np.ascontiguousarray(grid.coord, dtype=np.float32)
zcorn = np.ascontiguousarray(grid.zcorn, dtype=np.float32)
actnum = np.ascontiguousarray(grid.actnum, dtype=np.int32)


print("coordsv: ", len(coord), type(coord[0]))
print("zcornsv: ", len(zcorn), type(zcorn[0]))
print("actnumsv: ", len(actnum), type(actnum[0]))


print("Grid dimensions: ", grid.dimensions)
nx = grid.dimensions.ncol
ny = grid.dimensions.nrow
nz = grid.dimensions.nlay

project.create_corner_point_grid(name, nx, ny, nz, coord, zcorn, actnum)
