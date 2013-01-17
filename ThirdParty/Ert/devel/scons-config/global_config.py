#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'global_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import SCons
import os.path
import os
import commands
import stat
import sys
import os.path

sys.path.append( os.path.dirname( __file__ ))

# Local configuration options are loaded from the file 'local_config.py'; this
# file is not part of the distribution, and you must create it yourself. If the
# file 'local_config.py' can not be found the script will use the
# 'local_config_DEFAULT.py' file instead. This might be enough to get things to
# compile, but the resulting executable will probably not be usable for much.
#
# The file 'local_config_DEFAULT.py' contains documentation on how to create
# your own personal configuration file 'local_config.py'.

try:
    from local_config import *
except ImportError:
    from local_config_DEFAULT import * 



def add_program(env , conf , bin_path , target , src , **kwlist):
    P = env.Program( target , src , **kwlist)
    env.Install(bin_path , P)
    conf.local_install[ bin_path ]   = True



def add_static_library( env, conf , lib_path , target , src , **kwlist):
    LIB = env.StaticLibrary( target , src , **kwlist)
    env.Install( lib_path , LIB )
    conf.local_install[ lib_path ] = True



def add_shared_library( env, conf , lib_path , target , src , **kwlist):
    LIB = env.SharedLibrary( target , src , **kwlist )
    env.Install( lib_path , LIB )
    conf.local_install[ lib_path ]   = True


def add_header( env, conf , include_path , header_list ):
    env.Install( include_path , header_list )
    conf.local_install[ include_path ]   = True


def get_target( env , conf):
    def_list = []
    for target in conf.local_install.keys():
        def_list.append( target )
    
    print def_list
    return env.Alias('local' , def_list)


#################################################################


# Used as hash keys
LIBUTIL      = 0
LIBECL       = 1
LIBRMS       = 2
LIBENKF      = 3
LIBPLOT      = 4  
LIBJOB_QUEUE = 5
LIBSCHED     = 6
LIBCONFIG    = 7
LIBCONF      = 8
LIBANALYSIS  = 9
LIBGEOMETRY  = 10
LIBWELL      = 11

class conf:
    def __init__(self , cwd , sub_level_depth):

        self.SVN_VERSION      = commands.getoutput("svnversion ./")
        self.TIME_STAMP       = commands.getoutput("date")
        
        # These are site-dependant; and should really be set OUTSIDE
        # the central build configuration.
        self.USE_OPENMP          = False
        self.SITE_CONFIG_FILE    = SITE_CONFIG_FILE
        self.INCLUDE_LSF         = INCLUDE_LSF
        self.LSF_INCLUDE_PATH    = LSF_INCLUDE_PATH 
        self.LSF_LIB_PATH        = LSF_LIB_PATH
        self.g2c                 = g2c
        self.PLPLOT_INCLUDE_PATH = PLPLOT_INCLUDE_PATH + "/plplot"
        self.PLPLOT_LIB_PATH     = PLPLOT_LIB_PATH
        if M64:
            self.CCFLAGS = "-m64 "
        else:
            self.CCFLAGS = ""
        self.CCFLAGS = ""
        self.CCFLAGS            += "-O2 -std=gnu99 -g -fno-leading-underscore -Wall -pipe -DWITH_LATEX -DHAVE_VA_COPY -DHAVE_ISREG -DHAVE_PID_T -DHAVE_ROUND -DHAVE_GETOPT -DHAVE_USLEEP -DHAVE_OPENDIR -DWITH_LAPACK -DHAVE_REGEXP -DHAVE_FTRUNCATE -DHAVE_READLINKAT -DHAVE_PROC -DINTERNAL_LINK -DHAVE_FSYNC -DHAVE_GLOB -DHAVE_FNMATCH -DMKDIR_POSIX -DHAVE_LOCALTIME_R -DHAVE_LOCKF -DHAVE_ISFINITE -DPOSIX_SETENV -DHAVE_GETUID -DHAVE_SYMLINK -DHAVE_REALPATH -DHAVE_EXECINFO -DHAVE_FORK -DWITH_ZLIB -DWITH_PTHREAD"
        if self.USE_OPENMP:
            self.CCFLAGS +=" -fopenmp -DHAVE_OPENMP"
        self.ARFLAGS             = "csr"
        

        tmp = cwd.split("/")
        n   = len(tmp) - sub_level_depth
        self.BUILD_ROOT = ""
        for path in tmp[1:n]:
            self.BUILD_ROOT += "/%s" % path
            
        self.LIB = {}
        self.LIB[LIBUTIL]       = {"home": "%s/libutil"      % self.BUILD_ROOT , "name": "ert_util"}
        self.LIB[LIBECL]        = {"home": "%s/libecl"       % self.BUILD_ROOT , "name": "ecl"}
        self.LIB[LIBANALYSIS]   = {"home": "%s/libanalysis"  % self.BUILD_ROOT , "name": "analysis"}
        self.LIB[LIBRMS]        = {"home": "%s/librms"       % self.BUILD_ROOT , "name": "rms"}
        self.LIB[LIBCONF]       = {"home": "%s/libconf"      % self.BUILD_ROOT , "name": "conf"}
        self.LIB[LIBPLOT]       = {"home": "%s/libplot"      % self.BUILD_ROOT , "name": "plot"}
        self.LIB[LIBENKF]       = {"home": "%s/libenkf"      % self.BUILD_ROOT , "name": "enkf"}
        self.LIB[LIBJOB_QUEUE]  = {"home": "%s/libjob_queue" % self.BUILD_ROOT , "name": "job_queue"}
        self.LIB[LIBSCHED]      = {"home": "%s/libsched"     % self.BUILD_ROOT , "name": "sched"}
        self.LIB[LIBCONFIG]     = {"home": "%s/libconfig"    % self.BUILD_ROOT , "name": "config"}
        self.LIB[LIBGEOMETRY]   = {"home": "%s/libgeometry"  % self.BUILD_ROOT , "name": "geometry"}
        self.LIB[LIBWELL]       = {"home": "%s/libwell"      % self.BUILD_ROOT , "name": "well"}
        self.RPATH = self.PLPLOT_LIB_PATH



    def update_env( self , env , liblist , ext_liblist = None , ext_headerlist = None , link = False):
        self.local_install = {}
        CPPPATH = ["./"]
        LIBPATH = []
        LIBS    = []
        for lib_key in liblist:
            lib  = self.LIB[lib_key]
            home = lib["home"]
            CPPPATH.append("%s/include" % home )
            LIBPATH.append("%s/lib"     % home )
            
            if lib.has_key("name"):
                name = lib["name"]
                LIBS.append( name )
        env.Replace( CC = "gcc" )

        if ext_headerlist:
            for path in ext_headerlist:
                CPPPATH.append( path )

        if ext_liblist:
            LIBS += ext_liblist
        env.Replace( CPPPATH  = CPPPATH ,
                     CCFLAGS  = self.CCFLAGS )

        if self.USE_OPENMP:
            LIBS += ["gomp"]
        
        if link:
            env.Replace(LIBPATH  = LIBPATH ,
                        LIBS     = LIBS    , 
                        RPATH    = self.RPATH)

                
        
def get_conf(cwd ,  sub_level_depth):
    return conf( cwd , sub_level_depth )
        


