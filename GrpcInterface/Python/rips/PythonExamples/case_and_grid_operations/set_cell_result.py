######################################################################
# This script applies a cell result to the first view in the project
######################################################################
import rips

resinsight = rips.Instance.find()

view = resinsight.project.views()[0]
view.apply_cell_result(result_type="STATIC_NATIVE", result_variable="DX")
