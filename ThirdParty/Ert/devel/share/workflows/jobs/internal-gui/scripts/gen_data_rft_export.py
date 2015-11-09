import os
import re
import pandas
import numpy

from ert.ecl.rft import WellTrajectory


from ert.enkf import ErtPlugin, CancelPluginException
from ert.enkf import RealizationStateEnum
from ert.enkf.enums import EnkfObservationImplementationType
from ert.enkf.export import GenDataCollector, SummaryCollector, GenKwCollector, MisfitCollector, DesignMatrixReader,ArgLoader

from ert_gui.models.mixins.connectorless import  DefaultPathModel , DefaultBooleanModel, StringModel
from ert_gui.widgets.string_box import StringBox 
from ert_gui.widgets.checkbox import CheckBox 
from ert_gui.widgets.custom_dialog import CustomDialog
from ert_gui.widgets.list_edit_box import ListEditBox
from ert_gui.widgets.path_chooser import PathChooser


class GenDataRFTCSVExportJob(ErtPlugin):
    """Export of GEN_DATA based rfts to a CSV file. The csv file will in
    addition contain the depth as duplicated seperate row.

    The script expects four arguments:

      output_file: this is the path to the file to output the CSV data to

      key: this is the ert GEN_DATA key used for this particular RFT.

      report_step: This is the report step configured in the ert
        configuration file for this RFT.  
 
      trajectory_file: This is the the file containing the 

   Optional arguments:

    case_list: a comma separated list of cases to export (no spaces allowed)
               if no list is provided the current case is exported

    infer_iteration: If True the script will try to infer the iteration number by looking at the suffix of the case name
                     (i.e. default_2 = iteration 2)
                     If False the script will use the ordering of the case list: the first item will be iteration 0,
                     the second item will be iteration 1...
    """

    INFER_HELP = ("<html>"
                 "If this is checked the iteration number will be inferred from the name i.e.:"
                 "<ul>"
                 "<li>case_name -> iteration: 0</li>"
                 "<li>case_name_0 -> iteration: 0</li>"
                 "<li>case_name_2 -> iteration: 2</li>"
                 "<li>case_0, case_2, case_5 -> iterations: 0, 2, 5</li>"
                 "</ul>"
                 "Leave this unchecked to set iteration number to the order of the listed cases:"
                 "<ul><li>case_0, case_2, case_5 -> iterations: 0, 1, 2</li></ul>"
                 "<br/>"
                 "</html>")

    def getName(self):
        return "GEN_DATA RFT CSV Export"

    def getDescription(self):
        return "Export gen_data RFT results into a single CSV file."

    def inferIterationNumber(self, case_name):
        pattern = re.compile("_([0-9]+$)")
        match = pattern.search(case_name)

        if match is not None:
            return int(match.group(1))
        return 0


    def run(self, output_file, trajectory_path , case_list=None, infer_iteration=True):
        """The run method will export the RFT's for all wells and all cases.

        The successfull operation of this method hinges on two naming
        conventions:
        
          1. All the GEN_DATA RFT observations have key RFT_$WELL
          2. The trajectory files are in $trajectory_path/$WELL.txt or $trajectory_path/$WELL_R.txt
        
        """


        wells = set()
        obs_pattern = "RFT_*"
        enkf_obs = self.ert().getObservations()
        obs_keys = enkf_obs.getMatchingKeys(obs_pattern , obs_type = EnkfObservationImplementationType.GEN_OBS)
        
        cases = []
        if case_list is not None:
            cases = case_list.split(",")

        if case_list is None or len(cases) == 0:
            cases = [self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()]

        data_frame = pandas.DataFrame()
        for index, case in enumerate(cases):
            case = case.strip()
            case_frame = pandas.DataFrame()
            
            if not self.ert().getEnkfFsManager().caseExists(case):
                raise UserWarning("The case '%s' does not exist!" % case)

            if not self.ert().getEnkfFsManager().caseHasData(case):
                raise UserWarning("The case '%s' does not have any data!" % case)

            if infer_iteration:
                iteration_number = self.inferIterationNumber(case)
            else:
                iteration_number = index
                
            for obs_key in obs_keys:
                well = obs_key.replace("RFT_","")
                wells.add( well )
                obs_vector = enkf_obs[obs_key]
                data_key = obs_vector.getDataKey()
                report_step = obs_vector.activeStep()
                obs_node = obs_vector.getNode( report_step )
                
                rft_data = GenDataCollector.loadGenData( self.ert() , case , data_key , report_step )
                fs = self.ert().getEnkfFsManager().getFileSystem( case )
                realizations = fs.realizationList( RealizationStateEnum.STATE_HAS_DATA )
                
                # Trajectory
                trajectory_file = os.path.join( trajectory_path , "%s.txt" % well)
                if not os.path.isfile(trajectory_file):
                    trajectory_file = os.path.join( trajectory_path , "%s_R.txt" % well)
                    
                trajectory = WellTrajectory( trajectory_file )
                arg = ArgLoader.load( trajectory_file , column_names = ["utm_x" , "utm_y" , "md" , "tvd"])
                tvd_arg = arg["tvd"]
                data_size = len(tvd_arg)

                
                # Observations
                obs = numpy.empty(shape = (data_size , 2 ) , dtype=numpy.float64)
                obs.fill( numpy.nan )
                for obs_index in range(len(obs_node)):
                    data_index = obs_node.getDataIndex( obs_index )
                    value = obs_node.getValue( obs_index )
                    std = obs_node.getStandardDeviation( obs_index )
                    obs[data_index,0] = value
                    obs[data_index,1] = std


                for iens in realizations:
                    realization_frame = pandas.DataFrame( data = {"TVD" : tvd_arg , 
                                                                  "Pressure" : rft_data[iens],
                                                                  "ObsValue" : obs[:,0],
                                                                  "ObsStd"   : obs[:,1]},
                                                          columns = ["TVD" , "Pressure" , "ObsValue" , "ObsStd"])
                    
                    realization_frame["Realization"] = iens
                    realization_frame["Well"] = well
                    realization_frame["Case"] = case
                    realization_frame["Iteration"] = iteration_number

                    case_frame = case_frame.append(realization_frame)

                data_frame = data_frame.append(case_frame)

        data_frame.set_index(["Realization" , "Well" , "Case" , "Iteration"] , inplace = True)
        data_frame.to_csv(output_file)
        export_info = "Exported RFT information for wells: %s to: %s " % (", ".join(list(wells)) , output_file)
        return export_info


    def getArguments(self, parent=None):
        description = "The GEN_DATA RFT CSV export requires some information before it starts:"
        dialog = CustomDialog("Robust CSV Export", description, parent)
        
        output_path_model = DefaultPathModel("output.csv")
        output_path_chooser = PathChooser(output_path_model, path_label="Output file path")

        trajectory_model = DefaultPathModel("wellpath" , must_be_a_directory=True , must_be_a_file = False , must_exist = True)
        trajectory_chooser = PathChooser(trajectory_model, path_label="Trajectory file")

        fs_manager = self.ert().getEnkfFsManager()
        all_case_list = fs_manager.getCaseList()
        all_case_list = [case for case in all_case_list if fs_manager.caseHasData(case)]
        list_edit = ListEditBox(all_case_list, "List of cases to export")


        infer_iteration_model = DefaultBooleanModel()
        infer_iteration_checkbox = CheckBox(infer_iteration_model, label="Infer iteration number", show_label=False)
        infer_iteration_checkbox.setToolTip(GenDataRFTCSVExportJob.INFER_HELP)

        dialog.addOption(output_path_chooser)
        dialog.addOption(trajectory_chooser)
        dialog.addOption(list_edit)
        dialog.addOption(infer_iteration_checkbox)

        dialog.addButtons()

        success = dialog.showAndTell()

        if success:
            case_list = ",".join(list_edit.getItems())
            try:
                return [output_path_model.getPath(), trajectory_model.getPath() , case_list, infer_iteration_model.isTrue()]
            except ValueError:
                pass

        raise CancelPluginException("User cancelled!")


