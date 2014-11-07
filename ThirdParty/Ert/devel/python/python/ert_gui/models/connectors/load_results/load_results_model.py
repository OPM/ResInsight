#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'load_results_model.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.util import StringList

from ert_gui.models import ErtConnector

class LoadResultsModel(ErtConnector):

    def __init__(self):
        super(LoadResultsModel, self).__init__()

    def loadResults(self, selected_case, realisations, iteration):
        """
        @type selected_case: str
        @type realisations: BoolVector
        @type iteration: int
        """
        fs = self.ert().getEnkfFsManager().getFileSystem(selected_case)
        self.ert().loadFromForwardModel(realisations, iteration, fs)


    def isValidRunPath(self):
        """ @rtype: bool """
        run_path = self.ert().getModelConfig().getRunpathAsString()
        hasIterations = True
        try:
            formated = run_path % (0, 0)
            return True
        except TypeError:
            hasIterations = False

        hasRealization = True
        try:
            formated = run_path % 0
            return True
        except TypeError:
            hasRealization = False

        return False

    def getCurrentRunPath(self):
        """ @rtype: str """
        return self.ert().getModelConfig().getRunpathAsString()


    def getIterationCount(self):
        """ @rtype: int """
        run_path = self.ert().getModelConfig().getRunpathAsString()
        formated = None
        try:
            formated = run_path % (0, 0)
        except TypeError:
            return 0

        iteration = 0
        valid_directory = True
        while valid_directory:
            formated = run_path % (0, iteration)
            valid_directory = os.path.exists(formated)
            if valid_directory:
                iteration += 1

        return iteration


