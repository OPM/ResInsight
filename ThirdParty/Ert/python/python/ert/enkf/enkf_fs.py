#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_fs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, TimeMap, StateMap, SummaryKeySet, CustomKWConfigSet
from ert.enkf.enums import EnKFFSType


class EnkfFs(BaseCClass):
    def __init__(self, mount_point):
        c_ptr = EnkfFs.cNamespace().mount(mount_point)
        super(EnkfFs, self).__init__(c_ptr)

        self.__umounted = False # Keep track of umounting so we only do it once


    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        obj = super(EnkfFs, cls).createCReference(c_pointer, parent)
        if not obj is None:
            obj.__umounted = False
        return obj


    # def has_node(self, node_key, var_type, report_step, iens, state):
    #     return EnkfFs.cNamespace().has_node(self, node_key, var_type, report_step, iens, state)
    #
    # def has_vector(self, node_key, var_type, iens, state):
    #     return EnkfFs.cNamespace().has_vector(self, node_key, var_type, iens, state)
    #
    #
    # def fread_node(self, key, type, step, member, value):
    #     buffer = Buffer(100)
    #     EnkfFs.cNamespace().fread_node(self, buffer, key, type, step, member, value)
    #
    # def fread_vector(self, key, type, member, value):
    #     buffer = Buffer(100)
    #     EnkfFs.cNamespace().fread_vector(self, buffer, key, type, member, value)

    def getTimeMap(self):
        """ @rtype: TimeMap """
        self.__checkIfUmounted()
        return EnkfFs.cNamespace().get_time_map(self).setParent(self)

    def getStateMap(self):
        """ @rtype: StateMap """
        self.__checkIfUmounted()
        return EnkfFs.cNamespace().get_state_map(self).setParent(self)

    def getCaseName(self):
        """ @rtype: str """
        self.__checkIfUmounted()
        return EnkfFs.cNamespace().get_case_name(self)

    def isReadOnly(self):
        """ @rtype: bool """
        self.__checkIfUmounted()
        return EnkfFs.cNamespace().is_read_only(self)

    def refCount(self):
        self.__checkIfUmounted()
        return self.cNamespace().get_refcount(self)

    def writeCount(self):
        return self.cNamespace().get_writecount(self)

    @classmethod
    def exists(cls, path):
        return cls.cNamespace().exists(path)

    @classmethod
    def diskVersion(cls, path):
        disk_version = cls.cNamespace().disk_version(path)
        if disk_version < 0:
            raise IOError("No such filesystem: %s" % path)
        return disk_version


    @classmethod
    def updateVersion(cls, path, src_version , target_version):
        return cls.cNamespace().update_disk_version(path , src_version  ,target_version)

    
    @classmethod
    def createFileSystem(cls, path, fs_type, arg=None , mount = False):
        assert isinstance(path, str)
        assert isinstance(fs_type, EnKFFSType)
        fs = cls.cNamespace().create(path, fs_type, arg, mount)
        return fs


    def __checkIfUmounted(self):
        if self.__umounted:
            raise AssertionError("The EnkfFs instance has been umounted!")

    def umount(self):
        if not self.__umounted:
            EnkfFs.cNamespace().decref(self)
            self.__umounted = True

    def free(self):
        self.umount()


    def fsync(self):
        EnkfFs.cNamespace().fsync(self)

    def getSummaryKeySet(self):
        """ @rtype: SummaryKeySet """
        return EnkfFs.cNamespace().summary_key_set(self).setParent(self)

    def realizationList(self, state):
        """
        Will return list of realizations with state == the specified state.
        @type state: ert.enkf.enums.RealizationStateEnum
        @rtype: ert.util.IntVector
        """
        state_map = self.getStateMap()
        return state_map.realizationList(state)

    def getCustomKWConfigSet(self):
        """ @rtype: CustomKWConfigSet """
        return EnkfFs.cNamespace().config_kw_config_set(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("enkf_fs", EnkfFs)

EnkfFs.cNamespace().mount = cwrapper.prototype("c_void_p enkf_fs_mount(char* )")
EnkfFs.cNamespace().create = cwrapper.prototype("enkf_fs_ref enkf_fs_create_fs(char* , enkf_fs_type_enum , c_void_p , bool)")
EnkfFs.cNamespace().exists = cwrapper.prototype("bool enkf_fs_exists(char*)")
EnkfFs.cNamespace().disk_version = cwrapper.prototype("int enkf_fs_disk_version(char*)")
EnkfFs.cNamespace().update_disk_version = cwrapper.prototype("bool enkf_fs_update_disk_version(char*, int, int)")
EnkfFs.cNamespace().decref = cwrapper.prototype("int enkf_fs_decref(enkf_fs)")
EnkfFs.cNamespace().get_refcount = cwrapper.prototype("int enkf_fs_get_refcount(enkf_fs)")
EnkfFs.cNamespace().has_node = cwrapper.prototype("bool enkf_fs_has_node(enkf_fs, char*, c_uint, int, int, c_uint)")
EnkfFs.cNamespace().has_vector = cwrapper.prototype("bool enkf_fs_has_vector(enkf_fs, char*, c_uint, int, c_uint)")
EnkfFs.cNamespace().fread_node = cwrapper.prototype("void enkf_fs_fread_node(enkf_fs, buffer, char*, c_uint, int, int, c_uint)")
EnkfFs.cNamespace().fread_vector = cwrapper.prototype("void enkf_fs_fread_vector(enkf_fs, buffer, char*, c_uint, int, c_uint)")
EnkfFs.cNamespace().get_time_map = cwrapper.prototype("time_map_ref enkf_fs_get_time_map(enkf_fs)")
EnkfFs.cNamespace().get_state_map = cwrapper.prototype("state_map_ref enkf_fs_get_state_map(enkf_fs)")
EnkfFs.cNamespace().get_case_name = cwrapper.prototype("char* enkf_fs_get_case_name(enkf_fs)")
EnkfFs.cNamespace().is_read_only = cwrapper.prototype("bool enkf_fs_is_read_only(enkf_fs)")
EnkfFs.cNamespace().get_writecount = cwrapper.prototype("int enkf_fs_get_write_count(enkf_fs)")
EnkfFs.cNamespace().fsync = cwrapper.prototype("void enkf_fs_fsync(enkf_fs)")
EnkfFs.cNamespace().summary_key_set = cwrapper.prototype("summary_key_set_ref enkf_fs_get_summary_key_set(enkf_fs)")
EnkfFs.cNamespace().config_kw_config_set = cwrapper.prototype("custom_kw_config_set_ref enkf_fs_get_custom_kw_config_set(enkf_fs)")
