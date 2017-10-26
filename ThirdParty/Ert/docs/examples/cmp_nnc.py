#!/usr/bin/env python
import sys
from operator import itemgetter
from ecl.ecl import EclFile, EclGrid




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
        i1,j1,k1 = grid.get_ijk( global_index = g1 )
        i2,j2,k2 = grid.get_ijk( global_index = g2 )

        print "(%02d,%02d,%02d) -> (%02d,%02d,%02d)  T:%g" % (i1,j1,k2,i2,j2,k2,T)



