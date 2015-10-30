from ert.enkf import ErtImplType, GenKwConfig, CustomKWConfig


class KeyManager(object):

    def __init__(self, ert):
        super(KeyManager, self).__init__()
        """
        @type ert: ert.enkf.EnKFMain
        """
        self.__ert = ert

        self.__all_keys = None
        self.__all_keys_with_observations = None
        self.__summary_keys = None
        self.__summary_keys_with_observations = None
        self.__gen_data_keys = None
        self.__gen_kw_keys = None
        self.__custom_kw_keys = None
        self.__misfit_keys = None


    def ert(self):
        """ :rtype:  ert.enkf.EnKFMain """
        return self.__ert

    def ensembleConfig(self):
        """ :rtype: ert.enkf.EnsembleConfig """
        return self.ert().ensembleConfig()

    def summaryKeys(self):
        """ :rtype: list of str """
        if self.__summary_keys is None:
            self.__summary_keys = sorted([key for key in self.ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)], key=lambda k : k.lower())

        return self.__summary_keys

    def summaryKeysWithObservations(self):
        """ :rtype: list of str """
        if self.__summary_keys_with_observations is None:
            self.__summary_keys_with_observations = sorted([key for key in self.summaryKeys() if len(self.ensembleConfig().getNode(key).getObservationKeys()) > 0], key=lambda k : k.lower())

        return self.__summary_keys_with_observations

    def genKwKeys(self):
        """ :rtype: list of str """
        if self.__gen_kw_keys is None:
            gen_kw_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_KW)
            gen_kw_keys = [key for key in gen_kw_keys]

            gen_kw_list = []
            for key in gen_kw_keys:
                enkf_config_node = self.ert().ensembleConfig().getNode(key)
                gen_kw_config = enkf_config_node.getModelConfig()
                assert isinstance(gen_kw_config, GenKwConfig)

                for keyword_index, keyword in enumerate(gen_kw_config):
                    gen_kw_list.append("%s:%s" % (key, keyword))

                    if gen_kw_config.shouldUseLogScale(keyword_index):
                        gen_kw_list.append("LOG10_%s:%s" % (key, keyword))

            self.__gen_kw_keys = sorted(gen_kw_list, key=lambda k : k.lower())

        return self.__gen_kw_keys


    def customKwKeys(self):
        """ :rtype: list of str """
        if self.__custom_kw_keys is None:
            custom_kw_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.CUSTOM_KW)

            keys = []
            for name in custom_kw_keys:
                enkf_config_node = self.ert().ensembleConfig().getNode(name)
                custom_kw_config = enkf_config_node.getModelConfig()
                assert isinstance(custom_kw_config, CustomKWConfig)

                for key in custom_kw_config:
                    keys.append("%s:%s" % (name, key))

            self.__custom_kw_keys = sorted([key for key in keys], key=lambda k : k.lower())

        return self.__custom_kw_keys


    def genDataKeys(self):
        """ :rtype: list of str """
        if self.__gen_data_keys is None:
            gen_data_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_DATA)
            gen_data_list = []
            for key in gen_data_keys:
                enkf_config_node = self.ert().ensembleConfig().getNode(key)
                gen_data_config = enkf_config_node.getDataModelConfig()

                for report_step in range(self.ert().getHistoryLength() + 1):
                    if gen_data_config.hasReportStep(report_step):
                        gen_data_list.append("%s@%d" % (key, report_step))
            self.__gen_data_keys = sorted(gen_data_list, key=lambda k : k.lower())

        return self.__gen_data_keys

    def misfitKeys(self, sort_keys=True):
        """ @rtype: list of str """
        if self.__misfit_keys is None:
            keys = []
            for obs_vector in self.ert().getObservations():
                key = "MISFIT:%s" % obs_vector.getObservationKey()
                keys.append(key)

            keys.append("MISFIT:TOTAL")

            self.__misfit_keys = sorted(keys, key=lambda k : k.lower()) if sort_keys else keys

        return self.__misfit_keys


    def allDataTypeKeys(self):
        """ :rtype: list of str """
        if self.__all_keys is None:
            self.__all_keys = self.summaryKeys() + self.genKwKeys() + self.customKwKeys() + self.genDataKeys()

        return self.__all_keys

    def allDataTypeKeysWithObservations(self):
        """ :rtype: list of str """
        if self.__all_keys_with_observations is None:
            self.__all_keys_with_observations = self.summaryKeysWithObservations()

        return self.__all_keys_with_observations

    def isKeyWithObservations(self, key):
        """ :rtype: bool """
        return key in self.allDataTypeKeysWithObservations()

    def isSummaryKey(self, key):
        """ :rtype: bool """
        return key in self.summaryKeys()

    def isGenKwKey(self, key):
        """ :rtype: bool """
        return key in self.genKwKeys()

    def isCustomKwKey(self, key):
        """ :rtype: bool """
        return key in self.customKwKeys()

    def isGenDataKey(self, key):
        """ :rtype: bool """
        return key in self.genDataKeys()

    def isMisfitKey(self, key):
        """ :rtype: bool """
        return key in self.misfitKeys()