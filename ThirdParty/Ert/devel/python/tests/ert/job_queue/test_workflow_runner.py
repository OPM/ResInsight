import os
import time
from ert.job_queue import WorkflowJoblist, Workflow, WorkflowRunner
from ert.test import TestAreaContext, ExtendedTestCase
from ert.util.substitution_list import SubstitutionList
from .workflow_common import WorkflowCommon


class WorkflowRunnerTest(ExtendedTestCase):

    def test_workflow_thread_cancel_ert_script(self):
        with TestAreaContext("python/job_queue/workflow_runner_ert_script") as work_area:
            WorkflowCommon.createWaitJob()

            joblist = WorkflowJoblist()
            self.assertTrue(joblist.addJobFromFile("WAIT", "wait_job"))
            self.assertTrue("WAIT" in joblist)

            workflow = Workflow("wait_workflow", joblist)

            self.assertEqual(len(workflow), 3)


            workflow_runner = WorkflowRunner(workflow)

            self.assertFalse(workflow_runner.isRunning())

            workflow_runner.run()

            self.assertIsNone(workflow_runner.workflowResult())

            time.sleep(1) # wait for workflow to start
            self.assertTrue(workflow_runner.isRunning())
            self.assertFileExists("wait_started_0")

            time.sleep(1) # wait for first job to finish

            workflow_runner.cancel()
            time.sleep(1) # wait for cancel to take effect
            self.assertFileExists("wait_finished_0")


            self.assertFileExists("wait_started_1")
            self.assertFileExists("wait_cancelled_1")
            self.assertFileDoesNotExist("wait_finished_1")

            self.assertTrue(workflow_runner.isCancelled())

            workflow_runner.wait() # wait for runner to complete

            self.assertFileDoesNotExist("wait_started_2")
            self.assertFileDoesNotExist("wait_cancelled_2")
            self.assertFileDoesNotExist("wait_finished_2")




    def test_workflow_thread_cancel_external(self):
        with TestAreaContext("python/job_queue/workflow_runner_external") as work_area:
            WorkflowCommon.createWaitJob()

            joblist = WorkflowJoblist()
            self.assertTrue(joblist.addJobFromFile("WAIT", "external_wait_job"))
            self.assertTrue("WAIT" in joblist)

            workflow = Workflow("wait_workflow", joblist)

            self.assertEqual(len(workflow), 3)


            workflow_runner = WorkflowRunner(workflow, ert=None, context=SubstitutionList())

            self.assertFalse(workflow_runner.isRunning())

            workflow_runner.run()

            time.sleep(1) # wait for workflow to start
            self.assertTrue(workflow_runner.isRunning())
            self.assertFileExists("wait_started_0")

            time.sleep(1) # wait for first job to finish

            workflow_runner.cancel()
            time.sleep(1) # wait for cancel to take effect
            self.assertFileExists("wait_finished_0")

            self.assertFileExists("wait_started_1")
            self.assertFileDoesNotExist("wait_finished_1")

            self.assertTrue(workflow_runner.isCancelled())

            workflow_runner.wait() # wait for runner to complete

            self.assertFileDoesNotExist("wait_started_2")
            self.assertFileDoesNotExist("wait_cancelled_2")
            self.assertFileDoesNotExist("wait_finished_2")


    def test_workflow_success(self):
        with TestAreaContext("python/job_queue/workflow_runner_fast") as work_area:
            WorkflowCommon.createWaitJob()

            joblist = WorkflowJoblist()
            self.assertTrue(joblist.addJobFromFile("WAIT", "wait_job"))
            self.assertTrue(joblist.addJobFromFile("EXTERNAL_WAIT", "external_wait_job"))

            workflow = Workflow("fast_wait_workflow", joblist)

            self.assertEqual(len(workflow), 2)


            workflow_runner = WorkflowRunner(workflow, ert=None, context=SubstitutionList())

            self.assertFalse(workflow_runner.isRunning())

            workflow_runner.run()
            time.sleep(1) # wait for workflow to start
            workflow_runner.wait()

            self.assertFileExists("wait_started_0")
            self.assertFileDoesNotExist("wait_cancelled_0")
            self.assertFileExists("wait_finished_0")

            self.assertFileExists("wait_started_1")
            self.assertFileDoesNotExist("wait_cancelled_1")
            self.assertFileExists("wait_finished_1")

            self.assertTrue(workflow_runner.workflowResult())
