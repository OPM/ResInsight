#  Copyright (C) 2018  Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

"""
The eclfile package contains several classes for working directly with ECLIPSE
files. 

  fortio/FortIO: This is functionality to read and write binary
     fortran files.

  ecl_kw/EclKW: This class holds one ECLIPSE keyword, like SWAT, in
     restart format.

  ecl_type/EclDataType: This class is used to represent the data type
    of the elements in EclKW.

  ecl_file/EclFile: This class is used to load an ECLIPSE file in
     restart format, alternatively only parts of the file can be
     loaded. Internally it consists of a collection of EclKW
     instances.
"""

import ecl.util.util

from .fortio import FortIO, openFortIO
from .ecl_kw import EclKW
from .ecl_file_view import EclFileView
from .ecl_file import EclFile , openEclFile
from .ecl_3dkw import Ecl3DKW
from .ecl_3d_file import Ecl3DFile
from .ecl_init_file import EclInitFile
from .ecl_restart_file import EclRestartFile
