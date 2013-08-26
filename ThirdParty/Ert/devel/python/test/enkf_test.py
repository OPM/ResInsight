#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from   ert.util.tvector import *
from   test_util import approx_equal, approx_equalv

case = "/private/inmyr/ERT-Intro/testcase/ert_config"
site_conf_file = "/project/res/etc/ERT/site-config"
obs_config_file = "/private/inmyr/ERT-Intro/testcase/observations"

class EnKFtest( unittest.TestCase ):
    def setUp(self):
        pass


    def test_boot( self ):
        self.main = enkf.EnKFMain.bootstrap( case, site_conf_file )
        self.assertTrue( self.main , "Load failed" )
        del self

    def test_enum(self):
        self.assertEqual( enkf.enkf_state_enum.FORECAST , 2 )
        self.assertEqual( enkf.enkf_state_enum.ANALYZED , 4 )
        del self
        
    def test_config( self ):
        self.main   = enkf.EnKFMain.bootstrap( case, site_conf_file )
        config      = self.main.ensemble_config
        anal_config = self.main.analysis_config
        mod_config  = self.main.model_config
        loc_config  = self.main.local_config
        site_conf   = self.main.site_config
        ecl_conf    = self.main.ecl_config
        plot_conf   = self.main.plot_config
        self.main.load_obs(obs_config_file)
        ob          = self.main.get_obs
        temp        = self.main.get_templates
        enkf_fsout  = self.main.get_fs
        mem_conf    = self.main.iget_member_config(0)
        enkf_stat   = self.main.iget_state(0)
        self.assertTrue( isinstance( config      , ert.enkf.ens_config.EnsConfig))
        self.assertTrue( isinstance( anal_config , ert.enkf.analysis_config.AnalysisConfig))
        self.assertTrue( isinstance( mod_config  , ert.enkf.model_config.ModelConfig))
        self.assertTrue( isinstance( loc_config  , ert.enkf.local_config.LocalConfig))
        self.assertTrue( isinstance( site_conf   , ert.enkf.site_config.SiteConfig))
        self.assertTrue( isinstance( ecl_conf    , ert.enkf.ecl_config.EclConfig))
        self.assertTrue( isinstance( plot_conf   , ert.enkf.plot_config.PlotConfig))
        self.assertTrue( isinstance( ob          , ert.enkf.enkf_obs.EnkfObs))
        self.assertTrue( isinstance( temp        , ert.enkf.ert_templates.ErtTemplates))
        self.assertTrue( isinstance( enkf_fsout  , ert.enkf.enkf_fs.EnkfFs))
        self.assertTrue( isinstance( mem_conf    , ert.enkf.member_config.MemberConfig))
        self.assertTrue( isinstance( enkf_stat    , ert.enkf.enkf_state.EnKFState))
        del self
            
unittest.main()
e
