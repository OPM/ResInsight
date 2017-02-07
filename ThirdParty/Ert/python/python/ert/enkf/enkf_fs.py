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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf import TimeMap, StateMap, SummaryKeySet, CustomKWConfigSet
from ert.enkf.enums import EnKFFSType


class EnkfFs(BaseCClass):
    TYPE_NAME = "enkf_fs"

    _mount                = EnkfPrototype("void* enkf_fs_mount(char* )", bind = False)
    _exists               = EnkfPrototype("bool  enkf_fs_exists(char*)", bind = False)
    _disk_version         = EnkfPrototype("int   enkf_fs_disk_version(char*)", bind = False)
    _update_disk_version  = EnkfPrototype("bool  enkf_fs_update_disk_version(char*, int, int)", bind = False)
    _decref               = EnkfPrototype("int   enkf_fs_decref(enkf_fs)")
    _get_refcount         = EnkfPrototype("int   enkf_fs_get_refcount(enkf_fs)")
    _has_node             = EnkfPrototype("bool  enkf_fs_has_node(enkf_fs,     char*,  int,   int, int, int)")
    _has_vector           = EnkfPrototype("bool  enkf_fs_has_vector(enkf_fs,   char*,  int,   int, int)")
    _fread_node           = EnkfPrototype("void  enkf_fs_fread_node(enkf_fs,   buffer, char*, int, int, int, int)")
    _fread_vector         = EnkfPrototype("void  enkf_fs_fread_vector(enkf_fs, buffer, char*, int, int, int)")
    _get_case_name        = EnkfPrototype("char* enkf_fs_get_case_name(enkf_fs)")
    _is_read_only         = EnkfPrototype("bool  enkf_fs_is_read_only(enkf_fs)")
    _get_writecount       = EnkfPrototype("int   enkf_fs_get_write_count(enkf_fs)")
    _fsync                = EnkfPrototype("void  enkf_fs_fsync(enkf_fs)")
    _create               = EnkfPrototype("enkf_fs_ref   enkf_fs_create_fs(char* , enkf_fs_type_enum , void* , bool)", bind = False)
    _get_time_map         = EnkfPrototype("time_map_ref  enkf_fs_get_time_map(enkf_fs)")
    _get_state_map        = EnkfPrototype("state_map_ref enkf_fs_get_state_map(enkf_fs)")
    _summary_key_set      = EnkfPrototype("summary_key_set_ref enkf_fs_get_summary_key_set(enkf_fs)")
    _config_kw_config_set = EnkfPrototype("custom_kw_config_set_ref enkf_fs_get_custom_kw_config_set(enkf_fs)")

    def __init__(self, mount_point):
        c_ptr = self._mount(mount_point)
        super(EnkfFs, self).__init__(c_ptr)

        self.__umounted = False # Keep track of umounting so we only do it once


    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        obj = super(EnkfFs, cls).createCReference(c_pointer, parent)
        if not obj is None:
            obj.__umounted = False
        return obj


    # def has_node(self, node_key, var_type, report_step, iens, state):
    #     return self._has_node(node_key, var_type, report_step, iens, state)
    #
    # def has_vector(self, node_key, var_type, iens, state):
    #     return self._has_vector(node_key, var_type, iens, state)
    #
    #
    # def fread_node(self, key, type, step, member, value):
    #     buffer = Buffer(100)
    #     self._fread_node(buffer, key, type, step, member, value)
    #
    # def fread_vector(self, key, type, member, value):
    #     buffer = Buffer(100)
    #     self._fread_vector(buffer, key, type, member, value)

    def getTimeMap(self):
        """ @rtype: TimeMap """
        self.__checkIfUmounted()
        return self._get_time_map().setParent(self)

    def getStateMap(self):
        """ @rtype: StateMap """
        self.__checkIfUmounted()
        return self._get_state_map().setParent(self)

    def getCaseName(self):
        """ @rtype: str """
        self.__checkIfUmounted()
        return self._get_case_name()

    def isReadOnly(self):
        """ @rtype: bool """
        self.__checkIfUmounted()
        return self._is_read_only()

    def refCount(self):
        self.__checkIfUmounted()
        return self._get_refcount()

    def writeCount(self):
        return self._get_writecount()

    @classmethod
    def exists(cls, path):
        return cls._exists(path)

    @classmethod
    def diskVersion(cls, path):
        disk_version = cls._disk_version(path)
        if disk_version < 0:
            raise IOError("No such filesystem: %s" % path)
        return disk_version


    @classmethod
    def updateVersion(cls, path, src_version , target_version):
        return cls._update_disk_version(path , src_version  ,target_version)

    
    @classmethod
    def createFileSystem(cls, path, fs_type, arg=None , mount = False):
        assert isinstance(path, str)
        assert isinstance(fs_type, EnKFFSType)
        fs = cls._create(path, fs_type, arg, mount)
        return fs


    def __checkIfUmounted(self):
        if self.__umounted:
            raise AssertionError("The EnkfFs instance has been umounted!")

    def umount(self):
        if not self.__umounted:
            self._decref()
            self.__umounted = True

    def free(self):
        self.umount()

    def __repr__(self):
        cn = self.getCaseName()
        wc = self.writeCount()
        ad = self._ad_str()
        return 'EnkfFs(case_name = %s, write_count = %d) %s' % (cn, wc, ad)

    def fsync(self):
        self._fsync()

    def getSummaryKeySet(self):
        """ @rtype: SummaryKeySet """
        return self._summary_key_set().setParent(self)

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
        return self._config_kw_config_set()
