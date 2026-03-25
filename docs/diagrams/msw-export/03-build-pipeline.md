# MSW Export: Detailed Build Pipeline

## buildMswWellExportData — main bore + completions

```mermaid
flowchart TD
    ENTRY["buildMswWellExportData<br/>(RimEclipseCase, RimWellPath,<br/>maxSegmentLength, completionType)"]

    ENTRY --> GCS["generateCellSegments<br/>-> WellPathCellIntersectionInfo list"]
    GCS --> INITMD["computeInitialMeasuredDepth<br/>-> initialMD / initialTVD"]
    INITMD --> FI["filterIntersections<br/>clip list to heel MD"]

    FI --> HEADER["Build WelsegsHeader<br/>(well name, topLength, topDepth,<br/>infoType, pressureComponents)"]

    FI --> MB["buildMainBoreBranchFromGeometry<br/>-> RigMswBranch (branch 1)"]
    MB --> CSMAP["fills CellSegmentEntry map<br/>(MD range -> segment number)"]

    CSMAP --> SVALVE["standalone valves from<br/>RimWellPathValve (valveCollection)<br/>-> embed WsegvalvRow on segment"]

    CSMAP --> VB["buildValveBranchesFromGeometry<br/>(ICD/ICV/AICD/SICD inside perforations)<br/>-> vector~RigMswBranch~"]

    CSMAP --> FRB["buildFractureBranchesFromGeometry<br/>(if FRACTURES flag set)<br/>-> vector~RigMswBranch~"]

    CSMAP --> FBB["buildFishbonesBranchesFromGeometry<br/>(if FISHBONES flag set)<br/>-> vector~RigMswBranch~"]

    CSMAP --> LAT["buildLateralBranches (recursive)<br/>for each child RimWellPath with tie-in<br/>-> vector~RigMswBranch~"]

    HEADER & MB & VB & FRB & FBB & LAT --> RESULT["RigMswWellExportData<br/>{ header, branches }"]
```

## buildLateralBranches — recursive lateral handling

```mermaid
flowchart TD
    LAT["buildLateralBranches<br/>(eclipseCase, wellPath, mainGrid,<br/>outletSegNum, completionType)"]

    LAT --> TIEINMD["read tieInMD / tieInTVD<br/>from RimWellPathTieIn"]

    TIEINMD --> TICV{"outletValve<br/>(ICV) active?"}
    TICV -- yes --> TIEINSEG["build tie-in RigMswSegment<br/>+ WsegvalvRow<br/>-> stored as branch.tieInValve"]
    TICV -- no --> SKIP[use outletSegNum directly]

    TIEINSEG & SKIP --> GCS2["generateCellSegments<br/>filterIntersections"]

    GCS2 --> MB2["buildMainBoreBranchFromGeometry<br/>-> RigMswBranch (lateral)"]
    MB2 --> SVALVE2["standalone valves<br/>(valveCollection)"]
    MB2 --> CSMAP2["CellSegmentEntry map"]

    CSMAP2 --> VB2["buildValveBranchesFromGeometry"]
    CSMAP2 --> FRB2["buildFractureBranchesFromGeometry"]
    CSMAP2 --> FBB2["buildFishbonesBranchesFromGeometry"]

    CSMAP2 --> GRANDCHILD["for each grandchild wellPath<br/>buildLateralBranches (recurse)"]

    VB2 & FRB2 & FBB2 & GRANDCHILD --> RESULT2["vector~RigMswBranch~"]
```

## collectTableData — RigMswWellExportData to RigMswTableData

```mermaid
flowchart TD
    INPUT["RigMswWellExportData<br/>{ header, branches }"]

    INPUT --> HROW["setWelsegsHeader<br/>-> WelsegsHeader"]

    INPUT --> LOOP["for each RigMswBranch"]
    LOOP --> TIEIN{"tieInValve<br/>present?"}
    TIEIN -- yes --> EMIT0["emitSegment(tieInValve)"]
    TIEIN --> SEGLOOP["for each RigMswSegment"]

    SEGLOOP --> WROW["addWelsegsRow<br/>(segment, branch, outlet, length, depth)"]
    SEGLOOP --> CROW["addCompsegsRow per<br/>RigMswCellIntersection<br/>(i,j,k, branch, distStart, distEnd)"]
    SEGLOOP --> VALVROW{"valve data?"}
    VALVROW -- wsegvalvData --> VV["addWsegvalvRow"]
    VALVROW -- wsegaicdData --> VA["addWsegaicdRow"]
    VALVROW -- wsegsicdData --> VS["addWsegsicdRow"]

    LOOP --> ADDMB["addMswBranch<br/>(store RigMswBranch reference)"]

    HROW & WROW & CROW & VV & VA & VS & ADDMB --> TD["RigMswTableData"]
```
