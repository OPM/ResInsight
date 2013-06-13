#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'import_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import sys

def test_import( module ):
    print "Importing: %s ..." % module , 
    __import__( module )
    print "OK"


test_import( "ert" )

test_import( "ert.config" )
test_import( "ert.cwrap" )
test_import( "ert.ecl" )
test_import( "ert.enkf" )
test_import( "ert.ert")
test_import( "ert.geo" )
test_import( "ert.job_queue" )
test_import( "ert.rms" )
test_import( "ert.sched" )
test_import( "ert.util" )
test_import( "ert.well")

test_import("ert.config.config")
test_import("ert.config.config_enums")
test_import("ert.config.config_parser")

test_import("ert.cwrap.cclass")
test_import("ert.cwrap.cenum")
test_import("ert.cwrap.cfile")
test_import("ert.cwrap.clib")
test_import("ert.cwrap.cwrap")

test_import("ert.ecl.ecl")
test_import("ert.ecl.ecl_case")
test_import("ert.ecl.ecl_default")
test_import("ert.ecl.ecl_file")
test_import("ert.ecl.ecl_grav_calc")
test_import("ert.ecl.ecl_grav")
test_import("ert.ecl.ecl_grid")
test_import("ert.ecl.ecl_kw")
test_import("ert.ecl.ecl")
test_import("ert.ecl.ecl_queue")
test_import("ert.ecl.ecl_region")
test_import("ert.ecl.ecl_rft")
test_import("ert.ecl.ecl_rft_cell")
test_import("ert.ecl.ecl_subsidence")
test_import("ert.ecl.ecl_sum")
test_import("ert.ecl.ecl_util")
test_import("ert.ecl.fortio")
test_import("ert.ecl.libecl")

test_import("ert.enkf.enkf")
test_import("ert.enkf.analysis_config")
test_import("ert.enkf.block_obs")
test_import("ert.enkf.ecl_config")
test_import("ert.enkf.enkf_config_node")
test_import("ert.enkf.enkf_enum")
test_import("ert.enkf.enkf_fs")
test_import("ert.enkf.enkf_main")
test_import("ert.enkf.enkf_node")
test_import("ert.enkf.enkf_obs")
test_import("ert.enkf.enkf")
test_import("ert.enkf.enkf_state")
test_import("ert.enkf.ens_config")
test_import("ert.enkf.ert_template")
test_import("ert.enkf.ert_templates")
test_import("ert.enkf.field_config")
test_import("ert.enkf.field")
test_import("ert.enkf.gen_data_config")
test_import("ert.enkf.gen_kw_config")
test_import("ert.enkf.libenkf")
test_import("ert.enkf.local_config")
test_import("ert.enkf.model_config")
test_import("ert.enkf.obs_vector")
test_import("ert.enkf.plot_config")
test_import("ert.enkf.site_config")
test_import("ert.enkf.time_map")

test_import("ert.ert.c_enums")
test_import("ert.ert.enums")
test_import("ert.ert.erttypes")
test_import("ert.ert.ertwrapper")

test_import("ert.geo.geo_polygon")
test_import("ert.geo.geo")
test_import("ert.geo.libgeo")

test_import("ert.job_queue.job_queue")
test_import("ert.job_queue.driver")
test_import("ert.job_queue.ext_job")
test_import("ert.job_queue.queue")
test_import("ert.job_queue.ext_joblist")
test_import("ert.job_queue.forward_model")
test_import("ert.job_queue.job")
test_import("ert.job_queue.libjob_queue")

test_import("ert.rms.librms")
test_import("ert.rms.rms")

test_import("ert.sched.history")
test_import("ert.sched.libsched")
test_import("ert.sched.sched_file")
test_import("ert.sched.sched")

test_import("ert.util.ctime")
test_import("ert.util.latex")
test_import("ert.util.lookup_table")
test_import("ert.util.tvector")
test_import("ert.util.buffer")
#test_import("ert.util.hash")
test_import("ert.util.libutil")
test_import("ert.util.matrix")
test_import("ert.util.stat")
test_import("ert.util.util_func")
test_import("ert.util.log")
test_import("ert.util.stringlist")

test_import("ert.well.libwell")
test_import("ert.well.well_info")
test_import("ert.well.well")
test_import("ert.well.well_state")
test_import("ert.well.well_ts")

test_import("ert_gui")

test_import("ert_gui.gert_main")
test_import("ert_gui.newconfig")
test_import("ert_gui.pages")
test_import("ert_gui.widgets")

test_import("ert_gui.pages.application")
test_import("ert_gui.pages.config")
test_import("ert_gui.pages.init")
test_import("ert_gui.pages.plot")
test_import("ert_gui.pages.run")

test_import("ert_gui.pages.config.analysis")
test_import("ert_gui.pages.config.configpages")
test_import("ert_gui.pages.config.eclipse")
test_import("ert_gui.pages.config.ensemble")
test_import("ert_gui.pages.config.observations")
test_import("ert_gui.pages.config.plot")
test_import("ert_gui.pages.config.queuesystem")
test_import("ert_gui.pages.config.simulation")
test_import("ert_gui.pages.config.systemenv")
test_import("ert_gui.pages.config.jobs")
test_import("ert_gui.pages.config.parameters")
test_import("ert_gui.pages.config.simulations")            

test_import("ert_gui.pages.config.jobs.forwardmodelpanel")
test_import("ert_gui.pages.config.jobs.jobsdialog")
test_import("ert_gui.pages.config.jobs.jobspanel")

test_import("ert_gui.pages.config.parameters.datapanel")
test_import("ert_gui.pages.config.parameters.fieldpanel")
test_import("ert_gui.pages.config.parameters.keywordpanel")
test_import("ert_gui.pages.config.parameters.parameterdialog")
test_import("ert_gui.pages.config.parameters.parametermodels")
test_import("ert_gui.pages.config.parameters.parameterpanel")

test_import("ert_gui.pages.config.simulations.runpathpanel")
test_import("ert_gui.pages.config.simulations.runtemplatepanel")

test_import("ert_gui.pages.init.initandcopy")
test_import("ert_gui.pages.init.initpanel")

test_import("ert_gui.pages.plot.ensemblefetcher")
test_import("ert_gui.pages.plot.fetcher")
test_import("ert_gui.pages.plot.plotconfig")
test_import("ert_gui.pages.plot.plotdata")
test_import("ert_gui.pages.plot.plotfigure")
test_import("ert_gui.pages.plot.plotgenerator")
test_import("ert_gui.pages.plot.plotpanel")
test_import("ert_gui.pages.plot.plotrenderer")
test_import("ert_gui.pages.plot.plotsettings")
test_import("ert_gui.pages.plot.plotsettingsxml")
test_import("ert_gui.pages.plot.plotview")
test_import("ert_gui.pages.plot.rftfetcher")
test_import("ert_gui.pages.plot.zoomslider")

test_import("ert_gui.pages.run.legend")
test_import("ert_gui.pages.run.runpanel")
test_import("ert_gui.pages.run.simulation")
test_import("ert_gui.pages.run.simulationsdialog")

test_import("ert_gui.widgets.activelabel")
test_import("ert_gui.widgets.checkbox")
test_import("ert_gui.widgets.cogwheel")
test_import("ert_gui.widgets.combochoice")
test_import("ert_gui.widgets.configpanel")
test_import("ert_gui.widgets.helpedwidget")
test_import("ert_gui.widgets.help")
test_import("ert_gui.widgets.pathchooser")
test_import("ert_gui.widgets.reloadbutton")
test_import("ert_gui.widgets.searchablelist")
test_import("ert_gui.widgets.spinnerwidgets")
test_import("ert_gui.widgets.stringbox")
test_import("ert_gui.widgets.tablewidgets")
test_import("ert_gui.widgets.util")
test_import("ert_gui.widgets.validateddialog")


def test_suite( argv ):
    return False
