from ert.enkf import ErtImplType, CustomKWConfig, EnkfStateType, RealizationStateEnum
from ert.enkf.data.enkf_node import EnkfNode
from ert.enkf.export.custom_kw_collector import CustomKWCollector
from ert.enkf.node_id import NodeId
from ert.util import BoolVector
from ert_gui.shell import ShellFunction, assertConfigLoaded, extractFullArgument, autoCompleteListWithSeparator
from ert_gui.shell.shell_tools import matchItems


class CustomKWKeys(ShellFunction):
    def __init__(self, shell_context):
        super(CustomKWKeys, self).__init__("custom_kw", shell_context)
        self.addHelpFunction("list", None, "List all CustomKW keys.")
        self.addHelpFunction("print", "<key>", "Print all realization data for the specified key.")


    def fetchSupportedKeys(self):
        custom_kw_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.CUSTOM_KW)

        keys = []
        for name in custom_kw_keys:
            enkf_config_node = self.ert().ensembleConfig().getNode(name)
            custom_kw_config = enkf_config_node.getModelConfig()
            assert isinstance(custom_kw_config, CustomKWConfig)

            for key in custom_kw_config:
                keys.append("%s:%s" % (name, key))

        return keys

    @assertConfigLoaded
    def do_list(self, line):
        self.columnize(self.fetchSupportedKeys())


    @assertConfigLoaded
    def do_print(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one CustomKW key")
            return False

        case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()

        data = CustomKWCollector.loadAllCustomKWData(self.ert(), case_name, keys)
        print(data)


    @assertConfigLoaded
    def complete_print(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())