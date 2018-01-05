---
layout: docs
title: Summary Plots
permalink: /docs/summaryplots/
published: true
---
![]({{ site.baseurl }}/images/ResInsightMainPlotMediumSize.png)

A Summary Plot is a window displaying a graph in the main area of the **Plot Main Window**. It can contain Summary Curves, Grid time history curves and pasted ascii curves ( See below ).

A new plot can be created by using the context menu of a plot selecting ![]({{ site.baseurl }}/images/SummaryPlot16x16.png) **New Summary Plot**. The [Summary Plot Editor]({{ site.baseurl }}/docs/summaryploteditor) dialog will then open.

## Plot Data

ResInsight can create summary plots based on vectors from SUMMARY files ( _`*.SMSPEC`_ ), imported Observed Time History Data, Grid Cell Time history Curve and pasted ascii curves. 

### SUMMARY Files

When opening an Eclipse case in the 3D view, the associated summary file is opened automatically by default, and made available as a **Summary Case**.
Summary files can also be imported directly using the command: **File->Import->Import Summary Case**. All cases will be available under **Summary Cases** in the **Plot Object Project Tree**. 

A selection of cases can be grouped by right-clicking  a selection of summary cases and selecting the command **Group Summary Cases**. Summary cases can also be drag-dropped between summary groups. The groups will be used when listing the cases in the [Summary Plot Editor]({{ site.baseurl }}/docs/summaryploteditor).

### Observed Data

See [Observed Time History Data]({{ site.baseurl }}/docs/importobstimehistdata)

### Grid Cell Time History Curve

Time history curves from a grid cell property can also be added to a Summary Plot. 
See [Result Inspection]({{ site.baseurl }}/docs/resultinspection#result-plot).

### Pasted Ascii Curves

You can copy an ascii table directly from Excel or any text source and paste it directly into a Summary Plot using the command **Paste Excel Data to Summary Plot**. See [Paste Excel Time History Data]({{ site.baseurl }}/docs/pasteexceltimedata).

## Plot Settings

![]({{ site.baseurl }}/images/SummaryPlotTree.png)

Most of the settings for the Plot itself is controlled by its sub items in the Property Tree: 

- **Time Axis** -- Controls the properties for the time axis (font size, title text, time range)
- **Left Y-axis** -- Controls the properties for the left Y-axis
- **Right Y-axis** -- Controls the properties for the right Y-axis

### Time Axis Properties

![]({{ site.baseurl }}/images/SummaryTimeAxisProperties.png)

- **Axis Title**
  - **Show Title** -- Toggles whether to show the axis title 
  - **Title** -- A user defined name for the axis 
  - **Title Position** --  Either *Center* or *At End* 
  - **Font Size** -- The font size used for the axis title
- **Time Values**
  - **Time Mode** -- Option to show the time from Simulation Start, or as real date-times. 
  - **Time Unit** -- The time unit used to display **Time From Simulation Start** 
  - **Max**/**Min** -- The range of the visible time in the Plot in the appropriate time unit.  
    The format of dates is _`yyyy-mm-ddThh:mm:ssZ`_ 
  - **Font Size** -- The font size used for the date/times shown at the ticks of the axis 

### Y-axis Properties

![]({{ site.baseurl }}/images/summary_plot_yaxis_properties.png)
- **Axis Title**
  - **Auto Title** -- If enabled, the y-axis title is derived from the vectors associated with the axis. Long names, acronymes  and unit can be used. 
    - **Names** -- Use the long name of the quantities
    - **Acronymes** -- Add the shortname/acronyme of the quantities
  - **Title** -- If **Auto Title** is disabled, the plot title is set using this field.
- **Title Layout**
  - **Title Position** -- Controls the position of the title. Center or At End.
  - **Font Size** --  Defines the font size used by the axis title. 
- **Axis Values**
  - **Logarithmic Scale**  - Draw plot curves using a logarithmic scale. 
  - **Number Format** -- Defines how the legend numbers are formatted.
    - **Auto** -- Legend numbers are either using a scientific or decimal notation based on the number of digits of the value
    - **Decimal** -- Legend numbers are displayed using decimal notation.
    - **Scientific** -- Legend numbers are displayed using scientific notation (ie. 1.2e+6).
  - **Number of Decimals** -- Controls the number of digits after "." (for  **Decimal** or **Scientific** format options).
  - **Scale Factor** -- "Moves" the scale value away from the values along the axis and into the unit on the axis title (for  **Decimal** or **Scientific** format options).
  - **Max and Min** -- Defines the visible y range.
  - **Font Size** -- The font size used for the values shown at the ticks on the axis.   

### Plot Mouse Interaction

- **Value Tracking** -- When the mouse cursor is close to a curve, the closest curve sample is highlighted and the curve sample value at this location is displayed in a tooltip. 
- **Selection** -- Left mouse button click can be used to select several of the parts in the plot, and display them in the Property Editor:
  - The closest curve.
  - Each of the Plot Axes.
  - The Plot itself if none of the above is hit and the Plot window is activated by the mouse click.
- **Window Zoom** -- Window zoom is available by dragging the mouse when the left mouse button is pressed. Use ![]({{ site.baseurl }}/images/ZoomAll16x16.png) **Zoom All** to restore default zoom level.
- **Wheel Zoom** -- The mouse wheel will zoom the plot in and out towards the current mouse cursor position.

### Accessing the Plot Data

The command context command **Show Plot Data** will show a window containing the plot data in ascii format. The content of this window is easy to copy and paste into Excel or other tools for further processing.

It is also possible to save the ascii data to a file directly by using the context command **Export Plot Data to Text File** on the plot. 


## Summary Curves

Summary curves are normally created using the **Plot Editor** see [Summary Plot Editor]({{ site.baseurl }}/docs/summaryploteditor), but can be created directly using the context menu in the **Main Plot Window Project Tree**. Right click a Summary Plot, the Summary Curves folder or an existing curve and select the command ![]({{ site.baseurl }}/images/SummaryCurve16x16.png) **New Summary Curve**.

![]({{ site.baseurl }}/images/summary_curve_properties.png)

The property panel is divided in three main groups of options:

- **Summary Vector** -- Options selecting the value to plot.
- **Appearance Settings** -- Options controlling the color, symbol etc of the curve.
- **Curve Name Configuration** -- Controls how the curve is labeled in the legend.

### Summary Vector

This group of options is used to define the summary vector data that the curve will display. 

- **Case** -- Selects the imported Summary or Observed Data case to use as source.
- **Vector** -- Displays a short name/ acronyme of the selected vector.
- **Axis** -- Controls whether the curve is to be associated with the left or right Y-Axis. 

<div class="note">
Switching the Y-Axis for several curves in one go can be done using the context command <b>Switch Plot Axis</b>.  
</div>

To optional ways to select the curve data are available: The **Vector Selection Dialog** and the **Vector Selection Filter**.

The first is accessed by clicking the button **Vector Selection Dialog**. This opens a dialog similar to the one used as Plot Editor. See [Summary Plot Editor]({{ site.baseurl }}/docs/summaryploteditor).

The **Vector Selection Filter** group of options is a different way of selecting the curve data:
- **Search** -- This option controls the filtering mode. Several are available and controls witch search fields that are made available. The search modes are described below 
- **Options depending on Search Mode** -- Described below. 
- **List of vector names** -- This list displays the set of vectors filtered by the search options. Use this to select which of the vectors you want to plot.

In the following, all the search fields are wildcard-based text filters. An empty search string will match anything: any value or no value at all. A single _`*`_ however, will only match something: There has to be some value for that particular quantity to make the filter match.

The **Vector Name** field will match the name of the quantity itself, while the additional mode specific fields will match the item(s) being addressed. 

#### Search Modes with Filter Fields

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

### Curve Name 

The user can control the curve name used in the plot legend by using these options.

- **Contribute To Legend** -- This option controls whether the curve will be visible in the plot legend at all. A curves with an empty name will also be removed from the legend. 
- **Auto Name** -- If enabled, ResInsight will create a name for the curve automatically based on the settings in this option group.
- **Curve Name** -- If **Auto Name** is off, you can enter any name here. If empty, the curve will be removed from the legend, but still visible in the plot.
- **Case Name, Vector name ...** etc. -- These options controls what part of the summary vector information to use in the curve auto-name.

## Copy and Paste 

Copy and Paste of selections of Summary Plots and Curves, is possible using the Project Tree Context menu and standard keyboard shortcuts (CTRL-C/CTRL-V).

## Summary Source Stepping
Summary Source Stepping is a function which lets the user step through multiple vectors in one click. This function is available from both the toolbar and the **Summary Curves** property editor under a **Summary Plot** item in the **Main Plot Window Project Tree**.

The toolbar version may look like this

![]({{site.baseurl}}/images/SummarySourceSteppingToolbar.png)

and the property editor version may looks like this

![]({{site.baseurl}}/images/SummarySourceSteppingPropertyEditor.png)

In some cases some of the stepping components are hidden, depending on the set of summary curves currently visible. When ResInsight decides which stepping component to display, all visible curves in the current plot are taken into account. If, for instance, all curves display data from the same well, the well stepping component is displayed. This policy applies to the following source dimensions:
- Cases
- Wells
- Well groups
- Regions
- Vectors/Summaries

When one of the **next buttons** are clicked, all curves are changed to display data for the next item for the clicked source dimension. Example: The user clicks the **next well button**. Then the well source for all curves in the current plot are changed to display data for the next well.

