import os
from ert.test import ExtendedTestCase
from .ert_shell_test_context import ErtShellTestContext, ShellCapturing


class ErtShellContextTest(ExtendedTestCase):

    def test_ertshell_context(self):
        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtShellTestContext("python/ertshell/ert_shell_context", config_file=test_config, load_config=False) as shell:

            self.assertIsNone(shell.shellContext().ert())

            self.assertTrue(shell.invokeCommand("load_config mini_config"))

            self.assertIsNotNone(shell.shellContext().ert())


    def test_std_out_capture(self):

        test_config = self.createTestPath("local/custom_kw/mini_config")

        with ErtShellTestContext("python/ertshell/ert_shell_context", config_file=test_config, load_config=False) as shell:

            with ShellCapturing(shell) as out:
                shell.onecmd("cwd")

            cwd = out[0]
            prefix = "Current directory: "
            self.assertTrue(cwd.startswith(prefix))

            self.assertEqual(os.getcwd(), cwd[len(prefix):])

