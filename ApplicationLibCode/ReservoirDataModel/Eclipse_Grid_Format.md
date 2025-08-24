# Eclipse Grid Format: Pillars, COORD and ZCORN

## Overview

Eclipse uses a corner point geometry representation for reservoir grids. This format consists of two main arrays:
- **COORD**: Defines the pillar coordinates 
- **ZCORN**: Defines the depth values at pillar intersections

## Grid Structure and Pillars

### What are Pillars?

Pillars are vertical lines that define the structural framework of the grid. Each pillar connects corresponding points from the top to the bottom of the reservoir.

```
Grid Layout (2x3 example):
   
   Y ^
     |
     +----> X

Pillars (shown as |):
     |     |     |
     |     |     |  <- (nx+1) = 3 pillars in X direction
     |     |     |
   j=3 --- --- ---
     |     |     |
     |     |     |  
     |     |     |
   j=2 --- --- ---
     |     |     |
     |     |     |
     |     |     |
   j=1 --- --- ---
     |     |     |
     |     |     |
     |     |     |
   j=0 --- --- ---
    i=0   i=1   i=2

Total pillars: (nx+1) × (ny+1) = 3 × 4 = 12 pillars
```

## COORD Array

### Structure
- Size: `(nx+1) × (ny+1) × 6` values
- Each pillar has 6 values: `[x1, y1, z1, x2, y2, z2]`
  - `(x1,y1,z1)`: Top point of pillar
  - `(x2,y2,z2)`: Bottom point of pillar

### Organization
```
For pillar at position (i,j):
  pillarIndex = j × (nx+1) + i
  coordIndex = pillarIndex × 6
  
  COORD[coordIndex + 0] = x_top
  COORD[coordIndex + 1] = y_top  
  COORD[coordIndex + 2] = z_top
  COORD[coordIndex + 3] = x_bottom
  COORD[coordIndex + 4] = y_bottom
  COORD[coordIndex + 5] = z_bottom
```

### Example (2x3 grid)
```
Pillar (0,0): x1=456064, y1=5.936e6, z1=1709.18, x2=456064, y2=5.936e6, z2=1715.78
Pillar (1,0): x1=456194, y1=5.936e6, z1=1707.43, x2=456194, y2=5.936e6, z2=1714.08
...
```

## ZCORN Array

### Structure
- Size: `nx × ny × nz × 8` values
- Contains Z-coordinates for each corner of every cell
- 8 corners per cell: 4 top corners + 4 bottom corners

### Cell Corner Indexing

Each cell has 8 corners with this indexing convention:
```
Top Face (NEG_K):        Bottom Face (POS_K):
    3 ---- 2                 7 ---- 6
    |      |                 |      |  
    |      |                 |      |
    0 ---- 1                 4 ---- 5

Corner mapping:
0: (-I,-J,top)    4: (-I,-J,bottom)
1: (+I,-J,top)    5: (+I,-J,bottom)  
2: (+I,+J,top)    6: (+I,+J,bottom)
3: (-I,+J,top)    7: (-I,+J,bottom)
```

### ZCORN Organization

The ZCORN array is organized by layers and interfaces:

```
For each K layer:
  Top interface (NEG_K face):
    For each J row:
      Face 1: corners (0,3) for all cells in row → [z0, z3, z0, z3, ...]
      Face 2: corners (1,2) for all cells in row → [z1, z2, z1, z2, ...]
      
  Bottom interface (POS_K face):  
    For each J row:
      Face 1: corners (0,1) for all cells in row → [z4, z5, z4, z5, ...]
      Face 2: corners (3,2) for all cells in row → [z7, z6, z7, z6, ...]
```

### Pillar Intersection Sharing

Adjacent cells share pillar intersections, creating the characteristic duplication pattern:

```
2x3 Grid Cell Layout:
+---+---+ j=2
| 4 | 5 |
+---+---+ j=1  
| 2 | 3 |
+---+---+ j=0
| 0 | 1 |
+---+---+
i=0 i=1 i=2

Cell 0 corners (0,1,3,2) share pillars with:
- Corner 1 shared with Cell 1 corner 0
- Corner 2 shared with Cell 2 corner 0  
- Corner 3 shared with Cell 2 corner 1, Cell 4 corner 0, Cell 1 corner 3
```

### Example ZCORN Pattern (2x3x4 grid)

For layer k=0, the pattern shows shared intersections:
```
Layer 0 Top Interface:
J=0, Face 1: 1709.18 1707.43 1707.43 1715.72  <- Cell(0,0) and Cell(1,0)
J=0, Face 2: 1706.67 1705.04 1705.04 1709.57  <- shared pillar values
J=1, Face 1: 1706.67 1705.04 1705.04 1709.57  <- duplication from sharing
J=1, Face 2: 1701.68 1697.96 1697.96 1703.33
...
```

## Key Insights

1. **Pillar Continuity**: Pillars extend vertically through all layers, maintaining structural continuity
2. **Shared Intersections**: Adjacent cells share pillar intersection points, creating value duplication in ZCORN
3. **Interface Organization**: ZCORN is organized by layer interfaces (top/bottom), not individual cell corners
4. **Face-Based Grouping**: Within each interface, values are grouped by faces rather than individual corners

## Implementation Notes

The conversion from RigCell to Eclipse format requires:
1. Extracting pillar coordinates for COORD array
2. Organizing cell face corners according to Eclipse ZCORN specification
3. Ensuring proper pillar intersection sharing between adjacent cells
4. Following the specific layer-interface-face organization pattern