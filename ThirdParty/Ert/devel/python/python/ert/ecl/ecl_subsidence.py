#  Copyright (C) 2011  Statoil ASA, Norway. 
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

import  libecl
import  ecl_file 
import  ecl_region 
import  ecl_grid
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass


class EclSubsidence(CClass):
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

    def __init__( self , grid , init_file ):
        """
        Creates a new EclSubsidence instance. 

        The input arguments @grid and @init_file should be instances
        of EclGrid and EclFile respectively.
        """
        self.init_file = init_file   # Inhibit premature garbage collection of init_file
        self.init_cobj( cfunc.subsidence_alloc( grid , init_file ) , cfunc.free )
        

    def add_survey_PRESSURE( self , survey_name , restart_file ):
        """
        Add new survey based on PRESSURE keyword.

        Add a new survey; in this context a survey is the state of
        reservoir, i.e. an ECLIPSE restart file. The @survey_name
        input argument will be used when refering to this survey at a
        later stage. The @restart_file input argument should be an
        EclFile instance with data from one report step. A typical way
        to load the @restart_file argument is:

           import datetime
           import ert.ecl.ecl as ecl
           ...
           ...
           date = datetime.datetime( year , month , day )
           restart_file1 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , dtime = date)
           restart_file2 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , report_step = 67 )

        The pore volume is calculated from the initial pore volume and
        the PRESSURE keyword from the restart file.
        """
        cfunc.add_survey_PRESSURE( self , survey_name , restart_file )

                
    def eval(self , base_survey , monitor_survey , pos , compressibility , poisson_ratio , region = None):
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
        return cfunc.eval( self , base_survey , monitor_survey , region , pos[0] , pos[1] , pos[2] , compressibility, poisson_ratio)

# 2. Creating a wrapper object around the libecl library, 
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_subsidence" , EclSubsidence )

# 3. Installing the c-functions used to manipulate ecl_subsidence instances.
#    These functions are used when implementing the EclSubsidence class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_subsidence")

cfunc.subsidence_alloc  = cwrapper.prototype("c_void_p   ecl_subsidence_alloc( ecl_grid , ecl_file )")
cfunc.free              = cwrapper.prototype("void       ecl_subsidence_free( ecl_subsidence )")

# Return value ignored in the add_survey_xxx() functions:
cfunc.add_survey_PRESSURE   = cwrapper.prototype("c_void_p  ecl_subsidence_add_survey_PRESSURE( ecl_subsidence , char* , ecl_file )")
cfunc.eval                  = cwrapper.prototype("double    ecl_subsidence_eval( ecl_subsidence , char* , char* , ecl_region , double , double , double, double, double)")
