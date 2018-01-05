---
layout: docs
title: Octave Interface Reference
permalink: /docs/octaveinterfacereference/
published: true
---

## Introduction
To identify a ResInsight case uniquely in the Octave script, an integer Id (CaseId) is used. This Id can be retrieved in several ways, but there are two main modes of operation regarding this for a particular octave script: Either the script is designed to work on a single case (the "Current Case"), or the script is designed to access the selection and traverse the cases by itself.

<div class="note info"> 
<b>Note:</b> The Octave interface does not support Geomechanical cases and flow diagnostic results. 
</div>

### Single Case Scripts
Single case scripts do not need to address cases explicitly, but works on what ResInsight considers being the "Current Case". When the user selects several cases and executes a script on them, ResInsight loops over all cases in the selection, sets the current case and executes the script. All references to the "Current Case" from the script will then refer to the case currently being processed by ResInsight. 
The Current Case can be accessed directly using **riGetCurrentCase()**, but the more direct way is to *omit the CaseId parameter* in the functions, the Current Case is then automatically used. 

### Multi Case Scripts
Scripts can access the selection state in ResInsight, and also retrieve lists of Case Groups and cases including some meta information. This can be used if the scripts need to get values from some cases, and store the results in others, etc.

### Case Types
The case type (Labeled "CaseType" in the following specification) of a case is returned as a text string when retrieving lists of cases, and is one of the following:

| Case Type      | Description |
|----------------|-------------|
|ResultCase     | A binary Eclipse case |
|InputCase      | A case based on ASCII Eclipse input data |
|StatisticsCase | A statistics case based on many source cases in Grid Case Group | 
|SourceCase     | A binary Eclipse case in a Grid Case Group |

### Unresolved Issues
The issue around having multiple instances of ResInsight is still not addressed, but might affect the function signatures by adding a port number parameter to all of them. We will try to find ways to avoid this, but are still not certain that we will succeed.

## Specification

### Project Information
The case information is presented in an octave Structure called CaseInfo, and contains the following fields:

    CaseInfo = {
      CaseId      = int    # A project-unique integer used to address this
                           # particular case 
      CaseName    = string # The name that has been assigned to the case
                           # in ResInsight.
      CaseType    = string # See the description above
      CaseGroupId = int    # A project-unique integer identifying the
                           # CaseGroup this case is a member of.
                           # -1 if not in a CaseGroup. Valid only for 
                           # Statistics-, and SourceCases
    }

#### CaseInfo riGetCurrentCase()
This function returns a CaseInfo Structure for the Case considered being the "Current Case" by ResInsight. When ResInsight loops over a selection of cases and executes an Octave script for each of them, this function returns the CaseInfo for that particular Case. 

#### Vector[CaseInfo] riGetSelectedCases()
This function returns a CaseInfo Structure for each of the cases selected in ResInsight at the time when the script launched. 

#### Vector[CaseGroupInfo] riGetCaseGroups()
This function returns a CaseGroupInfo Structure for each of the case groups in the current ResInsight project.

    CaseGroupInfo = {
      CaseGroupId   = int    # A project-unique integer used to address
                             # this particular CaseGroup
      CaseGroupName = string # The name assigned to the CaseGroup 
                             # in ResInsight
    }
    
#### Vector[CaseInfo] riGetCases([CaseGroupId])
This function returns a CaseInfo Structure for all the cases in the current ResInsight project, including the Statistics cases and Source cases in a Grid Case Group. 
If a CaseGroupId is provided, only the cases in that Case Group will be returned.
 
### Retrieving Grid Metadata

#### Matrix[numActiveCells][9] riGetActiveCellInfo([CaseId], [PorosityModel = "Matrix"|"Fracture"] )
This function returns a two dimensional matrix containing grid and IJK information about each of the active cells in the requested case. The columns contain the following information:

    [GridIdx, I, J, K, ParentGridIdx, PI, PJ, PK, CoarseBoxIdx]
      GridIdx       # The index of the grid the cell resides in.
                    # Main grid has index 0
      I, J, K       # 1-based index address of the cell in the grid.
      ParentGridIdx # The index to the grid that this cell's grid 
                    # is residing in.
      PI, PJ, PK    # 1-based address of the parent grid cell that
                    # this cell is a part of.
      CoarseBoxIdx  # 1-based coarsening box index, -1 if none.
                    # Coarsening box info can be retrieved using
                    # **riGetCoarseningInfo()**
                    
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[numCoarseGroups][6] riGetCoarseningInfo([CaseId])
This function returns all coarse box definitions used in the grid.
The columns contain the following information:
[I1, I2, J1, J2, K1, K2]: 1-based index addresses of the min and max corners of the coarsening box.
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[numGrids][3] riGetGridDimensions([CaseId])
This function returns a two dimensional matrix: One row for each grid, starting with the main grid. 

*NOTE*: This means that the "normal" GridIndices where 0 means Main Grid does not work directly with this matrix. You have to add 1.

The columns contain the following information:
[NI, NJ, NK]:  I, J, K dimensions of the grid. 
If the CaseId is not defined, ResInsight's Current Case is used.

#### Vector[TimeStepDate] riGetTimeStepDates([CaseId])
This function returns the date information for each of the time steps in the case as a Vector of Structures.
The Structure is defined as: 

    TimeStepDate = {
      Year    = int # The year eg. 2013
      Month   = int # The month. Eg. 12
      Day     = int # The day in the month. Eg. 24
      Hour    = int # The hour of the day. Eg. 17
      Minute  = int # The minute in the hour. Eg. 55
      Second  = int # The second within the minute. Eg. 30
    }
    
If the CaseId is not defined, ResInsight's Current Case is used.

#### Vector[DecimalDay] riGetTimeStepDays([CaseId])
This function returns the time from the simulation start as decimal days for all the time steps as a Vector of doubles.
If the CaseId is not defined, ResInsight's Current Case is used.
 
### Retrieving Property Data

#### Vector[PropertyInfo] riGetPropertyNames([CaseId] ], [PorosityModel = "Matrix"|"Fracture"])
This function returns the name and type of all the properties in the case as a Vector of Structures.
The Structure is defined as: 

    PropertyInfo {
      PropName    = string # Name of the property as received from
                           # the analysis tool
      PropType    = string # The type of the property: "StaticNative",
                           # "DynamicNative", "Input", "Generated"
    }
    
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[numActiveCells][numTimestepsRequested] riGetActiveCellProperty([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])

This function returns a two dimensional matrix: [ActiveCells][Num TimestepsRequested] containing the requested property data from the case with CaseId.
If the case contains coarse-cells, the results are expanded onto the active cells. 
If the CaseId is not defined, ResInsight's Current Case is used. 
The RequestedTimeSteps must contain a list of indices to the requested time steps. If not defined, all the time steps are returned.

#### Matrix[numI][numJ][numK][numTimestepsRequested] riGetGridProperty([CaseId], GridIndex , PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])
This function returns a matrix of the requested property data for all the grid cells in the requested grid for each requested time step.
Grids are indexed from 0 (main grid) to max number of LGR's 
If the CaseId is not defined, ResInsight's Current Case is used.
The RequestedTimeSteps must contain a list of indices to the requested time steps. If not defined, all the time steps are returned.
Writing Back to ResInsight

#### riSetActiveCellProperty( Matrix[numActiveCells][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
Interprets the supplied matrix as a property set defined for the active cells in the case, and puts the data into ResInsight as a "Generated" property with the name "PropertyName". 
The "TimeStepIndices" argument is used to "label" all the time steps present in the supplied data matrix, and must thus be complete. The time step data will then be put into ResInsight at the time steps requested.  
If the CaseId is not defined, ResInsight's Current Case is used.

#### riSetGridProperty( Matrix[numI][numJ][numK][numTimeSteps], [CaseId], GridIndex, PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
Interprets the supplied matrix as a property set defined for all cells in one of the grids in a case, and puts the data into ResInsight as a "Generated" property with the name "PropertyName". 
The "TimeStepIndices" argument is used to "label" all the time steps present in the supplied data matrix, and must thus be complete. The time step data will then be put into ResInsight at the time steps requested.
If the CaseId is not defined, ResInsight's Current Case is used.
 
### Cell Geometry Functions

#### Matrix[numI][numJ][numK][3] riGetCellCenters([CaseId], GridIndex)
This function returns the UTM coordinates (X, Y, Z) of the center point of all the cells in the grid.
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[ActiveCells][3] riGetActiveCellCenters([CaseId], [PorosityModel = "Matrix"|"Fracture"])
This function returns the UTM coordinates (X, Y, Z) of the center point of each of the active cells.
If the CaseId is not defined, ResInsight's Current Case is used.
Cell Corner Index layout
The corner indices follow the ECLIPSE standard:

          6-------------7             |k     
         /|            /|             | /j   
        / |           / |             |/     
       /  |          /  |             *---i  
      4-------------5   |
      |   |         |   |
      |   2---------|---3
      |  /          |  /
      | /           | /
      |/            |/
      0-------------1

#### Matrix[numI][numJ][numK][8][3] riGetCellCorners([CaseId], GridIndex)
This function returns the UTM coordinates(X, Y, Z) of the 8 corners of all the cells in the grid.
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[ActiveCells][8][3] riGetActiveCellCorners([CaseId], [PorosityModel = "Matrix"|"Fracture"])
This function returns the UTM coordinates (X, Y, Z) of the 8 corners of each of the active cells.
If the CaseId is not defined, ResInsight's Current Case is used.

### Well Data Functions

#### Vector[WellNames] riGetWellNames([CaseId])
This function returns the names of all the wells in the case as a Vector of strings.
If the CaseId is not defined, ResInsight's Current Case is used.
Vector[WellCellInfo] riGetWellCells([CaseId], WellName, TimeStep)
This function returns the cells defined in the specified well for the time step requested as a vector of Structures. The Structure is defined as:

    WellCellInfo {
      I, J, K     = int # Index to the cell in the grid
      GridIndex   = int # the index of the grid. Main grid has index 0.
      CellStatus  = int # is either 0 or 1, meaning the cell is closed
                        # or open respectively
      BranchId    = int # Branch id of the branch intersecting the cell
      SegmentId   = int # Branch segment id of the branch intersecting the cell
    }
    
If the CaseId is not defined, ResInsight's Current Case is used.

#### Vector[WellStatus] riGetWellStatus ([CaseId], WellName, [RequestedTimeSteps])
This function returns the status information for a specified well for each requested time step as a vector of Structures. The Structure is defined as:

    WellStatus {
      WellType    = string # "Producer", "OilInjector",
                           # "WaterInjector", "GasInjector", "NotDefined"
      WellStatus  = int    # is either 0 or 1, meaning the well is shut
                           # or open respectively
    }
    
If the CaseId is not defined, ResInsight's Current Case is used.

#### Matrix[numSelectedCells][5] riGetSelectedCells([CaseId])

This function returns a two dimensional matrix containing the cell info for each selected cell in the case with `CaseId`.
The columns contain the following information:

    [CaseId, GridIdx, I, J, K]
      CaseId  # The ID of the case the cell resides in.
      GridIdx # The index of the grid the cell resides in.
              # Main grid has index 0
      I, J, K # 1-based index of the cell in the grid.


If the CaseId is not defined, ResInsight's Current Case is used.


#### Matrix[numSelectedCells][numTimestepsRequested] riGetGridPropertyForSelectedCells([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"] )

This function returns a two dimensional matrix: [numSelectedCells][numTimestepsRequested] containing the requested property data from the case with CaseId.

If the CaseId is not defined, ResInsight's Current Case is used.
The RequestedTimeSteps must contain a list of 1-based indices to the requested time steps. If not defined, all the time steps are returned.


#### Vector[PropertyInfo] riGetNNCPropertyNames([CaseId])

This function returns the name and type of all NNC properties in the case as a vector of structures.

The structure is defined as:

    PropertyInfo {
      PropName    = string # Name of the property as received from
                           # the analysis tool
      PropType    = string # The type of the property: "StaticNative",
                           # "DynamicNative", "Generated"
    }

If the CaseId is not defined, ResInsight's Current Case is used.


#### Matrix[numNNCConnections][2] riGetNNCConnections([CaseId])

This function returns a two dimensional matrix containing grid and IJK information about each NNC connection.
Each row contains a from and to cell for the connection.
The cells are specified in a structure defined as:

    CellInfo = {
      GridIndex = int # Index of the grid the cell resides in.
                      # Main grid has index 0.
      I, J, K   = int # 1-based index address of the cell in the grid.
    }

#### Matrix[numConnections][numTimestepsRequested] riGetDynamicNNCValues([CaseId], PropertyName, [RequestedTimeSteps])

This function returns a two dimensional matrix: [Num Connections][Num Time Steps Requested] containing the value of the requested property from the case with CaseId. The order of connections is the same as the order from `riGetNNCConnections`.

If the CaseId is not defined, ResInsight's Current Case is used.
The RequestedTimeSteps must contain a list of indices to the requested time steps. If not defined, all the time steps are returned.

#### Vector[numConnections] riGetStaticNNCValues([CaseId], PropertyName)

This function returns a vector of values for the requested static property for each NNC connection. The order of connections is the same as the order from `riGetNNCConnections`.

If the CaseId is not defined, ResInsight's Current Case is used.

#### riSetNNCProperty(Matrix[numNNCConnections][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices])

Interprets the supplied matrix as a property set defined for the NNC connections in the case, and puts the data into ResInsight as a "Generated" property with the name "PropertyName".
The "TimeStepIndices" argument is used to "label" all the steps present in the supplied data matrix and must thus be complete.
The time step data will then be put into ResInsight at the time steps requested.

If the CaseId is not defined, ResInsight's Current Case is used.
