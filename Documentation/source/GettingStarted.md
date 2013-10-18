# Getting started with ResInsight #

## Application overview ##
The main area of the application can contain several docking windows. The different docking windows can be managed from the **Windows** menu or directly using the local menu bar of the docking window.
- **Project Tree** - contains all application objects in a tree structure.
- **Property Editor** - displays all properties for the selected object in **Project Tree**
- **Process Monitor** - displays output from Octave when executing Octave scripts
- **Result Info** - displays info for the object being hit in the 3D scene when clicking with left mouse button

A new project tree and property editor can be added from **Windows->New Project and Property View**.

Multiple 3D views can be open at the same time, and to see views next to each other select the **Restore down** icon in the application show here for Windows :

![Show multiple views](/images/RestoreDown.png)

### Project Tree ###
Grid Models contains Eclipse cases organized in the following structure

- ![](/images/Case48x48.png) Binary case created from .EGRID or .GRID file 
- ![](/images/Case48x48.png) Binary case created from .EGRID or .GRID file 
- ![](/images/EclipseInput48x48.png) Input cases created from .GRDECL file 
- ![](/images/CreateGridCaseGroup16x16.png) A Grid Case Group can be created from a selection of binary files, or manually by assigning cases using the user interface of ResInsight.

Toggling a checkbox next to an item will toggle visibility in the 3D view. Toggling a checkbox for a collection of items will toggle visibility for all items in the collection.

## Open an Eclipse case ##
1. Select **File->Import->Import Eclipse Case** and select Eclipse file for import
2. The case is imported, and a view of the case is created
3. Select Cell Result in the Project Tree, and define the displayed result from Property Editor
4. Interact with the 3D model using the mouse

## Toolbar ##

A selected subset of actions are presented as controls in the toolbar. The different sections in the toolbar can be dragged and positioned anywhere as small floating toolbars. Management of the toolbar is done by right-clicking on the toolbar and then manipulating the displayed menu.

## Model navigation ##

ResInsight comes with a set of predefined 3D navigation modes. This mode can be set in **Preferences**.

Abbreviation | Meaning
-------------|-------
LMB          | Left mouse button
RMB          | Right mouse button
MMB          | Middle mouse button or scroll wheel button

### Ceetron navigation mode ###
Mouse interaction | Action
------------------|-------
LMB + move        | Move model
RMB + move        | Rotate model
RMB + move        | Rotate model
LMB + RMB + move  | Zoom
Scroll wheel      | Zoom
RMB click         | Context menu 
LMB click         | Update status bar and **Result Info**

### CAD navigation mode ###
Mouse interaction | Action
------------------|-------
MMB + move        | Rotate model
MMB + Shift       | Pan model
RMB + move        | Rotate model
LMB + RMB + move  | Zoom to mouse pointer location
Scroll wheel      | Zoom to mouse pointer location
RMB click         | Context menu 
LMB click         | Update status bar and **Result Info**

## Cell Edge Results ##
When working with a reservoir model, it is useful to map a set of grid cell properties in the same view. This can be achieved by activating **Cell Edge Result**. When selecting a result variable for cell edge, a second legend shows up in the 3D view in addition to different color mapping along the edges of a cell. Color legend management is available when selecting **Legend Definition**.

## Cell Filters ##
A cell filter can be used to control visibility of a cell in the 3D view. Three types of filters exists
- Range filter : Define a IJK subset of the model
- Property filter : Define a value range for a property to control cell visibility
- Well cell filter : Display grid cells where a well is passing through

All filters can be controlled from the **Property Editor**.

### Range filters ###

Using range filters enables the user to define a set of IJK visible regions in the 3D view. A single IJK-slice can be added from the context menu when rightclicking a grid cell. A new range filter can also be added by activating the context menu for the **Range Filters** collection in the **Project Tree**.

## Property filters ##

In addition to range filters, it is possible to filter visible grid cells based on a property value range. Add a new property filter by activating the context menu for **Property Filters**. The new property filter is based on currently selected cell result.

## Well range filter ##
Select **Wells** in the **Project Tree**. In the **Property Editor**, select **On** for **Add cells to range filter**. This will hide cells not part of a well.
In addition, all cells along a direction can be added as a fence. Enable this by checking **Use well fence**.


# Multiple realizations and statistics #

ResInsight features efficient support for management of large collection of realizations. To import a set of realizations, select **File->Import->Create Grid Case Group from Files**. An import dialog is opened, an a set of cases can be imported from multiple folders.
>NOTE: The search for Eclipse cases will be executed recursively for the specified folders.

Clicking the **Ok** button will import all cases into ResInsight, and they can be inspected one by one. To reduce the number of views, only a view for the first case is automatically created. Select *New view** from the context menu of a case to create a 3D view of the case.

## Statistics ##
After creating a grid case group, an empty statistics object is created. Select the properties to evaluate statistics for, and push **Compute** to start the computation of statistics. This can take a while for large models.
When the computation is complete, a view is automatically created containing the resulting generated statistics properties. Interaction with these generated properties are identical to interaction with other properties read from file.

A new statistical calculation can be created by activating the context menu for **Derived Statistic->New Statistics Case**.


# Export #
## Snapshot images ##
Image export of current 3D view can be launched from **File->Export->Snapshot To File**. If a project contains multiple views, all views can be exported using **File->Export->Snapshot All Views To File**. A snapshot can also be copied to clipboard using **Edit->Copy Snapshot To Clipboard**. All three commands are also available on the toolbar.

## Export of Eclipse Ascii data ##
Result data can be exported to Eclipse Ascii file by activating the context menu for a **Cell Result**.

'''
-- Exported from ResInsight
<keyword>
<data for all cells>
/
'''





# Script interface to Octave #
ResInsight provides a flexible interface to [Octave](http://www.gnu.org/software/octave/ "Octave") for scripts execution.

## Script execution - single case ##
A script can be started by navigating to the script in the **Project Tree**, and selecting **Execute** from the context menu. The currently active case will be manipulated by the Octave script. It is also possible to execute a script from the context menu of a case.

## Script execution - multiple cases ##
One script can be executed on many cases by first selecting a set of cases, and then activating **Execute script** from the context menu for the case selection.

## Script management ##
Octave scripts are available in the **Scripts** folder in the **Project Tree**. Multiple script folder locations can be defined in the field _Shared Script Folder(s)_ in **Edit=>Preferences**. These scripts can be edited by a text editor defined in _Script Editor_ in **Edit=>Preferences**.
