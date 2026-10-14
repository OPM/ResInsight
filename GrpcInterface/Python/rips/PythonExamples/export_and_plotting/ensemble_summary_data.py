###############################################################################
# Read ensemble summary vectors for all realizations
###############################################################################

import sys

import rips

resinsight = rips.Instance.find()
project = resinsight.project

ensembles = project.summary_ensembles()

if not ensembles:
    print("No summary ensemble found in the current project")
    sys.exit(1)

ensemble = ensembles[0]

values = ensemble.vector_values("FOPT", time_resolution="YEAR")
for real, data in values.items():
    print(real, data["values"])
