#  Copyright (C) 2013  Equinor ASA, Norway.
#
#  The file 'test_work_area.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os.path

from cwrap import BaseCClass
from ert.enkf import EnKFMain, EnkfPrototype

class ErtTest(BaseCClass):
    TYPE_NAME = "ert_test"

    _alloc         = EnkfPrototype("void* ert_test_context_alloc_python( char* , char*)", bind = False)
    _set_store     = EnkfPrototype("void* ert_test_context_set_store( ert_test , bool)")
    _free          = EnkfPrototype("void  ert_test_context_free( ert_test )")
    _get_cwd       = EnkfPrototype("char* ert_test_context_get_cwd( ert_test )")
    _get_enkf_main = EnkfPrototype("enkf_main_ref ert_test_context_get_main( ert_test )")



    def __init__(self, test_name, model_config, store_area=False):
        if not os.path.exists(model_config):
            raise IOError("The configuration file: %s does not exist" % model_config)
        else:
            c_ptr = self._alloc(test_name, model_config)
            super(ErtTest, self).__init__(c_ptr)
            self.setStore(store_area)

        self.__ert = None

    def setStore(self, store):
        self._set_store(store)

    def getErt(self):
        """ @rtype: EnKFMain """
        if self.__ert is None:
            self.__ert = self._get_enkf_main()

        return self.__ert

    def free(self):
        ert = self.getErt()
        ert.umount()
        self._free()

    def installWorkflowJob(self, job_name, job_path):
        """ @rtype: bool """
        if os.path.exists(job_path) and os.path.isfile(job_path):
            ert = self.getErt()
            workflow_list = ert.getWorkflowList()

            workflow_list.addJob(job_name, job_path)
            return workflow_list.hasJob(job_name)
        else:
            return False

    def runWorkflowJob(self, job_name, *arguments):
        """ @rtype: bool """
        ert = self.getErt()
        workflow_list = ert.getWorkflowList()

        if workflow_list.hasJob(job_name):
            job = workflow_list.getJob(job_name)
            job.run(ert, [arg for arg in arguments])
            return True
        else:
            return False


    def getCwd(self):
        """
        Returns the current working directory of this context.
        @rtype: string
        """
        return self._get_cwd()



class ErtTestContext(object):
    def __init__(self, test_name, model_config, store_area=False):
        self.__test_name = test_name
        self.__model_config = model_config
        self.__store_area = store_area
        self.__test_context = ErtTest(self.__test_name, self.__model_config, store_area=self.__store_area)


    def __enter__(self):
        """ @rtype: ErtTest """
        return self.__test_context


    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.__test_context
        return False


    def getErt(self):
        return self.__test_context.getErt()


    def getCwd(self):
        """
        Returns the current working directory of this context.
        @rtype: string
        """
        return self.__test_context.getCwd()
