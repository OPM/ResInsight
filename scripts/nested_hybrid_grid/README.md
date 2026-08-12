# Nested hybrid grid: FIPNEST INIT-file tooling (#14510)

ResInsight can import a "nested hybrid grid" (a single flat EGRID with the refined cells
appended in per-level I bands) and rebuild it as a true LGR hierarchy. The parent-child
description normally comes from the `<gridbase>_REFINE.grdecl` + `<gridbase>_OLDIJK.grdecl`
sidecar files. Issue #14510 prototypes a compact alternative that travels inside the INIT
file instead, as three full-length (NX*NY*NZ) INTE arrays:

- `FIPNEST` — per cell, the 1-based flat natural index of the immediate parent cell
  (0 for unrefined cells). Refined-away host slots carry their own link to their coarse
  host, so every chain terminates in an unrefined cell.
- `FIPSLOT` — `1 + offI + 100*offJ + 10000*offK`, the cell's position within its parent.
- `REFINE` — the per-cell nesting level (unchanged from the existing sidecar).

FIPNEST alone is not sufficient: the refinement level is not the parent-chain depth
(several levels can refine the coarse grid directly), and within-parent placement is
ambiguous from flat coordinates when a level has hole layers. See
`RigNestedHybridGridFipnestCodec` for the full reasoning.

## Workflow

1. Import the grid into ResInsight once with the REFINE/OLDIJK sidecars present. On a
   successful reconstruction ResInsight auto-exports `<gridbase>_FIPNEST.grdecl` next to
   the grid file (skipped if the file already exists).
2. Insert the arrays into a copy of the INIT file:

   ```bash
   python insert_fipnest_into_init.py DROGON_NESTED_FIPNEST.grdecl \
       DROGON_NESTED.INIT staged/DROGON_NESTED.INIT
   ```

   Requires `resfo` and `numpy` (`pip install resfo`).
3. Place the EGRID and the augmented INIT in a directory *without* the sidecar files and
   open it in ResInsight. The log shows
   `Nested hybrid grid: reconstructing from the FIPNEST/FIPSLOT/REFINE arrays in ...` and
   the reconstructed LGR hierarchy is identical to the sidecar-based import (covered by
   the `RigNestedHybridGridFipnestTest` unit tests).

When both the sidecars and a FIPNEST-augmented INIT are present, the INIT arrays take
precedence and the sidecars remain as the fallback.
