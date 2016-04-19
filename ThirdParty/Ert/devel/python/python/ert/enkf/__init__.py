#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.


import ert.cwrap.clib as clib
from ert.cwrap.metacwrap import Prototype
import ert.util
import ert.geo
import ert.ecl
import ert.rms
import ert.analysis
import ert.sched
import ert.config
import ert.job_queue

class EnkfPrototype(Prototype):
    lib = clib.ert_load("libenkf")

    def __init__(self, prototype, bind=True):
        super(EnkfPrototype, self).__init__(EnkfPrototype.lib, prototype, bind=bind)

        

ENKF_LIB = clib.ert_load("libenkf")

from .enums import *

from .node_id import NodeId

from .enkf_linalg import EnkfLinalg
from .util import TimeMap
from .state_map import StateMap
from .summary_key_set import SummaryKeySet
from .summary_key_matcher import SummaryKeyMatcher
from .custom_kw_config_set import CustomKWConfigSet
from .enkf_fs import EnkfFs

from .ert_workflow_list import ErtWorkflowList
from .active_list import ActiveList
from .config import *
from .data import *

from .local_dataset import LocalDataset
from .local_obsdata_node import LocalObsdataNode
from .local_obsdata import LocalObsdata
from .local_ministep import LocalMinistep
from .local_updatestep import LocalUpdateStep

from .observations import *

from .obs_block import ObsBlock
from .obs_data import ObsData
from .meas_block import MeasBlock
from .meas_data import MeasData

from .analysis_iter_config import AnalysisIterConfig
from .analysis_config import AnalysisConfig
from .ecl_config import EclConfig

from .ensemble_config import EnsembleConfig
from .enkf_obs import EnkfObs
from .enkf_state import EnKFState
from .ert_template import ErtTemplate
from .ert_templates import ErtTemplates
from .local_config import LocalConfig
from .model_config import ModelConfig
from .plot_config import PlotConfig
from .site_config import SiteConfig
from .runpath_list import RunpathList, RunpathNode
from .hook_workflow import HookWorkflow
from .hook_manager import HookManager

from .enkf_simulation_runner import EnkfSimulationRunner
from .enkf_fs_manager import EnkfFsManager
from .run_arg import RunArg
from .ert_run_context import ErtRunContext
from .enkf_main import EnKFMain
from .ert_log import ErtLog
from .forward_load_context import ForwardLoadContext

from ert.job_queue import ErtScript as ErtScript
from ert.job_queue import ErtPlugin as ErtPlugin, CancelPluginException as CancelPluginException
