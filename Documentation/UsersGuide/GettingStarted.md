# Getting started with ResInsight #

## Application overview ##

The main area of the application can contain several docking windows. The different docking 
windows can be managed from the **Windows** menu or directly using the local menu bar of the docking window.

- **Project Tree** - contains all application objects in a tree structure.
- **Property Editor** - displays all properties for the selected object in **Project Tree**
- **Process Monitor** - displays output from Octave when executing Octave scripts
- **Result Info** - displays info for the object being hit in the 3D scene when clicking with left mouse button

A new project tree and property editor can be added from **Windows->New Project and Property View**.

Multiple 3D views can be open at the same time, and to see views next to each other select 
the **Restore down** icon in the application show here for Windows :

![Show multiple views](images/RestoreDown.PNG)

Toggling a checkbox next to an item in the **Project Tree** will toggle visibility in the 3D view. Toggling a checkbox for a collection of items will toggle visibility for all items in the collection.

### Input data support ###
ResInsight supports the follwing type of Eclipse input data

#### ![](images/Case24x24.png) Binary case
Read grid from .EGRID or .GRID file and binary data from UNRST files.

#### ![](images/EclipseInput24x24.png) Input case
Read grid from from .GRDECL file and property data from Ascii files.

#### ![](images/CreateGridCaseGroup24x24.png) Grid Case Group
A Grid Case Group can be created from a selection of binary files, or manually by assigning cases using the user interface of ResInsight.


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


# Export #
## Snapshot images ##
Image export of current 3D view can be launched from **File->Export->Snapshot To File**. If a project contains multiple views, all views can be exported using **File->Export->Snapshot All Views To File**. A snapshot can also be copied to clipboard using **Edit->Copy Snapshot To Clipboard**. All three commands are also available on the toolbar.

## Export of Eclipse Ascii data ##
Result data can be exported to Eclipse Ascii file by activating the context menu for a **Cell Result**.

    -- Exported from ResInsight
    <keyword>
    <data for all cells>
    /

