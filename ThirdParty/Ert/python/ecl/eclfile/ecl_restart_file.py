#  Copyright (C) 2015  Equinor ASA, Norway.
#
#  The file 'ecl_restart_file.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ecl.util.util import monkey_the_camel
from ecl.util.util import CTime
from ecl import EclFileEnum
from ecl.eclfile import EclFile, Ecl3DFile, Ecl3DKW

class EclRestartHead(BaseCClass):
    TYPE_NAME = "ecl_rsthead"
    _alloc           = EclPrototype("void*  ecl_rsthead_alloc(ecl_file_view , int )", bind = False)
    _alloc_from_kw   = EclPrototype("void*  ecl_rsthead_alloc_from_kw(int , ecl_kw , ecl_kw , ecl_kw )", bind = False)
    _free            = EclPrototype("void   ecl_rsthead_free(ecl_rsthead)")
    _get_report_step = EclPrototype("int    ecl_rsthead_get_report_step(ecl_rsthead)")
    _get_sim_time    = EclPrototype("time_t ecl_rsthead_get_sim_time(ecl_rsthead)")
    _get_sim_days    = EclPrototype("double ecl_rsthead_get_sim_days(ecl_rsthead)")
    _get_nxconz      = EclPrototype("int   ecl_rsthead_get_nxconz(ecl_rsthead)")
    _get_ncwmax      = EclPrototype("int   ecl_rsthead_get_ncwmax(ecl_rsthead)")

    def __init__(self , kw_arg = None , rst_view = None):
        if kw_arg is None and rst_view is None:
            raise ValueError('Cannot construct EclRestartHead without one of kw_arg and rst_view, both were None!')

        if not kw_arg is None:
            report_step , intehead_kw , doubhead_kw , logihead_kw = kw_arg
            c_ptr = self._alloc_from_kw( report_step , intehead_kw , doubhead_kw , logihead_kw )
        else:
            c_ptr = self._alloc( rst_view , -1 )

        super(EclRestartHead, self).__init__(c_ptr)


    def free(self):
        self._free( )

    def get_report_step(self):
        return self._get_report_step( )

    def get_sim_date(self):
        ct = CTime( self._get_sim_time( ) )
        return ct.datetime( )

    def get_sim_days(self):
        return self._get_sim_days( )

    def well_details(self):
        return {"NXCONZ" : self._get_nxconz(),
                "NCWMAX" : self._get_ncwmax()}




class EclRestartFile(Ecl3DFile):

    def __init__(self , grid , filename , flags = 0):
        """Will open an Eclipse restart file.

        The EclRestartFile class will open an eclipse restart file, in
        unified or non unified format. The constructor will infer the
        file type based on the filename, and will raise a ValueError
        exception if the file type is not ECL_RESTART_FILE or
        ECL_UNIFIED_RESTART_FILE.

        The EclRestartFile will use a grid reference to create Ecl3DKw
        instances for all the keyword elements which have either
        'nactive' or 'nx*ny*nz' elements.
        """

        file_type , report_step , fmt_file = EclFile.getFileType( filename )
        if not file_type in [EclFileEnum.ECL_RESTART_FILE, EclFileEnum.ECL_UNIFIED_RESTART_FILE]:
            raise ValueError('The input filename "%s" does not correspond to a restart file.  Please follow the Eclipse naming conventions'
                             % filename)

        super(EclRestartFile , self).__init__( grid, filename , flags)
        self.rst_headers = None
        if file_type == EclFileEnum.ECL_RESTART_FILE:
            self.is_unified = False
            self.report_step = report_step
        else:
            self.is_unified = True



    def unified(self):
        """
        Will return True if the file we have opened is unified.
        """
        return self.is_unified


    def assert_headers(self):
        if self.rst_headers is None:
            self.rst_headers = []
            if self.unified():
                for index in range(self.num_named_kw("SEQNUM")):
                    self.rst_headers.append( EclRestartHead( rst_view = self.restartView( seqnum_index = index )))
            else:
                intehead_kw = self["INTEHEAD"][0]
                doubhead_kw = self["DOUBHEAD"][0]
                if "LOGIHEAD" in self:
                    logihead_kw = self["LOGIHEAD"][0]
                else:
                    logihead_kw = None

                self.rst_headers.append( EclRestartHead( kw_arg = (self.report_step , intehead_kw , doubhead_kw , logihead_kw) ))


    def time_list(self):
        """Will return a list of report_step, simulation time and days.

        The return value will be a list tuples. For a unified restart
        file with the three report steps {10,15,20} it can look like:

           [  (10, datetime.datetime( 2010 , 1 , 1 , 0 , 0 , 0 ) , 100.0),
              (15, datetime.datetime( 2010 , 3 , 1 , 0 , 0 , 0 ) , 160.0),
              (20, datetime.datetime( 2010 , 5 , 1 , 0 , 0 , 0 ) , 220.0) ]

        For a non-unified restart file the list will have only one element.
        """

        self.assertHeaders()
        time_list = []
        for header in self.rst_headers:
            time_list.append( (header.getReportStep() , header.getSimDate( ) , header.getSimDays( )) )

        return time_list


    def headers(self):
        self.assertHeaders()
        return self.rst_headers


    def get_header(self, index):
        self.assertHeaders()
        return self.rst_headers[index]

monkey_the_camel(EclRestartHead, 'getReportStep', EclRestartHead.get_report_step)
monkey_the_camel(EclRestartHead, 'getSimDate', EclRestartHead.get_sim_date)
monkey_the_camel(EclRestartHead, 'getSimDays', EclRestartHead.get_sim_days)

monkey_the_camel(EclRestartFile, 'assertHeaders', EclRestartFile.assert_headers)
monkey_the_camel(EclRestartFile, 'timeList', EclRestartFile.time_list)
