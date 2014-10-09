#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'load_results_iterations_model.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.util import BoolVector
from ert_gui.models.mixins import BasicModelMixin


class LoadResultsIterationsModel(BasicModelMixin):


    def __init__(self, iterations_count):
        self.__iterations_count = iterations_count
        self.__active_iteration = self.getDefaultValue()
        self.__custom = False
        super(LoadResultsIterationsModel, self).__init__()

    def getValue(self):
        """ @rtype: str """
        return self.__active_iteration

    def setValue(self, active_iterations):
        if active_iterations is None or active_iterations.strip() == "" or active_iterations == self.getDefaultValue():
            self.__custom = False
            self.__active_iteration = self.getDefaultValue()
        else:
            self.__custom = True
            self.__active_iteration = active_iterations

    def getDefaultValue(self):
        return 0

    def getActiveIteration(self):
        """ @rtype: int """
        return int(self.__active_iteration)












