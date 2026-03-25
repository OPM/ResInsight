# Fishbones MSW Model

## Well path layout

Each fishbones "sub" position on the main bore spawns one ICD branch and one
lateral branch per installed tube. All segments belong to `RigMswBranch`
structs; connectivity is expressed through `outletSegmentNumber`.

```
Heel (seg 1)
  |
  |  Branch 1 — main bore
  |
  o--[seg 2]--[seg 3]--[seg 4]--[seg 5]--[seg 6]-- ... --[seg N]-- Toe
                 |                  |
             Sub position       Sub position
                 |                  |
            [ICD seg]           [ICD seg]        <- Branch 2, 5, ...  (one per sub)
            (WSEGVALV)          (WSEGVALV)
             /     \             /     \
       [lat seg] [lat seg]  [lat seg] [lat seg]  <- Branch 3/4, 6/7, ... (one per lateral)
       COMPSEGS  COMPSEGS   COMPSEGS  COMPSEGS
```

## Branch and segment hierarchy

```mermaid
graph LR
    classDef mainBore fill:#4a90d9,color:#fff,stroke:#2c6fad
    classDef icd      fill:#e8a838,color:#fff,stroke:#b07e1e
    classDef lateral  fill:#5cb85c,color:#fff,stroke:#3d7a3d
    classDef heel     fill:#888,color:#fff,stroke:#555

    HEEL(["Heel<br/>seg 1"]):::heel

    HEEL --> S2["seg 2"]:::mainBore
    S2   --> S3["seg 3"]:::mainBore
    S3   --> S4["seg 4<br/>(sub A position)"]:::mainBore
    S4   --> S5["seg 5"]:::mainBore
    S5   --> S6["seg 6<br/>(sub B position)"]:::mainBore
    S6   --> TOE(["Toe<br/>seg N"]):::mainBore

    S4 --> ICD_A["ICD seg<br/>Branch 2<br/>WSEGVALV"]:::icd
    ICD_A --> LA1["Lateral 1<br/>Branch 3<br/>COMPSEGS"]:::lateral
    ICD_A --> LA2["Lateral 2<br/>Branch 4<br/>COMPSEGS"]:::lateral

    S6 --> ICD_B["ICD seg<br/>Branch 5<br/>WSEGVALV"]:::icd
    ICD_B --> LB1["Lateral 1<br/>Branch 6<br/>COMPSEGS"]:::lateral
    ICD_B --> LB2["Lateral 2<br/>Branch 7<br/>COMPSEGS"]:::lateral
```

## Per-sub data structure

```mermaid
flowchart TD
    SUB["RimFishbones sub<br/>(subIndex, measuredDepth)"]

    SUB --> ICD_SEG["RigMswSegment — ICD<br/>- segmentNumber = icdSegNum<br/>- outletSegmentNumber = main-bore seg at sub MD<br/>- wsegvalvData: cv, area (from icdCount + orifice diameter)<br/>- intersections: cells closest to sub (COMPSEGS)"]

    ICD_SEG --> LAT_BR["RigMswBranch per lateral<br/>(one branch per installed tube)"]

    LAT_BR --> LAT_SEG["RigMswSegment per grid-cell intersection<br/>- outletSegmentNumber chains along lateral<br/>  (first seg -> icdSegNum)<br/>- diameter = equivalentDiameter<br/>- roughness = openHoleRoughnessFactor<br/>- intersections: COMPSEGS (deduplicated<br/>  against cells already used by ICD or<br/>  other laterals on same sub)"]
```

## Outlet segment lookup

The ICD segment outlet is found with `findOutletSegmentForMD`:

```
cellSegMap (built during main-bore pass):

  [startMD ---- midpoint ---- endMD] -> lastSubSegmentNumber
  [  200.0 ---- 212.5 ----  225.0 ] -> seg 3
  [  225.0 ---- 237.5 ----  250.0 ] -> seg 4   <- sub A at MD 243 connects here
  [  250.0 ---- 262.5 ----  275.0 ] -> seg 5
  [  275.0 ---- 287.5 ----  300.0 ] -> seg 6   <- sub B at MD 290 connects here

Rule: pick the entry whose midpoint is closest to, but not greater than, the sub MD.
```
