#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
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

import datetime
import unittest
import ert
import ert.enkf.enkf as enkf
from   test_util import approx_equal, approx_equalv
from   ert.util.stringlist import StringList

case = "/private/inmyr/ERT-Intro/testcase/ert_config"
site_conf_file = "/project/res/etc/ERT/site-config"
obs_config_file = "/private/inmyr/ERT-Intro/testcase/observations"

class EnsConfigTest( unittest.TestCase ):
    def setUp(self):
        pass

    def test_key(self):
        main = enkf.EnKFMain.bootstrap( case , site_conf_file)
        conf = main.config
        self.assertTrue( conf.has_key("WWCT:OP_1" ))
        self.assertFalse( conf.has_key("WWCT:OP_1X" ))

    def test_enkf_conf_node(self):
        main = enkf.EnKFMain.bootstrap( case , site_conf_file)
        conf = main.config
        s = StringList(initial = None, c_ptr=conf.alloc_keylist)
        self.assertTrue( isinstance( conf.get_node("MULTFLT") , ert.enkf.enkf_config_node.EnkfConfigNode))
        self.assertTrue( isinstance( s , ert.util.stringlist.StringList))


        
unittest.main()
