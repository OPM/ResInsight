"""Parent-process watchdog for the ResInsight Cloud API service.

ResInsight launches this service as a child process and normally kills it on a clean
shutdown (``RiaCloudApiService::stop``). If ResInsight crashes or is force-killed that
shutdown path never runs, and on Windows the child is not reaped automatically, so the
service would be orphaned and keep holding its port.

To guard against that, ResInsight passes its own process id in the
``RESINSIGHT_PARENT_PID`` environment variable. This module watches that process and shuts
the service down when it disappears.

How it watches, per platform:

* Windows: a ``SYNCHRONIZE`` handle to the parent is opened *once* and held for the
  service's lifetime. Holding the handle keeps the parent's process object identifiable, so
  a recycled PID cannot be mistaken for a live parent, and ``WaitForSingleObject`` reports
  exit precisely.
* Linux: ``PR_SET_PDEATHSIG`` is installed for an immediate signal on parent death (the
  poll below is the fallback). Liveness in the poll is confirmed by start time, not PID
  alone, to defeat PID reuse.
* macOS / other POSIX: poll, confirming the parent's start time each cycle.

When the parent is found gone, shutdown is requested gracefully (uvicorn drains and runs
lifespan teardown) with a hard ``os._exit`` backstop so a stuck drain can never keep the
port held.
"""

from __future__ import annotations

import asyncio
import ctypes
import logging
import os
import signal
import subprocess
import sys
import threading

logger = logging.getLogger("ri_cloud_api")

_PARENT_PID_ENV = "RESINSIGHT_PARENT_PID"

# Exit code used by the hard backstop. The parent is already gone, so nothing is watching
# this code in practice; change it if your launcher logs child exit codes.
_EXIT_CODE = 0

# Ensures the shutdown sequence runs at most once, no matter which mechanism trips it.
_shutdown_started = threading.Event()


# --------------------------------------------------------------------------------------
# Windows kernel32 bindings (configured once, with correct pointer-sized signatures)
# --------------------------------------------------------------------------------------
# Defined unconditionally so static checkers always see them bound; only meaningful on
# Windows, where _WindowsParentMonitor is the only consumer. _kernel32 stays None elsewhere.
_SYNCHRONIZE = 0x00100000
_WAIT_OBJECT_0 = 0x00000000
_WAIT_TIMEOUT = 0x00000102
_ERROR_INVALID_PARAMETER = 87  # OpenProcess returns this for a non-existent PID

_kernel32 = None

if sys.platform == "win32":
    from ctypes import wintypes

    _kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    _kernel32.OpenProcess.restype = wintypes.HANDLE
    _kernel32.OpenProcess.argtypes = (wintypes.DWORD, wintypes.BOOL, wintypes.DWORD)
    _kernel32.WaitForSingleObject.restype = wintypes.DWORD
    _kernel32.WaitForSingleObject.argtypes = (wintypes.HANDLE, wintypes.DWORD)
    _kernel32.CloseHandle.restype = wintypes.BOOL
    _kernel32.CloseHandle.argtypes = (wintypes.HANDLE,)


# --------------------------------------------------------------------------------------
# Parent monitors. Each exposes ``disabled`` (watchdog should no-op) and ``is_alive()``.
# --------------------------------------------------------------------------------------
class _WindowsParentMonitor:
    """Holds a handle to the parent so liveness is immune to PID reuse."""

    def __init__(self, pid: int) -> None:
        self.disabled = False
        self._handle = None
        self._gone = False

        handle = _kernel32.OpenProcess(_SYNCHRONIZE, False, pid)
        if handle:
            self._handle = handle
            return

        err = ctypes.get_last_error()
        if err == _ERROR_INVALID_PARAMETER:
            # No process with this PID -> parent already gone.
            self._gone = True
        else:
            # Access denied or similar: don't risk a spurious shutdown, just stand down.
            logger.warning(
                "Parent watchdog disabled: OpenProcess(pid=%d) failed (err=%d).", pid, err
            )
            self.disabled = True

    def is_alive(self) -> bool:
        if self._gone or self._handle is None:
            return False
        result = _kernel32.WaitForSingleObject(self._handle, 0)
        if result == _WAIT_TIMEOUT:
            return True
        if result == _WAIT_OBJECT_0:
            return False  # parent signalled = exited
        # WAIT_FAILED or anything unexpected: fail safe, treat as gone.
        logger.warning(
            "Parent watchdog: WaitForSingleObject returned 0x%X; treating parent as gone.",
            result,
        )
        return False

    def close(self) -> None:
        if self._handle is not None:
            _kernel32.CloseHandle(self._handle)
            self._handle = None


class _PosixParentMonitor:
    """Confirms both PID existence and start time so a recycled PID is not trusted."""

    def __init__(self, pid: int) -> None:
        self._pid = pid
        self._start_token = _read_start_time(pid)
        # If we can't read the parent at startup, prefer to stand down rather than kill a
        # possibly-healthy service over a transient read glitch.
        self.disabled = self._start_token is None

    def is_alive(self) -> bool:
        token = _read_start_time(self._pid)
        return token is not None and token == self._start_token

    def close(self) -> None:
        pass


def _read_start_time(pid: int) -> str | None:
    """Return a stable per-process start-time token, or None if the process is gone.

    Linux reads ``/proc/<pid>/stat`` (cheap, no subprocess); other POSIX uses ``ps``.
    """
    if sys.platform.startswith("linux"):
        try:
            with open(f"/proc/{pid}/stat", "rb") as fh:
                data = fh.read()
        except (FileNotFoundError, ProcessLookupError, OSError):
            return None
        # comm (field 2) may contain spaces/parens; everything after the last ')' is fixed.
        # Remaining fields start at 'state' (field 3), so starttime (field 22) is index 19.
        try:
            fields = data[data.rindex(b")") + 1:].split()
            return fields[19].decode()
        except (ValueError, IndexError):
            return None

    # macOS / BSD: 'lstart' is a stable human-readable start time, fine for an identity key.
    try:
        proc = subprocess.run(
            ["ps", "-p", str(pid), "-o", "lstart="],
            capture_output=True,
            text=True,
            timeout=5,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if proc.returncode != 0:
        return None
    return proc.stdout.strip() or None


def _make_monitor(pid: int):
    if sys.platform == "win32":
        return _WindowsParentMonitor(pid)
    return _PosixParentMonitor(pid)


# --------------------------------------------------------------------------------------
# Shutdown: one idempotent path, graceful with a hard backstop.
# --------------------------------------------------------------------------------------
def _hard_exit() -> None:
    logger.warning("Parent watchdog: grace period elapsed, forcing exit.")
    os._exit(_EXIT_CODE)


def _trigger_shutdown(server, grace_period: float, reason: str) -> None:
    """Request shutdown once. Graceful if possible, hard-exit backstop always."""
    if _shutdown_started.is_set():
        return
    _shutdown_started.set()

    logger.warning("Parent ResInsight gone (%s); shutting down service.", reason)

    # Backstop runs in its own thread, so it fires even if the event loop is being torn
    # down or a graceful drain hangs.
    backstop = threading.Timer(grace_period, _hard_exit)
    backstop.daemon = True
    backstop.start()

    if server is not None:
        # Cross-platform graceful stop: uvicorn notices the flag, drains, runs teardown.
        server.should_exit = True
        return

    # No server handle was provided.
    if sys.platform == "win32":
        # uvicorn on Windows has no signal-based graceful stop we can invoke from here.
        # Pass `server` to get a graceful shutdown on Windows; otherwise exit promptly.
        _hard_exit()
    else:
        try:
            os.kill(os.getpid(), signal.SIGTERM)  # uvicorn handles SIGTERM gracefully
        except Exception:
            _hard_exit()


def _install_linux_fast_path(server, grace_period: float) -> None:
    """Immediate shutdown on parent death via PR_SET_PDEATHSIG, routed through SIGUSR1.

    SIGUSR1 is used (not SIGTERM) so this doesn't collide with uvicorn's own SIGTERM/SIGINT
    handlers, and it funnels into the same ``_trigger_shutdown`` as the poll.

    Caveat: the kernel sends PDEATHSIG when the *thread* that created this process exits,
    not necessarily the whole parent process. If ResInsight spawns the service from a
    short-lived worker thread, this may fire early; the start-time poll is the reliable
    fallback and will not be fooled. No-op on non-Linux platforms or if installation fails.
    """
    if not sys.platform.startswith("linux"):
        return

    try:
        # Resolved dynamically: SIGUSR1 is Unix-only, so a direct ``signal.SIGUSR1``
        # reference trips static checkers (e.g. Pylint no-member) on Windows even though
        # this branch never runs there. This early-returns above on non-Linux anyway.
        sigusr1 = getattr(signal, "SIGUSR1")

        def _on_parent_death(signum, frame):
            _trigger_shutdown(server, grace_period, reason="parent-death signal")

        # signal.signal must run in the main thread; watch_parent normally does.
        signal.signal(sigusr1, _on_parent_death)

        PR_SET_PDEATHSIG = 1  # from <sys/prctl.h>
        libc = ctypes.CDLL("libc.so.6", use_errno=True)
        if libc.prctl(PR_SET_PDEATHSIG, sigusr1) != 0:
            logger.debug(
                "prctl(PR_SET_PDEATHSIG) failed: errno %d", ctypes.get_errno()
            )
    except Exception as exc:  # best effort; the poll remains the reliable mechanism
        logger.debug("Linux parent-death fast path unavailable: %s", exc)


# --------------------------------------------------------------------------------------
# Public entry point
# --------------------------------------------------------------------------------------
async def watch_parent(
    server=None,
    poll_interval: float = 3.0,
    grace_period: float = 5.0,
) -> None:
    """Watch the parent ResInsight process and shut the service down when it is gone.

    Args:
        server: the running ``uvicorn.Server`` instance. Strongly recommended — it enables a
            clean, cross-platform graceful shutdown (and is required for graceful shutdown
            on Windows). If omitted, shutdown falls back to SIGTERM (POSIX) or a hard exit
            (Windows).
        poll_interval: seconds between liveness checks.
        grace_period: seconds to allow for graceful drain before the hard-exit backstop.

    If ``RESINSIGHT_PARENT_PID`` is unset or invalid (e.g. running the service by hand for
    debugging) the watchdog logs once and becomes a no-op.
    """
    raw_pid = os.environ.get(_PARENT_PID_ENV, "").strip()
    if not raw_pid:
        logger.info("Parent watchdog disabled: %s not set.", _PARENT_PID_ENV)
        return

    try:
        parent_pid = int(raw_pid)
    except ValueError:
        logger.warning(
            "Parent watchdog disabled: invalid %s=%r.", _PARENT_PID_ENV, raw_pid
        )
        return

    monitor = _make_monitor(parent_pid)
    if monitor.disabled:
        logger.warning(
            "Parent watchdog disabled: could not establish monitoring for pid %d.",
            parent_pid,
        )
        return

    logger.info("Parent watchdog active: monitoring ResInsight (pid %d).", parent_pid)

    # Linux fast path for sub-poll-interval latency; the loop below is the cross-platform
    # fallback and also covers the brief race where the parent dies before prctl runs.
    _install_linux_fast_path(server, grace_period)

    try:
        while True:
            if not monitor.is_alive():
                _trigger_shutdown(
                    server, grace_period, reason=f"parent pid {parent_pid} gone"
                )
                return
            await asyncio.sleep(poll_interval)
    finally:
        monitor.close()