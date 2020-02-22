#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file 'ecl_rft_cell.py' is part of ERT - Ensemble based Reservoir Tool.
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

from cwrap import BaseCClass
from ecl import EclPrototype


class RFTCell(BaseCClass):
    """The RFTCell is a base class for the cells which are part of an RFT/PLT.

    The RFTCell class contains the elements which are common to both
    RFT and PLT. The list of common elements include the corrdinates
    (i,j,k) the pressure and the depth of the cell. Actual user access
    should be based on the derived classes EclRFTCell and EclPLTCell.

    Observe that from june 2013 the properties i,j and k which return
    offset 1 coordinate values are deprecated, and you should rather
    use the methods get_i(), get_j() and get_k() which return offset 0
    coordinate values.
    """
    TYPE_NAME = "rft_cell"
    _free         = EclPrototype("void ecl_rft_cell_free(rft_cell)")
    _get_pressure = EclPrototype("double ecl_rft_cell_get_pressure(rft_cell)")
    _get_depth    = EclPrototype("double ecl_rft_cell_get_depth(rft_cell)")
    _get_i        = EclPrototype("int ecl_rft_cell_get_i(rft_cell)")
    _get_j        = EclPrototype("int ecl_rft_cell_get_j(rft_cell)")
    _get_k        = EclPrototype("int ecl_rft_cell_get_k(rft_cell)")


    def free(self):
        self._free()

    def get_i(self):
        return self._get_i()

    def get_j(self):
        return self._get_j()

    def get_k(self):
        return self._get_k()

    def get_ijk(self):
        return (self.get_i(), self.get_j(), self.get_k())

    @property
    def pressure(self):
        return self._get_pressure()

    @property
    def depth(self):
        return self._get_depth()


#################################################################


class EclRFTCell(RFTCell):
    TYPE_NAME  = "ecl_rft_cell"
    _alloc_RFT = EclPrototype("void* ecl_rft_cell_alloc_RFT(int, int, int, double, double, double, double)", bind = False)
    _get_swat  = EclPrototype("double ecl_rft_cell_get_swat(ecl_rft_cell)")
    _get_soil  = EclPrototype("double ecl_rft_cell_get_soil(ecl_rft_cell)")
    _get_sgas  = EclPrototype("double ecl_rft_cell_get_sgas(ecl_rft_cell)")

    def __init__(self, i, j, k, depth, pressure, swat, sgas):
        c_ptr = self._alloc_RFT(i, j, k, depth, pressure, swat, sgas)
        super(EclRFTCell, self).__init__(c_ptr)

    @property
    def swat(self):
        return self._get_swat()

    @property
    def sgas(self):
        return self._get_sgas()

    @property
    def soil(self):
        return 1 - (self._get_sgas() + self._get_swat())


#################################################################


class EclPLTCell(RFTCell):
    TYPE_NAME = "ecl_plt_cell"
    _alloc_PLT = EclPrototype("void* ecl_rft_cell_alloc_PLT(int, int, int, double, double, double, double, double, double, double, double, double, double, double)", bind=False)
    _get_orat = EclPrototype("double ecl_rft_cell_get_orat(ecl_plt_cell)")
    _get_grat = EclPrototype("double ecl_rft_cell_get_grat(ecl_plt_cell)")
    _get_wrat = EclPrototype("double ecl_rft_cell_get_wrat(ecl_plt_cell)")

    _get_flowrate = EclPrototype("double ecl_rft_cell_get_flowrate(ecl_plt_cell)")
    _get_oil_flowrate = EclPrototype("double ecl_rft_cell_get_oil_flowrate(ecl_plt_cell)")
    _get_gas_flowrate = EclPrototype("double ecl_rft_cell_get_gas_flowrate(ecl_plt_cell)")
    _get_water_flowrate = EclPrototype("double ecl_rft_cell_get_water_flowrate(ecl_plt_cell)")

    _get_conn_start = EclPrototype("double ecl_rft_cell_get_connection_start(ecl_plt_cell)")
    _get_conn_end   = EclPrototype("double ecl_rft_cell_get_connection_end(ecl_plt_cell)")


    def __init__(self, i, j, k, depth, pressure, orat, grat, wrat, conn_start,
                 conn_end, flowrate, oil_flowrate, gas_flowrate, water_flowrate):
        c_ptr = self._alloc_PLT(i, j, k, depth, pressure, orat, grat, wrat,
                                conn_start, conn_end, flowrate, oil_flowrate,
                                gas_flowrate, water_flowrate)
        super(EclPLTCell, self).__init__(c_ptr)


    @property
    def orat(self):
        return self._get_orat()

    @property
    def grat(self):
        return self._get_grat()

    @property
    def wrat(self):
        return self._get_wrat()

    @property
    def conn_start(self):
        """Will return the length from wellhead(?) to connection.

        For MSW wells this property will return the distance from a
        fixed point (wellhead) to the current connection. This value
        will be used to sort the completed cells along the well
        path. In the case of non MSW wells this will just return a
        fixed default value.
        """
        return self._get_conn_start()

    @property
    def conn_end(self):
        """Will return the length from wellhead(?) to connection end.

        For MSW wells this property will return the distance from a
        fixed point (wellhead) to the current connection end. This value
        will be used to sort the completed cells along the well
        path. In the case of non MSW wells this will just return a
        fixed default value.
        """
        return self._get_conn_end()

    @property
    def flowrate(self):
        return self._get_flowrate()

    @property
    def oil_flowrate(self):
        return self._get_oil_flowrate()

    @property
    def gas_flowrate(self):
        return self._get_gas_flowrate()

    @property
    def water_flowrate(self):
        return self._get_water_flowrate()
