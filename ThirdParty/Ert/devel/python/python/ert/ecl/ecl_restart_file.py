#  Copyright (C) 2015  Statoil ASA, Norway. 
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

from ert.util import CTime
from ert.ecl import ECL_LIB , EclFile, Ecl3DKW , Ecl3DFile, EclFileEnum
from ert.cwrap import CWrapper, BaseCClass

class EclRestartHead(BaseCClass):
    def __init__(self , kw_arg = None , rst_arg = None):
        if kw_arg is None and rst_arg is None:
            raise Exception("Invalid arguments")

        if not kw_arg is None:
            report_step , intehead_kw , doubhead_kw , logihead_kw = kw_arg
            c_ptr = EclRestartHead.cNamespace().alloc_from_kw( report_step , intehead_kw , doubhead_kw , logihead_kw )
        else:
            rst_file , occurence = rst_arg
            c_ptr = EclRestartHead.cNamespace().alloc( rst_file , occurence )

        super(EclRestartHead, self).__init__(c_ptr)

        
    def free(self):
        EclRestartHead.cNamespace().free( self )

    def getReportStep(self):
        return EclRestartHead.cNamespace().get_report_step( self )

    def getSimDate(self):
        ct = CTime( EclRestartHead.cNamespace().get_sim_time( self ) )
        return ct.datetime( )

    def getSimDays(self):
        return EclRestartHead.cNamespace().get_sim_days( self )
        
        

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
            raise ValueError("The input filename:%s does not correspond to a restart file - please follow the Eclipse naming conventions" % filename)
            
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

    
    def assertHeaders(self):
        if self.rst_headers is None:
            self.rst_headers = []
            if self.unified():
                for index in range(self.num_named_kw("SEQNUM")):
                    self.rst_headers.append( EclRestartHead( rst_arg = (self , index )))
            else:
                intehead_kw = self["INTEHEAD"][0]
                doubhead_kw = self["DOUBHEAD"][0]
                if "LOGIHEAD" in self:
                    logihead_kw = self["LOGIHEAD"][0]
                else:
                    logihead_kw = None

                self.rst_headers.append( EclRestartHead( kw_arg = (self.report_step , intehead_kw , doubhead_kw , logihead_kw) ))
                
            
    def timeList(self):
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

    

            
CWrapper.registerObjectType("ecl_rsthead", EclRestartHead)
cwrapper = CWrapper(ECL_LIB)
EclRestartHead.cNamespace().alloc           = cwrapper.prototype("c_void_p ecl_rsthead_ialloc(ecl_file , int )")
EclRestartHead.cNamespace().alloc_from_kw   = cwrapper.prototype("c_void_p ecl_rsthead_alloc_from_kw(int , ecl_kw , ecl_kw , ecl_kw )")
EclRestartHead.cNamespace().free            = cwrapper.prototype("void ecl_rsthead_free(ecl_rsthead)")
EclRestartHead.cNamespace().get_report_step = cwrapper.prototype("int ecl_rsthead_get_report_step(ecl_rsthead)")
EclRestartHead.cNamespace().get_sim_time    = cwrapper.prototype("time_t ecl_rsthead_get_sim_time(ecl_rsthead)")
EclRestartHead.cNamespace().get_sim_days    = cwrapper.prototype("double ecl_rsthead_get_sim_days(ecl_rsthead)")
