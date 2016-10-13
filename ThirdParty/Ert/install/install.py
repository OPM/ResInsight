#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from __future__ import print_function
import pwd
import grp
import os
import os.path
import shutil
import re
import stat
import py_compile

DEFAULT_DIR_MODE = 0755
DEFAULT_EXE_MODE = 0755
DEFAULT_REG_MODE = 0644

os.umask( 0 )


def msg( verbose , text , arg):
    text_width = 20
    if verbose:
        pad_text = text + (text_width - len( text)) * "." + ": "
        print(pad_text + arg)


def update_mode( path , mode , user , group):
    try:
        os.chmod( path , mode )
    
        if user:   # Only applicable for root:
            user_info = pwd.getpwnam( user )
            uid = user_info[ 2 ]
            os.chown( path , uid , -1 )
        
        if group:
            group_info = grp.getgrnam( group )
            gid = group_info[ 2 ]
            os.chown( path , -1 , gid )
    except:
        pass # Probably some missing permissions


def include_by_ext( full_path , ext_list):
    if not os.path.exists( full_path ):
        return False

    mode = os.stat( full_path )[stat.ST_MODE]
    (base , ext) = os.path.splitext( full_path )
    if stat.S_ISDIR( mode ):
        if ext == ".svn":
            return False
        else:
            return True
    else:
        if ext in ext_list:
            return True
        else:
            return False



def include_python( full_path ):
    return include_by_ext( full_path , [".py"])


def include_html( full_path ):
    return include_by_ext( full_path , [".html"])

def include_image( full_path ):
    return include_by_ext( full_path , [".png" , ".jpg"])


class File:
    
    def __init__( self , src , create_path = None , target_name = None ):
        self.src = src
        self.create_path = create_path
        self.name = os.path.basename( self.src )
        if target_name:
            self.target_name = target_name
        else:
            self.target_name = self.name


    # Modes should be a tuple of three elements: (DIR_MODE , REG_MODE , EXE_MODE);
    # all modes assume umask( 0 )!
    def install( self , src_root , target_root , modes , verbose , user , group):
        if self.create_path:
            target_path = target_root
            for path in self.create_path.split("/"):
                target_path += "/%s" % path
                if not os.path.exists( target_path ):
                    os.makedirs( target_path , modes[0] )
                    msg(verbose , "Creating directory" , target_path)
                update_mode( target_path , modes[0] , user, group)
            target_file = "%s/%s/%s" % (target_root , self.create_path , self.target_name)
        else:
            target_file = "%s/%s" % (target_root ,  self.target_name)

        if os.path.isabs(self.src):
            src_file = self.src
        else:
            src_file = "%s/%s" % (src_root , self.src)

        msg( verbose , "Copying file" , "%s -> %s" % (src_file , target_file))
        shutil.copyfile( src_file , target_file )
        if os.access( src_file , os.X_OK):
            update_mode( target_file , modes[2] , user , group)
        else:
            update_mode( target_file , modes[1] , user , group )
        (target_base , ext) = os.path.splitext( target_file )
        if ext == ".py":
            msg( verbose , "Byte compiling" , target_file)
            try:
                py_compile.compile( target_file )
                pyc_file = target_base + ".pyc"
                update_mode( pyc_file , modes[1] , user , group)
            except:
                pass # Some permissions missing ...
                        
                


class Install:
    def __init__( self , src_root , target_root):
        self.src_root = src_root
        self.target_root = target_root
        self.file_list = []
        self.link_list = []
        

    def install(self, modes = (DEFAULT_DIR_MODE , DEFAULT_REG_MODE, DEFAULT_EXE_MODE) , user = None , group = None , verbose = False):
        for file in self.file_list:
            file.install( self.src_root , self.target_root , modes , verbose , user , group)
        
        for (src,link) in self.link_list:
            full_link = "%s/%s" % ( self.target_root , link )
            if os.path.exists( full_link ):
                os.remove( full_link )

            msg( verbose , "Linking" , "%s/%s -> %s/%s" % (self.target_root , link , self.target_root , src))
            os.symlink( "%s/%s" % (self.target_root , src), "%s/%s" % (self.target_root , link ))
                 

    def add_link( self , src , link , verbose = False):
        self.link_list.append( ( src , link ))
            
        


    def add_path( self , path , include_callback = None , create_path = None , recursive = False):
        path_list = []
        for entry in os.listdir( "%s/%s" % (self.src_root , path )):
            if entry == ".svn":
                continue

            full_path = "%s/%s/%s" % ( self.src_root , path , entry)

            include = True
            if include_callback:
                include = include_callback( full_path )

            if not include:
                continue
                
            mode = os.stat( full_path )[stat.ST_MODE]
            if stat.S_ISREG( mode ):
                file = File("%s/%s" % (path , entry), create_path = create_path)
                self.add_file( file )
            elif stat.S_ISDIR( mode ):
                if recursive:
                    path_list.append( entry )

        for dir in path_list:
            if create_path:
                new_create_path = "%s/%s" % (create_path , dir)
            else:
                new_create_path = dir
            self.add_path( "%s/%s" % (path , dir) , include_callback = include_callback , create_path = new_create_path , recursive = True )
                    

    def add_file( self , file ):
        self.file_list.append( file )
    


            
        
