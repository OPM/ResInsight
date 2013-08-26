#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ensemble.py' is part of ERT - Ensemble based Reservoir Tool. 
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


# ----------------------------------------------------------------------------------------------
# Ensemble tab
# ----------------------------------------------------------------------------------------------
from PyQt4 import QtCore
from ert_gui.widgets.spinnerwidgets import IntegerSpinner
from parameters.parameterpanel import ParameterPanel
from parameters.parametermodels import SummaryModel, DataModel, FieldModel, KeywordModel
import ert.ert.enums as enums
import ert.enkf.enkf_config_node
import ert.enkf.gen_kw_config
import ert.enkf.gen_data_config

def createEnsemblePage(configPanel, parent):
    configPanel.startPage("Ensemble")

    r = configPanel.addRow(IntegerSpinner(parent, "Number of realizations", "config/ensemble/num_realizations", 1, 10000))

    r.initialize = lambda ert : ert.main.ens_size
    r.getter = lambda ert : ert.main.ens_size
    r.setter = lambda ert, value : ert.main.resize_ensemble( value)

    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("ensembleResized()"))


    configPanel.startGroup("Parameters")
    r = configPanel.addRow(ParameterPanel(parent, "", "")) # no help file necessary
    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("ensembleUpdated()"))


    def getEnsembleParameters(ert):
        keys = ert.main.ensemble_config.alloc_keylist

        parameters = []
        for key in keys:
            node = ert.main.ensemble_config.get_node( key)
            type = node.get_impl_type
            data = node.get_ref

            model = None
            if type == FieldModel.TYPE:
                model = FieldModel(key)

                field_type = node.field_model.get_type
                field_type = enums.field_type[field_type]
                model["type"] = field_type

                truncation = node.field_model.get_truncation_mode

                if truncation & enums.truncation_type.TRUNCATE_MAX:
                    model["max"] = node.field_model.get_truncation_max

                if truncation & enums.truncation_type.TRUNCATE_MIN:
                    model["min"] = node.field_model.get_truncation_min

                model["init"] = node.field_model.get_init_transform_name
                model["output"] = node.field_model.get_output_transform_name
                
                model["init_files"] = node.get_init_file_fmt
                model["min_std"] = node.get_min_std_file
                model["enkf_outfile"] = node.get_enkf_outfile
                model["enkf_infile"] = node.get_enkf_infile

            elif type == DataModel.TYPE:
                model = DataModel(key)

                output_format_value = node.data_model.get_output_format
                output_format = enums.gen_data_file_format.resolveValue(output_format_value)

                input_format_value = node.data_model.get_input_format
                input_format = enums.gen_data_file_format.resolveValue(input_format_value)

                template_file = node.data_model.get_template_file
                template_key = node.data_model.get_template_key
                init_file_fmt = node.get_init_file_fmt

                model["output_format"] = output_format
                model["input_format"] = input_format
                model["template_file"] = template_file
                model["template_key"] = template_key
                model["init_file_fmt"] = init_file_fmt

                min_std = node.get_min_std_file
                enkf_outfile = node.get_enkf_outfile
                enkf_infile = node.get_enkf_infile



                model["min_std"] = min_std
                model["enkf_outfile"] = enkf_outfile
                model["enkf_infile"] = enkf_infile

            elif type == KeywordModel.TYPE:
                model = KeywordModel(key)
                model["min_std"] = node.get_min_std_file
                model["enkf_outfile"] = node.get_enkf_outfile
                model["template"] = node.keyword_model.get_template_file
                model["init_file"] = node.get_init_file_fmt
                model["parameter_file"] = node.keyword_model.get_parameter_file
            elif type == SummaryModel.TYPE:
                model = SummaryModel(key)
            else:
                pass #Unknown type

            #model.setValid(ert.main.enkf_config_node.is_valid)

            parameters.append(model)

        return parameters

    def removeParameter(ert, parameter_key):
        ert.main.del_node(ert.main, parameter_key)

    def insertParameter(ert, parameter):
        key = parameter.getName()
        if parameter.getType() == FieldModel.TYPE:
            grid = ert.main.ecl_config.get_grid
            node = ert.main.ensemble_config.add_field( key, grid)
            parameter.setValid(ert.main.enkf_config_node.is_valid)
        elif parameter.getType() == DataModel.TYPE:
            node = ert.main.ensemble_config.add_gen_data( key)
            parameter.setValid(ert.main.enkf_config_node.is_valid)
        elif parameter.getType() == KeywordModel.TYPE:
            node = ert.main.ensemble_config.add_gen_kw( key)
            parameter.setValid(ert.main.enkf_config_node.is_valid)
        elif parameter.getType() == SummaryModel.TYPE:
            parameter.setValid(True)
            b = ert.main.ensemble_config.add_summary( key)
            return b > 0 #0 == NULL 
        else:
            print "Unknown type: ", parameter
            return False

        return True

    def updateParameter(ert, parameter_model):
        key  = parameter_model.getName()
        node = ert.main.ensemble_config.get_node( key)
        
        if isinstance(parameter_model, FieldModel):
            type = parameter_model["type"]

            minimum = parameter_model["min"]
            maximum = parameter_model["max"]
            truncate = truncation_type.resolveTruncationType(minimum, maximum)

            if minimum == "":
                minimum = 0.0

            if maximum == "":
                maximum = 0.0

            if type == field_type.ECLIPSE_RESTART: #dynamic
                ert.main.enkf_config_node.update_state_field(node,
                                                             truncate.value(),
                                                             float(minimum),
                                                             float(maximum))
            elif type == field_type.ECLIPSE_PARAMETER: #parameter
                ert.main.enkf_config_node.update_parameter_field(node,
                                                                 ert.nonify(parameter_model["enkf_outfile"]),
                                                                 ert.nonify(parameter_model["init_files"]),
                                                                 ert.nonify(parameter_model["min_std"]),
                                                                 truncate.value(),
                                                                 float(minimum),
                                                                 float(maximum),
                                                                 parameter_model["init"],
                                                                 parameter_model["output"])
            elif type == field_type.GENERAL: #general
                ert.main.enkf_config_node.update_general_field(node,
                                                               ert.nonify(parameter_model["enkf_outfile"]),
                                                               ert.nonify(parameter_model["enkf_infile"]),
                                                               ert.nonify(parameter_model["init_files"]),
                                                               ert.nonify(parameter_model["min_std"]),
                                                               truncate.value(),
                                                               float(minimum),
                                                               float(maximum),
                                                               parameter_model["init"],
                                                               None,
                                                               parameter_model["output"])

            parameter_model.setValid(ert.main.enkf_config_node.is_valid)

        elif isinstance(parameter_model, KeywordModel):
            enkf_outfile_fmt = parameter_model["enkf_outfile"]
            template_file = parameter_model["template"]
            parameter_file = parameter_model["parameter_file"]
            min_std_file = parameter_model["min_std"]
            init_file_fmt = parameter_model["init_files"]
            ert.main.enkf_config_node.update_gen_kw(node,
                                                    ert.nonify(enkf_outfile_fmt),
                                                    ert.nonify(template_file),
                                                    ert.nonify(parameter_file),
                                                    ert.nonify(min_std_file),
                                                    ert.nonify(init_file_fmt))
            parameter_model.setValid(ert.main.enkf_config_node.is_valid)
        elif isinstance(parameter_model, SummaryModel):
            #should never be called from SummaryModel...
            raise AssertionError("Summary keys can not be updated!")
        elif isinstance(parameter_model, DataModel):
            input_format = gen_data_file_format.resolveName(str(parameter_model["input_format"]))
            output_format = gen_data_file_format.resolveName(str(parameter_model["output_format"]))
            ert.main.enkf_config_node.update_gen_data(node,
                                                      input_format.value(),
                                                      output_format.value(),
                                                      ert.nonify(parameter_model["init_file_fmt"]),
                                                      ert.nonify(parameter_model["template_file"]),
                                                      ert.nonify(parameter_model["template_key"]),
                                                      ert.nonify(parameter_model["enkf_outfile"]),
                                                      ert.nonify(parameter_model["enkf_infile"]),
                                                      ert.nonify(parameter_model["min_std"]))
            parameter_model.setValid(ert.main.enkf_config_node.is_valid)
        else:
            raise AssertionError("Type is not supported: %s" % (parameter_model.__class__))
        
        if ert.main.enkf_config_node.is_valid:
            ert.main.update_node( ert.main , key )




    r.getter = getEnsembleParameters
    r.initialize = getEnsembleParameters
    r.remove = removeParameter
    r.insert = insertParameter
    r.setter = updateParameter
    configPanel.endGroup()

    configPanel.endPage()


