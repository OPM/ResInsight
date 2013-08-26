#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_ert_gui_import.py' is part of ERT - Ensemble based Reservoir Tool.
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
import traceback

from ert_tests import ExtendedTestCase


class ErtGuiImportTest(ExtendedTestCase):

    def test_ert_gui(self):
        self.assertImportable("ert_gui")
        self.assertImportable("ert_gui.gert_main")
        self.assertImportable("ert_gui.newconfig")

    def test_ert_gui_widgets(self):
        self.assertImportable("ert_gui.widgets")
        self.assertImportable("ert_gui.widgets.activelabel")
        self.assertImportable("ert_gui.widgets.checkbox")
        self.assertImportable("ert_gui.widgets.cogwheel")
        self.assertImportable("ert_gui.widgets.combochoice")
        self.assertImportable("ert_gui.widgets.configpanel")
        self.assertImportable("ert_gui.widgets.help")
        self.assertImportable("ert_gui.widgets.helpedwidget")
        self.assertImportable("ert_gui.widgets.pathchooser")
        self.assertImportable("ert_gui.widgets.reloadbutton")
        self.assertImportable("ert_gui.widgets.searchablelist")
        self.assertImportable("ert_gui.widgets.spinnerwidgets")
        self.assertImportable("ert_gui.widgets.stringbox")
        self.assertImportable("ert_gui.widgets.tablewidgets")
        self.assertImportable("ert_gui.widgets.util")
        self.assertImportable("ert_gui.widgets.validateddialog")

    def test_ert_gui_pages(self):
        self.assertImportable("ert_gui.pages")
        self.assertImportable("ert_gui.pages.application")

    def test_ert_gui_pages_init(self):
        self.assertImportable("ert_gui.pages.init")
        self.assertImportable("ert_gui.pages.init.initandcopy")
        self.assertImportable("ert_gui.pages.init.initpanel")

    def test_ert_gui_pages_run(self):
        self.assertImportable("ert_gui.pages.run")
        self.assertImportable("ert_gui.pages.run.legend")
        self.assertImportable("ert_gui.pages.run.runpanel")
        self.assertImportable("ert_gui.pages.run.simulation")
        self.assertImportable("ert_gui.pages.run.simulationsdialog")

    def test_ert_gui_pages_plot(self):
        self.assertImportable("ert_gui.pages.plot")
        self.assertImportable("ert_gui.pages.plot.ensemblefetcher")
        self.assertImportable("ert_gui.pages.plot.fetcher")
        self.assertImportable("ert_gui.pages.plot.plotconfig")
        self.assertImportable("ert_gui.pages.plot.plotdata")
        self.assertImportable("ert_gui.pages.plot.plotfigure")
        self.assertImportable("ert_gui.pages.plot.plotgenerator")
        self.assertImportable("ert_gui.pages.plot.plotpanel")
        self.assertImportable("ert_gui.pages.plot.plotrenderer")
        self.assertImportable("ert_gui.pages.plot.plotsettings")
        self.assertImportable("ert_gui.pages.plot.plotsettingsxml")
        self.assertImportable("ert_gui.pages.plot.plotview")
        self.assertImportable("ert_gui.pages.plot.rftfetcher")
        self.assertImportable("ert_gui.pages.plot.zoomslider")

    def test_ert_gui_pages_config(self):
        self.assertImportable("ert_gui.pages.config")
        self.assertImportable("ert_gui.pages.config.analysis")
        self.assertImportable("ert_gui.pages.config.configpages")
        self.assertImportable("ert_gui.pages.config.eclipse")
        self.assertImportable("ert_gui.pages.config.ensemble")
        self.assertImportable("ert_gui.pages.config.observations")
        self.assertImportable("ert_gui.pages.config.plot")
        self.assertImportable("ert_gui.pages.config.queuesystem")
        self.assertImportable("ert_gui.pages.config.simulation")
        self.assertImportable("ert_gui.pages.config.systemenv")

    def test_ert_gui_pages_config_jobs(self):
        self.assertImportable("ert_gui.pages.config.jobs")
        self.assertImportable("ert_gui.pages.config.jobs.forwardmodelpanel")
        self.assertImportable("ert_gui.pages.config.jobs.jobsdialog")
        self.assertImportable("ert_gui.pages.config.jobs.jobspanel")

    def test_ert_gui_pages_config_parameters(self):
        self.assertImportable("ert_gui.pages.config.parameters")
        self.assertImportable("ert_gui.pages.config.parameters.datapanel")
        self.assertImportable("ert_gui.pages.config.parameters.fieldpanel")
        self.assertImportable("ert_gui.pages.config.parameters.keywordpanel")
        self.assertImportable("ert_gui.pages.config.parameters.parameterdialog")
        self.assertImportable("ert_gui.pages.config.parameters.parametermodels")
        self.assertImportable("ert_gui.pages.config.parameters.parameterpanel")

    def test_ert_gui_pages_config_simulations(self):
        self.assertImportable("ert_gui.pages.config.simulations")
        self.assertImportable("ert_gui.pages.config.simulations.runpathpanel")
        self.assertImportable("ert_gui.pages.config.simulations.runtemplatepanel")
