---
layout: docs
title: Well Trajectories
permalink: /docs/wellpaths/
published: true
---
![]({{ site.baseurl }}/images/ResInsight_WellPathWithSimulationWell.png)

ResInsight can import Well trajectories from simple Ascii files. 
In addition, a Statoil specific solution to retrieve Well Trajectories from their internal web service is implemented.

## Ascii Well Trajectories

The command **File -> Import -> Import Well Paths From File** will read the well paths in the selected file, and create one entry for each trajectory under the  ![]({{ site.baseurl }}/images/WellCollection.png) **Wells** item in the **Project Tree**. 

The supported ASCII format is quite flexible but the main requirements are: 

- Each data line must contain four numbers: X Y TVD MD separated with white-space.
- Lines starting with "--" or "#" is considered to be comment lines
- A line starting with none-number-characters are used as a well name after the following rules:
  - If the line contains a pair of : _```  "'", "`", "´", "’" or "‘" ```_ the text between the quotation marks is used as a well name.
  - If the line contains the case insensitive string "name" with an optional ":" after then the rest of the line is used as a well name. 
  - If there are no quotes or "name"'s, the complete line is used as a well name.
  - If there are several consecutive name-like lines, only the last one will be used 
- If a well name is found, a new well is created and the following data points are added to it.

#### Example 1:

	WELLNAME: WELL1
    4507.0	5638.5	0.0	0.0
    4507	5638.5	4628.6	1628.6
	4297.4	5938.5	4632.4	1998.387
    -999
    WELLNAME: WELL2
	5507.0	4638.5	0.0	0.0
    5507	4638.5	3628.6	1628.6
    5297.4	4938.5	3632.4	1998.387
	-999

#### Example 2:
    X Y TVD MD
    Name Well_1
  	5507.0	4638.5	0.0	0.0
    5507	4638.5	3628.6	1628.6
    5297.4	4938.5	3632.4	1998.387
    
    -- A Comment new well
    This is not its name
    Name Well_2
    5507.0	4638.5	0.0	0.0
    5507	4638.5	3628.6	1628.6
    # a comment inside the data
    5297.4	4938.5	3632.4	1998.387
    
    3Q AHB-J
    5507.0	4638.5	0.0	0.0
    5507	4638.5	3628.6	1628.6

    
### Trajectory files are referenced
The trajectory data is not copied into the ResInsight project as such. The project file only stores the file path, and the next time you open the project, ResInsight will try to read the well data from the file again.  

<div class="note info">
If the well trajectory file is changed and you would like a running ResInsight to update accordingly, you will need to delete all the well trajectories that emerge from that file, and import it again.
</div>

## Importing from SSI-Hub (Internal Statoil web-service)

In order to import from SSI-Hub, a project file must be present and stored to file. All imported well paths from the web service will be stored relative to this project file location. If no project file exists, the menu item is disabled.

The command **File -> Import -> Import Well Paths From SSI-hub** launches a wizard to guide you through the process of selecting the well trajectories you need.

<div class="note info">
<h5>Access to web service</h5>
The import of well paths is using a web service. If you are a Statoil employee, make sure you have access to "EDM Landmark" and "EDM Compass".
</b>
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

