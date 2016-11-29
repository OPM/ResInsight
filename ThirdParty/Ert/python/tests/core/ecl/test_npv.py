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
import math

try:
    from unittest2 import skipIf, skipUnless, skipIf
except ImportError:
    from unittest import skipIf, skipUnless, skipIf

from ert.ecl import EclSum
from ert.ecl import EclNPV , NPVPriceVector

from ert.util import StringList, TimeVector, DoubleVector , CTime
from ert.test import ExtendedTestCase , TestAreaContext


base = "ECLIPSE"
path = "Statoil/ECLIPSE/Gurbat"
case = "%s/%s" % (path, base)

def callable(x):
    return 1

def linear1(x):
    return x

def linear2(x):
    return 2*x


class NPVTest(ExtendedTestCase):
    def setUp(self):
        self.case = self.createTestPath(case)


    def test_create(self):
        with self.assertRaises(Exception):
            npv = EclNPV("/does/not/exist")

        npv = EclNPV( self.case )


    def test_eval_npv(self):
        npv = EclNPV( self.case )
        with self.assertRaises(ValueError):
            npv.eval()

            
    def test_expression(self):
        npv = EclNPV( self.case )
        self.assertIsNone( npv.getExpression() )
        npv.setExpression( "[FOPT]*$OIL_PRICE - [FGIT]*$GAS_PRICE")
        self.assertEqual( npv.getExpression() , "[FOPT]*$OIL_PRICE - [FGIT]*$GAS_PRICE")
        self.assertIn( "FOPT" , npv.getKeyList() )
        self.assertIn( "FGIT" , npv.getKeyList() )

        with self.assertRaises(ValueError):
            npv.parseExpression("[FOPT")

        with self.assertRaises(ValueError):
            npv.parseExpression("FOPT]")

        with self.assertRaises(KeyError):
            npv.parseExpression("[FoPT]")
            
        with self.assertRaises(ValueError):
            npv.parseExpression("[FOPR]")
            
        parsedExpression = npv.parseExpression("[FOPT]")
        self.assertEqual( parsedExpression , "FOPT[i]")
        self.assertEqual( 1 , len(npv.getKeyList() ))


        parsedExpression = npv.parseExpression("[FOPT]*2 + [FGPT] - [WOPT:OP_1]")
        self.assertEqual( parsedExpression , "FOPT[i]*2 + FGPT[i] - WOPT_OP_1[i]")
        keyList = npv.getKeyList()
        self.assertEqual( 3 , len(keyList))
        self.assertIn( "FOPT" , keyList )
        self.assertIn( "FGPT" , keyList )
        self.assertIn( "WOPT:OP_1" , keyList )


    def test_period(self):
        npv = EclNPV( self.case )
        self.assertIsNone(npv.start)
        self.assertIsNone(npv.end)
        self.assertEqual("1Y" , npv.interval)


    def test_eval(self):
        npv = EclNPV(self.case)
        npv.compile("[FOPT]")
        npv1 = npv.evalNPV()

        npv2 = 0
        sum = EclSum(self.case)
        trange = sum.timeRange()
        fopr = sum.blockedProduction("FOPT" , trange)
        for v in fopr:
            npv2 += v
        self.assertAlmostEqual( npv1 , npv2 )
        
        npv.compile("[FOPT] - 0.5*[FOPT] - 0.5*[FOPT]")
        npv1 = npv.evalNPV()
        self.assertTrue( abs(npv1) < 1e-2 )

        npv.compile("[WOPT:OP_1] - 0.5*[WOPT:OP_1] - 0.5*[WOPT:OP_1]")
        npv1 = npv.evalNPV()
        self.assertTrue( abs(npv1) < 1e-2 )



    def test_price_vector(self):
        with self.assertRaises(ValueError):
            NPVPriceVector("NotList")

        with self.assertRaises(ValueError):
            NPVPriceVector(1.25)

        with self.assertRaises(ValueError):
            NPVPriceVector((1,25))
            
        with self.assertRaises(ValueError):
            NPVPriceVector([1,2,3])

        with self.assertRaises(ValueError):
            NPVPriceVector([(1,25) , ("String",100,100)])

        with self.assertRaises(ValueError):
            NPVPriceVector([(1,25) , ("String",100)])

        NPVPriceVector([(datetime.datetime(2010 , 1 , 1 , 0 , 0 , 0) , 100)])
        NPVPriceVector([(datetime.date(2010 , 1 , 1 ) , 100)])
        NPVPriceVector([("19/06/2010" , 100)])

        with self.assertRaises(ValueError):
            NPVPriceVector([("01/01/2000" , 100),
                            ("01/01/1999" , 100)])

        with self.assertRaises(ValueError):
            NPVPriceVector([("01/01/2000" , "String")])

        NPVPriceVector([("01/01/2000" , 100)])
        NPVPriceVector([("01/01/2000" , 77.99)])
        NPVPriceVector([("01/01/2000" , callable)])


        vec = NPVPriceVector([("01/01/2000" , 100),
                              ("01/02/2000" , 200),
                              ("01/03/2000" , 300)])

        with self.assertRaises(ValueError):
            vec.eval( datetime.date( 1999 , 1 , 1))

        self.assertEqual( datetime.date( 2000 , 1 , 1 ) , NPVPriceVector.assertDate( datetime.date(2000,1,1) )) 
        self.assertEqual( datetime.date( 2000 , 1 , 1 ) , NPVPriceVector.assertDate( CTime(datetime.date(2000,1,1)) ))
        

        self.assertEqual( 100 , vec.eval( datetime.date( 2000 , 1 , 10)))
        self.assertEqual( 100 , vec.eval( datetime.datetime( 2000 , 1 , 10 , 0,0,0)))
        self.assertEqual( 100 , vec.eval( CTime(datetime.datetime( 2000 , 1 , 10 , 0,0,0))))
        
        self.assertEqual( 300 , vec.eval( datetime.date( 2000 , 4, 1)))


        vec = NPVPriceVector([("01/01/2000" , linear1),
                              ("01/02/2000" , linear2),
                              ("01/03/2000" , 300)])
        
        self.assertEqual( 300 , vec.eval( datetime.date( 2000 , 3 , 10)))
        self.assertEqual( 0  , vec.eval( datetime.date( 2000 , 1 , 1) ))
        self.assertEqual( 10 , vec.eval( datetime.date( 2000 , 1 , 11) ))
        self.assertEqual( 20 , vec.eval( datetime.date( 2000 , 1 , 21) ))

        self.assertEqual( 0  , vec.eval( datetime.date( 2000 , 2 , 1) ))
        self.assertEqual( 20 , vec.eval( datetime.date( 2000 , 2 , 11) ))
        self.assertEqual( 40 , vec.eval( datetime.date( 2000 , 2 , 21) ))
