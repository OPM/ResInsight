######################################################################
# This script retrieves cell result data and alters it
######################################################################
import rips

resinsight  = rips.Instance.find()

view = resinsight.project.views()[0]
results = view.cell_result_data()
print ("Number of result values: ", len(results))

newresults = []
for i in range(0, len(results)):
    newresults.append(results[i] * -1.0)
view.set_cell_result_data(newresults)

    