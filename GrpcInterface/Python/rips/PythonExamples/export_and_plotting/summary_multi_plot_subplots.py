###############################################################################
# Create several subplots in the same summary multi plot
#
# Requires a project with at least one summary case
###############################################################################

import sys

import rips

resinsight = rips.Instance.find()
project = resinsight.project

summary_cases = project.descendants(rips.SummaryCase)
if not summary_cases:
    print("At least one summary case is required")
    sys.exit(1)

summary_plot_collection = project.summary_plot_collection()

# Create a multi plot with one subplot for each address
multi_plot = summary_plot_collection.new_summary_multi_plot(
    summary_cases=[summary_cases[0]], addresses=["FOPT", "FWPT", "FGPT"]
)

# Subplots can be added later to the same multi plot
multi_plot.append_subplot(summary_cases=[summary_cases[0]], address="FOPR")

multi_plot.number_of_columns = 2
multi_plot.rows_per_page = 2
multi_plot.update()

print("Number of subplots:", len(multi_plot.descendants(rips.SummaryPlot)))
