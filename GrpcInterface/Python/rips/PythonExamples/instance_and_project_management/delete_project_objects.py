# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
project = resinsight.project

# Any object in the project tree can be removed with delete(). The object is
# detached from its parent and deleted in ResInsight, and the Python object
# must not be used afterwards.

# Create two summary plots to work with. Each plot is placed in its own
# multi plot (the plot window).
summary_cases = project.descendants(rips.SummaryCase)
if len(summary_cases) == 0:
    print("No summary cases loaded. Import a summary case first.")
    exit()

summary_plot_collection = project.descendants(rips.SummaryPlotCollection)[0]
summary_plot_collection.new_summary_plot(summary_cases=summary_cases, address="FOPT")
summary_plot_collection.new_summary_plot(summary_cases=summary_cases, address="FWPT")

summary_plots = project.descendants(rips.SummaryPlot)
multi_plots = project.descendants(rips.MultiSummaryPlot)
print(f"Before: {len(summary_plots)} summary plots in {len(multi_plots)} plot windows")

# Delete a single summary plot. The plot window that contained it remains.
summary_plots[0].delete()

# Delete a whole plot window, including the summary plots inside it.
multi_plots[-1].delete()

summary_plots = project.descendants(rips.SummaryPlot)
multi_plots = project.descendants(rips.MultiSummaryPlot)
print(f"After:  {len(summary_plots)} summary plots in {len(multi_plots)} plot windows")
