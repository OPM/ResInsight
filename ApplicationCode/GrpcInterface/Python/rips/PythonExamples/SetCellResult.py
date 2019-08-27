######################################################################
# This script applies a cell result to the first view in the project
######################################################################
import rips

resInsight  = rips.Instance.find()

view = resInsight.project.view(0)
view.applyCellResult(resultType='STATIC_NATIVE', resultVariable='DX')
