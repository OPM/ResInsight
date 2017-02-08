from cwrap import BaseCClass

from ert.enkf import EnkfPrototype
from ert.util import Matrix
from ert.enkf.plot_data import PcaPlotVector


class PcaPlotData(BaseCClass):
    TYPE_NAME = "pca_plot_data"

    _alloc               = EnkfPrototype("void* pca_plot_data_alloc(char*, matrix, matrix , double_vector)", bind = False)
    _component_count     = EnkfPrototype("int   pca_plot_data_get_size(pca_plot_data)")
    _realization_count   = EnkfPrototype("int   pca_plot_data_get_ens_size(pca_plot_data)")
    _get                 = EnkfPrototype("pca_plot_vector_ref pca_plot_data_iget_vector(pca_plot_data, int)")
    _get_name            = EnkfPrototype("char* pca_plot_data_get_name(pca_plot_data)")
    _get_singular_values = EnkfPrototype("double_vector_ref pca_plot_data_get_singular_values(pca_plot_data)")
    _free                = EnkfPrototype("void  pca_plot_data_free(pca_plot_data)")

    def __init__(self, name, principal_component_matrix, observation_principal_component_matrix, singular_values):
        assert isinstance(name, str)
        assert isinstance(principal_component_matrix, Matrix)
        assert isinstance(observation_principal_component_matrix, Matrix)

        c_pointer = self._alloc(name, principal_component_matrix, observation_principal_component_matrix , singular_values)
        super(PcaPlotData, self).__init__(c_pointer)

    def componentCount(self):
        return len(self)
    def realizationCount(self):
        return self._realization_count()
    def name(self):
        return self._get_name()

    def __len__(self):
        """ @rtype: int """
        return self._component_count()


    def __getitem__(self, index):
        """ @rtype: PcaPlotVector """
        assert isinstance(index, int)
        return self._get(index).setParent(self)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1

    def getSingularValues(self):
        """ @rtype: DoubleVector """
        return self._get_singular_values().setParent(self)

    def free(self):
        self._free()

    def __repr__(self):
        nm = self.name()
        cc = len(self)
        rc = self.realizationCount()
        ad = self._ad_str()
        fmt = 'PcaPlotData(name = %s, components = %d, realizations = %d) %s'
        return fmt % (nm, cc, rc, ad)
