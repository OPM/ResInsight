###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  get the completion data for the first well found
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

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
    print("Well path name: " + well.name)

    print("WELSPECS")

    welspecs = well.completion_data(the_case.id).welspecs

    for line in welspecs:
        txt = line.well_name + "  "
        txt += line.group_name + "  "
        txt += str(line.grid_i) + "  "
        txt += str(line.grid_j) + "  "

        if line.HasField("bhp_depth"):
            txt += str(line.bhp_depth) + "  "
        else:
            txt += "1*  "

        txt += line.phase + "  "

        if line.HasField("drainage_radius"):
            txt += str(line.drainage_radius) + "  "
        else:
            txt += "1*  "

        if line.HasField("inflow_equation"):
            txt += line.inflow_equation + "  "
        else:
            txt += "1*  "

        if line.HasField("auto_shut_in"):
            txt += line.auto_shut_in + "  "
        else:
            txt += "1*  "

        if line.HasField("cross_flow"):
            txt += line.cross_flow + "  "
        else:
            txt += "1*  "

        if line.HasField("pvt_num"):
            txt += str(line.pvt_num) + "  "
        else:
            txt += "1*  "

        if line.HasField("hydrostatic_density_calc"):
            txt += line.hydrostatic_density_calc + "  "
        else:
            txt += "1*  "

        if line.HasField("fip_region"):
            txt += str(line.fip_region) + "  "
        else:
            txt += "1*  "

        print(txt)

    print("/\n")

    compdat = well.completion_data(the_case.id).compdat

    print("COMPDAT")

    for line in compdat:
        txt = ""

        if line.HasField("start_md"):
            txt += "-- MD In " + str(line.start_md) + "  MD Out " + str(line.end_md) + "\n"
        txt += "   "
        txt += line.well_name + "  "
        txt += str(line.grid_i) + "  "
        txt += str(line.grid_j) + "  "
        txt += str(line.upper_k) + "  "
        txt += str(line.lower_k) + "  "
        txt += line.open_shut_flag + "  "
        if line.HasField("saturation"):
            txt += str(line.saturation) + "  "
        else:
            txt += "1*  "

        txt += str(line.transmissibility) + "  "
        txt += str(line.diameter) + "  "
        txt += str(line.kh) + "  "
        if line.HasField("skin_factor"):
            txt += str(line.skin_factor) + "  "
        else:
            txt += "1*  "
        if line.HasField("d_factor"):
            txt += str(line.d_factor) + "  "
        else:
            txt += "1*  "   
        txt += "'%s'" % line.direction

        print(txt)

    print("/")
