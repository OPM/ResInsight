#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os
import datetime
import csv
from unittest import skipIf, skipUnless, skipIf

from ert.ecl import EclSum
from ert.test import ExtendedTestCase, TestAreaContext
from ert.test.ecl_mock import createEclSum

class SumTest(ExtendedTestCase):

    
    def test_mock(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        self.assertTrue("FOPT" in case)
        self.assertFalse("WWCT:OPX" in case)


    def test_TIME_special_case(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        keys = case.keys()
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )


        keys = case.keys(pattern = "*")
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )

        

    def test_csv_export(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        sep = ";"
        with TestAreaContext("ecl/csv"):
            case.exportCSV( "file.csv" , sep = sep)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            input_file = csv.DictReader( open("file.csv") , delimiter = sep )
            for row in input_file:
                self.assertIn("DAYS", row)
                self.assertIn("DATE", row)
                self.assertIn("FOPT", row)
                self.assertIn("FOPR", row)
                self.assertEqual( len(row) , 4 )
                break
                
            

        with TestAreaContext("ecl/csv"):
            case.exportCSV( "file.csv" , keys = ["FOPT"] , sep = sep)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            input_file = csv.DictReader( open("file.csv") , delimiter=sep)
            for row in input_file:
                self.assertIn("DAYS", row)
                self.assertIn("DATE", row)
                self.assertIn("FOPT", row)
                self.assertEqual( len(row) , 3 )
                break
            
            

        with TestAreaContext("ecl/csv"):
            date_format = "%y-%m-%d"
            sep = ","
            case.exportCSV( "file.csv" , keys = ["F*"] , sep=sep , date_format = date_format)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            with open("file.csv") as f:
                time_index = -1
                for line in f.readlines():
                    tmp = line.split( sep )
                    self.assertEqual( len(tmp) , 4)

                    if time_index >= 0:
                        d = datetime.datetime.strptime( tmp[1] , date_format )
                        self.assertEqual( case.iget_date( time_index ) , d )

                    time_index += 1
                
                
