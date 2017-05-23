from __future__ import absolute_import, division, print_function, unicode_literals

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

import cProfile
import pstats
import sys


class Profiler(object):

    __profiler = None
    """ :type: Profile """

    @classmethod
    def startProfiler(cls, subcalls=True, builtins=True):
        cls.__profiler = cProfile.Profile()
        cls.__profiler.enable(subcalls=subcalls, builtins=builtins)

    @classmethod
    def stopProfiler(cls, sort_method="cumulative"):
        if cls.__profiler is not None:
            cls.__profiler.disable()
            stream = StringIO()
            stats_printer = pstats.Stats(cls.__profiler, stream=stream).sort_stats(sort_method)
            stats_printer.print_stats()
            cls.__profiler = None
            print(stream.getvalue())
        else:
            sys.stderr.write("WARNING: Profiler has not been started!\n")

