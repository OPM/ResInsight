from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class RunModelMixin(ModelMixin):

    def startSimulations(self):
        raise AbstractMethodError(self, "startSimulations")

    def killAllSimulations(self):
        raise AbstractMethodError(self, "startSimulations")