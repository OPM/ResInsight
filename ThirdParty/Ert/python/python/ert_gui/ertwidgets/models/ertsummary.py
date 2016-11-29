from ert.enkf.enums.enkf_obs_impl_type_enum import EnkfObservationImplementationType
from ert.enkf.enums.enkf_var_type_enum import EnkfVarType
from ert_gui import ERT

class ErtSummary(object):

    def getForwardModels(self):
        """ @rtype: list of str """
        forward_model  = ERT.ert.getModelConfig().getForwardModel()
        return [job for job in forward_model.joblist()]

    def getParameters(self):
        """ @rtype: list of str """
        parameters = ERT.ert.ensembleConfig().getKeylistFromVarType(EnkfVarType.PARAMETER)
        return sorted([parameter for parameter in parameters], key=lambda k : k.lower())


    def getObservations(self):
        """ @rtype: list of str """
        gen_obs = ERT.ert.getObservations().getTypedKeylist(EnkfObservationImplementationType.GEN_OBS)


        summary_obs = ERT.ert.getObservations().getTypedKeylist(EnkfObservationImplementationType.SUMMARY_OBS)

        keys = []
        summary_keys_count = {}
        summary_keys = []
        for key in summary_obs:
            data_key = ERT.ert.getObservations()[key].getDataKey()

            if not data_key in summary_keys_count:
                summary_keys_count[data_key] = 1
                summary_keys.append(data_key)
            else:
                summary_keys_count[data_key] += 1

            if key == data_key:
                keys.append(key)
            else:
                keys.append("%s [%s]" % (key, data_key))

        # keys = []
        # for key in summary_keys:
        #     count = summary_keys_count[key]
        #     if count > 1:
        #         #keys.append("%s (%d)" % (key, count))
        #         keys.append("%s" % key)
        #     else:
        #         keys.append(key)

        obs_keys = [observation for observation in gen_obs] + summary_keys
        return sorted(obs_keys, key=lambda k : k.lower())






