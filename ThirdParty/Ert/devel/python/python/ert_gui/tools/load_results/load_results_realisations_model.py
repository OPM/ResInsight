#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'load_results_realizations_model.py' is part of ERT - Ensemble based Reservoir Tool.
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


class LoadResultsRealizationsModel(BasicModelMixin):


    def __init__(self, realization_count):
        self.__realization_count = realization_count
        self.__active_realizations = self.getDefaultValue()
        self.__custom = False
        super(LoadResultsRealizationsModel, self).__init__()


    def getValue(self):
        """ @rtype: str """
        return self.__active_realizations

    def setValue(self, active_realizations):
        if active_realizations is None or active_realizations.strip() == "" or active_realizations == self.getDefaultValue():
            self.__custom = False
            self.__active_realizations = self.getDefaultValue()
        else:
            self.__custom = True
            self.__active_realizations = active_realizations


    def getDefaultValue(self):
        return "0-%d" % (self.__realization_count - 1)

    def getActiveRealizationsMask(self):
        mask = BoolVector(False, self.__realization_count)
        mask.updateActiveMask(self.getValue())
        # mask = BoolVector.active_mask(self.getValue())

        if mask is None:
            raise ValueError("Error while parsing range string!")

        if len(mask) > self.__realization_count:
            raise ValueError("Mask size changed %d != %d!" % (self.__realization_count, len(mask)))

        return mask












