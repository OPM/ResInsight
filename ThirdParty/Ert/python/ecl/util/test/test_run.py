#  Copyright (C) 2013  Equinor ASA, Norway. 
#   
#  The file 'test_run.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, teither version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 
import random
import os.path
import subprocess
import argparse
from  .test_area import TestAreaContext


def path_exists( path ):
    if os.path.exists( path ):
        return (True , "Path:%s exists" % path)
    else:
        return (False , "ERROR: Path:%s does not exist" % path)


class TestRun(object):
    default_ert_cmd = "ert"
    default_ert_version = "stable"
    default_path_prefix = None

    def __init__(self , config_file , args = [] , name = None):
        if os.path.exists( config_file ) and os.path.isfile( config_file ):
            self.parseArgs(args)
            self.__ert_cmd = TestRun.default_ert_cmd
            self.path_prefix = TestRun.default_path_prefix
            self.config_file = config_file
            
            self.check_list = []
            self.workflows = []
            if name:
                self.name = name
            else:
                self.name = config_file.replace("/" , ".")
                while True:
                    if self.name[0] == ".":
                        self.name = self.name[1:]
                    else:
                        break
            self.name += "/%08d" % random.randint(0,100000000)
        else:
            raise IOError("No such config file: %s" % config_file)

    
    def parseArgs(self , args):
        parser = argparse.ArgumentParser()
        parser.add_argument("-v" , "--version" , default = self.default_ert_version)
        parser.add_argument("args" , nargs="*")
        result = parser.parse_args(args)
        self.ert_version = result.version
        self.args = result.args
        

    def get_config_file(self):
        return self.__config_file

    def set_config_file(self , input_config_file):
        self.__config_file = os.path.basename( input_config_file )
        self.abs_config_file = os.path.abspath( input_config_file )

    config_file = property( get_config_file , set_config_file )

    #-----------------------------------------------------------------

    def set_path_prefix(self , path_prefix):
        self.__path_prefix = path_prefix
    
    def get_path_prefix(self):
        return self.__path_prefix

    path_prefix = property( get_path_prefix , set_path_prefix )

    #-----------------------------------------------------------------
    
    def get_ert_cmd(self):
        return self.__ert_cmd

    def set_ert_cmd(self , cmd):
        self.__ert_cmd = cmd

    ert_cmd = property( get_ert_cmd , set_ert_cmd)

    #-----------------------------------------------------------------

    def get_workflows(self):
        return self.workflows

    
    def add_workflow(self , workflow):
        self.workflows.append( workflow )

    #-----------------------------------------------------------------

    def get_args(self):
        return self.args

    #-----------------------------------------------------------------

    def add_check( self , check_func , arg):
        if callable(check_func):
            self.check_list.append( (check_func , arg) )
        else:
            raise Exception("The checker:%s is not callable" % check_func )

    #-----------------------------------------------------------------

    def __run(self , work_area ):
        argList = [ self.ert_cmd , "-v" , self.ert_version ]
        for arg in self.args:
            argList.append( arg )

        argList.append( self.config_file )
        for wf in self.workflows:
            argList.append( wf )

        status = subprocess.call( argList )
        if status == 0:
            return (True , "ert has run successfully")
        else:
            return (False , "ERROR:: ert exited with status code:%s" % status)

    
    def run(self):
        if len(self.workflows):
            with TestAreaContext(self.name , prefix = self.path_prefix , store_area = False) as work_area:
                test_cwd = work_area.get_cwd()
                work_area.copy_parent_content( self.abs_config_file )
                status = self.__run( work_area )
                global_status = status[0]

                status_list = [ status ]
                if status[0]:
                    for (check_func , arg) in self.check_list:
                        status = check_func( arg )
                        status_list.append( status )
                        if not status[0]:
                            global_status = False

                if not global_status:
                    work_area.set_store( True )
                    
            return (global_status , test_cwd , status_list)
        else:
            raise Exception("Must have added workflows before invoking start()")
            
