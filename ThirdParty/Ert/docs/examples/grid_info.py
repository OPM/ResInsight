#!/usr/bin/env python
import sys
from ecl.ecl import EclGrid, EclRegion


def volume_min_max(grid):
    vols = [c.volume for c in grid if c.active]
    return min(vols), max(vols)

def main(grid):
    vmin,vmax = volume_min_max(grid)

    dz_limit = 0.3
    region = EclRegion(grid, False)
    region.select_thin(dz_limit)

    print "Smallest cell     : %g" % vmin
    print "Largest cell      : %g" % vmax
    print "Thin active cells : %d" % region.active_size()

    for ai in region.get_active_list():
        c = grid.cell(active_index=ai)
        print('dz(%2d, %2d, %2d) = %.3f' % (c.i, c.j, c.k, c.dz))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        exit('usage: grid_info.py path/to/file.EGRID')
    case = sys.argv[1]
    grid = EclGrid(case)
    main(grid)
