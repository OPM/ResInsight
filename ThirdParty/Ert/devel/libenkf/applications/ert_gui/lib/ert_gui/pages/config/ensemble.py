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
from PyQt4 import QtGui, QtCore
from ert_gui.widgets.spinnerwidgets import IntegerSpinner
from parameters.parameterpanel import ParameterPanel, enums
from parameters.parametermodels import SummaryModel, DataModel, FieldModel, KeywordModel
from ert.ert.enums import field_type
from ert.ert.enums import truncation_type
from ert.ert.enums import gen_data_file_format

def createEnsemblePage(configPanel, parent):
    configPanel.startPage("Ensemble")

    r = configPanel.addRow(IntegerSpinner(parent, "Number of realizations", "config/ensemble/num_realizations", 1, 10000))

    r.getter = lambda ert : ert.enkf.enkf_main_get_ensemble_size(ert.main)
    r.setter = lambda ert, value : ert.enkf.enkf_main_resize_ensemble(ert.main, value)

    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("ensembleResized()"))


    configPanel.startGroup("Parameters")
    r = configPanel.addRow(ParameterPanel(parent, "", "")) # no help file necessary
    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("ensembleUpdated()"))

    def getEnsembleParameters(ert):
        keys = ert.getStringList(ert.enkf.ensemble_config_alloc_keylist(ert.ensemble_config), free_after_use=True)

        parameters = []
        for key in keys:
            node = ert.enkf.ensemble_config_get_node(ert.ensemble_config, key)
            type = ert.enkf.enkf_config_node_get_impl_type(node)
            data = ert.enkf.enkf_config_node_get_ref(node)
            #print key, type

            model = None
            if type == FieldModel.TYPE:
                model = FieldModel(key)

                field_type = ert.enkf.field_config_get_type(data)
                field_type = enums.field_type[field_type]
                model["type"] = field_type

                truncation = ert.enkf.field_config_get_truncation_mode(data)

                if truncation & enums.truncation_type.TRUNCATE_MAX:
                    model["max"] = ert.enkf.field_config_get_truncation_max(data)

                if truncation & enums.truncation_type.TRUNCATE_MIN:
                    model["min"] = ert.enkf.field_config_get_truncation_min(data)

                model["init"] = ert.enkf.field_config_get_init_transform_name(data)
                model["output"] = ert.enkf.field_config_get_output_transform_name(data)
                
                model["init_files"] = ert.enkf.field_config_get_init_file_fmt(data)
                model["min_std"] = ert.enkf.enkf_config_node_get_min_std_file(node)
                model["enkf_outfile"] = ert.enkf.enkf_config_node_get_enkf_outfile(node)
                model["enkf_infile"] = ert.enkf.enkf_config_node_get_enkf_infile(node)

            elif type == DataModel.TYPE:
                model = DataModel(key)

                output_format_value = ert.enkf.gen_data_config_get_output_format(data)
                output_format = gen_data_file_format.resolveValue(output_format_value)

                input_format_value = ert.enkf.gen_data_config_get_input_format(data)
                input_format = gen_data_file_format.resolveValue(input_format_value)

                template_file = ert.enkf.gen_data_config_get_template_file(data)
                template_key = ert.enkf.gen_data_config_get_template_key(data)
                init_file_fmt = ert.enkf.gen_data_config_get_init_file_fmt(data)

                model["output_format"] = output_format
                model["input_format"] = input_format
                model["template_file"] = template_file
                model["template_key"] = template_key
                model["init_file_fmt"] = init_file_fmt

                min_std = ert.enkf.enkf_config_node_get_min_std_file(node)
                enkf_outfile = ert.enkf.enkf_config_node_get_enkf_outfile(node)
                enkf_infile = ert.enkf.enkf_config_node_get_enkf_infile(node)



                model["min_std"] = min_std
                model["enkf_outfile"] = enkf_outfile
                model["enkf_infile"] = enkf_infile

            elif type == KeywordModel.TYPE:
                model = KeywordModel(key)
                model["min_std"] = ert.enkf.enkf_config_node_get_min_std_file(node)
                model["enkf_outfile"] = ert.enkf.enkf_config_node_get_enkf_outfile(node)
                model["template"] = ert.enkf.gen_kw_config_get_template_file(data)
                model["init_file"] = ert.enkf.gen_kw_config_get_init_file_fmt(data)
                model["parameter_file"] = ert.enkf.gen_kw_config_get_parameter_file(data)
            elif type == SummaryModel.TYPE:
                model = SummaryModel(key)
            else:
                pass #Unknown type

            model.setValid(ert.enkf.enkf_config_node_is_valid(node))

            parameters.append(model)

        return parameters

    def removeParameter(ert, parameter_key):
        ert.enkf.enkf_main_del_node(ert.main, parameter_key)

    def insertParameter(ert, parameter):
        key = parameter.getName()
        if parameter.getType() == FieldModel.TYPE:
            grid = ert.enkf.ecl_config_get_grid(ert.ecl_config)
            node = ert.enkf.ensemble_config_add_field(ert.ensemble_config, key, grid)
            parameter.setValid(ert.enkf.enkf_config_node_is_valid(node))
        elif parameter.getType() == DataModel.TYPE:
            node = ert.enkf.ensemble_config_add_gen_data(ert.ensemble_config, key)
            parameter.setValid(ert.enkf.enkf_config_node_is_valid(node))
        elif parameter.getType() == KeywordModel.TYPE:
            node = ert.enkf.ensemble_config_add_gen_kw(ert.ensemble_config, key)
            parameter.setValid(ert.enkf.enkf_config_node_is_valid(node))
        elif parameter.getType() == SummaryModel.TYPE:
            parameter.setValid(True)
            b = ert.enkf.ensemble_config_add_summary(ert.ensemble_config, key)
            return b > 0 #0 == NULL 
        else:
            print "Unknown type: ", parameter
            return False

        return True

    def updateParameter(ert, parameter_model):
        key  = parameter_model.getName()
        node = ert.enkf.ensemble_config_get_node(ert.ensemble_config, key)
        
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
                ert.enkf.enkf_config_node_update_state_field(node,
                                                             truncate.value(),
                                                             float(minimum),
                                                             float(maximum))
            elif type == field_type.ECLIPSE_PARAMETER: #parameter
                ert.enkf.enkf_config_node_update_parameter_field(node,
                                                                 ert.nonify(parameter_model["enkf_outfile"]),
                                                                 ert.nonify(parameter_model["init_files"]),
                                                                 ert.nonify(parameter_model["min_std"]),
                                                                 truncate.value(),
                                                                 float(minimum),
                                                                 float(maximum),
                                                                 parameter_model["init"],
                                                                 parameter_model["output"])
            elif type == field_type.GENERAL: #general
                ert.enkf.enkf_config_node_update_general_field(node,
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

            parameter_model.setValid(ert.enkf.enkf_config_node_is_valid(node))

        elif isinstance(parameter_model, KeywordModel):
            enkf_outfile_fmt = parameter_model["enkf_outfile"]
            template_file = parameter_model["template"]
            parameter_file = parameter_model["parameter_file"]
            min_std_file = parameter_model["min_std"]
            init_file_fmt = parameter_model["init_files"]
            ert.enkf.enkf_config_node_update_gen_kw(node,
                                                    ert.nonify(enkf_outfile_fmt),
                                                    ert.nonify(template_file),
                                                    ert.nonify(parameter_file),
                                                    ert.nonify(min_std_file),
                                                    ert.nonify(init_file_fmt))
            parameter_model.setValid(ert.enkf.enkf_config_node_is_valid(node))
        elif isinstance(parameter_model, SummaryModel):
            #should never be called from SummaryModel...
            raise AssertionError("Summary keys can not be updated!")
        elif isinstance(parameter_model, DataModel):
            input_format = gen_data_file_format.resolveName(str(parameter_model["input_format"]))
            output_format = gen_data_file_format.resolveName(str(parameter_model["output_format"]))
            ert.enkf.enkf_config_node_update_gen_data(node,
                                                      input_format.value(),
                                                      output_format.value(),
                                                      ert.nonify(parameter_model["init_file_fmt"]),
                                                      ert.nonify(parameter_model["template_file"]),
                                                      ert.nonify(parameter_model["template_key"]),
                                                      ert.nonify(parameter_model["enkf_outfile"]),
                                                      ert.nonify(parameter_model["enkf_infile"]),
                                                      ert.nonify(parameter_model["min_std"]))
            parameter_model.setValid(ert.enkf.enkf_config_node_is_valid(node))
        else:
            raise AssertionError("Type is not supported: %s" % (parameter_model.__class__))
        
        if ert.enkf.enkf_config_node_is_valid(node):
            ert.enkf.enkf_main_update_node( ert.main , key )




    r.getter = getEnsembleParameters
    r.remove = removeParameter
    r.insert = insertParameter
    r.setter = updateParameter
    configPanel.endGroup()

    configPanel.endPage()


