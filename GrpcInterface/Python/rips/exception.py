from typing import Optional


class RipsError(Exception):
    """Exception raised by the rips Python client.

    Carries optional gRPC context so callers can branch on code/details
    when a remote ResInsight call fails. Backwards compatible with the
    legacy ``RipsError("message")`` form.
    """

    def __init__(
        self,
        message: object,
        *,
        code: Optional[object] = None,
        details: Optional[str] = None,
        location: Optional[str] = None,
    ) -> None:
        super().__init__(message)
        self.code = code
        self.details = details
        self.location = location

    @classmethod
    def from_rpc_error(
        cls, rpc_error: Exception, location: Optional[str] = None
    ) -> "RipsError":
        """Build a RipsError from a grpc.RpcError-like object."""
        code_fn = getattr(rpc_error, "code", None)
        details_fn = getattr(rpc_error, "details", None)
        code = code_fn() if callable(code_fn) else None
        details = details_fn() if callable(details_fn) else ""

        parts = ["ResInsight gRPC call failed"]
        if code is not None:
            parts.append(f"({code})")
        if details:
            parts.append(f"- {details}")
        if location:
            parts.append(f"[server={location}]")
        return cls(
            " ".join(parts),
            code=code,
            details=details or None,
            location=location,
        )
