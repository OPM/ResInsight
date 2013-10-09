from weakref import *


class weak_callable(object):
    def __init__(self, obj, func):
        self.__object = obj
        self.__method = func

    def __call__(self, *args, **kws):
        if self.__object is not None:
            return self.__method(self.__object(), *args, **kws)
        else:
            return self.__method(*args, **kws)

    def __getattr__(self, attr):
        if attr == 'im_self':
            return self.__object
        if attr == 'im_func':
            return self.__method
        raise AttributeError, attr


class WeakMethod(object):
    """ Wraps a function or, more importantly, a bound method, in
    a way that allows a bound method's object to be GC'd, while
    providing the same interface as a normal weak reference. """

    def __init__(self, func):
        try:
            self.__object = ref(func.im_self)
            self.__method = func.im_func
        except AttributeError:
            # It's not a bound method.
            self.__object = None
            self.__method = func

    def __call__(self):
        if self.isDead():
            return None

        return weak_callable(self.__object, self.__method)

    def __eq__(self, other):
        if isinstance(other, WeakMethod):
            return super(WeakMethod, self).__eq__(other)
        else:
            try:
                o = ref(other.im_self)
                m = other.im_func
                return self.__object == o and self.__method == m
            except AttributeError:
                # It's not a bound method.
                return self.__method == other

    def __str__(self):
        dead = ""
        if self.isDead():
            dead = "Dead "

        binding = "bound"

        if self.__object is None:
            binding = "unbound"

        return "%sWeakMethod for %s function. " % (dead, binding)  + super(WeakMethod, self).__str__()


    def isDead(self):
        return self.__object is not None and self.__object() is None