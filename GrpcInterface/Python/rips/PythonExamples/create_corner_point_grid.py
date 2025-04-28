######################################################################
# This script sets values for all grid cells in the first case in the project
# The script is intended to be used for TEST10K_FLT_LGR_NNC.EGRID
# This grid case contains one LGR
######################################################################
import rips
import xtgeo
from xtgeo.grid3d._egrid import EGrid, RockModel
from xtgeo.io._file import FileFormat, FileWrapper
import numpy as np

grid_filepath = "/home/resinsight/testdata/01_drogon_ahm/realization-0/iter-0/eclipse/model/DROGON-0.EGRID"

grid = EGrid.from_file(grid_filepath, fileformat=FileFormat.EGRID)
print("Grid: ", grid)
print("Grid type: ", type(grid))
print("coord: ", grid.coord.shape, grid.coord.dtype)
print("zcorn:", grid.zcorn.shape, grid.zcorn.dtype)
print("actnum: ", grid.actnum.shape, grid.actnum.dtype)

print("zcorn", grid.zcorn.shape);

resinsight = rips.Instance.find()

project = resinsight.project

coordsv = np.ascontiguousarray(grid.coord, dtype=np.float32).flatten().tolist()
zcornsv = np.ascontiguousarray(grid.zcorn, dtype=np.float32).flatten().tolist()
actnumsv = np.ascontiguousarray(grid.actnum, dtype=np.float32).flatten().tolist()


print("coordsv: ", len(coordsv), type(coordsv[0]))
print("zcornsv: ", len(zcornsv), type(zcornsv[0]))
print("actnumsv: ", len(actnumsv), type(actnumsv[0]))


print("Grid dimensions: ", grid.dimensions)
nx = grid.dimensions.ncol
ny = grid.dimensions.nrow
nz = grid.dimensions.nlay

project.create_corner_point_grid(nx, ny, nz, coordsv, zcornsv, actnumsv)
