######################################################################
# This script sets values for all grid cells in the first case in the project
# The script is intended to be used for TEST10K_FLT_LGR_NNC.EGRID
# This grid case contains one LGR
######################################################################
import rips

resinsight = rips.Instance.find()

project = resinsight.project

coordsv = []
zcornsv = []
actnumsv = []

grid_cell_count = 129
values = []
for i in range(0, grid_cell_count):
    coordsv.append(float(i) * 1)
    zcornsv.append(float(i) * 2)
    actnumsv.append(float(i) * 3)

project.create_corner_point_grid(coordsv, zcornsv, actnumsv)

