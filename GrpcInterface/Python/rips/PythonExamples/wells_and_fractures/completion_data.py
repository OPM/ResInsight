###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  get the completion data for the first well found
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# helper method to format printing for optional fields
def fieldValueOrDefaultText(grpc_object, optional_field_name : str):
    if not grpc_object.HasField(optional_field_name):
        return "1*"
    return str(grpc_object.__getattribute__(optional_field_name))


# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is None:
    exit(1)

# Get a list of all wells
wells = resinsight.project.well_paths()

# Get a list of all cases
cases = resinsight.project.cases()

# Use the first one
the_case = cases[0]
print("Using case " + the_case.name)

print("Got " + str(len(wells)) + " well paths: ")
for well in wells:
    print("Well path name: " + well.name + "\n\n")

    print("WELSPECS")

    welspecs = well.completion_data(the_case.id).welspecs

    for line in welspecs:
        txt = line.well_name + "  "
        txt += line.group_name + "  "
        txt += str(line.grid_i) + "  "
        txt += str(line.grid_j) + "  "
        txt += fieldValueOrDefaultText(line, "bhp_depth") + "  "
        txt += line.phase + "  "
        txt += fieldValueOrDefaultText(line, "drainage_radius") + "  "
        txt += fieldValueOrDefaultText(line, "inflow_equation") + "  "
        txt += fieldValueOrDefaultText(line, "auto_shut_in") + "  "
        txt += fieldValueOrDefaultText(line, "cross_flow") + "  "
        txt += fieldValueOrDefaultText(line, "pvt_num") + "  "
        txt += fieldValueOrDefaultText(line, "hydrostatic_density_calc") + "  "
        txt += fieldValueOrDefaultText(line, "fip_region") + "  "

        print(txt)

    print("/\n")

    compdat = well.completion_data(the_case.id).compdat

    print("COMPDAT")

    for line in compdat:
        txt = ""

        if line.HasField("start_md"):
            txt += "-- Perforation MD In " + str(line.start_md) 
            txt += ", MD Out " + str(line.end_md) + "\n"

        txt += "   "
        txt += line.well_name + "  "
        txt += str(line.grid_i) + "  "
        txt += str(line.grid_j) + "  "
        txt += str(line.upper_k) + "  "
        txt += str(line.lower_k) + "  "
        txt += line.open_shut_flag + "  "
        txt += fieldValueOrDefaultText(line, "saturation") + "  "
        txt += str(line.transmissibility) + "  "
        txt += str(line.diameter) + "  "
        txt += str(line.kh) + "  "
        txt += fieldValueOrDefaultText(line, "skin_factor") + "  "
        txt += fieldValueOrDefaultText(line, "d_factor") + "  "
        txt += "'%s'" % line.direction

        print(txt)

    print("/")
