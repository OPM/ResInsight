######################################################################
# This script applies a cell result to the first view in the project
######################################################################
import rips

resinsight  = rips.Instance.find()

view = resinsight.project.view(0)
view.apply_cell_result(resultType='STATIC_NATIVE', resultVariable='DX')
