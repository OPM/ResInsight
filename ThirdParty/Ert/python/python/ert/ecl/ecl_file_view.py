import types
from cwrap import BaseCClass
from ert.ecl import EclPrototype, EclKW
from ert.util import CTime

class EclFileView(BaseCClass):
    TYPE_NAME             = "ecl_file_view"
    _iget_kw              = EclPrototype("ecl_kw_ref    ecl_file_view_iget_kw( ecl_file_view , int)")
    _iget_named_kw        = EclPrototype("ecl_kw_ref    ecl_file_view_iget_named_kw( ecl_file_view , char* , int)")
    _get_size             = EclPrototype("int           ecl_file_view_get_size( ecl_file_view )")
    _get_num_named_kw     = EclPrototype("int           ecl_file_view_get_num_named_kw( ecl_file_view , char* )")
    _get_unique_size      = EclPrototype("int           ecl_file_view_get_num_distinct_kw( ecl_file_view )")
    _create_block_view    = EclPrototype("ecl_file_view_ref ecl_file_view_add_blockview( ecl_file_view , char*, int )")
    _create_block_view2   = EclPrototype("ecl_file_view_ref ecl_file_view_add_blockview2( ecl_file_view , char*, char*, int )")
    _restart_view         = EclPrototype("ecl_file_view_ref ecl_file_view_add_restart_view( ecl_file_view , int, int, time_t, double )")
    

    def __init__(self):
        raise NotImplementedError("Can not instantiate directly")


    def __iget(self , index):
        return self._iget_kw( index ).setParent( parent = self )


    

    def iget_named_kw(self, kw_name , index):
        if not kw_name in self:
            raise KeyError("No such keyword: %s" % kw_name)

        if index >= self.numKeywords( kw_name ):
            raise IndexError("Too large index: %d" % index)
        
        return self._iget_named_kw( kw_name , index ).setParent( parent = self )



    def __getitem__(self , index):
        """
        Implements [] operator; index can be integer or key.

        Will look up EclKW instances from the current EclFile
        instance. The @index argument can either be an integer, in
        which case the method will return EclKW number @index, or
        alternatively a keyword string, in which case the method will
        return a list of EclKW instances with that keyword:

           restart_file = ecl_file.EclFile("ECLIPSE.UNRST")
           kw9 = restart_file[9]
           swat_list = restart_file["SWAT"]

        The keyword based lookup can be combined with an extra [] to
        get EclKW instance nr:

           swat9 = restart_file["SWAT"][9]

        Will return the 10'th SWAT keyword from the restart file. The
        following example will iterate over all the SWAT keywords in a
        restart file:

           restart_file = ecl_file.EclFile("ECLIPSE.UNRST")
           for swat in restart_file["SWAT"]:
               ....
        """

        if isinstance( index , types.IntType):
            if index < 0 or index >= len(self):
                raise IndexError
            else:
                kw = self.__iget( index )
                return kw

        if isinstance( index , slice ):
            indices = index.indices( len(self) )
            kw_list = []
            for i in range(*indices):
                kw_list.append( self[i] )
            return kw_list
        else:
            if isinstance( index , types.StringType):
                if index in self:
                    kw_index = index
                    kw_list = []
                    for index in range( self.numKeywords( kw_index )):
                        kw_list.append(  self.iget_named_kw( kw_index , index))
                    return kw_list
                else:
                    raise KeyError("Unrecognized keyword:\'%s\'" % index)
            else:
                raise TypeError("Index must be integer or string (keyword)")


    def __len__(self):
        return self._get_size( )
            
            
    def __contains__(self , kw):
        if self.numKeywords(kw) > 0:
            return True
        else:
            return False

            
    def numKeywords(self , kw):
        return self._get_num_named_kw( kw )

    
    def uniqueSize(self):
        return self._get_unique_size( )

    def blockView2(self , start_kw , stop_kw, start_index):
        if start_kw:
            if not start_kw in self:
                raise KeyError("The keyword:%s is not in file" % start_kw)

            if start_index >= self.numKeywords( start_kw ):
                raise IndexError("Index too high")
            
        if stop_kw:
            if not stop_kw in self:
                raise KeyError("The keyword:%s is not in file" % stop_kw)

        view = self._create_block_view2( start_kw , stop_kw , start_index )
        view.setParent( parent = self )
        return view

    
        
    def blockView(self , kw , kw_index):
        num = self.numKeywords( kw )

        if num == 0:
            raise KeyError("Unknown keyword: %s" % kw)

        if kw_index >= num:
            raise IndexError("Index too high")
        
        view = self._create_block_view( kw , kw_index )
        view.setParent( parent = self )
        return view


    
    def restartView(self , seqnum_index = None, report_step = None , sim_time = None , sim_days = None):
        if report_step is None:
            report_step = -1

        if sim_time is None:
            sim_time = -1

        if sim_days is None:
            sim_days = -1

        if seqnum_index is None:
            seqnum_index = -1
        
        view = self._restart_view( seqnum_index , report_step , CTime( sim_time ) , sim_days )
        if view is None:
            raise ValueError("No such restart block could be identiefied")

        view.setParent( parent = self )
        return view
