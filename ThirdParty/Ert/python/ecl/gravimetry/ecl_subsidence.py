#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file 'ecl_subsidence.py' is part of ERT - Ensemble based
#  Reservoir Tool.
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
Calculate dynamic change in gravitational strength.

The ecl_subsidence module contains functionality to load time-lapse ECLIPSE
results and calculate the change in seafloor subsidence between the
different surveys. The implementation is a thin wrapper around the
ecl_subsidence.c implementation in the libecl library.
"""
from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.util.util import monkey_the_camel
import ecl.grid

class EclSubsidence(BaseCClass):
    """
    Holding ECLIPSE results for calculating subsidence changes.

    The EclSubsidence class is a collection class holding the results from
    ECLIPSE forward modelling of subsidence surveys. Observe that the
    class is focused on the ECLIPSE side of things, and does not have
    any notion of observed values or measurement locations; that
    should be handled by the scope using the EclSubsidence class.

    Typical use of the EclSubsidence class involves the following steps:

      1. Create the EclSubsidence instance.
      2. Add surveys with the add_survey_XXXX() methods.
      3. Evalute the subsidence response with the eval() method.
    """
    TYPE_NAME = "ecl_subsidence"
    _alloc               = EclPrototype("void* ecl_subsidence_alloc( ecl_grid , ecl_file )" , bind = False)
    _free                = EclPrototype("void ecl_subsidence_free( ecl_subsidence )")
    _add_survey_PRESSURE = EclPrototype("void*  ecl_subsidence_add_survey_PRESSURE( ecl_subsidence , char* , ecl_file_view )")
    _eval                = EclPrototype("double ecl_subsidence_eval( ecl_subsidence , char* , char* , ecl_region , double , double , double, double, double)")
    _eval_geertsma       = EclPrototype("double ecl_subsidence_eval_geertsma( ecl_subsidence , char* , char* , ecl_region , double , double , double, double, double, double)")
    _eval_geertsma_rporv = EclPrototype("double ecl_subsidence_eval_geertsma_rporv( ecl_subsidence , char* , char* , ecl_region , double , double , double, double, double, double)")
    _has_survey          = EclPrototype("bool  ecl_subsidence_has_survey( ecl_subsidence , char*)")

    def __init__( self, grid, init_file ):
        """
        Creates a new EclSubsidence instance.

        The input arguments @grid and @init_file should be instances
        of EclGrid and EclFile respectively.
        """
        self.init_file = init_file   # Inhibit premature garbage collection of init_file
        c_ptr = self._alloc( grid , init_file )
        super( EclSubsidence , self ).__init__( c_ptr )


    def __contains__(self , survey_name):
        return self._has_survey( survey_name )



    def add_survey_PRESSURE( self, survey_name, restart_file ):
        """
        Add new survey based on PRESSURE keyword.

        Add a new survey; in this context a survey is the state of
        reservoir, i.e. an ECLIPSE restart file. The @survey_name
        input argument will be used when refering to this survey at a
        later stage. The @restart_file input argument should be an
        EclFile instance with data from one report step. A typical way
        to load the @restart_file argument is:

           import datetime
           import ecl.ecl.ecl as ecl
           ...
           ...
           date = datetime.datetime( year , month , day )
           restart_file1 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , dtime = date)
           restart_file2 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , report_step = 67 )

        The pore volume is calculated from the initial pore volume and
        the PRESSURE keyword from the restart file.
        """
        self._add_survey_PRESSURE( survey_name, restart_file)


    def eval_geertsma(self, base_survey, monitor_survey, pos, youngs_modulus, poisson_ratio, seabed, region=None):
        if not base_survey in self:
            raise KeyError("No such survey: %s" % base_survey)

        if monitor_survey is not None:
            if not monitor_survey in self:
                raise KeyError("No such survey: %s" % monitor_survey)

        return self._eval_geertsma(base_survey, monitor_survey, region, pos[0], pos[1], pos[2], youngs_modulus, poisson_ratio, seabed)

    def eval_geertsma_rporv(self, base_survey, monitor_survey, pos, youngs_modulus, poisson_ratio, seabed, region=None):
        if not base_survey in self:
            raise KeyError("No such survey: %s" % base_survey)

        if monitor_survey is not None:
            if not monitor_survey in self:
                raise KeyError("No such survey: %s" % monitor_survey)

        return self._eval_geertsma_rporv(base_survey, monitor_survey, region, pos[0], pos[1], pos[2], youngs_modulus, poisson_ratio, seabed)


    def eval(self, base_survey, monitor_survey, pos, compressibility, poisson_ratio, region=None):
        """
        Calculates the subsidence change between two surveys.

        This is the method everything is leading up to; will calculate
        the change in subsidence, in centimeters,
        between the two surveys named @base_survey and
        @monitor_survey.

        The monitor survey can be 'None' - the resulting answer has
        nothing whatsovever to do with subsidence, but can be
        interesting to determine the numerical size of the quantities
        which are subtracted in a 4D study.

        The @pos argument should be a tuple of three elements with the
        (utm_x , utm_y , depth) position where we want to evaluate the
        change in subsidence.

        If supplied the optional argument @region should be an
        EclRegion() instance; this region will be used to limit the
        part of the reserviour included in the subsidence calculations.

        The argument @compressibility is the total reservoir compressibility.
        """
        if not base_survey in self:
            raise KeyError("No such survey: %s" % base_survey)

        if not monitor_survey in self:
            raise KeyError("No such survey: %s" % monitor_survey)

        return self._eval(base_survey, monitor_survey, region, pos[0], pos[1], pos[2], compressibility,poisson_ratio)



    def free(self):
        self._free( )


monkey_the_camel(EclSubsidence, 'evalGeertsma', EclSubsidence.eval_geertsma)
