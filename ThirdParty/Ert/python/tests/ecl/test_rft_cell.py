#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'test_rft_cell.py' is part of ERT - Ensemble based Reservoir Tool.
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 


from ecl.ecl import EclRFTCell, EclPLTCell
from ecl.test import ExtendedTestCase


# def out_of_range():
#     rftFile = ecl.EclRFTFile(RFT_file)
#     rft = rftFile[100]


class RFTCellTest(ExtendedTestCase):
    def setUp(self):
        self.RFT_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.RFT")
        self.PLT_file = self.createTestPath("Statoil/ECLIPSE/RFT/TEST1_1A.RFT")

    def test_RFT_cell(self):
        i = 10
        j = 8
        k = 100
        depth = 100
        pressure = 65
        swat = 0.56
        sgas = 0.10
        cell = EclRFTCell(i, j, k, depth, pressure, swat, sgas)

        self.assertEqual(i, cell.get_i())
        self.assertEqual(j, cell.get_j())
        self.assertEqual(k, cell.get_k())

        self.assertFloatEqual(pressure, cell.pressure)
        self.assertFloatEqual(depth, cell.depth)
        self.assertFloatEqual(swat, cell.swat)
        self.assertFloatEqual(sgas, cell.sgas)
        self.assertFloatEqual(1 - (sgas + swat), cell.soil)


    def test_PLT_cell(self):
        i = 2
        j = 16
        k = 100
        depth = 100
        pressure = 65
        orat = 0.78
        grat = 88
        wrat = 97213
        conn_start = 214
        conn_end = 400
        flowrate = 111
        oil_flowrate = 12
        gas_flowrate = 132
        water_flowrate = 13344

        cell = EclPLTCell(i, j, k, depth, pressure, orat, grat, wrat, conn_start, conn_end, flowrate,
                          oil_flowrate, gas_flowrate, water_flowrate)


        self.assertEqual(i, cell.get_i())
        self.assertEqual(j, cell.get_j())
        self.assertEqual(k, cell.get_k())

        self.assertFloatEqual(pressure, cell.pressure)
        self.assertFloatEqual(depth, cell.depth)
        self.assertFloatEqual(orat, cell.orat)
        self.assertFloatEqual(grat, cell.grat)
        self.assertFloatEqual(wrat, cell.wrat)

        self.assertFloatEqual(conn_start, cell.conn_start)
        self.assertFloatEqual(conn_end, cell.conn_end)
        self.assertFloatEqual(flowrate, cell.flowrate)
        self.assertFloatEqual(oil_flowrate, cell.oil_flowrate)
        self.assertFloatEqual(gas_flowrate, cell.gas_flowrate)
        self.assertFloatEqual(water_flowrate, cell.water_flowrate)
    
        



        
