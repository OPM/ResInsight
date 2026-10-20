######################################################################
# This script applies a geomechanical cell result to the first view
# of the first GeoMech case in the project
######################################################################
import rips

resinsight = rips.Instance.find()

cases = resinsight.project.descendants(rips.GeoMechCase)
if not cases:
    print("No GeoMech cases in project")
    exit(1)

view = cases[0].views()[0]

cell_result = view.cell_result()
cell_result.result_position_type = "ELEMENT_NODAL"
cell_result.result_field_name = "SE"
cell_result.result_component_name = "S33"
cell_result.update()
