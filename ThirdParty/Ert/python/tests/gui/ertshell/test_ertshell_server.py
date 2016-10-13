from ert.test import ExtendedTestCase
from tests.gui.ertshell.ert_shell_test_context import ErtShellTestContext


class ErtShellServerTest(ExtendedTestCase):

    def test_server(self):
        config = self.createTestPath("local/snake_oil_no_data/snake_oil.ert")
        with ErtShellTestContext("python/ertshell/server", config, load_config=False) as shell:
            server_settings = shell.shellContext()["server_settings"]
            self.assertTrue(shell.invokeCommand("help server"))

            self.assertTrue(shell.invokeCommand("server hostname"))
            self.assertEqual(server_settings._hostname, "localhost")

            self.assertTrue(shell.invokeCommand("server hostname new.hostname.no"))
            self.assertEqual(server_settings._hostname, "new.hostname.no")

            self.assertTrue(shell.invokeCommand("server port"))
            self.assertEqual(server_settings._port, 0)

            self.assertTrue(shell.invokeCommand("server port 99"))
            self.assertEqual(server_settings._port, 99)
