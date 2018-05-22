---
layout: docs
title: Command File
permalink: /docs/commandfile/
published: true
---

The command file interface allows a sequence of specified commands to be run in order from the command line.
The interface is used by supplying the command file as a [command line parameter]({{site.baseurl}}/docs/commandlineinterface).
Once the command file is finished executing, ResInsight will exit.

## Command List

#### Case Control Commands

* [openProject](#openproject)
* [closeProject](#closeproject)
* [setStartDir](#setstartdir)
* [loadCase](#loadcase)
* [replaceCase](#replacecase)
* [replaceSourceCases](#replacesourcecases)

#### Export Commands

* [exportMultiCaseSnapshots](#exportmulticasesnapshots)
* [exportSnapshots](#exportsnapshots)
* [exportProperty](#exportproperty)
* [exportWellPathCompletions](#exportwellpathcompletions)
* [exportSimWellFractureCompletions](#exportsimwellfracturecompletions)
* [exportMsw](#exportmsw)
* [setExportFolder](#setexportfolder)

#### Other Commands

* [runOctaveScript](#runoctavescript)
* [setMainWindowSize](#setmainwindowsize)
* [computeCaseGroupStatistics](#computecasegroupstatistics)
* [setTimeStep](#settimestep)
* [scaleFractureTemplate](#scalefracturetemplate)
* [setFractureContainment](#setfracturecontainment)


## Syntax


The command file is comprised of a set of commands to be executed in order. Each command must begin on a separate line, i.e. there cannot be two commands on the same line.

The line starts with the command name, followed by parenthesis. Within the parenthesis, parameters can be supplied. Each parameter must be named, followed by an equals sign and its value.

As an example; `openProject(path="/path/to/ResInsightProject.rsp")` will execute the command `openProject` with the parameter `path`. `path` is a string, see [Types](#types) for a list of all types a parameter can have.

Not all parameters are required, in which case they can be omitted and their value will be defaulted. The order of parameters does not matter.

### Types

There are different types of parameters that can be supplied.

| Type    | Input                                                                                              | Example         |
|---------|----------------------------------------------------------------------------------------------------|-----------------|
| Integer | Number                                                                                             | 5               |
| Double  | Decimal number                                                                                     | 3.14            |
| String  | Sequence of characters contained in quotes (`""`)<sup>1</sup>                                      | "/path/to/file" |
| Enum    | Choice of a set of given options                                                                   | ALL             |
| Boolean | `true` or `false`                                                                                  | true            |
| List    | Multiple choices of another type, written within square brackets (`[]`) separated by a comma (`,`) | [1, 2, 3]       |

<sup>1</sup> The backslash (`\`) character is used as an escape character within strings, so to use a quote within a string, use `"escape \" with \""`. To input a literal backslash character, use `"\\"`.

## Case Control Commands


### openProject


Opens a ResInsight project file.

| Parameter | Description                    | Type   | Required |
|-----------|--------------------------------|--------|----------|
| path      | File path to the project file  | String | &#10004; |

#### Example

`openProject(path="/home/user/ResInsightProject.rsp")`


### closeProject

Closes the current open project.

#### Example

`closeProject()`


### setStartDir

Set startup directory.

| Parameter | Description                                    | Type   | Required |
|-----------|------------------------------------------------|--------|----------|
| path      | Path to directory to use as startup directory  | String | &#10004; |

#### Example

`setStartDir(path="/home/user")`


### loadCase

Import Eclipse case from file.

| Parameter | Description                    | Type   | Required |
|-----------|--------------------------------|--------|----------|
| path      | File path to the case to load  | String | &#10004; |

#### Example

`loadCase(path="/home/user/reservoir.EGRID")`


### replaceCase


Replaces a case in the current project with the specified new case.

`openProject` must be called before this command to set which project file is to be used when replacing cases. This command re-opens the project with the new case replaced. To replace more than one case at the same time, use `replaceSourceCases`.

| Parameter   | Description                                        | Type    | Required |
|-------------|----------------------------------------------------|---------|----------|
| newGridFile | File path to the new grid file to replace with     | String  | &#10004; |
| caseId      | ID of the case to replace. Defaults to first case  | Integer |          |

#### Example

`replaceCase(newGridFile="/home/user/otherReservoir.EGRID", caseId=4)`


### replaceSourceCases

Replaces multiple source cases in the current project.

`openProject` must be called before this command to set which project file is to be used when replacing cases. This command re-opens tje project with the cases replaced.

| Parameter    | Description                                                     | Type    | Required |
|--------------|-----------------------------------------------------------------|---------|----------|
| gridListFile | File path to file containing list of cases to replace with      | String  | &#10004; |
| caseGroupId  | ID of group to replace cases in. Defaults to first group        | Integer |          |

#### Example

`replaceSourceCases(gridListFile="C:/resinsight/replacement_files.txt")`


## Export Commands


### exportMultiCaseSnapshots

Replaces the first case in the current project with each case in the given file and saves snapshots of all views.

`openProject` must be called before this command to set which project file is to be used when replacing cases.

Folder to output snapshots should be set using `setExportFolder` with `SNAPSHOTS` type.

| Parameter    | Description                                                        | Type    | Required |
|--------------|--------------------------------------------------------------------|---------|----------|
| gridListFile | File path to file containing list of cases to create snapshots of  | String  | &#10004; |

#### Example

`exportMultiCaseSnapshots(gridListFile="C:\\resinsight\\replacement_files.txt")`


### exportSnapshots

Export snapshots of specified type.

Folder to output snapshots should be set using `setExportFolder` with `SNAPSHOTS` type.

| Parameter | Description                                                                       | Type | Required |
|-----------|-----------------------------------------------------------------------------------|------|----------|
| type      | Type of snapshots to export. Choices: `ALL`, `VIEWS`, `PLOTS`. Defaults to `ALL`  | Enum |          |

#### Example

`exportSnapshots(type=PLOTS)`


### exportProperty

Exports a property to file in Eclipse format.

This command changes the selected property on the first view of the selected case.

| Parameter      | Description                                                                                     | Type    | Required |
|----------------|-------------------------------------------------------------------------------------------------|---------|----------|
| caseId         | ID of case to export property from                                                              | Integer | &#10004; |
| property       | Name of property to export                                                                      | String  | &#10004; |
| eclipseKeyword | Eclipse keyword to use. Defaults to the value of `property` parameter                           | String  |          |
| undefinedValue | Value to use for undefined values. Defaults to 0.0                                              | Double  |          |
| exportFile     | File to export to. Defaults to export folder for `PROPERTIES` with `property` name as filename  | String  |          |

#### Example

`exportProperty(caseId=1, property="SOIL")`


### exportWellPathCompletions

Export well path completions.

| Parameter                   | Description                                                                                                                                               | Type           | Required |
|-----------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|----------------|----------|
| caseId                      | ID of case to export well paths for                                                                                                                       | Integer        | &#10004; |
| timeStep                    | The time step to export completions for                                                                                                                   | Integer        | &#10004; |
| wellPathNames               | Names of well paths to export for. Defaults to all checked wells. If a list of well names are provided, those wells are included even if unchecked        | List of String |          |
| fileSplit                   | How the files are split. Choices: `UNIFIED_FILE`, `SPLIT_ON_WELL`, `SPLIT_ON_WELL_AND_COMPLETION_TYPE`. Defaults to `UNIFIED_FILE`                        | Enum           |          |
| compdatExport               | Chose whether transmissibilities are exported. Choices: `TRANSMISSIBILITIES`, `WPIMULT_AND_DEFAULT_CONNECTION_FACTORS`. Defaults to `TRANSMISSIBILITIES`  | Enum           |          |
| includePerforations         | Whether main bore perforations should be included. Defaults to `true`                                                                                     | Boolean        |          |
| includeFishbones            | Whether fishbones should be included. Defaults to `true`                                                                                                  | Boolean        |          |
| excludeMainBoreForFishbones | Whether main bore completions should be excluded for cells with fishbones. Defaults to `false`                                                           | Boolean        |          |

#### Example

`exportWellPathCompletions(caseId=3, timeStep=5, includeFishbones=false)`

### exportSimWellFractureCompletions

Export fracture completions for simulation wells.

| Parameter                   | Description                                                                                                                                               | Type           | Required |
|-----------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|----------------|----------|
| caseId                      | ID of case to export simulation well fracture completions for                                                                                             | Integer        | &#10004; |
| viewName                    | The name of the view to export simulation well fracture completions for. Since view names are not unique, all views sharing the given name will be used   | String         | &#10004; |
| timeStep                    | The time step to export completions for                                                                                                                   | Integer        | &#10004; |
| simulationWellNames         | Names of simulation wells to export for. Defaults to all checked wells. If a list of names are provided, those wells are included even if unchecked       | List of String |          |
| fileSplit                   | How the files are split. Choices: `UNIFIED_FILE`, `SPLIT_ON_WELL`, `SPLIT_ON_WELL_AND_COMPLETION_TYPE`. Defaults to `UNIFIED_FILE`                        | Enum           |          |
| compdatExport               | Chose whether transmissibilities are exported. Choices: `TRANSMISSIBILITIES`, `WPIMULT_AND_DEFAULT_CONNECTION_FACTORS`. Defaults to `TRANSMISSIBILITIES`  | Enum           |          |

#### Example

`exportSimWellFractureCompletions(caseId=3, viewName="View 2", timeStep=5)`

### exportMsw

Export multi-segment wells.

| Parameter | Description                                    | Type    | Required |
|-----------|------------------------------------------------|---------|----------|
| caseId    | ID of case to export well paths for            | Integer | &#10004; |
| wellPath  | Name of well path to export well segments for  | String  | &#10004; |

#### Example

`exportMsw(caseId=1, wellPath="MainWell")`


### setExportFolder

Set the folder to export different types of data to. Set this before attempting to export data to ensure it is exported to desired location.

| Parameter | Description                                                                                    | Type   | Required |
|-----------|------------------------------------------------------------------------------------------------|--------|----------|
| type      | Type of export folder to set. Choices: `COMPLETIONS`, `SNAPSHOTS`, `PROPERTIES`, `STATISTICS`  | Enum   | &#10004; |
| path      | Directory to export the given type to                                                          | String | &#10004; |

#### Example

`setExportFolder(type=SNAPSHOTS, path="/home/user/snapshots")`


## Other

### runOctaveScript

Execute an Octave script.

| Parameter | Description                                                                                     | Type            | Required |
|-----------|-------------------------------------------------------------------------------------------------|-----------------|----------|
| path      | Path to the octave script to execute                                                            | Integer         | &#10004; |
| caseIds   | The cases to run the octave script on. Defaults to running the script without a specified case  | List of Integer |          |

#### Example

`runOctaveScript(path="/home/user/octave/something.m", caseIds=[1,2,6])`


### setMainWindowSize

Resize the main window to the specified size.

| Parameter | Description                            | Type    | Required |
|-----------|----------------------------------------|---------|----------|
| width     | The width to set for the main window   | Integer | &#10004; |
| height    | The height to set for the main window  | Integer | &#10004; |

#### Example

`setMainWindowSize(width=1920, height=1200)`


### computeCaseGroupStatistics

Compute statistics for statistics cases.

| Parameter | Description                                                          | Type            | Required |
|-----------|----------------------------------------------------------------------|-----------------|----------|
| caseIds   | IDs of statistics cases to compute. Default is all statistics cases  | List of Integer |          |

#### Example

`computeCaseGroupStatistics(caseIds=[5])`

`computeCaseGroupStatistics(caseIds=[2,4,8])`


### setTimeStep

Set the time step for a given case. The time step is used for all views on the case.

| Parameter | Description                      | Type    | Required |
|-----------|----------------------------------|---------|----------|
| caseId    | ID of case to set time step for  | Integer | &#10004; |
| timeStep  | Index of time step to switch to  | Integer | &#10004; |

#### Example

`setTimeStep(caseId=1, timeStep=8)`

### scaleFractureTemplate

Scale fracture template parameters.

| Parameter    | Description                      | Type    | Required |
|--------------|----------------------------------|---------|----------|
| id           | ID of fracture template          | Integer | &#10004; |
| width        | Width scale factor               | Double  |          |
| height       | Height scale factor              | Double  |          |
| dFactor      | D-factor scale factor            | Double  |          |
| conductivity | Conductivity scale factor        | Double  |          |

#### Example

`scaleFractureTemplate(id=1, width=2, height=1.5)`

### setFractureContainment

Set fracture template containment parameters.

| Parameter    | Description                      | Type    | Required |
|--------------|----------------------------------|---------|----------|
| id           | ID of fracture template          | Integer | &#10004; |
| topLayer     | Top layer containment            | Integer | &#10004; |
| baseLayer    | Base layer containment           | Integer | &#10004; |

#### Example

`setFractureContainment(id=1, topLayer=2, baseLayer=7)`
