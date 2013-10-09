from ert.enkf import EnKFMain

#http://stackoverflow.com/questions/13789235/how-to-initialize-singleton-derived-object-once
class Singleton(type):
    __instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls.__instances:
            cls.__instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls.__instances[cls]

class ErtConnector(object):
    __metaclass__ = Singleton

    """
    ErtConnector uses the Singleton pattern.
    Every time you instantiate an ErtConnector or a subclass of ErtConnector the same instance is returned.
    """
    __ert = None

    def __init__(self, *args, **kwargs):
        super(ErtConnector, self).__init__(*args, **kwargs)

    def __new__(cls, *args):
        if cls is ErtConnector:
            return None

        return super(ErtConnector, cls).__new__(cls)


    def ert(self):
        """ @rtype: EnKFMain """
        return ErtConnector.__ert

    @classmethod
    def setErt(cls, ert):
        ErtConnector.__ert = ert
