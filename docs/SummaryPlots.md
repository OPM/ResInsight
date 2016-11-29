---
layout: docs
title: Summary Plots
permalink: /docs/summaryplots/
published: true
---
![]({{ site.baseurl }}/images/ResInsightMainPlotMediumSize.png)

ResInsight can create summary plots based on vectors from SUMMARY files (`*.SMSPEC`). 

When opening an Eclipse case in the 3D view, the associated summary file is opened automatically by default, and made available as a **Summary Case**.
Summary files can also be imported directly using the command: **File->Import->Import Summary Case**.

When a summary case has been imported, a Summary Plot with a default **Curve Filter** is created.

The default behaviours can be configured in the  [ Preferences ]({{ site.baseurl }}/docs/preferences).

## Summary Plots

A Summary Plot is a window displaying a graph in the main area of the Plot Mian Window. It can contain **Summary Curve Filters** and **Summary Curves** (See below) in addition to a title, left and right Y-Axis, and the Time-axis.

A new plot can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryPlot16x16.png) **New Summary Plot**.

![]({{ site.baseurl }}/images/SummaryPlotTree.png)

Most of the settings for the Plot itself is controlled by its sub items in the Property Tree: 

- **Time Axis** - Controls the properties for the time axis (font size, title text, time range)
- **Left Y-axis** - Controls the properties for the left Y-axis
- **Right Y-axis** - Controls the properties for the right Y-axis

### Y-axis properties

![]({{ site.baseurl }}/images/summary_plot_yaxis_properties.png)

| Parameter | Description |
|-----------|-------------|
| **Auto Title** | If enabled, the y-axis title is derived from the vectors associated with the axis. Names and unit are used. |
| **Title** | If **Auto Title** is disabled, the plot title is set using this field |
| **Title Position** | Controls the position of the title. Center or At End |
| **Font Size**  | Defines the font size used by the axis title |
| **Max and Min** | Defines the visible y range |
| **Number Format** | Defines how the legend numbers are formatted |
| **Logarithmic Scale**  | Draw plot curves using a logarithmic scale |

#### Number Format
- **Auto** - Legend numbers are either using a scientific or decimal notation based on the number of digits of the value
- **Decimal** - Legend numbers are displayed using decimal notation
- **Scientific** - Legend numbers are displayed using scientific notation (ie. 1.2e+6)

### Time Axis Properties
![]({{ site.baseurl }}/images/SummaryTimeAxisProperties.png)

| Parameter | Description |
|-----------|-------------|
| **Show Title** | Toggles wheter to show the axis title |
| **Title** | A user defined name for the axis |
| **Title Position** |  Either *Center* or *At End* |
| **Font Size** | The font Size used for the date/times shown at the ticks of the axis |
| **Time Mode** | Option to show the time from Simulation Start, or as real date-times. |
|  |  When activated, The **Time Unit** Option will appear, with options to show the relative time in different units|
| **Max**/**Min** | The range of the visible time in the Plot in the appropriate time unit. The format of dates is yyyy-mm-ddThh:mm:ssZ |

### Plot mouse interaction

- **Value Tracking** - When the mouse cursor is close to a curve, the closest curve sample is highlighted and the curve sample value at this location is displayed in a tooltip. 

- **Selection** - Left mouse button click can be used to select several of the parts in the plot, and display them in the Property Editor:
  - The closest curve
  - Each of the Plot Axes
  - The Plot itself if none of the above is hit and the Plot window is activated by the mouse click.

- **Window Zoom** - Window zoom is available by dragging the mouse when the left mouse button is pressed. Use ![]({{ site.baseurl }}/images/ZoomAll16x16.png) **Zoom All** to restore default zoom level.



## Summary Curve Filter

A Summary Curve filter is a simplified way of creating and controlling many related curves at the same time. It enables efficient search options to select a sensible subset of vectors and controls the appearance and naming of the resulting curves.

A new curve filter can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryCurveFilter16x16.png) **New Summary Curve Filter**.

![]({{ site.baseurl }}/images/summary_curve_filter_properties.PNG)

The property panel is divided in three main groups of options:

- **Summary Vectors** - Selecting what vectors to create curves from
- **Appearance Settings** - Options controlling how colors, symbols etc are assigned to the curves
- **Curve Name Configuration** - Control how the curves are named

In the following sections these groups of options are described in more detail.

In addition to the option groups there are three general items in the property panel:

| Name | Description |
|-----------|-------------|
| **Axis** | Controls wether the curves are to be associated with the left or right Y-Axis |
| **Auto Apply Changes** | When toggled, the changes in the property panel is instanly reflected in the generated and controlled curves |
| **Apply** | Applies the settings, and thus generates and updates the controlled curves |

### *Summary Vectors* - option group

This group of options is used to define the selection of summary vectors of interest. Several filtering tools are available to make this as convenient as possible. 

| Name | Description |
|-----------|-------------|
| **Cases** | Selects the cases to be used when searching for data vectors. Several Cases can be selected at the same time. If a particular case is not selected, unique vectors defined in this case will not appear in the filter. | 
| **Search** | This option controls the filtering mode. Several are available and controls witch search fields that are made available. This is described in more detail below.|
| *unnamed list of vectors* | This list displays the set of vectors filtered by the search options. Use this to select which of the vectors you want as curves. **Ctrl-A** selects them all.|

In the following, all the search fields are wildcard based text filters. An empty search string will match anything: any value or no value at all. A single `*` however, will only match something: There has to be some value for that particular quantity to make the filter match.

|**Search** mode | Description |
|----------------|--------------|
| **All**   |  A wildcard search filter applied to the colon-separated string that describes the complete vector. Eg `"*:*, 55, *"` or `"WBHP:*"`. This mode is the default. |
| **Field**, **Well**, **Group**, **Completion**, **Segment** ,**Block**, **Region**, **Region-Region**, **Lgr-Well**, **Lgr-Completion**, **Lgr-Block**, **Misc**, **Aquifer**, **Network** | These filter modes will only match vectors of the corresponding Eclipse output. The **Vector Name** field will match the name of the quantity itself, while the additional mode specific fields will match the item(s) beeing addressed.    |
| **All (Advanced)** | This is a complete combined search mode with all the different search options available to create advanced cross item type searches.| 

### *Appearance Settings* - option group

Curves created by a curve filter are assigned individual visual properties like colors and symbols in a systematic manner to make the plots easy to read. Different aspects of the vectors are assigned to different curve appearances. Eg. using symbols to distinguish cases, while using colors to distinguish quantity.

These assignments can be controlled using the options in the **Appearance Settings** group. 

![]({{ site.baseurl }}/images/SummaryCurveFilterAppearance.png)

When set to **Auto** ResInsight assigns visual properties based on the present vector categories and the number of different values in each category.

When disabling the **Auto** option, you can select which of the visual curve properties to use for which vector category. The vector Category that currently can be used is Case, Vector, Well, Group and Region. The visual properties supported types are Color, Symbols, Line Style, Gradient and Line Thickness.

### *Curve Name Configuration* - option group

The user can control the curve names by toggeling what part of the summary vector information to use in the name.

## Summary Curve
A new curve can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryCurve16x16.png) **New Summary Curve**.

![]({{ site.baseurl }}/images/summary_curve_properties.png)

Many of the properties of a single curve is similar to the properties described for a curve filter. The appearance however, is controlled directly. 

<div class="note">
The appearance set on a curve in a curve filter will override the settings in the curvefilter until the curvefilter settings are applied again. Then the clocal changes on the curve are overwritten. 
</div>


## Copy and Paste 

Copy and Paste of selections of Summary Plots, Curves, or Curve Filter is possible using the Project Tree Context menu and standard keyboard shortcuts (CTRL-C/CTRL-V).
