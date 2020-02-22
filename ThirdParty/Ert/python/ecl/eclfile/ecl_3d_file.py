#  Copyright (C) 2015  Equinor ASA, Norway.
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

from ecl.eclfile import EclFile, Ecl3DKW


class Ecl3DFile(EclFile):

    def __init__(self, grid, filename, flags=0):
        self.grid = grid
        super(Ecl3DFile, self).__init__(filename, flags)


    def __getitem__(self, index):
        return_arg = super(Ecl3DFile, self).__getitem__(index)
        if isinstance(return_arg,list):
            kw_list = return_arg
        else:
            kw_list = [return_arg]

        # Go through all the keywords and try inplace promotion to Ecl3DKW
        for kw in kw_list:
            try:
                Ecl3DKW.castFromKW(kw, self.grid)
            except ValueError:
                pass

        return return_arg
