import sys
import os

import rips

use_platform_offscreen = True
if use_platform_offscreen:
    # To use offscreen, the path to fonts must be specified in the environment variable QT_QPA_FONTDIR="C:/windows/fonts"
    resinsight = rips.Instance.launch(
        command_line_parameters=["-platform", "offscreen", "--size", 1200, 1000]
    )

    qpa_fontdir = os.environ["QT_QPA_FONTDIR"]
    print("Environment var QT_QPA_FONTDIR : " + qpa_fontdir)
else:
    resinsight = rips.Instance.find()

summary_filename = "NORNE.SMSPEC"

project = resinsight.project
summary_case = project.import_summary_case(summary_filename)

summary_plot_collection = project.descendants(rips.SummaryPlotCollection)[0]

summary_plot_collection.new_summary_plot(summary_cases=[summary_case], address="FOPR")
summary_plot_collection.new_summary_plot(
    summary_cases=[summary_case], address="WOPR:A*;WOPR:B*"
)

plots = resinsight.project.plots()
for plot in plots:
    plot.export_snapshot()
    # plot.export_snapshot(output_format="PDF")

resinsight.exit()
