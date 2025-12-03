###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  get the completion data for the first well found
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips


# helper method to format printing for optional fields
def fieldValueOrDefaultText(grpc_object, optional_field_name: str):
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

    completion_data = well.completion_data(the_case.id)

    print("WELSPECS")

    welspecs = completion_data.welspecs
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
        txt += "/"

        print(txt)

    print("/\n")

    compdat = completion_data.compdat

    complump_entries = []

    print("COMPDAT")

    for line in compdat:
        txt = ""
        complump = ""

        if line.HasField("start_md"):
            txt += "-- Perforation MD In " + str(line.start_md)
            txt += ", MD Out " + str(line.end_md) + "--\n"

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
        txt += " /"

        if (line.HasField("completion_number")) and (line.completion_number > 0):
            complump += "   "
            complump += line.well_name + "  "
            complump += str(line.grid_i) + "  "
            complump += str(line.grid_j) + "  "
            complump += str(line.upper_k) + "  "
            complump += str(line.lower_k) + "  "
            complump += str(line.completion_number) + "  "
            complump += " /"

            complump_entries.append(complump)

        print(txt)

    print("/\n")

    if len(complump_entries) > 0:
        print("COMPLUMP")
        for complump_entry in complump_entries:
            print(complump_entry)
        print("/\n")
    else:
        print("-- No COMPLUMP entries --\n")

    print("WELSEGS")
    welsegs = completion_data.welsegs
    if (welsegs is None) or (len(welsegs) == 0):
        print("  -- No WELSEGS data --\n")
    else:
        for welsegs_entry in welsegs:
            # Print WELSEGS header
            header = welsegs_entry.header
            txt = "-- Header: " + header.well_name + "\n"
            txt += "   " + header.well_name + "  "
            txt += str(header.top_depth) + "  "
            txt += str(header.top_length) + "  "
            txt += fieldValueOrDefaultText(header, "wellbore_volume") + "  "
            txt += header.info_type + "  "
            txt += fieldValueOrDefaultText(header, "pressure_components") + "  "
            txt += fieldValueOrDefaultText(header, "flow_model")
            txt += " /"
            print(txt)

            # Print WELSEGS segment rows
            for row in welsegs_entry.row:
                txt = "   "
                txt += str(row.segment_1) + "  "
                txt += str(row.segment_2) + "  "
                txt += str(row.branch) + "  "
                txt += str(row.join_segment) + "  "
                txt += str(row.length) + "  "
                txt += str(row.depth) + "  "
                txt += fieldValueOrDefaultText(row, "diameter") + "  "
                txt += fieldValueOrDefaultText(row, "roughness") + "  "
                txt += " /"
                print(txt)

    print("/")

    # Print COMPSEGS (MSW completion segments)
    print("\nCOMPSEGS")
    compsegs = completion_data.compsegs
    if (compsegs is None) or (len(compsegs) == 0):
        print("  -- No COMPSEGS data --")
    else:
        for compseg in compsegs:
            txt = "   "
            txt += str(compseg.i) + "  "
            txt += str(compseg.j) + "  "
            txt += str(compseg.k) + "  "
            txt += str(compseg.branch) + "  "
            txt += str(compseg.distance_start) + "  "
            txt += str(compseg.distance_end) + "  "
            txt += fieldValueOrDefaultText(compseg, "grid_name")
            txt += " /"
            print(txt)

    print("/\n")
