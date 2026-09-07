import sys
import os
import threading
import time

import grpc
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips  # noqa: E402
from rips.exception import RipsError  # noqa: E402

import dataroot  # noqa: E402


class _FakeRpcError(grpc.RpcError):
    def __init__(self, code, details):
        self._code = code
        self._details = details

    def code(self):
        return self._code

    def details(self):
        return self._details


def test_rips_error_is_constructible_from_message_only():
    err = RipsError("boom")
    assert str(err) == "boom"
    assert err.code is None
    assert err.details is None
    assert err.location is None


def test_rips_error_carries_structured_fields():
    err = RipsError("boom", code=grpc.StatusCode.NOT_FOUND, details="x", location="h:1")
    assert err.code == grpc.StatusCode.NOT_FOUND
    assert err.details == "x"
    assert err.location == "h:1"


def test_rips_error_from_rpc_error_extracts_code_and_details():
    rpc = _FakeRpcError(grpc.StatusCode.INVALID_ARGUMENT, "bad arg")
    err = RipsError.from_rpc_error(rpc, location="localhost:50051")
    assert err.code == grpc.StatusCode.INVALID_ARGUMENT
    assert err.details == "bad arg"
    assert err.location == "localhost:50051"
    text = str(err)
    assert "INVALID_ARGUMENT" in text
    assert "bad arg" in text
    assert "localhost:50051" in text


def test_pdm_method_error_includes_code_and_details(rips_instance, initialize_test):
    # Trigger a server-side INVALID_ARGUMENT on a real gRPC round-trip and
    # confirm the new structured fields on RipsError are populated.
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(path=case_path)

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    with pytest.raises(RipsError) as excinfo:
        surface_collection.new_regular_surface(increment_x=-1.0)

    err = excinfo.value
    assert err.code is not None and err.code != grpc.StatusCode.OK
    assert err.details and "Invalid increment X" in err.details


def test_heartbeat_starts_and_stops(rips_instance, initialize_test):
    rips_instance.start_heartbeat(interval_sec=0.5, deadline_sec=1.0)
    rips_instance.stop_heartbeat()
    rips_instance.check_alive()


class _FakeApp:
    """Stub App stub whose GetVersion always fails with a given status code."""

    def __init__(self, code, details=""):
        self.calls = 0
        self._code = code
        self._details = details

    def GetVersion(self, _request, timeout=None):
        self.calls += 1
        raise _FakeRpcError(self._code, self._details)


class _FakeChannel:
    def __init__(self):
        self.closed = False

    def close(self):
        self.closed = True


class _FakeInstance:
    """Duck-typed stand-in that exercises Instance.start_heartbeat without a
    running ResInsight."""

    def __init__(self, code, exit_code=None):
        self.location = "localhost:50051"
        self.app = _FakeApp(code)
        self.channel = _FakeChannel()
        self._connection_lost = False
        self._connection_lost_message = None
        self._heartbeat_thread = None
        self._heartbeat_stop = None
        self._exit_code = exit_code

    def _process_exit_code(self):
        return self._exit_code

    start_heartbeat = rips.Instance.start_heartbeat
    stop_heartbeat = rips.Instance.stop_heartbeat
    check_alive = rips.Instance.check_alive


def _run_heartbeat(instance, seconds=0.5):
    failures = []
    instance.start_heartbeat(
        interval_sec=0.02,
        deadline_sec=0.02,
        failure_threshold=2,
        busy_warning_sec=0.05,
        on_failure=failures.append,
    )
    time.sleep(seconds)
    instance.stop_heartbeat()
    return failures


def test_heartbeat_tolerates_busy_server():
    """A busy-but-alive server leaves pings unanswered (DEADLINE_EXCEEDED).
    The connection must not be declared lost: closing the channel would abort
    the long-running call the user is waiting for (issue #14683)."""
    instance = _FakeInstance(grpc.StatusCode.DEADLINE_EXCEEDED)

    failures = _run_heartbeat(instance)

    assert instance.app.calls > 2, "heartbeat should keep pinging a busy server"
    assert not instance._connection_lost
    assert not instance.channel.closed
    assert failures == []
    instance.check_alive()


def test_heartbeat_detects_dead_server():
    """A gone server fails pings with UNAVAILABLE, which must close the
    channel so pending calls unblock instead of hanging."""
    instance = _FakeInstance(grpc.StatusCode.UNAVAILABLE)

    failures = _run_heartbeat(instance)

    assert instance._connection_lost
    assert instance.channel.closed
    assert len(failures) == 1
    with pytest.raises(RipsError):
        instance.check_alive()


def test_heartbeat_reports_exited_process_with_diagnostic():
    """When the launched process has exited, say so explicitly instead of
    surfacing a bare 'Channel closed!'."""
    instance = _FakeInstance(grpc.StatusCode.DEADLINE_EXCEEDED, exit_code=-11)

    _run_heartbeat(instance)

    assert instance._connection_lost
    with pytest.raises(RipsError) as excinfo:
        instance.check_alive()
    message = str(excinfo.value)
    assert "exited unexpectedly" in message
    assert "-11" in message


def test_server_answers_pings_while_main_thread_is_busy(rips_instance, initialize_test):
    """ResInsight processes API requests on its main thread, so a long request
    used to leave liveness pings unanswered for as long as it ran, making a busy
    server look like a dead one to the client heartbeat (issue #14683).

    Pings must be answered promptly while a blocking request is in flight."""
    from Definitions_pb2 import Empty

    stop = threading.Event()
    latencies = []
    errors = []

    def ping_loop():
        while not stop.is_set():
            started = time.monotonic()
            try:
                rips_instance.app.GetVersion(Empty(), timeout=30.0)
                latencies.append(time.monotonic() - started)
            except grpc.RpcError as exc:  # pragma: no cover - failure diagnostics
                errors.append(exc)
            time.sleep(0.02)

    thread = threading.Thread(target=ping_loop, daemon=True)
    thread.start()
    try:
        # A case load blocks the main thread for as long as it takes to read the grid.
        case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
        started = time.monotonic()
        rips_instance.project.load_case(path=case_path)
        elapsed = time.monotonic() - started
    finally:
        stop.set()
        thread.join(timeout=10.0)

    if elapsed < 0.2:
        pytest.skip(f"blocking call was too short to be meaningful ({elapsed:.3f} s)")

    assert not errors, f"pings failed while the server was busy: {errors}"

    # A ping issued just after the blocking call starts would previously only be
    # answered once that call finished, i.e. a latency of roughly `elapsed`.
    assert max(latencies) < 0.5 * elapsed, (
        f"pings were starved by the busy main thread: max latency "
        f"{max(latencies):.3f} s for a {elapsed:.3f} s blocking call"
    )
    assert len(latencies) >= 3, "expected several pings during the blocking call"
