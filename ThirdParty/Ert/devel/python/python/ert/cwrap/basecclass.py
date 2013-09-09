import ctypes
from ert.cwrap import CNamespace


class BaseCClass(object):
    namespaces = {}

    def __init__(self, c_pointer, parent=None, is_reference=False):
        self._c_pointer = None
        self._parent = None
        self._is_reference = False

        if not c_pointer > 0:
            raise ValueError("Must have a valid pointer value!")

        self._c_pointer = c_pointer
        self._parent = parent
        self._is_reference = is_reference


    @classmethod
    def cNamespace(cls):
        """ @rtype: CNamespace """
        if not BaseCClass.namespaces.has_key(cls):
            BaseCClass.namespaces[cls] = CNamespace(cls.__name__)
        return BaseCClass.namespaces[cls]

    @classmethod
    def from_param(cls, c_class_object):
        if not isinstance(c_class_object, BaseCClass):
            raise ValueError("c_class_object must be an BaseCClass instance!")

        if c_class_object is None or not hasattr(c_class_object, "_c_pointer"):
            return ctypes.c_void_p()
        else:
            return ctypes.c_void_p(c_class_object._c_pointer)

    @classmethod
    def createPythonObject(cls, c_pointer):
        if not c_pointer == 0:
            new_obj = cls.__new__(cls)
            BaseCClass.__init__(new_obj, c_pointer=c_pointer, parent=None, is_reference=False)
            return new_obj
        return None

    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        if not c_pointer == 0:
            new_obj = cls.__new__(cls)
            BaseCClass.__init__(new_obj, c_pointer=c_pointer, parent=parent, is_reference=True)
            return new_obj
        return None

    def setParent(self, parent=None):
        if self._is_reference:
            self._parent = parent
        else:
            raise UserWarning("Can only set parent on reference types!")

        return self

    def isReference(self):
        """ @rtype: bool """
        return self._is_reference

    def parent(self):
        return self._parent

    def free(self):
        raise NotImplementedError("A CClass requires a free method implementation!")

    def __del__(self):
        if self.free is not None:
            if hasattr(self, "_is_reference") and not self._is_reference:
                # Important to check the c_pointer; in the case of failed object creation
                # we can have a Python object with c_pointer == None.
                if self._c_pointer > 0:
                    self.free()