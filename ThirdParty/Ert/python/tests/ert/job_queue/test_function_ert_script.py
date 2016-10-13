import ctypes
from cwrap import clib, CWrapper
from ert.test import ExtendedTestCase, TestAreaContext
from ert.job_queue import WorkflowJob
from .workflow_common import WorkflowCommon

class FunctionErtScriptTest(ExtendedTestCase):

    def test_compare(self):
        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createInternalFunctionJob()

            parser = WorkflowJob.configParser( )
            with self.assertRaises(IOError):
                workflow_job = WorkflowJob.fromFile("no/such/file")

            workflow_job = WorkflowJob.fromFile("compare_job", name = "COMPARE", parser = parser)
            self.assertEqual( workflow_job.name() , "COMPARE")
            
            result = workflow_job.run(None , ["String", "string"])
            self.assertNotEqual(result, 0)

            result = workflow_job.run(None, ["String", "String"])
            # result is returned as c_void_p -> automatic conversion to None if value is 0
            self.assertIsNone(result)

            workflow_job = WorkflowJob.fromFile("compare_job")
            self.assertEqual( workflow_job.name() , "compare_job")
