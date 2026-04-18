######################################################################
# This script creates a discrete (integer) property for all active
# cells in the first case in the project.
#
# Passing data_type="INTEGER" flags the property as discrete regardless
# of the property name (previously this required the name to end with
# "NUM").
######################################################################
import rips

resinsight = rips.Instance.find()

case = resinsight.project.case(case_id=0)

# Derive a discrete region id from a continuous property.
poro = case.active_cell_property("STATIC_NATIVE", "PORO", 0)
region_ids = [int(value * 100) % 4 for value in poro]

print("Applying discrete values to active cells")
case.set_active_cell_property(
    region_ids, "GENERATED", "MY_REGION", 0, data_type="INTEGER"
)
