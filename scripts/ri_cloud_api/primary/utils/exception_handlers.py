import logging

from fastapi import FastAPI
from fastapi.requests import Request
from fastapi.responses import JSONResponse
from starlette.status import HTTP_500_INTERNAL_SERVER_ERROR

from ri_cloud_services.service_exceptions import ServiceLayerException

logger = logging.getLogger(__name__)


def _service_layer_exception_handler(request: Request, exc: ServiceLayerException) -> JSONResponse:
    logger.error(
        f"[EXC] Service exception in {request.method} {request.url.path} "
        f"-> {exc.get_error_type_str()}: {str(exc)}",
        exc_info=exc,
    )
    return JSONResponse(
        status_code=HTTP_500_INTERNAL_SERVER_ERROR,
        content={
            "error": {
                "type": exc.get_error_type_str(),
                "message": exc.message,
                "service": exc.service,
            }
        },
    )


def add_exception_handlers(app: FastAPI) -> None:
    app.add_exception_handler(ServiceLayerException, _service_layer_exception_handler)  # type: ignore
