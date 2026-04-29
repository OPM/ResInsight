import sys
import os

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
