---
layout: docs
title: Well Trajectories
permalink: /docs/wellpaths/
published: true
---
![]({{ site.baseurl }}/images/ResInsight_WellPathWithSimulationWell.png)

ResInsight can import Well trajectories from simple Ascii files. 
In addition, a Statoil specific solution to retrieve Well Trajectories from their internal web service is implemented.

ResInsight also supports creation of your own user-defined well paths. See [Create Well Paths]({{ site.baseurl }}/docs/createwellpaths)

## Ascii Well Trajectories

The command **File -> Import -> Well Data -> Import Well Paths From File** will read the well paths in the selected file, and create one entry for each trajectory under the  ![]({{ site.baseurl }}/images/WellCollection.png) **Wells** item in the **Project Tree**. 

The supported ASCII format is quite flexible but the main requirements are: 

- Each data line must contain four numbers: X Y TVD MD separated with white-space.
- Lines starting with `"--" or "#"` is considered to be comment lines
- A line starting with none-number-characters are used as a well name after the following rules:
  - If the line contains a pair of : ```  "'", "`", "´", "’" or "‘" ``` the text between the quotation marks is used as a well name.
  - If the line contains the case insensitive string "name" with an optional ":" after then the rest of the line is used as a well name. 
  - If there are no quotes or "name"'s, the complete line is used as a well name.
  - If there are several consecutive name-like lines, only the last one will be used 
- If a well name is found, a new well is created and the following data points are added to it.

#### Example 1:
    WELLNAME: WELL1
    6507.1	725	2542	2590
    6523.5	757	2549	2626.6
    6523.5	760	2559	2637.1
    -999
    WELLNAME: WELL2
    550.7 1020.2  2410   2410
    551   1004.1  2422.2 2430.2
    555.2  993.9  2425   2441.6
    -999

#### Example 2:
    X Y TVD MD
    Name Well_1
    6507.1	725	2542	2542
    6523.5	757	2549	2578.6
    6523.5	760	2559	2589.1
    
    -- A Comment new well
    This is not its name
    Name Well_2
    550.7	1020.2	2410	2520
    551	1004.1	2422.2	2540.2
    # a comment inside the data 
    555.2	993.9	2425	2551.6
    
    3Q AHB-J
    5507.0	4638.5	0.0	0.0
    5507	4638.5	1628.6	1628.6

    
### Trajectory Files are Referenced
The trajectory data is not copied into the ResInsight project as such. The project file only stores the file path, and the next time you open the project, ResInsight will try to read the well data from the file again.  

<div class="note info">
If the well trajectory file is changed and you would like a running ResInsight to update accordingly, you will need to delete all the well trajectories that emerge from that file, and import it again.
</div>

## Importing from SSI-Hub (Internal Statoil Web-service)

In order to import from SSI-Hub, a project file must be present and stored to file. All imported well paths from the web service will be stored relative to this project file location. If no project file exists, the menu item is disabled.

The command **File -> Import -> Well Data -> Import Well Paths From SSI-hub** launches a wizard to guide you through the process of selecting the well trajectories you need.

<div class="note info">
<h5>Access to web service</h5>
The import of well paths is using a web service. If you are a Statoil employee, make sure you have access to "EDM Landmark" and "EDM Compass".
</div>


After completing the wizard, the wells imported are accessible as Items under the  ![]({{ site.baseurl }}/images/WellCollection.png) **Wells** item in the **Project Tree**.

The trajectory data is not copied into the  ResInsight project as such, but is stored in files in a directory called *ProjectFileName_wellpaths* in the same directory as your project file.   

## Well Trajectory Visualization

All the imported well trajectories are available below the ![]({{ site.baseurl }}/images/WellCollection.png) **Wells** item in the **Project Tree**. 

![]({{ site.baseurl }}/images/WellsInTree.png)

The visible wells are always shown in all the 3D Views in the complete project, so the toggles and settings control the overall project visibility of the Well Trajectories. The **Property Editor** of the **Wells** item is shown below 

![]({{ site.baseurl }}/images/WellPathCollectionProperties.png)

- **Global well path visibility** -- This option forces the well paths on or off, ignoring the individual settings unless it is set to Individual.
- **Clip Well Paths** -- This option hides the top of the Well Trajectories to avoid displaying the very long lines from the reservoir to the sea surface.
- **Well Path clipping depth distance** -- This number is the distance from the top of the reservoir to the clipping depth.

## Individual Well Trajectories
A well trajectory (well path) will hold well log data and well path data imported from files. A well path file is placed inside the well path item, while one or more well log files are placed as child items under the well path in the project tree.

### Importing Well Log Files
Well log data is usually imported from LAS-files (_\*.las_). LAS-files can be imported using the command: **File->Import-> Well Data->Import Well Logs from File**.

ResInsight will look for the the well name in the imported LAS-files among the existing **Well Paths**.
If a match is found, the LAS-file is placed as a child of that trajectory. If not, a new empty well path entry is created with the imported LAS-file under it. A well path may have more than one LAS-files as children.

![]({{ site.baseurl }}/images/LasFilesInTree.png)

If the LAS-file does not contain a well name, the file name is used instead. 

#### Moving LAS-file
If ResInsight's automatic well matching fails and a LAS-file is matched with the wrong well path, it is possible to move the LAS-file to the correct well path. Select the LAS-file context menu click **Move LAS File to Well Path** and select destination well path.

![]({{site.baseurl}}/images/MoveLasFileMenu.png)

### Importing Well Path Files
See [Importing Well Paths]({{ site.baseurl }}/docs/wellpaths#ascii-well-trajectories)

### Look for an Existing Well Path
Well log names may vary slightly among different files from the same well. When importing a well log file or a well log path file, ResInsight have to look for an existing well path item to ensure that the well log data and well path are imported to the correct well path item. The lookup is based on name comparison this way:
- First remove any prefix (like `xxxxx1111/1111-` or `xxxxx1111/1111_`)
- Then try an exact name match
- If not found, try to match the names ignoring all spaces, dashes and underscores
- If still no match, no existing well was found and a new one is created

### Well Path Property Editor
The well path property editor lets the user control the appearance of the well path and associate the well path to a simulation well. It also gives some information about the well path metadata.

![]({{ site.baseurl }}/images/WellPathPropertyEditor.png)

- **Appearance group** -- Settings in this group affect the well path appearance in the 3D view
- **File group** -- Information about the well path file
- **Simulation Well group** -- Associated simulation well. ResInsight will try to associate each well path with a simulation well. This is done in the exact same way as looking up an existing well path. If the auto-association fails, the user can set the correct simulation well here.
- **Well Info group** -- Metadata for the well path
- **Well Picks group** -- Information about imported [well picks]({{site.baseurl}}/docs/formations#well-picks) file containing data for the current well path

### Casing Design
Some Casing Design elements can be assigned to the well path by selecting **Create Casign Design** from the context menu of the well path.

This will create a new child object for the Well Path, named **Casing Design**. In the **Casign Design** Property editor 
well path containment properties such as Casing (with Casing Shoe) and Liner can be added to the well path along with a start and end depth and a diameter. 

![]({{ site.baseurl }}/images/CasignDesign.png)

These can be visualised in the 3D View and Well Log Plots on a [Well Log Track]({{site.baseurl}}/docs/welllogsandplots#tracks).

![]({{ site.baseurl }}/images/CasignDesign3D.png) ![]({{ site.baseurl }}/images/CasignDesignPlot.png)
