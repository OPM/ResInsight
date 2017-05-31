#!/usr/bin/env python
import sys
import time 
from ert.enkf import EnKFMain
from ert.enkf.enums import ErtImplType


# This will instantiate the EnkFMain object and create a handle to
# "everything" ert related for this instance.
ert = EnKFMain( sys.argv[1] )


# Ask the EnKFMain instance how many realisations it has. Observe that
# the answer to this question is just the value of the
# NUM_REALISATIONS setting in the configuration file.
print("This instance has %d realisations" % ert.getEnsembleSize())


# Get the ensemble configuration object, and ask for all GEN_KW keys:
ens_config = ert.ensembleConfig( )
for key in ens_config.getKeylistFromImplType(ErtImplType.GEN_KW):
    config_node = ens_config[key]

    # "Downcast" to GEN_KW configuration.
    gen_kw_config = config_node.getModelConfig( )
    print("%s : %s" % (key , gen_kw_config.getKeyWords( )))

