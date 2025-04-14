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
print("zcordsv:", grid._zcornsv.shape)
print("actnumsv: ", grid._actnumsv.shape)
#        self._zcornsv = zcornsv
#        self._actnumsv = actnumsv

resinsight = rips.Instance.find()

project = resinsight.project

coordsv = np.ascontiguousarray(grid._coordsv, dtype=np.float32).flatten().tolist()
zcornsv = []
actnumsv = []

grid_cell_count = 129
values = []
for i in range(0, grid_cell_count):
#    coordsv.append(float(i) * 1)
    zcornsv.append(float(i) * 2)
    actnumsv.append(float(i) * 3)


print("Length of coordsv: ", len(coordsv), type(coordsv), type(coordsv[0]))
project.create_corner_point_grid(coordsv, zcornsv, actnumsv)

