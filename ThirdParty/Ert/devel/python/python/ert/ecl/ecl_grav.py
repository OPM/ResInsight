#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_grav.py' is part of ERT - Ensemble based Reservoir Tool. 
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

The ecl_grav module contains functionality to load time-lapse ECLIPSE
results and calculate the change in gravitational strength between the
different surveys. The implementation is a thin wrapper around the
ecl_grav.c implementation in the libecl library.
"""

import  libecl
import  ecl_file 
import  ecl_region 
import  ecl_grid
from    ecl_util              import ECL_WATER_PHASE , ECL_OIL_PHASE , ECL_GAS_PHASE, ecl_phase_enum
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass

class EclGrav(CClass):
    """
    Holding ECLIPSE results for calculating gravity changes.
    
    The EclGrav class is a collection class holding the results from
    ECLIPSE forward modelling of gravity surveys. Observe that the
    class is focused on the ECLIPSE side of things, and does not have
    any notion of observed values or measurement locations; that
    should be handled by the scope using the EclGrav class.

    Typical use of the EclGrav class involves the following steps:

      1. Create the EclGrav instance.
      2. Add surveys with the add_survey_XXXX() methods.
      3. Evalute the gravitational response with the eval() method.
    """

    def __init__( self , grid , init_file ):
        """
        Creates a new EclGrav instance. 

        The input arguments @grid and @init_file should be instances
        of EclGrid and EclFile respectively.
        """
        self.init_file = init_file   # Inhibit premature garbage collection of init_file
        self.init_cobj( cfunc.grav_alloc(grid , init_file) , cfunc.free)
        

        

    def add_survey_RPORV( self , survey_name , restart_file ):
        """
        Add new survey based on RPORV keyword.

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

        The pore volume of each cell will be calculated based on the
        RPORV keyword from the restart files. The methods
        add_survey_PORMOD() and add_survey_FIP() are alternatives
        which are based on other keywords.
        """
        cfunc.add_survey_RPORV( self , survey_name , restart_file )

    def add_survey_PORMOD( self , survey_name , restart_file ):
        """
        Add new survey based on PORMOD keyword.
        
        The pore volum is calculated from the initial pore volume and
        the PORV_MOD keyword from the restart file; see
        add_survey_RPORV() for further details.
        """
        cfunc.add_survey_PORMOD( self , survey_name , restart_file )

    def add_survey_FIP( self , survey_name , restart_file ):
        """
        Add new survey based on FIP keywords.

        This method adds a survey as add_survey_RPORV() and
        add_survey_PORMOD; but the mass content in each cell is
        calculated based on the FIPxxx keyword along with the mass
        density at standard conditions of the respective phases.

        The mass density at standard conditions must be specified with
        the new_std_density() (and possibly also add_std_density())
        method before calling the add_survey_FIP() method.
        """
        cfunc.add_survey_FIP( self , survey_name , restart_file )

    def add_survey_RFIP( self , survey_name , restart_file ):
        """
        Add new survey based on RFIP keywords.

        This method adds a survey as add_survey_RPORV() and
        add_survey_PORMOD; but the mass content in each cell is
        calculated based on the RFIPxxx keyword along with the
        per-cell mass density of the respective phases.
        """
        cfunc.add_survey_RFIP( self , survey_name , restart_file )

                
    def eval(self , base_survey , monitor_survey , pos , region = None , phase_mask = ECL_OIL_PHASE + ECL_GAS_PHASE + ECL_WATER_PHASE):
        """
        Calculates the gravity change between two surveys.
        
        This is the method everything is leading up to; will calculate
        the change in gravitational strength, in units of micro Gal,
        between the two surveys named @base_survey and
        @monitor_survey. 

        The monitor survey can be 'None' - the resulting answer has
        nothing whatsovever to do with gravitation, but can be
        interesting to determine the numerical size of the quantities
        which are subtracted in a 4D study.
        
        The @pos argument should be a tuple of three elements with the
        (utm_x , utm_y , depth) position where we want to evaluate the
        change in gravitational strength.

        If supplied the optional argument @region should be an
        EclRegion() instance; this region will be used to limit the
        part of the reserviour included in the gravity calculations.

        The optional argument @phase_mask is an integer flag to
        indicate which phases you are interested in. It should be a
        sum of the relevant integer constants 'ECL_OIL_PHASE',
        'ECL_GAS_PHASE' and 'ECL_WATER_PHASE'.
        """
        return cfunc.eval( self , base_survey , monitor_survey , region , pos[0] , pos[1] , pos[2] , phase_mask)
    

    def new_std_density( self , phase_enum , default_density):
        """
        Adds a new phase with a corresponding density.

        @phase_enum is one of the integer constants ECL_OIL_PHASE,
        ECL_GAS_PHASE or ECL_WATER_PHASE, all available in the
        ecl_util and also ecl modules.

        @default_density is the density, at standard conditions, for
        this particular phase. By default @default_density will be
        used for all the cells in the model; by using the
        add_std_density() method you can specify different densities
        for different PVT regions.

        The new_std_density() and add_std_density() methods must be
        used before you use the add_survey_FIP() method to add a
        survey based on the FIP keyword.
        """
        cfunc.new_std_density( self , phase_enum , default_density )
        
    def add_std_density( self , phase_enum , pvtnum , density):
        """
        Add standard conditions density for PVT region @pvtnum.
        
        The new_std_density() method will add a standard conditions
        density which applies to all cells in the model. Using the
        add_std_density() method it is possible to add standard
        conditions densities on a per PVT region basis. You can add
        densities for as many PVT regions as you like, and then fall
        back to the default density for the others.

        The new_std_density() method must be called before calling the
        add_std_density() method.
        
        The new_std_density() and add_std_density() methods must be
        used before you use the add_survey_FIP() method to add a
        survey based on the FIP keyword.
        """
        cfunc.add_std_density( self , phase_enum , pvtnum , density )


# 2. Creating a wrapper object around the libecl library, 
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_grav" , EclGrav )


# 3. Installing the c-functions used to manipulate ecl_grav instances.
#    These functions are used when implementing the EclGrav class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_grav")

cfunc.grav_alloc  = cwrapper.prototype("c_void_p   ecl_grav_alloc( ecl_grid , ecl_file )")
cfunc.free        = cwrapper.prototype("void       ecl_grav_free( ecl_grav )")

# Return value ignored in the add_survey_xxx() functions:
cfunc.add_survey_RPORV   = cwrapper.prototype("c_void_p  ecl_grav_add_survey_RPORV( ecl_grav , char* , ecl_file )")
cfunc.add_survey_PORMOD  = cwrapper.prototype("c_void_p  ecl_grav_add_survey_PORMOD( ecl_grav , char* , ecl_file )")
cfunc.add_survey_FIP     = cwrapper.prototype("c_void_p  ecl_grav_add_survey_FIP( ecl_grav , char* , ecl_file )")
cfunc.add_survey_RFIP    = cwrapper.prototype("c_void_p  ecl_grav_add_survey_RFIP( ecl_grav , char* , ecl_file )")

cfunc.new_std_density    = cwrapper.prototype("void      ecl_grav_new_std_density( ecl_grav , int , double)")
cfunc.add_std_density    = cwrapper.prototype("void      ecl_grav_add_std_density( ecl_grav , int , int , double)")
cfunc.eval               = cwrapper.prototype("double    ecl_grav_eval( ecl_grav , char* , char* , ecl_region , double , double , double, int)")

