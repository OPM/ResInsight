#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plot.py' is part of ERT - Ensemble based Reservoir Tool. 
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


# ----------------------------------------------------------------------------------------------
# Plot tab
# ----------------------------------------------------------------------------------------------
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.combochoice import ComboChoice
from ert_gui.widgets.spinnerwidgets import IntegerSpinner


def createPlotPage(configPanel, parent):
    configPanel.startPage("Plot")

    r = configPanel.addRow(PathChooser(parent, "Output path", "config/plot/path"))
    r.getter = lambda ert : ert.enkf.plot_config_get_path(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_path(ert.plot_config, str(value))

    r = configPanel.addRow(ComboChoice(parent, ["PLPLOT", "TEXT"], "Driver", "config/plot/plot_driver"))
    r.getter = lambda ert : ert.enkf.plot_config_get_driver(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_driver(ert.plot_config, str(value))

    r = configPanel.addRow(IntegerSpinner(parent, "Errorbar max", "config/plot/plot_errorbar_max", 1, 10000000))
    r.getter = lambda ert : ert.enkf.plot_config_get_errorbar_max(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_errorbar_max(ert.plot_config, value)

    r = configPanel.addRow(IntegerSpinner(parent, "Width", "config/plot/width", 1, 10000))
    r.getter = lambda ert : ert.enkf.plot_config_get_width(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_width(ert.plot_config, value)

    r = configPanel.addRow(IntegerSpinner(parent, "Height", "config/plot/plot_height", 1, 10000))
    r.getter = lambda ert : ert.enkf.plot_config_get_height(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_height(ert.plot_config, value)

    r = configPanel.addRow(PathChooser(parent, "Image Viewer", "config/plot/image_viewer", True))
    r.getter = lambda ert : ert.enkf.plot_config_get_viewer(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_viewer(ert.plot_config, str(value))

    r = configPanel.addRow(ComboChoice(parent, ["bmp", "jpg", "png", "tif"], "Image type", "config/plot/image_type"))
    r.getter = lambda ert : ert.enkf.plot_config_get_image_type(ert.plot_config)
    r.setter = lambda ert, value : ert.enkf.plot_config_set_image_type(ert.plot_config, str(value))


    configPanel.endPage()
