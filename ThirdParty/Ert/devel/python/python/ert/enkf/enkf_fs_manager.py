from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB, EnkfFs, EnkfStateType, StateMap
from ert.util import StringList


class EnkfFsManager(BaseCClass):

    def __init__(self, enkf_main):
        assert isinstance(enkf_main, BaseCClass)
        super(EnkfFsManager, self).__init__(enkf_main.from_param(enkf_main).value, parent=enkf_main, is_reference=True)
        self.__cached_file_systems = {}
        """ @type: dict of (str, EnkfFs) """


    def isCaseInitialized(self, case):
        return EnkfFsManager.cNamespace().is_case_initialized(self, case, None)

    def selectFileSystem(self, path):
        EnkfFsManager.cNamespace().select_fs(self, path)

    # def fs_exists(self, case):
    #     """ @rtype: bool """
    #     return EnkfFsManager.cNamespace().fs_exists(self, case)
    #
    # def get_alt_fs(self, fs, read_only, create):
    #     """ @rtype: EnkfFs """
    #     return EnkfFsManager.cNamespace().get_alt_fs(self, fs, read_only, create).setParent(self)

    def isInitialized(self):
        """ @rtype: bool """
        return EnkfFsManager.cNamespace().is_initialized(self, None) # what is the bool_vector mask???

    def getFileSystem(self):
        """ @rtype: EnkfFs """
        return EnkfFsManager.cNamespace().get_fs(self).setParent(self)

    def getCaseList(self):
        """ @rtype: StringList """
        return EnkfFsManager.cNamespace().alloc_caselist(self)

    # def getCurrentFileSystem(self):
    #     """ @rtype: str """
    #     return EnkfFsManager.cNamespace().get_current_fs(self)

    def userSelectFileSystem(self, input_case):
        EnkfFsManager.cNamespace().user_select_fs(self, input_case)


    def initializeFromExistingCase(self, source_case, source_report_step, source_state, member_mask, ranking_key, node_list):
        assert isinstance(source_state, EnkfStateType)
        EnkfFsManager.cNamespace().initialize_from_existing(self, source_case, source_report_step, source_state, member_mask, ranking_key, node_list)


    def initializeFromScratch(self, parameter_list, iens1, iens2, force_init=True):
        EnkfFsManager.cNamespace().initialize_from_scratch(self, parameter_list, iens1, iens2, force_init)

    # def set_case_table(self, case_table_file):
    #     EnkfFsManager.cNamespace().set_case_table(self, case_table_file)

    def mountAlternativeFileSystem(self, case, read_only, create):
        """ @rtype: EnkfFs """
        assert isinstance(case, str)
        assert isinstance(read_only, bool)
        assert isinstance(create, bool)

        if case in self.__cached_file_systems and not read_only:
            fs = self.__cached_file_systems[case]

            if fs.isReadOnly():
                print("Removed a read only file system from cache: %s" % case)
                del self.__cached_file_systems[case]

        if not case in self.__cached_file_systems:
            print("Added a file system to cache: %s" % case)
            self.__cached_file_systems[case] = EnkfFsManager.cNamespace().mount_alt_fs(self, case, read_only, create)
        else:
            print("Provided a file system from cache: %s" % case)

        return self.__cached_file_systems[case]


    def switchFileSystem(self, files_system):
        assert isinstance(files_system, EnkfFs)
        EnkfFsManager.cNamespace().switch_fs(self, files_system, None)


    def getStateMapForCase(self, case):
        """ @rtype: StateMap """
        assert isinstance(case, str)
        return EnkfFsManager.cNamespace().alloc_readonly_state_map(self, case)




cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_fs_manager", EnkfFsManager)

EnkfFsManager.cNamespace().get_fs = cwrapper.prototype("enkf_fs_ref enkf_main_get_fs(enkf_fs_manager)")
EnkfFsManager.cNamespace().mount_alt_fs = cwrapper.prototype("enkf_fs_ref enkf_main_mount_alt_fs(enkf_fs_manager, char*, bool, bool)")
EnkfFsManager.cNamespace().switch_fs = cwrapper.prototype("void enkf_main_set_fs(enkf_fs_manager, enkf_fs, char*)")
EnkfFsManager.cNamespace().user_select_fs = cwrapper.prototype("void enkf_main_user_select_fs(enkf_fs_manager, char*)")
# EnkfFsManager.cNamespace().get_current_fs = cwrapper.prototype("char* enkf_main_get_current_fs(enkf_fs_manager)")
EnkfFsManager.cNamespace().select_fs = cwrapper.prototype("void enkf_main_select_fs(enkf_fs_manager, char*)")
EnkfFsManager.cNamespace().fs_exists = cwrapper.prototype("bool enkf_main_fs_exists(enkf_fs_manager, char*)")
EnkfFsManager.cNamespace().alloc_caselist = cwrapper.prototype("stringlist_obj enkf_main_alloc_caselist(enkf_fs_manager)")
EnkfFsManager.cNamespace().set_case_table = cwrapper.prototype("void enkf_main_set_case_table(enkf_fs_manager, char*)")

EnkfFsManager.cNamespace().initialize_from_scratch = cwrapper.prototype("void enkf_main_initialize_from_scratch(enkf_fs_manager, stringlist, int, int, bool)")
EnkfFsManager.cNamespace().is_initialized = cwrapper.prototype("bool enkf_main_is_initialized(enkf_fs_manager, bool_vector)")
EnkfFsManager.cNamespace().is_case_initialized = cwrapper.prototype("bool enkf_main_case_is_initialized(enkf_fs_manager, char*, bool_vector)")
EnkfFsManager.cNamespace().initialize_from_existing = cwrapper.prototype("void enkf_main_initialize_from_existing__(enkf_fs_manager, char*, int, enkf_state_type_enum, bool_vector, char*, stringlist)")

EnkfFsManager.cNamespace().alloc_readonly_state_map = cwrapper.prototype("state_map_obj enkf_main_alloc_readonly_state_map(enkf_fs_manager, char*)")

