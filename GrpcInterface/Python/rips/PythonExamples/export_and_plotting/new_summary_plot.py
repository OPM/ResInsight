# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
project = resinsight.project

summary_cases = project.descendants(rips.SummaryCase)
summary_plot_collection = project.descendants(rips.SummaryPlotCollection)[0]
if len(summary_cases) > 0:
    summary_plot = summary_plot_collection.new_summary_plot(
        summary_cases=summary_cases, address="FOP*"
    )
