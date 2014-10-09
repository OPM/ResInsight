#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'ert_server.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import threading
import json
import os

from ert.enkf import EnKFMain

class ErtServer(object):
    site_config = None

    def __init__(self , config_file = None):
        self.ert_handle = None
        if config_file:
            if os.path.exists(config_file):
                self.open( config_file )
            else:
                raise IOError("The config file:%s does not exist" % config_file)


    def open(self , config_file):
        self.config_file = config_file
        self.ert_handle = EnKFMain( config_file , ErtServer.site_config )
        


    def close(self):
        # More cleanup first ...
        self.ert_handle = None


    def isConnected(self):
        if self.ert_handle:
            return True
        else:
            return False


    def __del__(self):
        if self.isConnected():
            self.close()



    def evalCmd(self , cmd_expr):
        cmd = cmd_expr[0]
        if cmd == "STATUS":
            if self.isConnected():
                return ["OPEN"]
            else:
                return ["CLOSED"]
