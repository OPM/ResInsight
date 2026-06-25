from enum import Enum


class Service(str, Enum):
    GENERAL = "general"
    SUMO = "sumo"


class ServiceLayerException(Exception):
    def __init__(self, message: str, service: Service):
        super().__init__(f"{message} [service={service.value}]")
        self.message = message
        self.service = service

    def get_error_type_str(self) -> str:
        return self.__class__.__name__

    def __str__(self) -> str:
        return f"{self.message} [service={self.service.value}]"

class AuthorizationError(ServiceLayerException):
    """
    Raised when a user is not authorized to perform some action.
    """


class ServiceTimeoutError(ServiceLayerException):
    """
    Some underlying service timed out, eg. Sumo timed out.
    """


class ServiceUnavailableError(ServiceLayerException):
    """
    Some underlying service is unavailable, eg. Sumo is down.
    """


class ServiceRequestError(ServiceLayerException):
    """
    An error occurred while make a call/request to some underlying service, eg. when making an REST call to a user session service
    """


class InvalidDataError(ServiceLayerException):
    """
    Raised when the source data is invalid for a service operation, eg. we get invalid data from Sumo.
    """


class NoDataError(ServiceLayerException):
    """
    Raised when some operation expects to find data, but is unable to find any matching data.
    """


class MultipleDataMatchesError(ServiceLayerException):
    """
    Raised when som operation expects to find exactly one data item, but actually finds multiple items.
    """


class InvalidParameterError(ServiceLayerException):
    """
    Raised when an invalid parameter value is passed to a service operation.
    """
