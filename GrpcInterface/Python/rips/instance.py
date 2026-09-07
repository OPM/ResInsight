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
import sys
import json
import subprocess
import threading

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
from .exception import RipsError

from typing import Callable, List, Optional, Tuple
from pathlib import Path

logger = logging.getLogger(__name__)

# gRPC status codes that mean "the server did not answer in time" rather than
# "the server is gone". ResInsight serves gRPC requests from its main thread, so
# any long-running request (large exports, schedule generation on a big event
# timeline, heavy grid computations) makes pings time out while the process is
# perfectly healthy. Tearing down the connection in that situation aborts the
# very call the user is waiting for (see issue #14683).
_BUSY_STATUS_CODES = (
    grpc.StatusCode.DEADLINE_EXCEEDED,
    grpc.StatusCode.RESOURCE_EXHAUSTED,
)


def _is_server_busy_error(rpc_error: grpc.RpcError) -> bool:
    """True when a failed ping indicates a busy server, not a dead one."""
    code_fn = getattr(rpc_error, "code", None)
    code = code_fn() if callable(code_fn) else None
    return code in _BUSY_STATUS_CODES


class Instance:
    """The ResInsight Instance class. Use to launch or find existing ResInsight instances

    Attributes:
        launched (bool): Tells us whether the application was launched as a new process.
            If the application was launched we may need to close it when exiting the script.
        port (int): The port number used to connect to this instance.
        commands (Commands): Command executor. Set when creating an instance.
        project (Project): Current project in ResInsight.
            Set when creating an instance and updated when opening/closing projects.
    """

    _last_version_check_error: Optional[grpc.RpcError]
    _connection_lost: bool
    _heartbeat_thread: Optional[threading.Thread]
    _heartbeat_stop: Optional[threading.Event]

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
    def __read_port_number_from_file(file_path: str, max_attempts: int) -> int:
        retry_count = 0
        while not os.path.exists(file_path) and retry_count < max_attempts:
            time.sleep(1)
            retry_count = retry_count + 1

        if os.path.isfile(file_path):
            with open(file_path) as f:
                value = f.readline()
                return int(value)

        if retry_count == max_attempts:
            logger.warning(
                "Waiting for port number file timed out after %d seconds. File: %s",
                max_attempts,
                file_path,
            )

        return -1

    @staticmethod
    def __kill_process(pid: int) -> None:
        """
        Kill the process with a given pid.
        """
        if hasattr(signal, "CTRL_C_EVENT"):
            # windows - use SIGTERM for process termination
            os.kill(pid, signal.SIGTERM)
        else:
            # linux/unix
            os.kill(pid, signal.SIGKILL)

    @staticmethod
    def launch(
        resinsight_executable: str = "",
        console: bool = False,
        launch_port: int = 0,
        init_timeout: int = 300,
        command_line_parameters: List[str] = [],
        enable_heartbeat: bool = True,
    ) -> Instance:
        """Launch a new Instance of ResInsight. This requires the environment variable
        RESINSIGHT_EXECUTABLE to be set or the parameter resinsight_executable to be provided.
        The RESINSIGHT_GRPC_PORT environment variable can be set to an alternative port number.

        Args:
            resinsight_executable (str): Path to a valid ResInsight executable. If set
                will take precedence over what is provided in the RESINSIGHT_EXECUTABLE
                environment variable.
            console (bool): If True, launch as console application, without GUI.
            launch_port(int): If 0, GRPC will find an available port.
                             If -1, use the default port 50051 or RESINSIGHT_GRPC_PORT
                             If anything else, ResInsight will try to launch with the specified portnumber.
            init_timeout: Number of seconds to wait for initialization before timing out.
            command_line_parameters(list): Additional parameters as string entries in the list.
            enable_heartbeat(bool): If True (default), a background thread pings the
                server periodically and aborts pending RPCs if the process dies.
                Pings left unanswered by a busy server are not treated as a
                failure, so long-running calls are never aborted.
        Returns:
            Instance: a connected instance object. Raises :class:`RipsError` on failure.
        """

        requested_port: int = 50051
        port_env = os.environ.get("RESINSIGHT_GRPC_PORT")
        if port_env:
            requested_port = int(port_env)
        if launch_port != -1:
            requested_port = launch_port

        if not resinsight_executable:
            filename = Path(sys.prefix) / "share" / "rips" / "rips_config.json"
            if filename.is_file():
                f = open(filename)
                data = json.load(f)
                resinsight_executable = data["resinsight_executable"]
                if resinsight_executable:
                    logger.info(
                        "In './share/rips/rips_config.json', found resinsight_executable: %s",
                        resinsight_executable,
                    )

        if not resinsight_executable:
            resinsight_executable_from_env = os.environ.get("RESINSIGHT_EXECUTABLE")
            if not resinsight_executable_from_env:
                raise RipsError(
                    "Could not launch ResInsight because the environment variable"
                    " RESINSIGHT_EXECUTABLE is not set"
                )
            else:
                resinsight_executable = resinsight_executable_from_env

        # Check if executable file exists
        if not os.path.isfile(resinsight_executable):
            raise RipsError(
                f"ResInsight executable not found at path: {resinsight_executable}"
            )

        logger.info("Trying to launch %s", resinsight_executable)
        with tempfile.TemporaryDirectory() as tmp_dir_path:
            port_number_file = tmp_dir_path + "/portnumber.txt"
            parameters: List[str] = [
                resinsight_executable,
                "--server",
                str(requested_port),
                "--portnumberfile",
                str(port_number_file),
            ] + command_line_parameters
            if console:
                logger.info("Launching as console app")
                parameters.append("--console")

            # Stringify all parameters
            for i in range(0, len(parameters)):
                parameters[i] = str(parameters[i])

            process = subprocess.Popen(parameters)
            pid = process.pid
            port = Instance.__read_port_number_from_file(port_number_file, init_timeout)
            if port == -1:
                # Need to kill the process using PID since there is no GRPC connection to use.
                Instance.__kill_process(pid)
                raise RipsError("Unable to read port number. Launch failed.")
            return Instance(
                port=port,
                launched=True,
                enable_heartbeat=enable_heartbeat,
                process=process,
            )

    @staticmethod
    def find(
        start_port: int = 50051,
        end_port: int = 50071,
        enable_heartbeat: bool = True,
    ) -> Instance:
        """Search for an existing Instance of ResInsight by testing ports.

        By default we search from port 50051 to 50071 or if the environment
        variable RESINSIGHT_GRPC_PORT is set we search
        RESINSIGHT_GRPC_PORT to RESINSIGHT_GRPC_PORT+20

        Args:
            start_port (int): start searching from this port
            end_port (int): search up to but not including this port
            enable_heartbeat(bool): If True (default), a background thread pings the
                server periodically and aborts pending RPCs if the process dies.
                Pings left unanswered by a busy server are not treated as a
                failure, so long-running calls are never aborted.
        """
        port_env = os.environ.get("RESINSIGHT_GRPC_PORT")
        if port_env:
            logger.info("Got port %s from environment", port_env)
            start_port = int(port_env)
            end_port = start_port + 20

        for try_port in range(start_port, end_port):
            logger.debug("Trying port %d", try_port)
            if Instance.__is_port_in_use(try_port) and Instance.__is_valid_port(
                try_port
            ):
                return Instance(port=try_port, enable_heartbeat=enable_heartbeat)

        raise RipsError(
            f"Could not find any ResInsight instances responding between ports {start_port} and {end_port}"
        )

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
            self._last_version_check_error = None
            return True, major_version_ok and minor_version_ok
        except grpc.RpcError as exception:
            self._last_version_check_error = exception
            return False, False

    def __init__(
        self,
        port: int = 50051,
        launched: bool = False,
        enable_heartbeat: bool = True,
        process: Optional["subprocess.Popen[bytes]"] = None,
    ) -> None:
        """Attempts to connect to ResInsight at a specific port on localhost

        Args:
            port(int): port number
            launched(bool): True if this Python process launched ResInsight.
            enable_heartbeat(bool): If True (default), a background thread
                pings the server periodically. On detection of a dead server
                the channel is closed so any in-flight RPC unblocks
                immediately with a :class:`RipsError`.
            process: The ResInsight process handle when this Python process
                launched it. Used by the heartbeat to tell a crashed server
                apart from one that is merely busy.
        """
        self.location: str = "localhost:" + str(port)
        self.port: int = port
        self._last_version_check_error = None
        self._connection_lost = False
        self._connection_lost_message: Optional[str] = None
        self._heartbeat_thread = None
        self._heartbeat_stop = None
        self._process = process

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
                should_abort=lambda: self._connection_lost,
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

        if enable_heartbeat:
            self.start_heartbeat()

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
            last_error = self._last_version_check_error
            cause_text = ""
            code = None
            details = None
            if last_error is not None:
                code_fn = getattr(last_error, "code", None)
                details_fn = getattr(last_error, "details", None)
                if callable(code_fn):
                    code = code_fn()
                if callable(details_fn):
                    details = details_fn()
                if code is not None or details:
                    cause_text = f" (gRPC {code}: {details or ''})"

            if self.launched:
                raise RipsError(
                    f"Could not connect to ResInsight at {location}.{cause_text} "
                    f"{retry_policy.time_out_message()}",
                    code=code,
                    details=details,
                    location=location,
                ) from last_error
            raise RipsError(
                f"Could not connect to ResInsight at {location}.{cause_text}",
                code=code,
                details=details,
                location=location,
            ) from last_error
        if not version_ok:
            raise RipsError(
                f"Wrong Version of ResInsight at {location}. "
                f"Executable: {self.version_string()}, "
                f"rips: {self.client_version_string()}",
                location=location,
            )

    def __version_message(self) -> App_pb2.Version:
        return self.app.GetVersion(Empty())

    def start_heartbeat(
        self,
        interval_sec: float = 5.0,
        deadline_sec: float = 5.0,
        failure_threshold: int = 3,
        busy_warning_sec: float = 60.0,
        on_failure: Optional[Callable[["RipsError"], None]] = None,
    ) -> None:
        """Start a background thread that periodically pings ResInsight.

        The heartbeat detects a *dead* server, not a *busy* one. ResInsight
        serves gRPC requests from its main thread, so a long-running call
        (e.g. ``generate_schedule`` on a large event timeline) leaves pings
        unanswered for as long as that call runs. Such pings fail with
        ``DEADLINE_EXCEEDED`` while the process is perfectly healthy, so they
        are reported as "busy" and never counted towards
        ``failure_threshold``. Only failures that indicate a broken
        connection (``UNAVAILABLE`` and friends), or an exited process when
        this Python process launched ResInsight, declare the connection lost.

        A single connection failure is not fatal either: the heartbeat
        tolerates up to ``failure_threshold - 1`` consecutive failures (e.g.
        transient slowness on a loaded CI box) before declaring the
        connection lost. A successful ping resets the counter.

        Once the connection is declared lost, the instance is marked as
        ``connection_lost``, the underlying gRPC channel is closed so
        any in-flight calls fail fast with ``UNAVAILABLE``, and the
        retry interceptor is told to stop retrying. Subsequent API
        calls that go through :meth:`check_alive` raise
        :class:`RipsError` with the captured cause.

        Args:
            interval_sec: Seconds between pings.
            deadline_sec: Per-ping deadline. A ping exceeding this deadline
                means the server is busy, not that it is gone.
            failure_threshold: Number of consecutive *connection* failures
                required before the connection is declared lost.
                Must be >= 1.
            busy_warning_sec: Log a warning when the server has been unable
                to answer pings for this many seconds in a row. The pending
                call is still allowed to run to completion.
            on_failure: Optional callback invoked once when the
                heartbeat detects a lost connection. Receives a
                :class:`RipsError`.
        """
        if failure_threshold < 1:
            raise RipsError("failure_threshold must be >= 1")

        if self._heartbeat_thread is not None and self._heartbeat_thread.is_alive():
            return

        stop_event = threading.Event()
        self._heartbeat_stop = stop_event

        def _declare_lost(err: RipsError) -> None:
            self._connection_lost = True
            self._connection_lost_message = str(err)
            # Close the channel so any pending RPC unblocks
            # immediately with UNAVAILABLE instead of waiting
            # for TCP keepalive (which can take many minutes).
            try:
                self.channel.close()
            except Exception:
                logger.exception("Failed to close gRPC channel from heartbeat")
            if on_failure is not None:
                try:
                    on_failure(err)
                except Exception:
                    logger.exception("Heartbeat on_failure callback raised")

        def _run() -> None:
            consecutive_failures = 0
            busy_since: Optional[float] = None
            busy_warned = False
            while not stop_event.is_set():
                try:
                    self.app.GetVersion(Empty(), timeout=deadline_sec)
                    consecutive_failures = 0
                    busy_since = None
                    busy_warned = False
                except grpc.RpcError as exc:
                    exit_code = self._process_exit_code()
                    if exit_code is not None:
                        _declare_lost(
                            RipsError(
                                f"ResInsight at {self.location} exited unexpectedly "
                                f"(process exit code {exit_code})",
                                location=self.location,
                            )
                        )
                        return

                    if _is_server_busy_error(exc):
                        # The server is alive, but its main thread is occupied by a
                        # long-running request. Never tear down the connection for
                        # this: that would kill the very call we are waiting for.
                        now = time.monotonic()
                        if busy_since is None:
                            busy_since = now
                        busy_elapsed = now - busy_since
                        if not busy_warned and busy_elapsed >= busy_warning_sec:
                            busy_warned = True
                            logger.warning(
                                "ResInsight at %s has not answered heartbeat pings for "
                                "%.0f s. It is most likely busy with a long-running "
                                "request; still waiting.",
                                self.location,
                                busy_elapsed,
                            )
                        else:
                            logger.debug(
                                "Heartbeat ping timed out after %.1f s (server busy)",
                                deadline_sec,
                            )
                        stop_event.wait(interval_sec)
                        continue

                    consecutive_failures += 1
                    if consecutive_failures < failure_threshold:
                        logger.warning(
                            "Heartbeat ping failed (%d/%d): %s",
                            consecutive_failures,
                            failure_threshold,
                            exc,
                        )
                        stop_event.wait(interval_sec)
                        continue

                    _declare_lost(RipsError.from_rpc_error(exc, location=self.location))
                    return
                stop_event.wait(interval_sec)

        thread = threading.Thread(target=_run, name="rips-heartbeat", daemon=True)
        self._heartbeat_thread = thread
        thread.start()

    def stop_heartbeat(self) -> None:
        """Stop the background heartbeat thread, if running."""
        if self._heartbeat_stop is not None:
            self._heartbeat_stop.set()
        if self._heartbeat_thread is not None:
            self._heartbeat_thread.join(timeout=2.0)
        self._heartbeat_thread = None
        self._heartbeat_stop = None

    def _process_exit_code(self) -> Optional[int]:
        """Exit code of the ResInsight process we launched, or None when the
        process is still running (or was not launched by this client)."""
        if self._process is None:
            return None
        try:
            return self._process.poll()
        except Exception:
            return None

    def check_alive(self) -> None:
        """Raise :class:`RipsError` if the heartbeat has flagged a
        lost connection. Cheap to call before issuing API requests."""
        if self._connection_lost:
            message = self._connection_lost_message or (
                f"ResInsight at {self.location} is no longer responding "
                "(detected by heartbeat)"
            )
            raise RipsError(message, location=self.location)

    def __del__(self):
        try:
            self.stop_heartbeat()
        except Exception:
            pass

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
        logger.info("Telling ResInsight to Exit")
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
