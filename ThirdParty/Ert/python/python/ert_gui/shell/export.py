from __future__ import print_function
from ert.util import IntVector
from ert.enkf.enums import ErtImplType
from ert.enkf.data import EnkfNode
from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import autoCompleteList, splitArguments


class Export(ErtShellCollection):
    DEFAULT_EXPORT_PATH = "export/%s/%s_%%d"

    def __init__(self, parent):
        super(Export, self).__init__("export", parent)
        default_path = Export.DEFAULT_EXPORT_PATH % ("{KEY}", "{KEY}")

        self.addShellFunction(name="FIELD",
                              function=Export.exportFIELD,
                              completer=Export.completeFIELD,
                              help_arguments="<keyword> [%s] [1,4,7-10]" % default_path,
                              help_message="Export parameters; path and realisations in [...] are optional.")

    def supportedFIELDKeys(self):
        ens_config = self.ert().ensembleConfig()
        key_list = ens_config.getKeylistFromImplType(ErtImplType.FIELD)
        return key_list

    @assertConfigLoaded
    def completeFIELD(self, text, line, begidx, endidx):
        arguments = splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []

        return autoCompleteList(text, self.supportedFIELDKeys())

    @assertConfigLoaded
    def exportFIELD(self, line):
        arguments = splitArguments(line)

        if len(arguments) >= 1:
            ens_config = self.ert().ensembleConfig()
            key = arguments[0]
            if key in self.supportedFIELDKeys():
                config_node = ens_config[key]
                if len(arguments) >= 2:
                    path_fmt = arguments[1]
                else:
                    path_fmt = Export.DEFAULT_EXPORT_PATH % (key, key) + ".grdecl"

                if len(arguments) >= 3:
                    range_string = "".join(arguments[2:])
                    iens_list = IntVector.active_list(range_string)
                else:
                    ens_size = self.ert().getEnsembleSize()
                    iens_list = IntVector.createRange(0, ens_size, 1)

                fs_manager = self.ert().getEnkfFsManager()
                fs = fs_manager.getCurrentFileSystem()
                init_file = self.ert().fieldInitFile(config_node)
                if init_file:
                    print('Using init file: %s' % init_file)

                EnkfNode.exportMany(config_node, path_fmt, fs, iens_list, arg=init_file)
            else:
                self.lastCommandFailed("No such FIELD node: %s" % key)
        else:
            self.lastCommandFailed("Expected at least one argument: <keyword> received: '%s'" % line)
