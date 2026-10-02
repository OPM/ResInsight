"""Version guard and shared error type for the taskmaestro helper.

The helper relies on a few taskmaestro internals (YAML loader models,
``import_class`` and the CLI describe implementation), so it only supports the
0.3 series.
"""

from __future__ import annotations

MIN_TASKMAESTRO_VERSION = (0, 3)
MAX_TASKMAESTRO_VERSION = (0, 4)


class HelperError(Exception):
    """An error reported to ResInsight as ``{"status": "invalid", "error": ...}``."""

    def __init__(
        self, message: str, *, task: str | None = None, field: str | None = None
    ) -> None:
        super().__init__(message)
        self.message = message
        self.task = task
        self.field = field

    def to_json(self) -> dict[str, str | None]:
        return {"message": self.message, "task": self.task, "field": self.field}


def _version_tuple(version: str) -> tuple[int, ...]:
    parts: list[int] = []
    for part in version.split("."):
        digits = "".join(ch for ch in part if ch.isdigit())
        if not digits:
            break
        parts.append(int(digits))
    return tuple(parts)


def taskmaestro_version() -> str:
    import taskmaestro

    return str(getattr(taskmaestro, "__version__", "0"))


def require_taskmaestro() -> None:
    """Raise HelperError unless a supported taskmaestro version is installed."""
    try:
        version = taskmaestro_version()
    except ImportError as exc:
        raise HelperError(f"taskmaestro is not installed: {exc}") from exc
    parsed = _version_tuple(version)
    if parsed < MIN_TASKMAESTRO_VERSION or parsed >= MAX_TASKMAESTRO_VERSION:
        raise HelperError(
            f"taskmaestro {version} is not supported; "
            "install taskmaestro>=0.3,<0.4 to edit workflows"
        )
