from ert_gui.shell import ErtShellCollection
from ert_gui.shell.libshell import splitArguments, getPossibleFilenameCompletions, extractFullArgument


import ctypes
import os

NO_SORT = 0
STRING_SORT = 1
OFFSET_SORT = 2

import ert.cwrap.clib as clib
UTIL_LIB = clib.ert_load("libert_util")

UTIL_LIB.block_fs_is_mount.restype = ctypes.c_bool
UTIL_LIB.block_fs_mount.restype = ctypes.c_void_p
UTIL_LIB.block_fs_alloc_filelist.restype = ctypes.c_void_p
UTIL_LIB.block_fs_close.restype = ctypes.c_void_p

UTIL_LIB.vector_get_size.restype = ctypes.c_int
UTIL_LIB.vector_iget_const.restype = ctypes.c_void_p
UTIL_LIB.vector_free.restype = ctypes.c_void_p

UTIL_LIB.user_file_node_get_filename.restype = ctypes.c_char_p
UTIL_LIB.user_file_node_get_data_size.restype = ctypes.c_int
UTIL_LIB.user_file_node_get_node_offset.restype = ctypes.c_long

class Storage(ErtShellCollection):
    def __init__(self, parent):
        super(Storage, self).__init__("storage", parent)

        self.addShellFunction(name="ls",
                              function=Storage.ls,
                              completer=Storage.completeLs,
                              help_arguments="<block_file> [wildcard_pattern]",
                              help_message="Will list all elements in 'block_file' matching optional 'wildcard_pattern'")

    def ls(self, line):
        arguments = splitArguments(line)

        if len(arguments) == 0:
            self.lastCommandFailed("A 'block file' is required.")
        else:
            block_file = arguments[0]
            pattern = None if len(arguments) == 1 else arguments[1]

            if not os.path.isfile(block_file):
                self.lastCommandFailed("The path: '%s' is not a file." % block_file)
            else:
                if not UTIL_LIB.block_fs_is_mount(block_file):
                    _, filename = os.path.split(block_file)
                    self.lastCommandFailed("The file: '%s' is not a block mount file." % filename)
                else:
                    block_fs = UTIL_LIB.block_fs_mount(block_file, 1, 0, 1, 0, False, True, False)
                    files = UTIL_LIB.block_fs_alloc_filelist(block_fs, pattern, OFFSET_SORT, False)

                    file_count = UTIL_LIB.vector_get_size(files)

                    if file_count > 0:
                        fmt = " %-40s %10d %10d"
                        print(" %-40s %10s %10s" % ("Keyword", "Size", "Offset"))

                        for index in range(file_count):
                            node = UTIL_LIB.vector_iget_const(files, index)
                            node_filename = UTIL_LIB.user_file_node_get_filename(node)
                            node_size = UTIL_LIB.user_file_node_get_data_size(node)
                            node_offset = UTIL_LIB.user_file_node_get_node_offset(node)
                            print(fmt % (node_filename, node_size, node_offset))

                    UTIL_LIB.vector_free(files)
                    UTIL_LIB.block_fs_close(block_fs)


    def completeLs(self, text, line, begidx, endidx):
        arguments = splitArguments(line)
        last_argument = extractFullArgument(line, endidx)

        if len(arguments) == 1 and len(text) == 0:
            ert = self.ert()
            if ert is not None:
                return [ert.getModelConfig().getEnspath() + os.path.sep]
            else:
                return getPossibleFilenameCompletions("")
        elif len(arguments) == 2 and len(last_argument) > 0:
            return getPossibleFilenameCompletions(last_argument)
        elif len(arguments) == 3 and len(text) > 0:
            return [] # pattern completion...
        else:
            return []
