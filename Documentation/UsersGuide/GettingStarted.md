## Getting started with ResInsight 

### Installation
Installation instructions for ResInsight can be found here: [Installation and Configuration](Installation.md) 


### User interface overview 

The application has a main area and several docking windows. The different docking 
windows can be managed from the **Windows** menu or directly using the local menu bar of the docking window.

![ResInsight User Interface] (images/ResInsightUIFullSizeWindows.png)


#### Docking Windows

- **Project Tree** - contains all application objects in a tree structure.
- **Property Editor** - displays all properties for the selected object in the **Project Tree**
- **Process Monitor** - displays output from Octave when executing Octave scripts
- **Result Info** - displays info for the object being hit in the 3D scene when clicking with left mouse button

*TIP:* A new project tree and property editor can be added from **Windows->New Project and Property View**.


#### 3D Views 

In the main area of the application, several 3D views can be open at the same time.  One of them will be active and the active view can be either maximized to use the whole main area, or normalized so that you can see all the open 3D views.

*TIP:* To see views next to each other select the **Restore down** icon in the application show here for Windows :

![Restore Down](images/RestoreDown.PNG)

#### Editing the views

Most of the settings and features of ResInsight is accesible through the **Project Tree** and the **Property Editor**. Selecting an item in the **Project Tree** activates the corresponding 3D View, and shows the Items properties in the **Property Editor**, available for editing. 

Toggling a checkbox next to an item in the **Project Tree** will toggle visibility in the 3D view. Toggling a checkbox for a collection of items will toggle visibility for all items in the collection.

Context menu commands are also available, to do special operations on a selected set of items.

### Cases and their types

A *Case* in ResInsight means a Grid model with a particular set of results or property data. There are  
three different Case types: 

##### ![](images/Case24x24.png) Result case
This is a Case based on the results of an Eclipse analysis, read from a grid file together with restart data.

##### ![](images/EclipseInput24x24.png) Input case
This Case type is based on a `*.GRDECL` file, or a part of an Eclipse *Input* file. This Case type supports loading single ascii files defining Eclipse Cell Properies, and also to export miodified property sets to ascii files.

##### ![](images/EclipseInput24x24.png) Statistics case
This is a Case type that belongs to a *Grid Case Group* and makes statistical calculations based on a set of source cases available. 

##### ![](images/CreateGridCaseGroup24x24.png) Grid Case Group
A Grid Case Group can be created from a selection of binary files, or manually by assigning cases using the user interface of ResInsight.

### Importing data

#### Input data support ###

ResInsight supports the follwing type of Eclipse input data:
- `*.GRID` and `*.EGRID` files along with their `*.INIT` and restart files `*.XNNN` and `*.UNRST`. 
- Grid and Property data from  `*.GRDECL` files.

### Open an Eclipse case 

1. Select **File->Import->Import Eclipse Case** and select Eclipse file for import
2. The case is imported, and a view of the case is created
3. Select Cell Result in the Project Tree, and define the displayed result from Property Editor
4. Interact with the 3D model using the mouse

### Toolbar 

A selected subset of actions are presented as controls in the toolbar. The different sections in the toolbar can be dragged and positioned anywhere as small floating toolbars. Management of the toolbar is done by right-clicking on the toolbar and then manipulating the displayed menu.

### Model navigation 

ResInsight comes with a set of predefined 3D navigation modes. This mode can be set in the **Preferences** dialog (**Edit->Preferences**).

Abbreviation | Meaning
-------------|-------
LMB          | Left mouse button
RMB          | Right mouse button
MMB          | Middle mouse button or scroll wheel button

#### Ceetron navigation mode
Mouse interaction | Action
------------------|-------
LMB + move        | Move model
RMB + move        | Rotate model
RMB + move        | Rotate model
LMB + RMB + move  | Zoom
Scroll wheel      | Zoom
RMB click         | Context menu 
LMB click         | Update status bar and **Result Info**

#### CAD navigation mode
Mouse interaction | Action
------------------|-------
MMB + move        | Rotate model
MMB + Shift       | Pan model
RMB + move        | Rotate model
LMB + RMB + move  | Zoom to mouse pointer location
Scroll wheel      | Zoom to mouse pointer location
RMB click         | Context menu 
LMB click         | Update status bar and **Result Info**

### Project files and related data `*.rsp`

### Export
#### Snapshot images 
Image export of current 3D view can be launched from **File->Export->Snapshot To File**. If a project contains multiple views, all views can be exported using **File->Export->Snapshot All Views To File**. A snapshot can also be copied to clipboard using **Edit->Copy Snapshot To Clipboard**. All three commands are also available on the toolbar.

#### Export of Eclipse Ascii data
Result data can be exported to Eclipse Ascii file by activating the context menu for a **Cell Result**.

    -- Exported from ResInsight
    <keyword>
    <data for all cells>
    /

