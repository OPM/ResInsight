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
    r.initialize = lambda ert : ert.main.plot_config.get_path    
    r.getter = lambda ert : ert.main.plot_config.get_path
    r.setter = lambda ert, value : ert.main.plot_config.set_path( str(value))

    r = configPanel.addRow(ComboChoice(parent, ["PLPLOT", "TEXT"], "Driver", "config/plot/plot_driver"))
    r.initialize = lambda ert : ert.main.plot_config.get_driver    
    r.getter = lambda ert : ert.main.plot_config.get_driver
    r.setter = lambda ert, value : ert.main.plot_config.set_driver( str(value))

    r = configPanel.addRow(IntegerSpinner(parent, "Errorbar max", "config/plot/plot_errorbar_max", 1, 10000000))
    r.initialize = lambda ert : ert.main.plot_config.get_errorbar_max
    r.getter = lambda ert : ert.main.plot_config.get_errorbar_max
    r.setter = lambda ert, value : ert.main.plot_config.set_errorbar_max( value)

    r = configPanel.addRow(IntegerSpinner(parent, "Width", "config/plot/width", 1, 10000))
    r.initialize = lambda ert : ert.main.plot_config.get_width
    r.getter = lambda ert : ert.main.plot_config.get_width
    r.setter = lambda ert, value : ert.main.plot_config.set_width( value)

    r = configPanel.addRow(IntegerSpinner(parent, "Height", "config/plot/plot_height", 1, 10000))
    r.initialize = lambda ert : ert.main.plot_config.get_height    
    r.getter = lambda ert : ert.main.plot_config.get_height
    r.setter = lambda ert, value : ert.main.plot_config.set_height( value)

    r = configPanel.addRow(PathChooser(parent, "Image Viewer", "config/plot/image_viewer", True))
    r.initialize = lambda ert : ert.main.plot_config.get_viewer
    r.getter = lambda ert : ert.main.plot_config.get_viewer
    r.setter = lambda ert, value : ert.main.plot_config.set_viewer( str(value))

    r = configPanel.addRow(ComboChoice(parent, ["bmp", "jpg", "png", "tif"], "Image type", "config/plot/image_type"))
    r.initialize = lambda ert : ert.main.plot_config.get_image_type
    r.getter = lambda ert : ert.main.plot_config.get_image_type
    r.setter = lambda ert, value : ert.main.plot_config.set_image_type( str(value))


    configPanel.endPage()
