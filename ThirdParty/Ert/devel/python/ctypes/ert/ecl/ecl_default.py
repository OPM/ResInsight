#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_default.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Module containing default values for ECLIPSE.

This module contains site specific default values for various
variables related to the eclipse environment. Observe that this module
itself does not set any default values, instead it will try to import
a module ecl_local; and then subsequently read all values from this
module. The ecl_local module is not provided in the ert source
distribution, you must create this yourself and install along with
this module.

It is not necessary to create a ecl_local module, but if you try to
access the default properties and have not created a ecl_local module
a NotImplemtedError exception will be raised.

The intention is not to use the EclDefault explicitly, rather the
ert-python code will consult the EclDefault class internally. For
instance the EclCase method to submit an ECLIPSE simulation looks like
this:

   import ecl_default.default as default
   ...
   ...
   def submit(self , version = None , cmd = None):
       if not cmd:
          cmd = default.cmd
       if not version:
          version = default.version
       ...
       
I.e. if the user has not supplied values for the optional arguments
version and cmd the method will consult the default instance from the
ecl_default module.
   
The ecl_local module can define the following variables:

  cmd:
  version:
  lsf_resource_request:

Depending on the computing environment some variables might be
irrelevant; it is not necessary to set variables which will not be
used.
""" 

class EclDefault:
    __ecl_cmd = None
    __ecl_version = None
    __lsf_resource_request = None
    __driver_type = None
    __driver_options = None

    def __init__(self):
        pass

    def safe_get( self , attr ):
        """
        Internal function to get class attributes.
        
        Will raise NotImplemtedError if the attribute has not been
        loaded from the site spesific ecl_default module.
        """
        if hasattr( self , "__%s" % attr):
            value = getattr( self , "__%s" % attr)
            return value
        else:
            raise NotImplementedError("The default attribute:%s has not been set - you must update/provide a ecl_local module." % attr)
        
    @property
    def ecl_cmd( self ):
        """
        The path to the executable used to invoke ECLIPSE.
        """
        return self.safe_get( "ecl_cmd" )

    @property
    def driver_type( self ):
        return self.safe_get( "driver_type" )

    @property
    def driver_options( self ):
        return self.safe_get( "driver_options" )

    @property
    def ecl_version( self ):
        return self.safe_get( "ecl_version" )

    @property
    def lsf_resource_request( self ):
        return self.safe_get( "lsf_resource_request" )


try:
    import ecl_local

    if hasattr( ecl_local , "ecl_cmd"):
        EclDefault.__ecl_cmd = ecl_local.ecl_cmd

    if hasattr( ecl_local , "ecl_version"):
        EclDefault.__ecl_version = ecl_local.ecl_version

    if hasattr( ecl_local , "lsf_resource_request"):
        EclDefault.__lsf_resource_request = ecl_local.lsf_resource_request

    if hasattr( ecl_local , "driver_type"):
        EclDefault.__driver_type = ecl_local.driver_type

    if hasattr( ecl_local , "driver_options"):
        EclDefault.__driver_options = ecl_local.driver_options

    
except ImportError:
    pass


default = EclDefault()

