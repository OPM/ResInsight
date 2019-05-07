#!/usr/bin/env python
import sys
from operator import itemgetter
from ecl.eclfile import EclFile
from ecl.grid import EclGrid




if __name__ == "__main__":
    case = sys.argv[1]
    grid_file = EclFile("%s.EGRID" % case)
    init_file = EclFile("%s.INIT" % case)
    grid = EclGrid("%s.EGRID" % case)

    nnc1 = grid_file["NNC1"][0]
    nnc2 = grid_file["NNC2"][0]
    tran = init_file["TRANNNC"][0]

    nnc_list = []
    for g1,g2,t in zip(nnc1,nnc2,tran):
        nnc_list.append((g1,g2,t))

    nnc_list = sorted(nnc_list, key = itemgetter(0))
    for (g1,g2,T) in nnc_list:
        # grid_ijk assumes 0-based indexing, g1/g2 are 1-based (FORTRAN)
        # Convert them to zero based ones.
        g1 = g1 - 1
        g2 = g2 - 1
        i1,j1,k1 = grid.get_ijk( global_index = g1 )
        i2,j2,k2 = grid.get_ijk( global_index = g2 )

        # print 1-based indices just like in eclipse PRT files
        print "(%02d,%02d,%02d) -> (%02d,%02d,%02d)  T:%g" % (i1+1,j1+1,k1+1,i2+1,j2+1,k2+1,T)



