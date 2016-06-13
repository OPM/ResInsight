from ert.enkf.plot import DataFetcher, ObservationGenDataFetcher, BlockObservationDataFetcher, EnsembleDataFetcher
from ert.enkf.plot_data import PcaPlotData
from ert.enkf.enums import RealizationStateEnum, EnkfObservationImplementationType
from ert.enkf import LocalObsdata, LocalObsdataNode, EnkfLinalg, MeasData, ObsData
from ert.util import Matrix, BoolVector, DoubleVector


class PcaDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(PcaDataFetcher, self).__init__(ert)
        self.__prior_singular_values = None

    def fetchSupportedKeys(self):
        summary_keys = EnsembleDataFetcher(self.ert()).getSupportedKeys()

        keys = []
        for key in summary_keys:
            obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()
            if len(obs_keys) > 0:
                keys.append(key)

        keys += BlockObservationDataFetcher(self.ert()).getSupportedKeys()
        keys += ObservationGenDataFetcher(self.ert()).getSupportedKeys()

        return keys


    def truncationOrNumberOfComponents(self, truncation_or_ncomp):
        """ @rtype: (float, int) """
        truncation = -1
        ncomp = -1

        if truncation_or_ncomp < 1:
            truncation = truncation_or_ncomp
        else:
            ncomp = int(truncation_or_ncomp)

        return truncation, ncomp


    def calculatePrincipalComponent(self, fs, local_obsdata, truncation_or_ncomp=3):
        pc = Matrix(1, 1)
        pc_obs = Matrix(1, 1)
        singular_values = DoubleVector()

        state_map = fs.getStateMap()
        ens_mask = BoolVector(False, self.ert().getEnsembleSize())
        state_map.selectMatching(ens_mask, RealizationStateEnum.STATE_HAS_DATA)
        active_list = ens_mask.createActiveList( )

        if len(ens_mask) > 0:
            meas_data = MeasData(ens_mask)
            obs_data = ObsData()

            self.ert().getObservations().getObservationAndMeasureData(fs, local_obsdata, active_list, meas_data, obs_data)

            meas_data.deactivateZeroStdSamples(obs_data)

            active_size = len(obs_data)

            if active_size > 0:
                S = meas_data.createS()
                D_obs = obs_data.createDObs()

                truncation, ncomp = self.truncationOrNumberOfComponents(truncation_or_ncomp)

                obs_data.scale(S, D_obs=D_obs)
                EnkfLinalg.calculatePrincipalComponents(S, D_obs, truncation, ncomp, pc, pc_obs, singular_values)
                if self.__prior_singular_values is None:
                    self.__prior_singular_values = singular_values
                else:
                    for row in range(pc.rows()):
                        factor = singular_values[row]/self.__prior_singular_values[row]
                        pc.scaleRow( row , factor )
                        pc_obs.scaleRow( row , factor )


                return PcaPlotData(local_obsdata.getName(), pc , pc_obs , singular_values)
        return None



    def getAllObsKeys(self):
        observations = self.ert().getObservations()
        summary_obs_keys = observations.getTypedKeylist(EnkfObservationImplementationType.SUMMARY_OBS)
        gen_data_obs_keys =  observations.getTypedKeylist(EnkfObservationImplementationType.GEN_OBS)
        block_obs_keys =  observations.getTypedKeylist(EnkfObservationImplementationType.BLOCK_OBS)

        summary_obs_keys = [key for key in summary_obs_keys]
        gen_data_obs_keys = [key for key in gen_data_obs_keys]
        block_obs_keys = [key for key in block_obs_keys]

        return summary_obs_keys + gen_data_obs_keys + block_obs_keys


    def getObsKeys(self, data_key):
        ensemble_data_fetcher = EnsembleDataFetcher(self.ert())
        block_observation_data_fetcher = BlockObservationDataFetcher(self.ert())
        gen_data_observation_data_fetcher = ObservationGenDataFetcher(self.ert())

        if ensemble_data_fetcher.supportsKey(data_key):
            return self.ert().ensembleConfig().getNode(data_key).getObservationKeys()
        elif block_observation_data_fetcher.supportsKey(data_key):
            return [data_key]
        elif gen_data_observation_data_fetcher.supportsKey(data_key):
            return gen_data_observation_data_fetcher.getAllObsKeysForKey(data_key)


    def filterObsKeys(self, obs_keys, fs):
        active_mask = BoolVector(True, self.ert().getEnsembleSize())
        ert_obs = self.ert().getObservations()

        result = []
        for obs_key in obs_keys:
            obsVector = ert_obs[obs_key]
            if obsVector.hasData(active_mask, fs):
                result.append(obs_key)
        return result


    def fetchData(self, obs_keys, case=None):
        data = {"x": None,
                "y": None,
                "obs_y": None,
                "min_y": None,
                "max_y": None,
                "min_x": None,
                "max_x": None}

        fs = self.ert().getEnkfFsManager().getFileSystem(case)
        obs_keys = self.filterObsKeys(obs_keys, fs)

        step_1 = 0
        step_2 = self.ert().getHistoryLength()

        local_obsdata = LocalObsdata("PCA Observations %s" % case)

        for obs_key in obs_keys:
            if not obs_key in local_obsdata:
                obs_node = LocalObsdataNode(obs_key)
                obs_node.addRange(step_1, step_2)
                local_obsdata.addNode(obs_node)

        if len(local_obsdata) > 0:
            pca_data = self.calculatePrincipalComponent(fs, local_obsdata)

            if pca_data is not None:
                data["x"] = []
                data["y"] = []
                data["obs_y"] = []

                data["min_x"] = 1
                data["max_x"] = len(pca_data)

                component_number = 0
                for pca_vector in pca_data:
                    component_number += 1
                    data["x"].append(component_number)

                    obs_y = pca_vector.getObservation()

                    if data["min_y"] is None or data["min_y"] > obs_y:
                        data["min_y"] = obs_y

                    if data["max_y"] is None or data["max_y"] < obs_y:
                        data["max_y"] = obs_y

                    data["obs_y"].append(obs_y)
                    for index, value in enumerate(pca_vector):
                        if len(data["y"]) == index:
                            data["y"].append([])

                        y = data["y"][index]
                        y.append(value)

                        if data["min_y"] is None or data["min_y"] > value:
                            data["min_y"] = value

                        if data["max_y"] is None or data["max_y"] < value:
                            data["max_y"] = value

        return data
