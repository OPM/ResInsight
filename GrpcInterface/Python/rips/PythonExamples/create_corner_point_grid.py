######################################################################
# This script sets values for all grid cells in the first case in the project
# The script is intended to be used for TEST10K_FLT_LGR_NNC.EGRID
# This grid case contains one LGR
######################################################################
import rips
import xtgeo
import numpy as np

grid_filepath = "/home/resinsight/testdata/01_drogon_ahm/realization-0/iter-0/eclipse/model/DROGON-0.EGRID"
grid = xtgeo.grid_from_file(grid_filepath)


print("Grid: ", grid)
print("Coordsv: ", grid._coordsv.shape, grid._coordsv.dtype)
print("zcordsv:", grid._zcornsv.shape, grid._zcornsv.dtype)
print("actnumsv: ", grid._actnumsv.shape, grid._actnumsv.dtype)

resinsight = rips.Instance.find()

project = resinsight.project

coordsv = np.ascontiguousarray(grid._coordsv, dtype=np.float32).flatten().tolist()
zcornsv = np.ascontiguousarray(grid._zcornsv, dtype=np.float32).flatten().tolist()
actnumsv = np.ascontiguousarray(grid._actnumsv, dtype=np.float32).flatten().tolist()

print("Length of coordsv: ", len(coordsv), type(coordsv), type(coordsv[0]))
project.create_corner_point_grid(coordsv, zcornsv, actnumsv)

