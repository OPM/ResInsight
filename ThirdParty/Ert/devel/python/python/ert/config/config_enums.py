#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'config_enums.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    ert.cwrap.cenum       import create_enum
import  libconfig

create_enum( libconfig.lib , "config_schema_item_type_enum_iget" , "content_type" , name_space = globals())

create_enum( libconfig.lib , "config_schema_item_unrecognized_enum_iget" , "unrecognized" , name_space = globals())
