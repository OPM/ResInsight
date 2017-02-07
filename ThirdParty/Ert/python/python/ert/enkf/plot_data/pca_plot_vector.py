from cwrap import BaseCClass

from ert.enkf import EnkfPrototype
from ert.util import Matrix


class PcaPlotVector(BaseCClass):
    TYPE_NAME = "pca_plot_vector"

    _alloc              = EnkfPrototype("void*  pca_plot_vector_alloc(int, matrix, matrix)", bind = False)
    _size               = EnkfPrototype("int    pca_plot_vector_get_size(pca_plot_vector)")
    _get                = EnkfPrototype("double pca_plot_vector_iget_sim_value(pca_plot_vector, int)")
    _get_obs            = EnkfPrototype("double pca_plot_vector_get_obs_value(pca_plot_vector)")
    _get_singular_value = EnkfPrototype("double pca_plot_vector_get_singular_value(pca_plot_vector)")
    _free               = EnkfPrototype("void   pca_plot_vector_free(pca_plot_vector)")


    def __init__(self, component, principal_component_matrix, observation_principal_component_matrix):
        assert isinstance(component, int)
        assert isinstance(principal_component_matrix, Matrix)
        assert isinstance(observation_principal_component_matrix, Matrix)

        c_pointer = self._alloc(component, principal_component_matrix, observation_principal_component_matrix)
        super(PcaPlotVector, self).__init__(c_pointer)


    def __len__(self):
        """ @rtype: int """
        return self._size()


    def __getitem__(self, index):
        """
        @type index: int
        @rtype: float 
        """
        assert isinstance(index, int)
        return self._get(index)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1

    def getObservation(self):
        """ @rtype: float """
        return self._get_obs()

    def getSingularValue(self):
        """ @rtype: float """
        return self._get_singular_value()
        

    def free(self):
        self._free()

    def __repr__(self):
        si = len(self)
        ob = self.getObservation()
        sv = self.getSingularValue()
        ad = self._ad_str()
        fmt = 'PcaPlotVector(size = %d, observation = %f, singular = %f) %s'
        return fmt % (si, ob, sv, ad)
