###############################################################################
# Compare multiple ensembles in the same plot, and add more subplots to an
# existing summary plot
#
###############################################################################

import sys

import rips

resinsight = rips.Instance.find()
project = resinsight.project

ensembles = project.summary_ensembles()

if len(ensembles) < 2:
    print("At least two summary ensembles are required")
    sys.exit(1)

summary_plot_collection = project.summary_plot_collection()

# One curve set per ensemble is created in the same subplot
plot = summary_plot_collection.new_summary_plot(ensembles=ensembles, address="FOPT")

# Append more subplots to the multi plot containing the first subplot
multi_plot = project.descendants(rips.MultiSummaryPlot)[-1]
multi_plot.append_subplot(ensembles=ensembles, address="FWPT")
multi_plot.append_subplot(ensembles=ensembles, address="FGPT")

multi_plot.number_of_columns = 2
multi_plot.update()
