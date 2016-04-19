import os.path
from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB, EnkfFs, EnkfStateType, StateMap, TimeMap, RealizationStateEnum
from ert.util import StringList

import re

def naturalSortKey(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower() for text in re.split(_nsre, s)]

class FileSystemRotator(object):
    def __init__(self, capacity):
        super(FileSystemRotator, self).__init__()
        self.__capacity = capacity
        """:type: int"""
        self.__fs_list = []
        """:type: list of str"""
        self.__fs_map = {}
        """:type: dict[str, EnkfFs]"""

    def __len__(self):
        return len(self.__fs_list)

    def addFileSystem(self, file_system, full_name):
        if self.atCapacity():
            self.dropOldestFileSystem()

        self.__fs_list.append(full_name)
        self.__fs_map[full_name] = file_system

    def dropOldestFileSystem(self):
        if len(self.__fs_list) > 0:
            case_name = self.__fs_list[0]
            fs = self.__fs_map[case_name]
            fs.umount()
            del self.__fs_list[0]
            del self.__fs_map[case_name]

            print("Dropped filesystem: %s" % case_name)

    def atCapacity(self):
        return len(self.__fs_list) == self.__capacity

    def __contains__(self, full_case_name):
        return full_case_name in self.__fs_list

    def __getitem__(self, case):
        """ @rtype: EnkfFs """
        if isinstance(case, str):
            return self.__fs_map[case]
        elif isinstance(case, int) and 0 <= case < len(self):
            case_name = self.__fs_list[case]
            return self.__fs_map[case_name]
        else:
            raise IndexError("Value '%s' is not a proper index or case name." % case)


    def umountAll(self):
        while len(self.__fs_list) > 0:
            self.dropOldestFileSystem()



# For normal use from ert all filesystems will be located in the same
# folder in the filesystem - corresponding to the ENSPATH setting in
# the config file; in this implementation that setting is stored in
# the @mount_root field. Currently @mount_root is fixed to the value
# returned by EnKFMain.getMountPoint(), but in principle a different
# path could be sent as the the optional second argument to the
# getFS() method. 

class EnkfFsManager(BaseCClass):
    DEFAULT_CAPACITY = 5

    def __init__(self, enkf_main, capacity=DEFAULT_CAPACITY):
        assert isinstance(enkf_main, BaseCClass)
        super(EnkfFsManager, self).__init__(enkf_main.from_param(enkf_main).value, parent=enkf_main, is_reference=True)

        self.__fs_rotator = FileSystemRotator(capacity)
        self.__mount_root = enkf_main.getMountPoint()

        self.__fs_type = enkf_main.getModelConfig().getFSType()
        self.__fs_arg = None

        self.getCurrentFileSystem()

    def __createFullCaseName(self, mount_root, case_name):
        return os.path.join(mount_root, case_name)


    def getFileSystem(self, case_name, mount_root=None):
        """
        @rtype: EnkfFs
        """
        if mount_root is None:
            mount_root = self.__mount_root

        full_case_name = self.__createFullCaseName(mount_root, case_name)

        if not full_case_name in self.__fs_rotator:
            if not EnkfFs.exists(full_case_name):
                if self.__fs_rotator.atCapacity():
                    self.__fs_rotator.dropOldestFileSystem()

                EnkfFs.createFileSystem(full_case_name, self.__fs_type, self.__fs_arg)

            new_fs = EnkfFs(full_case_name)
            self.__fs_rotator.addFileSystem(new_fs, full_case_name)

        fs = self.__fs_rotator[full_case_name]

        return fs


    def isCaseRunning(self, case_name, mount_root=None):
        """ Returns true if case is mounted and write_count > 0
        @rtype: bool
        """
        if self.isCaseMounted(case_name, mount_root):
            case_fs = self.getFileSystem(case_name, mount_root)
            return case_fs.writeCount() > 0
        return False


    def caseExists(self, case_name):
        """ @rtype: bool """
        return case_name in self.getCaseList()


    def caseHasData(self, case_name):
        """ @rtype: bool """
        case_has_data = False
        state_map = self.getStateMapForCase(case_name)

        for state in state_map:
            if state == RealizationStateEnum.STATE_HAS_DATA:
                case_has_data = True

        return case_has_data


    def getCurrentFileSystem(self):
        """ Returns the currently selected file system
        @rtype: EnkfFs
        """
        current_fs = EnkfFsManager.cNamespace().get_current_fs(self)
        case_name = current_fs.getCaseName()
        full_name = self.__createFullCaseName(self.__mount_root, case_name)
        
        if not full_name in self.__fs_rotator:
            self.__fs_rotator.addFileSystem(current_fs, full_name)
        else:
            current_fs.umount()

        return self.getFileSystem(case_name, self.__mount_root)


    def umount(self):
        self.__fs_rotator.umountAll()


    def getFileSystemCount(self):
        return len(self.__fs_rotator)


    def switchFileSystem(self, file_system):
        assert isinstance(file_system, EnkfFs)
        EnkfFsManager.cNamespace().switch_fs(self, file_system, None)


    def isCaseInitialized(self, case):
        return EnkfFsManager.cNamespace().is_case_initialized(self, case, None)

    def isInitialized(self):
        """ @rtype: bool """
        return EnkfFsManager.cNamespace().is_initialized(self, None) # what is the bool_vector mask???


    def getCaseList(self):
        """ @rtype: list[str] """
        caselist = [case for case in EnkfFsManager.cNamespace().alloc_caselist(self)]
        return sorted(caselist, key=naturalSortKey)


    def customInitializeCurrentFromExistingCase(self, source_case, source_report_step, source_state, member_mask,
                                                node_list):
        assert isinstance(source_state, EnkfStateType)
        source_case_fs = self.getFileSystem(source_case)
        EnkfFsManager.cNamespace().custom_initialize_from_existing(self, source_case_fs, source_report_step,
                                                                   source_state, node_list, member_mask)

    def initializeCurrentCaseFromExisting(self, source_fs, source_report_step, source_state):
        assert isinstance(source_state, EnkfStateType)
        assert isinstance(source_fs, EnkfFs);
        EnkfFsManager.cNamespace().initialize_current_case_from_existing(self, source_fs, source_report_step,
                                                                         source_state)

    def initializeCaseFromExisting(self, source_fs, source_report_step, source_state, target_fs):
        assert isinstance(source_state, EnkfStateType)
        assert isinstance(source_fs, EnkfFs);
        assert isinstance(target_fs, EnkfFs);
        EnkfFsManager.cNamespace().initialize_case_from_existing(self, source_fs, source_report_step, source_state,
                                                                 target_fs)


    def initializeCaseFromScratch(self, case , parameter_list, from_iens, to_iens, force_init=True):
        EnkfFsManager.cNamespace().initialize_from_scratch(self, case , parameter_list, from_iens, to_iens, force_init)

        
    def initializeFromScratch(self, parameter_list, from_iens, to_iens, force_init=True):
        case = self.getCurrentFileSystem( )
        self.initializeCaseFromScratch( case , parameter_list , from_iens , to_iens , force_init )



    def isCaseMounted(self, case_name, mount_root=None):
        if mount_root is None:
            mount_root = self.__mount_root

        full_case_name = self.__createFullCaseName(mount_root, case_name)

        return full_case_name in self.__fs_rotator


    def getStateMapForCase(self, case):
        """ @rtype: StateMap """
        assert isinstance(case, str)

        if self.isCaseMounted(case):
            fs = self.getFileSystem(case)
            return fs.getStateMap()
        else:
            return EnkfFsManager.cNamespace().alloc_readonly_state_map(self, case)

    def getTimeMapForCase(self, case):
        """ @rtype: TimeMap """
        assert isinstance(case, str)
        return EnkfFsManager.cNamespace().alloc_readonly_time_map(self, case)

    def isCaseHidden(self, case_name):
        return case_name.startswith(".")


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_fs_manager", EnkfFsManager)

EnkfFsManager.cNamespace().get_current_fs = cwrapper.prototype("enkf_fs_ref enkf_main_get_fs_ref(enkf_fs_manager)")
EnkfFsManager.cNamespace().switch_fs = cwrapper.prototype("void enkf_main_set_fs(enkf_fs_manager, enkf_fs, char*)")
EnkfFsManager.cNamespace().fs_exists = cwrapper.prototype("bool enkf_main_fs_exists(enkf_fs_manager, char*)")
EnkfFsManager.cNamespace().alloc_caselist = cwrapper.prototype("stringlist_obj enkf_main_alloc_caselist(enkf_fs_manager)")
EnkfFsManager.cNamespace().set_case_table = cwrapper.prototype("void enkf_main_set_case_table(enkf_fs_manager, char*)")

EnkfFsManager.cNamespace().initialize_from_scratch = cwrapper.prototype("void enkf_main_initialize_from_scratch(enkf_fs_manager, enkf_fs , stringlist, int, int, bool)")
EnkfFsManager.cNamespace().is_initialized = cwrapper.prototype("bool enkf_main_is_initialized(enkf_fs_manager, bool_vector)")
EnkfFsManager.cNamespace().is_case_initialized = cwrapper.prototype("bool enkf_main_case_is_initialized(enkf_fs_manager, char*, bool_vector)")
EnkfFsManager.cNamespace().initialize_current_case_from_existing = cwrapper.prototype("void enkf_main_init_current_case_from_existing(enkf_fs_manager, enkf_fs, int, enkf_state_type_enum)")
EnkfFsManager.cNamespace().initialize_case_from_existing = cwrapper.prototype("void enkf_main_init_case_from_existing(enkf_fs_manager, enkf_fs, int, enkf_state_type_enum, enkf_fs)")
EnkfFsManager.cNamespace().custom_initialize_from_existing = cwrapper.prototype("void enkf_main_init_current_case_from_existing_custom(enkf_fs_manager, enkf_fs, int, enkf_state_type_enum, stringlist, bool_vector)")

EnkfFsManager.cNamespace().alloc_readonly_state_map = cwrapper.prototype("state_map_obj enkf_main_alloc_readonly_state_map(enkf_fs_manager, char*)")
EnkfFsManager.cNamespace().alloc_readonly_time_map = cwrapper.prototype("time_map_obj enkf_main_alloc_readonly_time_map(enkf_fs_manager, char*)")

