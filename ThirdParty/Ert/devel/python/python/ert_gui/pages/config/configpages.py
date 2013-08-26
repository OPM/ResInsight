#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'configpages.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from   ert_gui.widgets.configpanel import ConfigPanel
import eclipse
import analysis
import queuesystem
import systemenv
import plot
import observations
import simulation
import ensemble


class ConfigPages(ConfigPanel):
    

    def __init__(self, parent):
        ConfigPanel.__init__(self, parent)

        eclipse.createEclipsePage(self, parent)
        analysis.createAnalysisPage(self, parent)
        queuesystem.createQueueSystemPage(self, parent)
        systemenv.createSystemPage(self, parent)
        ensemble.createEnsemblePage(self, parent)
        observations.createObservationsPage(self, parent)
        simulation.createSimulationsPage(self, parent)
        plot.createPlotPage(self, parent)
