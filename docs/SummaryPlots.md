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

When a summary case has been imported, a Summary Plot with a default **Curve Filter** is created. This default behaviour can be configured in the  [ Preferences ]({{ site.baseurl }}/docs/preferences).

## Summary Plots

A Summary Plot is a window displaying a graph in the main area of the **Plot Main Window**. It can contain **Summary Curve Filters** and **Summary Curves** ( See below ).

A new plot can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryPlot16x16.png) **New Summary Plot**. 

![]({{ site.baseurl }}/images/SummaryPlotTree.png)

Most of the settings for the Plot itself is controlled by its sub items in the Property Tree: 

- **Time Axis** -- Controls the properties for the time axis (font size, title text, time range)
- **Left Y-axis** -- Controls the properties for the left Y-axis
- **Right Y-axis** -- Controls the properties for the right Y-axis

### Time Axis Properties

![]({{ site.baseurl }}/images/SummaryTimeAxisProperties.png)

- **Show Title** -- Toggles whether to show the axis title 
- **Title** -- A user defined name for the axis 
- **Title Position** --  Either *Center* or *At End* 
- **Font Size** -- The font Size used for the date/times shown at the ticks of the axis 
- **Time Mode** -- Option to show the time from Simulation Start, or as real date-times. 
- **Time Unit** -- The time unit used to display **Time From Simulation Start** 
- **Max**/**Min** -- The range of the visible time in the Plot in the appropriate time unit.  
  The format of dates is _`yyyy-mm-ddThh:mm:ssZ`_ 

### Y-axis properties

![]({{ site.baseurl }}/images/summary_plot_yaxis_properties.png)

- **Auto Title** -- If enabled, the y-axis title is derived from the vectors associated with the axis. Names and unit are used. 
- **Title** -- If **Auto Title** is disabled, the plot title is set using this field 
- **Title Position** -- Controls the position of the title. Center or At End 
- **Font Size** --  Defines the font size used by the axis title 
- **Logarithmic Scale**  - Draw plot curves using a logarithmic scale 
- **Number Format** -- Defines how the legend numbers are formatted 
- **Max and Min** -- Defines the visible y range 

#### Number Format

- **Auto** -- Legend numbers are either using a scientific or decimal notation based on the number of digits of the value
- **Decimal** -- Legend numbers are displayed using decimal notation
- **Scientific** -- Legend numbers are displayed using scientific notation (ie. 1.2e+6)

### Plot mouse interaction

- **Value Tracking** -- When the mouse cursor is close to a curve, the closest curve sample is highlighted and the curve sample value at this location is displayed in a tooltip. 
- **Selection** -- Left mouse button click can be used to select several of the parts in the plot, and display them in the Property Editor:
  - The closest curve
  - Each of the Plot Axes
  - The Plot itself if none of the above is hit and the Plot window is activated by the mouse click.
- **Window Zoom** -- Window zoom is available by dragging the mouse when the left mouse button is pressed. Use ![]({{ site.baseurl }}/images/ZoomAll16x16.png) **Zoom All** to restore default zoom level.
- **Wheel Zoom** -- The mouse wheel will zoom the plot in and out towards the current mouse cursor position 

### Accessing the Plot Data

The command context command **Show Plot Data** will show a window containing the plot data in ascii format. The content of this window is easy to copy and paste into Excel or other tools for further processing.

It is also possible to save the ascii data to a file directly by using the context command **Export Plot Data to Text File** on the plot. 

## Summary Curve Filter

A Summary Curve filter is a simplified way of creating and controlling many related curves at the same time. It enables efficient search options to select a sensible subset of vectors and controls the appearance and naming of the resulting curves.

A new curve filter can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryCurveFilter16x16.png) **New Summary Curve Filter**.

![]({{ site.baseurl }}/images/summary_curve_filter_properties.PNG)

The property panel is divided in four main groups of options:

- **Cases** -- Selecting the cases to extract data from
- **Vector Selection** -- Selecting what vectors to create curves from
- **Appearance Settings** -- Options controlling how colors, symbols etc are assigned to the curves
- **Curve Name Configuration** -- Control how the curves are named

In addition you have the following options:

- **Axis** -- Controls whether the curves are to be associated with the left or right Y-Axis 
- **Auto Apply Changes** -- When toggled, the changes in the property panel is instantly reflected in the generated and controlled curves 
- **Apply** -- Applies the settings, and thus generates and updates the controlled curves 

In the following sections the option groups are described in more detail.

### Cases

Selects the cases to be used when searching for data vectors. Several Cases can be selected at the same time and the filter will contain the union of all vectors in those cases. Curves will be created for each selected case for the selected set of vectors. 

### Vector Selection

This group of options is used to define the selection of summary vectors of interest. Several filtering tools are available to make this as convenient as possible. 

- **Search** -- This option controls the filtering mode. Several are available and controls witch search fields that are made available. The search modes are described below 
- *Options depending on Search Mode* -- Described below. 
- *list of vector names* -- This list displays the set of vectors filtered by the search options. Use this to select which of the vectors you want as curves. **Ctrl-A** selects them all.

In the following, all the search fields are wildcard-based text filters. An empty search string will match anything: any value or no value at all. A single _`*`_ however, will only match something: There has to be some value for that particular quantity to make the filter match.

The **Vector Name** field will match the name of the quantity itself, while the additional mode specific fields will match the item(s) being addressed. 

#### Search Modes with filter fields

- **All** -- A wildcard search filter applied to the colon-separated string that describes the complete vector. Eg. _`"*:*, 55, *"`_ or _`"WBHP:*"`_. This mode is the default.
   - **Filter** -- The actual filter text to apply
- **Field** -- Select Field related vectors only
  -  **Vector name** -- Filter for Field related vector names 
- **Well** -- Select Well related vectors only
   - **Vector name** -- Filter for Well related vector names 
   - **Well name** --  Well name filter 
- **Group** - Select Group related vectors only
   - **Vector name** -- Filter for Group related vector names 
   - **Group name** --  Group name filter 
- **Completion**   -- Select Completion related vectors only
   - **Vector name**  -- Filter for Completion related vector names 
   - **Well name** --  Well name filter 
   - **I, J, K** -- Text based filter of the I, J, K value string of the completion. Eg _`"18,*,*"`_ to find vectors with I = 18 only 
- **Segment** -- Select Segment related vectors only    
   - **Vector name**  -- Filter for Segment related vector names 
   - **Well name** -- Well name filter 
   - **Segment number** -- Text based filter of the segment numbers
- **Block** -- Select I, J, K -- Block related vectors only 
   - **Vector name**  -- Filter for cell Block related vector names 
   - **I, J, K** -- Text based filter of the I, J, K value string of the Block. 
- **Region** -- Select Region related vectors only  
   - **Vector name**  -- Filter for Region related vector names 
   - **Region number** -- Text based filter of the Region numbers
- **Region-Region** -- Select Region to Region related vectors only  
   - **Vector name**  -- Filter for Region to Region related vector names 
   - **Region number** -- Text based filter of the first Region number
   - **2. Region number** -- Text based filter of the second Region number
- **Lgr-Well** -- Select Well in LGR related vectors only
   - **Vector name** -- Filter for Well in Lgr related vector names 
   - **Well name** -- Well name filter 
   - **Lgr name** -- Lgr name filter 
- **Lgr-Completion** -- Select Completion in LGR related vectors only
   - **Vector name** -- Filter for Well in Lgr related vector names 
   - **Well name** --  Well name filter 
   - **Lgr name** -- Lgr name filter 
   - **I, J, K** -- Text based filter of the I, J, K value string of the completion in the Lgr.
- **Lgr-Block** -- Select I, J, K - Block in LGR related vectors only
   - **Vector name**  -- Filter for cell Block related vector names 
   - **Lgr name** -- Lgr name filter 
   - **I, J, K** -- Text based filter of the I, J, K value string of the Block in the Lgr. 
- **Misc** -- Select vectors in the Misc category only 
   - **Vector name** -- Filter for Misc category vector names 
- **Aquifer** -- Select Aquifer category vectors only 
   - **Vector name** -- Filter for Aquifer category vector names 
- **Network** -- Select Network category vectors only  
   - **Vector name** -- Filter for Network category vector names 
- **All (Advanced)** -- This is a complete combined search mode with all the different search options available to create advanced cross item type searches.  

### Appearance Settings

Curves created by a curve filter are assigned individual visual properties like colors and symbols in a systematic manner to make the plots easy to read. Different aspects of the vectors are assigned to different curve appearances. Eg. using symbols to distinguish cases, while using colors to distinguish quantity.

These assignments can be controlled using the options in the **Appearance Settings** group. 

![]({{ site.baseurl }}/images/SummaryCurveFilterAppearance.png)

When set to **Auto** ResInsight assigns visual properties based on the present vector categories and the number of different values in each category.

When disabling the **Auto** option, you can select which of the visual curve properties to use for which vector category. The vector Category that currently can be used is Case, Vector, Well, Group and Region. The visual properties supported types are Color, Symbols, Line Style, Gradient and Line Thickness.

### Curve Name Configuration 

The user can control the curve names by toggling what part of the summary vector information to use in the name.

#### Contribute To Legend

This option controls whether the curves created by the filter will be visible in the plot legend at all. In addition will  Curves with an empty name also be removed from the legend.  

## Summary Curve
A new curve can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryCurve16x16.png) **New Summary Curve**.

![]({{ site.baseurl }}/images/summary_curve_properties.png)

Many of the properties of a single curve is similar to the properties described for a curve filter. There are some differences, however:

### Appearance

The curve's appearance is controlled directly, and not automatically as for **Curve Filters**.
<div class="note">
The appearance set on a curve in a <b>Curve Filter</b> will override the settings in the <b>Curve Filter</b> until the <b>Curve Filter</b> settings are applied again. Then the local changes on the curve are overwritten. 
</div>

### Curve Name

- **Contribute To Legend** -- This option controls whether the curve will be visible in the plot legend at all. A curves with an empty name will also be removed from the legend. 
- **Auto Name** -- If enabled, ResInsight will create a name for the curve automatically based on the settings in this option group.
- **Curve Name** -- If **Auto Name** is off, you can enter any name here. If empty, the curve will be removed from the legend, but still visible in the plot.

## Copy and Paste 

Copy and Paste of selections of Summary Plots, Curves, or Curve Filter is possible using the Project Tree Context menu and standard keyboard shortcuts (CTRL-C/CTRL-V).
