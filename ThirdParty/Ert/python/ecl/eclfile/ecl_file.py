#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'ecl_file.py' is part of ERT - Ensemble based Reservoir Tool.
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
The ecl_file module contains functionality to load a an ECLIPSE file
in 'restart format'. Files of 'restart format' include restart files,
init files, grid files, summary files and RFT files.

The ecl_file implementation is agnostic[1] to the content and
structure of the file; more specialized classes like EclSum and
EclGrid use the EclFile functionality for low level file loading.

The typical usage involves loading a complete file, and then
subsequently querying for various keywords. In the example below we
load a restart file, and ask for the SWAT keyword:

   file = EclFile( "ECLIPSE.X0067" )
   swat_kw = file.iget_named_kw( "SWAT" , 0 )

The ecl_file module is a thin wrapper around the ecl_file.c
implementation from the libecl library.

[1]: In particular for restart files, which do not have a special
     RestartFile class, there is some specialized functionality.
"""
import re
import types
import datetime
import ctypes

from cwrap import BaseCClass

from ecl import EclPrototype
from ecl.util.util import CTime
from ecl.util.util import monkey_the_camel
from ecl import EclFileEnum
from ecl.eclfile import EclKW, EclFileView


class EclFile(BaseCClass):
    TYPE_NAME = "ecl_file"
    _open                        = EclPrototype("void*       ecl_file_open( char* , int )" , bind = False)
    _get_file_type               = EclPrototype("ecl_file_enum ecl_util_get_file_type( char* , bool* , int*)" , bind = False)
    _writable                    = EclPrototype("bool        ecl_file_writable( ecl_file )")
    _save_kw                     = EclPrototype("void        ecl_file_save_kw( ecl_file , ecl_kw )")
    _close                       = EclPrototype("void        ecl_file_close( ecl_file )")
    _iget_restart_time           = EclPrototype("time_t      ecl_file_iget_restart_sim_date( ecl_file , int )")
    _iget_restart_days           = EclPrototype("double      ecl_file_iget_restart_sim_days( ecl_file , int )")
    _get_restart_index           = EclPrototype("int         ecl_file_get_restart_index( ecl_file , time_t)")
    _get_src_file                = EclPrototype("char*       ecl_file_get_src_file( ecl_file )")
    _replace_kw                  = EclPrototype("void        ecl_file_replace_kw( ecl_file , ecl_kw , ecl_kw , bool)")
    _fwrite                      = EclPrototype("void        ecl_file_fwrite_fortio( ecl_file , fortio , int)")
    _has_report_step             = EclPrototype("bool        ecl_file_has_report_step( ecl_file , int)")
    _has_sim_time                = EclPrototype("bool        ecl_file_has_sim_time( ecl_file , time_t )")
    _get_global_view             = EclPrototype("ecl_file_view_ref ecl_file_get_global_view( ecl_file )")
    _write_index                 = EclPrototype("bool        ecl_file_write_index( ecl_file , char*)")
    _fast_open                   = EclPrototype("void*       ecl_file_fast_open( char* , char* , int )" , bind=False)


    @staticmethod
    def get_filetype(filename):
        fmt_file    = ctypes.c_bool()
        report_step = ctypes.c_int()

        file_type = EclFile._get_file_type( filename , ctypes.byref( fmt_file ) , ctypes.byref(report_step))
        if file_type in [EclFileEnum.ECL_RESTART_FILE , EclFileEnum.ECL_SUMMARY_FILE]:
            report_step = report_step.value
        else:
            report_step = None

        if file_type in [EclFileEnum.ECL_OTHER_FILE , EclFileEnum.ECL_DATA_FILE]:
            fmt_file = None
        else:
            fmt_file = fmt_file.value


        return (file_type , report_step , fmt_file )



    @classmethod
    def restart_block( cls , filename , dtime = None , report_step = None):
        raise NotImplementedError("The restart_block implementation has been removed - open file normally and use EclFileView.")



    @classmethod
    def contains_report_step( cls , filename , report_step ):
        """
        Will check if the @filename contains @report_step.

        This classmethod works by opening the file @filename and
        searching through linearly to see if an ecl_kw with value
        corresponding to @report_step can be found. Since this is a
        classmethod it is invoked like this:

           import ecl.ecl.ecl as ecl
           ....
           if ecl.EclFile.contains_report_step("ECLIPSE.UNRST" , 20):
              print "OK - file contains report step 20"
           else:
              print "File does not contain report step 20"

        If you have already loaded the file into an EclFile instance
        you should use the has_report_step() method instead.
        """
        obj = EclFile( filename )
        return obj.has_report_step( report_step )


    @classmethod
    def contains_sim_time( cls , filename , dtime ):
        """
        Will check if the @filename contains simulation at @dtime.

        This classmethod works by opening the file @filename and
        searching through linearly to see if a result block at the
        time corresponding to @dtime can be found. Since this is a
        classmethod it is invoked like this:

           import ecl.ecl.ecl as ecl
           ....
           if ecl.EclFile.contains_sim_time("ECLIPSE.UNRST" , datetime.datetime( 2007 , 10 , 10) ):
              print "OK - file contains 10th of October 2007"
           else:
              print "File does not contain 10th of October 2007"

        If you have already loaded the file into an EclFile instance
        you should use the has_sim_time() method instead.
        """
        obj = EclFile( filename )
        return obj.has_sim_time( dtime )

    @property
    def report_list(self):
        report_steps = []
        try:
            seqnum_list = self["SEQNUM"]
            for s in seqnum_list:
                report_steps.append( s[0] )
        except KeyError:
            # OK - we did not have seqnum; that might be because this
            # a non-unified restart file; or because this is not a
            # restart file at all.
            fname = self.getFilename( )
            matchObj = re.search("\.[XF](\d{4})$" , fname)
            if matchObj:
                report_steps.append( int(matchObj.group(1)) )
            else:
                raise TypeError('Tried get list of report steps from file "%s" - which is not a restart file' % fname)


        return report_steps



    @classmethod
    def file_report_list( cls , filename ):
        """
        Will identify the available report_steps from @filename.
        """

        file = EclFile( filename )
        return file.report_list



    def __repr__(self):
        fn = self.getFilename()
        wr = ', read/write' if self._writable() else ''
        return self._create_repr('"%s"%s' % (fn,wr))


    def __init__( self , filename , flags = 0 , index_filename = None):
        """
        Loads the complete file @filename.

        Will create a new EclFile instance with the content of file
        @filename. The file @filename must be in 'restart format' -
        otherwise it will be crash and burn.

        The optional argument flags can be an or'ed combination of the
        flags:

           ecl.ECL_FILE_WRITABLE : It is possible to update the
              content of the keywords in the file.

           ecl.ECL_FILE_CLOSE_STREAM : The underlying FILE * is closed
              when not used; to save number of open file descriptors
              in cases where a high number of EclFile instances are
              open concurrently.

        When the file has been loaded the EclFile instance can be used
        to query for and get reference to the EclKW instances
        constituting the file, like e.g. SWAT from a restart file or
        FIPNUM from an INIT file.
        """
        if index_filename is None:
            c_ptr = self._open( filename , flags )
        else:
            c_ptr = self._fast_open(filename, index_filename, flags)

        if c_ptr is None:
            raise IOError('Failed to open file "%s"' % filename)
        else:
            super(EclFile , self).__init__(c_ptr)
            self.global_view = self._get_global_view( )
            self.global_view.setParent( self )


    def save_kw( self , kw ):
        """
        Will write the @kw back to file.

        This function should typically be used in situations like this:

          1. Create an EclFile instance around an ECLIPSE output file.
          2. Extract a keyword of interest and modify it.
          3. Call this method to save the modifications to disk.

        There are several restrictions to the use of this function:

          1. The EclFile instance must have been created with the
             optional read_only flag set to False.

          2. You can only modify the content of the keyword; if you
             try to modify the header in any way (i.e. size, datatype
             or name) the function will fail.

          3. The keyword you are trying to save must be exactly the
             keyword you got from this EclFile instance, otherwise the
             function will fail.
        """
        if self._writable( ):
            self._save_kw(  kw )
        else:
            raise IOError('save_kw: the file "%s" has been opened read only.' % self.getFilename( ))


    def __len__(self):
        return len(self.global_view)


    def close(self):
        if self:
            self._close( )
            self._invalidateCPointer( )


    def free(self):
        self.close()


    def block_view(self, kw, kw_index):
        if not kw in self:
            raise KeyError('No such keyword "%s".' % kw)
        ls = self.global_view.numKeywords(kw)
        idx = kw_index
        if idx < 0:
            idx += ls
        if 0 <= idx < ls:
            return self.global_view.blockView(kw, idx)
        raise IndexError('Index out of range, must be in [0, %d), was %d.' % (ls, kw_index))


    def block_view2(self, start_kw, stop_kw, start_index):
        return self.global_view.blockView2( start_kw , stop_kw, start_index )


    def restart_view(self, seqnum_index=None, report_step=None, sim_time=None, sim_days=None):
        return self.global_view.restartView( seqnum_index, report_step , sim_time, sim_days )


    def select_block(self, kw, kw_index):
        raise NotImplementedError("The select_block implementation has been removed - use EclFileView")


    def select_global( self ):
        raise NotImplementedError("The select_global implementation has been removed - use EclFileView")


    def select_restart_section( self, index = None , report_step = None , sim_time = None):
        raise NotImplementedError("The select_restart_section implementation has been removed - use EclFileView")
        """
        Will select a restart section as the active section.

        You must specify a report step with the @report_step argument,
        a true time with the @sim_time argument or a plain index to
        select restart block. If none of arguments are given exception
        TypeError will be raised. If present the @sim_time argument
        should be a datetime instance.

        If the restart section you ask for can not be found the method
        will raise a ValueError exeception. To protect against this
        you can query first with the has_report_step(),
        has_sim_time() or num_report_steps() methods.

        This method should be used when you have already loaded the
        complete file; if you only want to load a section from the
        file you can use the classmethod restart_block().

        The method will return 'self' which can be used to aid
        readability.
        """



    def select_last_restart( self ):
        raise NotImplementedError("The select_restart_section implementation has been removed - use EclFileView")
        """
        Will select the last SEQNUM block in restart file.

        Works by searching for the last SEQNUM keyword; the SEQNUM
        Keywords are only present in unified restart files. If this
        is a non-unified restart file (or not a restart file at all),
        the method will do nothing and return False.
        """





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
        if isinstance(index, int):
            ls = len(self)
            idx = index
            if idx < 0:
                idx += ls
            if 0 <= idx < ls:
                return self.global_view[idx]
            else:
                raise IndexError('Index must be in [0, %d), was: %d.' % (ls, index))
        return self.global_view[index]


    def iget_kw( self , index , copy = False):
        """
        Will return EclKW instance nr @index.

        In the files loaded with the EclFile implementation the
        ECLIPSE keywords come sequentially in a long series, an INIT
        file might have the following keywords:

          INTEHEAD
          LOGIHEAD
          DOUBHEAD
          PORV
          DX
          DY
          DZ
          PERMX
          PERMY
          PERMZ
          MULTX
          MULTY
          .....

        The iget_kw() method will give you a EclKW reference to
        keyword nr @index. This functionality is also available
        through the index operator []:

           file = EclFile( "ECLIPSE.INIT" )
           permx = file.iget_kw( 7 )
           permz = file[ 9 ]

        Observe that the returned EclKW instance is only a reference
        to the data owned by the EclFile instance.

        The method iget_named_kw() which lets you specify the name of
        the keyword you are interested in is in general more useful
        than this method.
        """
        kw = self[index]
        if copy:
            return EclKW.copy( kw )
        else:
            return kw


    def iget_named_kw( self , kw_name , index , copy = False):
        return self.global_view.iget_named_kw( kw_name , index )



    def restart_get_kw( self , kw_name , dtime , copy = False):
        """Will return EclKW @kw_name from restart file at time @dtime.

        This function assumes that the current EclFile instance
        represents a restart file. It will then look for keyword
        @kw_name exactly at the time @dtime; @dtime is a datetime
        instance:

            file = EclFile( "ECLIPSE.UNRST" )
            swat2010 = file.restart_get_kw( "SWAT" , datetime.datetime( 2000 , 1 , 1 ))

        By default the returned kw instance is a reference to the
        ecl_kw still contained in the EclFile instance; i.e. the kw
        will become a dangling reference if the EclFile instance goes
        out of scope. If the optional argument @copy is True the
        returned kw will be a true copy.

        If the file does not have the keyword at the specified time
        the function will raise IndexError(); if the file does not
        have the keyword at all - KeyError will be raised.
        """
        index = self._get_restart_index( CTime( dtime ) )
        if index >= 0:
            if self.num_named_kw(kw_name) > index:
                kw = self.iget_named_kw( kw_name , index )
                if copy:
                    return EclKW.copy( kw )
                else:
                    return kw
            else:
                if self.has_kw(kw_name):
                    raise IndexError('Does not have keyword "%s" at time:%s.' % (kw_name , dtime))
                else:
                    raise KeyError('Keyword "%s" not recognized.' % kw_name)
        else:
            raise IndexError('Does not have keyword "%s" at time:%s.' % (kw_name , dtime))


    def replace_kw( self , old_kw , new_kw):
        """
        Will replace @old_kw with @new_kw in current EclFile instance.

        This method can be used to replace one of the EclKW instances
        in the current EclFile. The @old_kw reference must be to the
        actual EclKW instance in the current EclFile instance (the
        final comparison is based on C pointer equality!), i.e. it
        must be a reference (not a copy) from one of the ??get_kw??
        methods of the EclFile class. In the example below we replace
        the SWAT keyword from a restart file:

           swat = file.iget_named_kw( "SWAT" , 0 )
           new_swat = swat * 0.25
           file.replace_kw( swat , new_swat )


        The C-level ecl_file_type structure takes full ownership of
        all installed ecl_kw instances; mixing the garbage collector
        into it means that this is quite low level - and potentially
        dangerous!
        """

        # We ensure that this scope owns the new_kw instance; the
        # new_kw will be handed over to the ecl_file instance, and we
        # can not give away something we do not alreeady own.
        if not new_kw.data_owner:
            new_kw = EclKW.copy( new_kw )

        # The ecl_file instance will take responsability for freeing
        # this ecl_kw instance.
        new_kw.data_owner = False
        self._replace_kw( old_kw , new_kw , False )



    @property
    def size(self):
        """
        The number of keywords in the current EclFile object.
        """
        return len(self)

    @property
    def unique_size( self ):
        """
        The number of unique keyword (names) in the current EclFile object.
        """
        return self.global_view.uniqueSize( )



    def keys(self):
        """
        Will return a list of unique kw names - like keys() on a dict.
        """
        header_dict = {}
        for index in range(len(self)):
            kw = self[index]
            header_dict[ kw.getName() ] = True
        return header_dict.keys()


    @property
    def headers(self):
        """
        Will return a list of the headers of all the keywords.
        """
        header_list = []
        for index in range(self.size):
            kw = self[index]
            header_list.append( kw.header )
        return header_list

    @property
    def report_steps( self ):
        """
        Will return a list of all report steps.

        The method works by iterating through the whole restart file
        looking for 'SEQNUM' keywords; if the current EclFile instance
        is not a restart file it will not contain any 'SEQNUM'
        keywords and the method will simply return an empty list.
        """
        steps = []
        seqnum_list = self["SEQNUM"]
        for kw in self["SEQNUM"]:
            steps.append( kw[0] )
        return steps

    @property
    def report_dates( self ):
        """
        Will return a list of the dates for all report steps.

        The method works by iterating through the whole restart file
        looking for 'SEQNUM/INTEHEAD' keywords; the method can
        probably be tricked by other file types also containing an
        INTEHEAD keyword.
        """
        if self.has_kw('SEQNUM'):
            dates = []
            for index in range( self.num_named_kw( 'SEQNUM' )):
                dates.append( self.iget_restart_sim_time( index ))
            return dates
        elif 'INTEHEAD' in self:
            # This is a uber-hack; should export the ecl_rsthead
            # object as ctypes structure.
            intehead = self["INTEHEAD"][0]
            year = intehead[66]
            month = intehead[65]
            day = intehead[64]
            date = datetime.datetime( year , month , day )
            return [ date ]
        return None


    @property
    def dates( self ):
        """
        Will return a list of the dates for all report steps.
        """
        return self.report_dates


    def num_named_kw( self , kw):
        """
        The number of keywords with name == @kw in the current EclFile object.
        """
        return self.global_view.numKeywords( kw )


    def has_kw( self , kw , num = 0):
        """
        Check if current EclFile instance has a keyword @kw.

        If the optional argument @num is given it will check if the
        EclFile has at least @num occurences of @kw.
        """

        return self.num_named_kw( kw ) > num

    def __contains__(self , kw):
        """
        Check if the current file contains keyword @kw.
        """
        return self.has_kw( kw )

    def has_report_step( self , report_step ):
        """
        Checks if the current EclFile has report step @report_step.

        If the EclFile in question is not a restart file, you will
        just get False. If you want to check if the file contains the
        actual report_step before loading the file, you should use the
        classmethod contains_report_step() instead.
        """
        return self._has_report_step( report_step )

    def num_report_steps( self ):
        """
        Returns the total number of report steps in the restart file.

        Works by counting the number of 'SEQNUM' instances, and will
        happily return 0 for a non-restart file. Observe that the
        report_steps present in a unified restart file are in general
        not consecutive, i.e. the last report step will typically be
        much higher than the return value from this function.
        """
        return len( self["SEQNUM"] )



    def has_sim_time( self , dtime ):
        """
        Checks if the current EclFile has data for time @dtime.

        The implementation goes through all the INTEHEAD headers in
        the EclFile, i.e. it can be fooled (and probably crash and
        burn) if the EclFile instance in question is has INTEHEAD
        keyword(s), but is still not a restart file. The @dtime
        argument should be a normal python datetime instance.
        """
        return self._has_sim_time( CTime(dtime) )


    def iget_restart_sim_time( self , index ):
        """
        Will locate restart block nr @index and return the true time
        as a datetime instance.
        """
        ct = CTime(self._iget_restart_time( index ))
        return ct.datetime()


    def iget_restart_sim_days( self , index ):
        """
        Will locate restart block nr @index and return the number of days
        (in METRIC at least ...) since the simulation started.

        """
        return self._iget_restart_days( index )


    def get_filename(self):
        """
        Name of the file currently loaded.
        """
        fn = self._get_src_file()
        return str(fn) if fn else ''

    def fwrite( self , fortio ):
        """
        Will write current EclFile instance to fortio stream.

        ECLIPSE is written in Fortran; and a "special" handle for
        Fortran IO must be used when reading and writing these files.
        This method will write the current EclFile instance to a
        FortIO stream already opened for writing:

           import ecl.ecl.ecl as ecl
           ...
           fortio = ecl.FortIO( "FILE.XX" )
           file.fwrite( fortio )
           fortio.close()

        """
        self._fwrite(  fortio , 0 )

    def write_index(self, index_file_name):
        if not self._write_index(index_file_name):
            raise IOError("Failed to write index file:%s" % index_file_name)


class EclFileContextManager(object):

    def __init__(self , ecl_file):
        self.__ecl_file = ecl_file

    def __enter__(self):
        return self.__ecl_file

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__ecl_file.close()
        return False


def openEclFile( file_name , flags = 0):
    print('The function openEclFile is deprecated, use open_ecl_file.')
    return open_ecl_file(file_name, flags)

def open_ecl_file(file_name, flags=0):
    return EclFileContextManager(EclFile(file_name, flags))



monkey_the_camel(EclFile, 'getFileType', EclFile.get_filetype, staticmethod)
monkey_the_camel(EclFile, 'blockView', EclFile.block_view)
monkey_the_camel(EclFile, 'blockView2', EclFile.block_view2)
monkey_the_camel(EclFile, 'restartView', EclFile.restart_view)
monkey_the_camel(EclFile, 'getFilename', EclFile.get_filename)
