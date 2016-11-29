from ert.enkf import EnkfPrototype


class ErtLog(object):
    _init = EnkfPrototype("void ert_log_init_log(int, char*, bool)", bind=False)
    _write_log = EnkfPrototype("void ert_log_add_message_py(int, char*)", bind=False)
    _get_filename = EnkfPrototype("char* ert_log_get_filename()", bind=False)

    @classmethod
    def init(cls, log_level, log_filename, verbose):
        cls._init(log_level, log_filename, verbose)

    @classmethod
    def log(cls, log_level, message):
        cls._write_log(log_level, message)

    @classmethod
    def getFilename(cls):
        """ @rtype: string """
        return cls._get_filename()
