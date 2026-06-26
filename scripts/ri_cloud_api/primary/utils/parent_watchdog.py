"""Parent-process watchdog for the ResInsight Cloud API service.

ResInsight launches this service as a child process and normally kills it on a clean
shutdown (``RiaCloudApiService::stop``). If ResInsight crashes or is force-killed that
shutdown path never runs, and on Windows the child is not reaped automatically, so the
service would be orphaned and keep holding its port.

To guard against that, ResInsight passes its own process id in the ``RESINSIGHT_PARENT_PID``
environment variable. This module polls whether that process is still alive and exits the
service when it disappears.
"""

from __future__ import annotations

import asyncio
import logging
import os
import sys

logger = logging.getLogger("ri_cloud_api")

_PARENT_PID_ENV = "RESINSIGHT_PARENT_PID"


def _parent_alive_posix(pid: int) -> bool:
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        # The process exists but is owned by another user.
        return True
    return True


def _parent_alive_windows(pid: int) -> bool:
    import ctypes
    from ctypes import wintypes

    SYNCHRONIZE = 0x00100000
    WAIT_OBJECT_0 = 0x00000000
    WAIT_TIMEOUT = 0x00000102

    kernel32 = ctypes.windll.kernel32
    handle = kernel32.OpenProcess(SYNCHRONIZE, False, pid)
    if not handle:
        # Could not open the process -> treat as gone.
        return False
    try:
        result = kernel32.WaitForSingleObject(wintypes.HANDLE(handle), 0)
        if result == WAIT_TIMEOUT:
            return True
        if result == WAIT_OBJECT_0:
            # The process has signalled (exited).
            return False
        # Any other result (e.g. WAIT_FAILED): assume gone to fail safe.
        return False
    finally:
        kernel32.CloseHandle(handle)


def _parent_alive(pid: int) -> bool:
    if sys.platform == "win32":
        return _parent_alive_windows(pid)
    return _parent_alive_posix(pid)


def _install_linux_parent_death_signal() -> None:
    """Ask the Linux kernel to send SIGTERM to this process when its parent dies.

    This is the idiomatic Linux mechanism: it fires immediately and is not subject to the
    PID-reuse race that any polling approach has. It complements the cross-platform poll in
    ``watch_parent`` (which stays as the fallback and also covers re-parenting). No-op on
    non-Linux platforms.
    """
    if not sys.platform.startswith("linux"):
        return

    try:
        import ctypes
        import signal

        PR_SET_PDEATHSIG = 1  # from <sys/prctl.h>
        libc = ctypes.CDLL("libc.so.6", use_errno=True)
        if libc.prctl(PR_SET_PDEATHSIG, signal.SIGTERM) != 0:
            errno = ctypes.get_errno()
            logger.debug("prctl(PR_SET_PDEATHSIG) failed: errno %d", errno)
    except Exception as exc:  # best effort; polling remains as the fallback
        logger.debug("Could not install parent-death signal: %s", exc)


async def watch_parent(poll_interval: float = 3.0) -> None:
    """Poll the parent ResInsight process and exit when it is gone.

    If ``RESINSIGHT_PARENT_PID`` is unset or invalid (e.g. when running the service
    manually for debugging) the watchdog logs once and becomes a no-op.
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

    logger.info("Parent watchdog active: monitoring ResInsight (pid %d).", parent_pid)

    # Linux fast path: immediate SIGTERM when the parent dies. The poll below remains the
    # cross-platform fallback (and covers the small race where the parent died before this
    # call). On non-Linux platforms this is a no-op.
    _install_linux_parent_death_signal()

    while True:
        await asyncio.sleep(poll_interval)
        if not _parent_alive(parent_pid):
            logger.warning(
                "Parent ResInsight (pid %d) is gone, shutting down service.", parent_pid
            )
            os._exit(0)
