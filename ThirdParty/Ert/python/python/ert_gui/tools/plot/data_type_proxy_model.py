#  Copyright (C) 2014  Statoil ASA, Norway.
#   
#  The file 'data_type_proxy_model.py' is part of ERT - Ensemble based Reservoir Tool.
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

from PyQt4.QtCore import Qt
from PyQt4.QtGui import QSortFilterProxyModel
from ert_gui.tools.plot import DataTypeKeysListModel


class DataTypeProxyModel(QSortFilterProxyModel):

    def __init__(self, model , parent=None):
        QSortFilterProxyModel.__init__(self, parent)
        self.__show_summary_keys = True
        self.__show_block_keys = True
        self.__show_gen_kw_keys = True
        self.__show_gen_data_keys = True
        self.__show_custom_kw_keys = True
        self.__show_custom_pca_keys = True

        self.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self.setSourceModel(model)

    def filterAcceptsRow(self, index, q_model_index):
        show = QSortFilterProxyModel.filterAcceptsRow(self, index, q_model_index)

        if show:
            source_model = self.sourceModel()
            source_index = source_model.index(index, 0, q_model_index)
            key = source_model.itemAt(source_index)

            if not self.__show_summary_keys and source_model.isSummaryKey(key):
                show = False

            elif not self.__show_block_keys and source_model.isBlockKey(key):
                show = False

            elif not self.__show_gen_kw_keys and source_model.isGenKWKey(key):
                show = False

            elif not self.__show_gen_data_keys and source_model.isGenDataKey(key):
                show = False

            elif not self.__show_custom_kw_keys and source_model.isCustomKwKey(key):
                show = False

            elif not self.__show_custom_pca_keys and source_model.isCustomPcaKey(key):
                show = False


        return show

    def sourceModel(self):
        """ @rtype: DataTypeKeysListModel """
        return QSortFilterProxyModel.sourceModel(self)

    def setShowSummaryKeys(self, visible):
        self.__show_summary_keys = visible
        self.invalidateFilter()

    def setShowBlockKeys(self, visible):
        self.__show_block_keys = visible
        self.invalidateFilter()

    def setShowGenKWKeys(self, visible):
        self.__show_gen_kw_keys = visible
        self.invalidateFilter()

    def setShowGenDataKeys(self, visible):
        self.__show_gen_data_keys = visible
        self.invalidateFilter()

    def setShowCustomKwKeys(self, visible):
        self.__show_custom_kw_keys = visible
        self.invalidateFilter()

    def setShowCustomPcaKeys(self, visible):
        self.__show_custom_pca_keys = visible
        self.invalidateFilter()

