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


class EclDefault(object):
    _initialized = False

    _ecl_cmd = None
    _ecl_version = None
    _lsf_resource_request = None
    _driver_type = None
    _driver_options = None

    @staticmethod
    def _initializeEclDefaults():
        if not EclDefault._initialized:
            try:
                import ert.ecl.ecl_local as ecl_local

                if hasattr(ecl_local, "ecl_cmd"):
                    EclDefault._ecl_cmd = ecl_local.ecl_cmd

                if hasattr(ecl_local, "ecl_version"):
                    EclDefault._ecl_version = ecl_local.ecl_version

                if hasattr(ecl_local, "lsf_resource_request"):
                    EclDefault._lsf_resource_request = ecl_local.lsf_resource_request

                if hasattr(ecl_local, "driver_type"):
                    EclDefault._driver_type = ecl_local.driver_type

                if hasattr(ecl_local, "driver_options"):
                    EclDefault._driver_options = ecl_local.driver_options


            except ImportError as e:
                pass

            EclDefault._initialized = True


    @staticmethod
    def ecl_cmd():
        """
        The path to the executable used to invoke ECLIPSE.
        """
        EclDefault._initializeEclDefaults()

        if EclDefault._ecl_cmd is None:
            EclDefault._attributeIsNotSet("ecl_cmd")
        return EclDefault._ecl_cmd

    @staticmethod
    def driver_type():
        EclDefault._initializeEclDefaults()

        if EclDefault._driver_type is None:
            EclDefault._attributeIsNotSet("driver_type")
        return EclDefault._driver_type

    @staticmethod
    def driver_options():
        EclDefault._initializeEclDefaults()

        if EclDefault._driver_options is None:
            EclDefault._attributeIsNotSet("driver_options")
        return EclDefault._driver_options

    @staticmethod
    def ecl_version():
        EclDefault._initializeEclDefaults()

        if EclDefault._ecl_version is None:
            EclDefault._attributeIsNotSet("ecl_version")
        return EclDefault._ecl_version

    @staticmethod
    def lsf_resource_request():
        EclDefault._initializeEclDefaults()

        if EclDefault._lsf_resource_request is None:
            EclDefault._attributeIsNotSet("lsf_resource_request")
        return EclDefault._lsf_resource_request

    @staticmethod
    def _attributeIsNotSet(attribute_name):
        raise NotImplementedError("The default attribute:%s has not been set - you must update/provide a ecl_local module." % attribute_name)


#Legacy import support

class default_wrapper(object):
    #from warnings import warn
    #warn("The ecl_default namespace is deprecated! Please retrieve ecl_default values from the class: ert.ecl.EclDefault!")

    @property
    def ecl_cmd(self):
        return EclDefault.ecl_cmd()

    @property
    def driver_type(self):
        return EclDefault.driver_type()

    @property
    def driver_options(self):
        return EclDefault.driver_options()

    @property
    def ecl_version(self):
        return EclDefault.ecl_version()

    @property
    def lsf_resource_request(self):
        return EclDefault.lsf_resource_request()


default = default_wrapper()