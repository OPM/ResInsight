from ert_gui.shell import assertConfigLoaded
from ert_gui.shell.libshell import ShellCollection


class ErtShellCollection(ShellCollection):
    def __init__(self, name, parent=None, description="No description available"):
        super(ErtShellCollection, self).__init__(name, parent, description)

    def shellContext(self):
        """ @rtype: ert_gui.shell.ErtShellContext """
        return super(ErtShellCollection, self).shellContext()

    @assertConfigLoaded
    def ert(self):
        """ @rtype: ert.enkf.enkf_main.EnKFMain """
        return self.shellContext().ert()

    def columnize(self, items, displaywidth=80):
        self.shellContext().shell().columnize(items, displaywidth=displaywidth)