import ctypes
from ert.cwrap import clib, CWrapper
from ert.test import ExtendedTestCase, TestAreaContext
from .workflow_common import WorkflowCommon

test_lib  = clib.ert_load("libjob_queue") # create a local namespace
cwrapper =  CWrapper(test_lib)

alloc_config = cwrapper.prototype("c_void_p workflow_job_alloc_config()")
alloc_from_file = cwrapper.prototype("workflow_job_obj workflow_job_config_alloc(char*, c_void_p, char*)")

class FunctionErtScriptTest(ExtendedTestCase):

    def test_compare(self):
        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createInternalFunctionJob()

            config = alloc_config()
            workflow_job = alloc_from_file("COMPARE", config, "compare_job")

            result = workflow_job.run(None, ["String", "string"])
            self.assertNotEqual(result, 0)

            result = workflow_job.run(None, ["String", "String"])
            # result is returned as c_void_p -> automatic conversion to None if value is 0
            self.assertIsNone(result)

