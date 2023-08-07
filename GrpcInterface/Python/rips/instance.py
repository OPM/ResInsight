# pylint: disable=no-self-use
"""
The main entry point for ResInsight connections
The Instance class contained have static methods launch and find for
creating connections to ResInsight
"""

from __future__ import annotations
import os
import socket
import logging
import time
import tempfile
import signal

import grpc

import App_pb2
import App_pb2_grpc
import Commands_pb2
import Commands_pb2_grpc
from Definitions_pb2 import Empty

import RiaVersionInfo

from .project import Project
from .retry_policy import ExponentialBackoffRetryPolicy
from .grpc_retry_interceptor import RetryOnRpcErrorClientInterceptor
from .generated.generated_classes import CommandRouter

from typing import List, Optional, Tuple
from typing_extensions import Self


class Instance:
    """The ResInsight Instance class. Use to launch or find existing ResInsight instances

    Attributes:
        launched (bool): Tells us whether the application was launched as a new process.
            If the application was launched we may need to close it when exiting the script.
        commands (Commands): Command executor. Set when creating an instance.
        project (Project): Current project in ResInsight.
            Set when creating an instance and updated when opening/closing projects.
    """

    @staticmethod
    def __is_port_in_use(port: int) -> bool:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as my_socket:
            my_socket.settimeout(0.2)
            return my_socket.connect_ex(("localhost", port)) == 0

    @staticmethod
    def __is_valid_port(port: int) -> bool:
        location = "localhost:" + str(port)
        channel = grpc.insecure_channel(
            location, options=[("grpc.enable_http_proxy", False)]
        )
        app = App_pb2_grpc.AppStub(channel)
        try:
            app.GetVersion(Empty(), timeout=1)
        except grpc.RpcError:
            return False
        return True

    @staticmethod
    def __read_port_number_from_file(file_path: str) -> int:
        retry_count = 0
        while not os.path.exists(file_path) and retry_count < 60:
            time.sleep(1)
            retry_count = retry_count + 1

        print("Portnumber file retry count : ", retry_count)

        if os.path.isfile(file_path):
            with open(file_path) as f:
                value = f.readline()
                return int(value)
        else:
            return -1

    @staticmethod
    def __kill_process(pid: int) -> None:
        """
        Kill the process with a given pid.
        """
        if hasattr(signal, "CTRL_C_EVENT"):
            # windows does not have kill
            os.kill(pid, signal.CTRL_C_EVENT)
        else:
            # linux/unix
            os.kill(pid, signal.SIGKILL)

    @staticmethod
    def launch(
        resinsight_executable: str = "",
        console: bool = False,
        launch_port: int = -1,
        command_line_parameters: List[str] = [],
    ) -> Optional[Instance]:
        """Launch a new Instance of ResInsight. This requires the environment variable
        RESINSIGHT_EXECUTABLE to be set or the parameter resinsight_executable to be provided.
        The RESINSIGHT_GRPC_PORT environment variable can be set to an alternative port number.

        Args:
            resinsight_executable (str): Path to a valid ResInsight executable. If set
                will take precedence over what is provided in the RESINSIGHT_EXECUTABLE
                environment variable.
            console (bool): If True, launch as console application, without GUI.
            launch_port(int): If -1 will use the default port 50051 or RESINSIGHT_GRPC_PORT
                             if anything else, ResInsight will try to launch with this port.
                             If 0 a random port will be used.
            command_line_parameters(list): Additional parameters as string entries in the list.
        Returns:
            Instance: an instance object if it worked. None if not.
        """

        requested_port: int = 50051
        port_env = os.environ.get("RESINSIGHT_GRPC_PORT")
        if port_env:
            requested_port = int(port_env)
        if launch_port != -1:
            requested_port = launch_port

        if not resinsight_executable:
            resinsight_executable_from_env = os.environ.get("RESINSIGHT_EXECUTABLE")
            if not resinsight_executable_from_env:
                print(
                    "ERROR: Could not launch ResInsight because the environment variable"
                    " RESINSIGHT_EXECUTABLE is not set"
                )
                return None
            else:
                resinsight_executable = resinsight_executable_from_env

        print("Trying to launch", resinsight_executable)
        with tempfile.TemporaryDirectory() as tmp_dir_path:
            port_number_file = tmp_dir_path + "/portnumber.txt"
            parameters: List[str] = [
                "ResInsight",
                "--server",
                str(requested_port),
                "--portnumberfile",
                str(port_number_file),
            ] + command_line_parameters
            if console:
                print("Launching as console app")
                parameters.append("--console")

            # Stringify all parameters
            for i in range(0, len(parameters)):
                parameters[i] = str(parameters[i])

            pid = os.spawnv(os.P_NOWAIT, resinsight_executable, parameters)
            if pid:
                port = Instance.__read_port_number_from_file(port_number_file)
                if port == -1:
                    print("Unable to read port number. Launch failed.")
                    # Need to kill the process using PID since there is no  GRPC connection to use.
                    Instance.__kill_process(pid)
                else:
                    instance = Instance(port=port, launched=True)
                    return instance
        return None

    @staticmethod
    def find(start_port: int = 50051, end_port: int = 50071) -> Optional[Instance]:
        """Search for an existing Instance of ResInsight by testing ports.

        By default we search from port 50051 to 50071 or if the environment
        variable RESINSIGHT_GRPC_PORT is set we search
        RESINSIGHT_GRPC_PORT to RESINSIGHT_GRPC_PORT+20

        Args:
            start_port (int): start searching from this port
            end_port (int): search up to but not including this port
        """
        port_env = os.environ.get("RESINSIGHT_GRPC_PORT")
        if port_env:
            print("Got port " + port_env + " from environment")
            start_port = int(port_env)
            end_port = start_port + 20

        for try_port in range(start_port, end_port):
            print("Trying port " + str(try_port))
            if Instance.__is_port_in_use(try_port) and Instance.__is_valid_port(
                try_port
            ):
                return Instance(port=try_port)

        print(
            "Error: Could not find any ResInsight instances responding between ports "
            + str(start_port)
            + " and "
            + str(end_port)
        )
        return None

    def __execute_command(self, **command_params):
        return self.commands.Execute(Commands_pb2.CommandParams(**command_params))

    def __check_version(self) -> Tuple[bool, bool]:
        try:
            major_version_ok = self.major_version() == int(
                RiaVersionInfo.RESINSIGHT_MAJOR_VERSION
            )
            minor_version_ok = self.minor_version() == int(
                RiaVersionInfo.RESINSIGHT_MINOR_VERSION
            )
            return True, major_version_ok and minor_version_ok
        except grpc.RpcError:
            return False, False

    def __init__(self, port: int = 50051, launched: bool = False) -> None:
        """Attempts to connect to ResInsight at a specific port on localhost

        Args:
            port(int): port number
        """
        logging.basicConfig()
        self.location: str = "localhost:" + str(port)

        self.channel = grpc.insecure_channel(
            self.location, options=[("grpc.enable_http_proxy", False)]
        )
        self.launched = launched
        self.commands = Commands_pb2_grpc.CommandsStub(self.channel)

        # Main version check package
        self.app = App_pb2_grpc.AppStub(self.channel)

        self._check_connection_and_version(self.channel, launched, self.location)

        # Intercept UNAVAILABLE errors and retry on failures
        interceptors = (
            RetryOnRpcErrorClientInterceptor(
                retry_policy=ExponentialBackoffRetryPolicy(
                    min_backoff=100, max_backoff=5000, max_num_retries=20
                ),
                status_for_retry=(grpc.StatusCode.UNAVAILABLE,),
            ),
        )

        intercepted_channel = grpc.intercept_channel(self.channel, *interceptors)

        # Recreate command stubs with the retry policy
        self.commands = Commands_pb2_grpc.CommandsStub(intercepted_channel)

        # Service packages
        self.project = Project.create(intercepted_channel)

        # Command Router object used as entry point for independent processing functions
        self.command_router = CommandRouter(
            self.app.GetPdmObject(Empty()), intercepted_channel
        )

        path = os.getcwd()
        self.set_start_dir(path=path)

    def _check_connection_and_version(
        self, channel: grpc.Channel, launched: bool, location: str
    ) -> None:
        connection_ok = False
        version_ok = False

        retry_policy = ExponentialBackoffRetryPolicy()
        if self.launched:
            for num_tries in range(0, retry_policy.num_retries()):
                connection_ok, version_ok = self.__check_version()
                if connection_ok:
                    break
                retry_policy.sleep(num_tries)
        else:
            connection_ok, version_ok = self.__check_version()

        if not connection_ok:
            if self.launched:
                raise Exception(
                    "Error: Could not connect to resinsight at ",
                    location,
                    ".",
                    retry_policy.time_out_message(),
                )
            raise Exception("Error: Could not connect to resinsight at ", location)
        if not version_ok:
            raise Exception(
                "Error: Wrong Version of ResInsight at ",
                location,
                self.version_string(),
                " ",
                self.client_version_string(),
            )

    def __version_message(self) -> App_pb2.Version:
        return self.app.GetVersion(Empty())

    def set_start_dir(self, path: str):
        """Set current start directory

        Arguments:
            path (str): path to directory

        """
        return self.__execute_command(
            setStartDir=Commands_pb2.FilePathRequest(path=path)
        )

    def set_export_folder(
        self, export_type: str, path: str, create_folder: bool = False
    ):
        """
        Set the export folder used for all export functions

        **Parameters**::

            Parameter        | Description                                  | Type
            ---------------- | -------------------------------------------- | -----
            export_type      | String specifying what to export             | String
            path             | Path to folder                               | String
            create_folder    | Create folder if it doesn't exist?           | Boolean

        **Enum export_type**::

            Option          | Description
            --------------- | ------------
            "COMPLETIONS"   |
            "SNAPSHOTS"     |
            "PROPERTIES"    |
            "STATISTICS"    |

        """
        return self.__execute_command(
            setExportFolder=Commands_pb2.SetExportFolderRequest(
                type=export_type, path=path, createFolder=create_folder
            )
        )

    def set_main_window_size(self, width: int, height: int):
        """
        Set the main window size in pixels

        **Parameters**::

            Parameter | Description      | Type
            --------- | ---------------- | -----
            width     | Width in pixels  | Integer
            height    | Height in pixels | Integer

        """
        return self.__execute_command(
            setMainWindowSize=Commands_pb2.SetWindowSizeParams(
                width=width, height=height
            )
        )

    def set_plot_window_size(self, width: int, height: int):
        """
        Set the plot window size in pixels

        **Parameters**::

            Parameter | Description      | Type
            --------- | ---------------- | -----
            width     | Width in pixels  | Integer
            height    | Height in pixels | Integer
        """
        return self.__execute_command(
            setPlotWindowSize=Commands_pb2.SetWindowSizeParams(
                width=width, height=height
            )
        )

    def major_version(self) -> int:
        """Get an integer with the major version number"""
        return int(self.__version_message().major_version)

    def minor_version(self) -> int:
        """Get an integer with the minor version number"""
        return int(self.__version_message().minor_version)

    def patch_version(self) -> int:
        """Get an integer with the patch version number"""
        return int(self.__version_message().patch_version)

    def version_string(self) -> str:
        """Get a full version string, i.e. 2019.04.01"""
        return (
            str(self.major_version())
            + "."
            + str(self.minor_version())
            + "."
            + str(self.patch_version())
        )

    def client_version_string(self) -> str:
        """Get a full version string, i.e. 2019.04.01"""
        version_string: str = RiaVersionInfo.RESINSIGHT_MAJOR_VERSION + "."
        version_string += RiaVersionInfo.RESINSIGHT_MINOR_VERSION + "."
        version_string += RiaVersionInfo.RESINSIGHT_PATCH_VERSION
        return version_string

    def exit(self):
        """Tell ResInsight instance to quit"""
        print("Telling ResInsight to Exit")
        return self.app.Exit(Empty())

    def is_console(self) -> bool:
        """Returns true if the connected ResInsight instance is a console app"""
        return bool(
            self.app.GetRuntimeInfo(Empty()).app_type
            == App_pb2.ApplicationTypeEnum.Value("CONSOLE_APPLICATION")
        )

    def is_gui(self) -> bool:
        """Returns true if the connected ResInsight instance is a GUI app"""
        return bool(
            self.app.GetRuntimeInfo(Empty()).app_type
            == App_pb2.ApplicationTypeEnum.Value("GUI_APPLICATION")
        )
