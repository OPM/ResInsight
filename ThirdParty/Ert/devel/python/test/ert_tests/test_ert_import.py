#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_ert_import.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert_tests import ExtendedTestCase


class ErtImportTest(ExtendedTestCase):

    def test_ert(self):
        self.assertImportable("ert")

    def test_ert_config(self):
        self.assertImportable("ert.config")
        self.assertImportable("ert.config.content_type_enum")
        self.assertImportable("ert.config.config_parser")
        self.assertImportable("ert.config.unrecognized_enum")


    def test_ert_cwrap(self):
        self.assertImportable("ert.cwrap")
        self.assertImportable("ert.cwrap.basecclass")
        self.assertImportable("ert.cwrap.basecenum")
        self.assertImportable("ert.cwrap.cclass")
        self.assertImportable("ert.cwrap.cenum")
        self.assertImportable("ert.cwrap.cfile")
        self.assertImportable("ert.cwrap.clib")
        self.assertImportable("ert.cwrap.cnamespace")
        self.assertImportable("ert.cwrap.cwrap")

    def test_ert_ecl(self):
        self.assertImportable("ert.ecl")
        self.assertImportable("ert.ecl.ecl")
        self.assertImportable("ert.ecl.ecl_case")
        self.assertImportable("ert.ecl.ecl_default")
        self.assertImportable("ert.ecl.ecl_file")
        self.assertImportable("ert.ecl.ecl_grav")
        self.assertImportable("ert.ecl.ecl_grav_calc")
        self.assertImportable("ert.ecl.ecl_grid")
        self.assertImportable("ert.ecl.ecl_kw")
        self.assertImportable("ert.ecl.ecl_local")
        self.assertImportable("ert.ecl.ecl_queue")
        self.assertImportable("ert.ecl.ecl_region")
        self.assertImportable("ert.ecl.ecl_rft")
        self.assertImportable("ert.ecl.ecl_rft_cell")
        self.assertImportable("ert.ecl.ecl_subsidence")
        self.assertImportable("ert.ecl.ecl_sum")
        self.assertImportable("ert.ecl.ecl_sum_node")
        self.assertImportable("ert.ecl.ecl_sum_vector")
        self.assertImportable("ert.ecl.ecl_util")
        self.assertImportable("ert.ecl.fortio")


    def test_ert_enkf(self):
        self.assertImportable("ert.enkf")
        self.assertImportable("ert.enkf.analysis_config")
        self.assertImportable("ert.enkf.block_obs")
        self.assertImportable("ert.enkf.ecl_config")
        self.assertImportable("ert.enkf.enkf_enum")
        self.assertImportable("ert.enkf.enkf_fs")
        self.assertImportable("ert.enkf.enkf_main")
        self.assertImportable("ert.enkf.enkf_obs")
        self.assertImportable("ert.enkf.enkf_state")
        self.assertImportable("ert.enkf.ens_config")
        self.assertImportable("ert.enkf.ert_template")
        self.assertImportable("ert.enkf.ert_templates")
        self.assertImportable("ert.enkf.local_config")
        self.assertImportable("ert.enkf.model_config")
        self.assertImportable("ert.enkf.plot_config")
        self.assertImportable("ert.enkf.site_config")


    def test_ert_enkf_data(self):
        self.assertImportable("ert.enkf.data")
        self.assertImportable("ert.enkf.data.enkf_config_node")
        self.assertImportable("ert.enkf.data.enkf_node")
        self.assertImportable("ert.enkf.data.field")
        self.assertImportable("ert.enkf.data.field_config")
        self.assertImportable("ert.enkf.data.gen_data_config")
        self.assertImportable("ert.enkf.data.gen_kw_config")
        self.assertImportable("ert.enkf.data.summary_config")

    def test_ert_enkf_util(self):
        self.assertImportable("ert.enkf.util")
        self.assertImportable("ert.enkf.util.obs_vector")
        self.assertImportable("ert.enkf.util.time_map")

    # def test_ert_ert(self):
    #     self.assertImportable("ert.ert")
    #     self.assertImportable("ert.ert.c_enums")
    #     self.assertImportable("ert.ert.enums")
    #     self.assertImportable("ert.ert.ertwrapper")

    def test_ert_geo(self):
        self.assertImportable("ert.geo")
        self.assertImportable("ert.geo.geo_polygon")

    def test_ert_job_queue(self):
        self.assertImportable("ert.job_queue")
        self.assertImportable("ert.job_queue.driver")
        self.assertImportable("ert.job_queue.ext_job")
        self.assertImportable("ert.job_queue.ext_joblist")
        self.assertImportable("ert.job_queue.forward_model")
        self.assertImportable("ert.job_queue.job")
        self.assertImportable("ert.job_queue.queue")

    def test_ert_rms(self):
        self.assertImportable("ert.rms")
        self.assertImportable("ert.rms.librms")
        self.assertImportable("ert.rms.rms")

    def test_ert_sched(self):
        self.assertImportable("ert.sched")
        self.assertImportable("ert.sched.history")
        self.assertImportable("ert.sched.history_source_enum")
        self.assertImportable("ert.sched.sched_file")

    def test_ert_util(self):
        self.assertImportable("ert.util")
        self.assertImportable("ert.util.buffer")
        self.assertImportable("ert.util.ctime")
        self.assertImportable("ert.util.hash")
        self.assertImportable("ert.util.latex")
        self.assertImportable("ert.util.log")
        self.assertImportable("ert.util.lookup_table")
        self.assertImportable("ert.util.matrix")
        self.assertImportable("ert.util.stat")
        self.assertImportable("ert.util.stringlist")
        self.assertImportable("ert.util.substitution_list")
        self.assertImportable("ert.util.test_area")
        self.assertImportable("ert.util.tvector")
        self.assertImportable("ert.util.util_func")

    def test_ert_well(self):
        self.assertImportable("ert.well")
        self.assertImportable("ert.well.libwell")
        self.assertImportable("ert.well.well")
        self.assertImportable("ert.well.well_info")
        self.assertImportable("ert.well.well_state")
        self.assertImportable("ert.well.well_ts")



