#  Copyright (C) 2011  Equinor ASA, Norway.
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

from ecl import EclPrototype

__arglist  = 'double, double, double, '
__arglist += 'ecl_grid, ecl_file, '
__arglist += 'ecl_kw, ecl_kw, ecl_kw, ecl_kw, ecl_kw, ecl_kw'
_phase_deltag = EclPrototype("double ecl_grav_phase_deltag(%s)" % __arglist)

def phase_deltag(xyz, grid, init, sat1, rho1, porv1, sat2, rho2, porv2):
    return _phase_deltag(xyz[0], xyz[1], xyz[2],
                         grid.c_ptr, init.c_ptr,
                         sat1.c_ptr, rho1.c_ptr, porv1.c_ptr,
                         sat2.c_ptr, rho2.c_ptr, porv2.c_ptr)


def deltag(xyz, grid, init_file, restart_file1, restart_file2):
    """
    1. All restart files should have water, i.e. the SWAT keyword.
    2. All phases present in the restart file should also be present as densities,
       in addition the model must contain one additional phase - which should have a density.
    3. The restart files can never contain oil saturation.
    """

    swat1 = restart_file1.iget_named_kw("SWAT", 0)
    swat2 = restart_file2.iget_named_kw("SWAT", 0)

    phase_list = [(swat1, swat2)]

    if restart_file1.has_kw("SGAS"):
        # This is a three phase Water / Gas / Oil system
        sgas1 = restart_file1.iget_named_kw("SGAS", 0)
        sgas2 = restart_file2.iget_named_kw("SGAS", 0)

        soil1 = 1 - (sgas1 + swat1)
        soil2 = 1 - (sgas2 + swat2)
        soil1.name = "SOIL"
        soil2.name = "SOIL"
        phase_list += [(sgas1, sgas2),
                       (soil1, soil2)]
    else:
        # This is a two phase Water / xxx System. Will look for
        # OIL_DEN and GAS_DEN keywords to determine whether it is a
        # Water / Oil or Water / Gas system.

        if restart_file1.has_kw("OIL_DEN"):
            # Oil / Water system
            soil1 = 1 - swat1
            soil2 = 1 - swat2
            soil1.name = "SOIL"
            soil2.name = "SOIL"
            phase_list += [(soil1, soil2)]
        else:
            # Gas / Water system
            sgas1 = 1 - swat1
            sgas2 = 1 - swat2
            sgas1.name = "SGAS"
            sgas2.name = "SGAS"
            phase_list += [(sgas1, sgas2)]

    porv1 = restart_file1.iget_named_kw("RPORV", 0)
    porv2 = restart_file2.iget_named_kw("RPORV", 0)

    deltag = 0
    for (sat1, sat2) in phase_list:
        rho_name = "%s_DEN" % sat1.name[1:]
        rho1 = restart_file1.iget_named_kw(rho_name, 0)
        rho2 = restart_file2.iget_named_kw(rho_name, 0)
        deltag += phase_deltag(xyz, grid, init_file, sat1, rho1, porv1, sat2, rho2, porv2)
    return deltag
