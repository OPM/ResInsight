###############################################################################
# Create summary plots for an ensemble
#
###############################################################################

import sys

import rips

# Connect to a running ResInsight instance
resinsight = rips.Instance.find()

project = resinsight.project

# Summary ensembles and summary groups are both represented by
# SummaryCaseSubCollection. Use the 'is_ensemble' flag to pick the ensembles.
ensembles = [
    collection
    for collection in project.descendants(rips.SummaryCaseSubCollection)
    if collection.is_ensemble
]

if not ensembles:
    print("No summary ensemble found in the current project")
    sys.exit(1)

summary_plot_collection = project.descendants(rips.SummaryPlotCollection)[0]

for ensemble in ensembles:
    print("Creating plots for ensemble: " + ensemble.summary_collection_name)

    # One vector per plot
    summary_plot_collection.new_summary_plot(ensemble=ensemble, address="FOPT")

    # Multiple vectors in the same plot, separated by ';'
    summary_plot_collection.new_summary_plot(
        ensemble=ensemble, address="FOPR;FGPR;FWPR"
    )

    # Wildcards are supported, here all oil production totals for all wells
    summary_plot_collection.new_summary_plot(ensemble=ensemble, address="WOPT:*")

# Each call to new_summary_plot() appends a new summary multi plot to the
# project. Adjust the layout of the plots created above if required.
for multi_plot in project.descendants(rips.MultiSummaryPlot):
    multi_plot.number_of_columns = 2
    multi_plot.rows_per_page = 2
    multi_plot.update()
