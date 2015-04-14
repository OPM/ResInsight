from ert_gui.models import Observable

class AbstractMethodError(NotImplementedError):
    def __init__(self, obj, function_name):
        super(AbstractMethodError, self).__init__("Class %s has not implemented support for %s()" % (obj.__class__.__name__, function_name))


class ModelMixin(object):
    def __init__(self, *args, **kwargs):
        pass
        # print("%s init" % self.__class__.__name__)

    def __new__(cls, *args, **kwargs):
        # print("Construct: %s" % cls.__name__)
        obj = super(ModelMixin, cls).__new__(cls)
        obj.__observable = Observable(cls.__name__)
        obj.registerDefaultEvents()
        return obj


    def registerDefaultEvents(self):
        """Register all events that applicable to the model. """
        pass


    def observable(self):
        """ @rtype: Observable """
        return self.__observable

