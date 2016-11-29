Python plugins
==============

When you are using one of the python based frontends :code:`gert` or
:code:`ertshell` you can write plugins to ert in Python. The plugins
will run as part of the running ert process, and can do anything ert
can do itself. Writing a plugin is quite similar to writing an
:ref:`ert_script` - the main difference is that the class should
inherit from :code:`ErtPlugin`. This plugin will print the ensemble
size in the console:

.. code:: python

    from ert.util import DoubleVector
    from ert.enkf import ErtScript

    class PrintEnsembleSize(ErtPlugin):

       # The run method is the entry point which is called
       # when the plugin is invoked.
       def run(self):
            ert = self.ert()
            print("Ensemble size: %d" % ert.getEnsembleSize( ))


       # This is the name of the plugin which will appear in the
       # "Plugin" menu in the gui.
       def getName(self):
           return "Size plugin"


           
The plugins should be *installed* with a small configuration
file. Assume that the small code above is implemented in the script
file :code:`script/ens_size.py` then it is installed in ert as:

   INTERNAL  TRUE                 -- The job will call an internal function of the current running ERT instance.
   SCRIPT sripts/ens_size.py      -- An existing Python script

ERT will detect that the class inherits from :code:`ErtPlugin`, and
the functionality will be available as :code:`Size plugin` in the plugin menu.

As an example of a more complete plugin is the :code:`CSV export`
plugin from the ert code, this code can be used to export simulated
results, parameters and observations to a CSV file[#csv_export]_.

.. code:: python

    import os
    import re

    import pandas
    from PyQt4.QtGui import QCheckBox

    from ert.enkf import ErtPlugin, CancelPluginException
    from ert.enkf.export import SummaryCollector, GenKwCollector, MisfitCollector, DesignMatrixReader, CustomKWCollector
    from ert_gui.ertwidgets.customdialog import CustomDialog
    from ert_gui.ertwidgets.listeditbox import ListEditBox
    from ert_gui.ertwidgets.models.path_model import PathModel
    from ert_gui.ertwidgets.pathchooser import PathChooser


    class CSVExportJob(ErtPlugin):
        """
        Export of summary, custom_kw, misfit, design matrix data and gen kw into a single CSV file.

        The script expects a single argument:

        output_file: this is the path to the file to output the CSV data to

        Optional arguments:

        case_list: a comma separated list of cases to export (no spaces allowed)
                   if no list is provided the current case is exported
                   a single * can be used to export all cases

        design_matrix: a path to a file containing the design matrix

        infer_iteration: If True the script will try to infer the iteration number by looking at the suffix of the case name
                         (i.e. default_2 = iteration 2)
                         If False the script will use the ordering of the case list: the first item will be iteration 0,
                         the second item will be iteration 1...

        The script also looks for default values for output path and design matrix path to present in the GUI. These can
        be specified with DATA_KW keyword in the config file:
            DATA_KW CSV_OUTPUT_PATH <some path>
            DATA_KW DESIGN_MATRIX_PATH <some path>
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
            return "CSV Export"

        def getDescription(self):
            return "Export GenKW, CustomKW, design matrix, misfit data and summary data into a single CSV file."

        def inferIterationNumber(self, case_name):
            pattern = re.compile("_([0-9]+$)")
            match = pattern.search(case_name)

            if match is not None:
                return int(match.group(1))
            return 0


        def run(self, output_file, case_list=None, design_matrix_path=None, infer_iteration=True):
            cases = []

            if case_list is not None:
                if case_list.strip() == "*":
                    cases = self.getAllCaseList()
                else:
                    cases = case_list.split(",")

            if case_list is None or len(cases) == 0:
                cases = [self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()]

            if design_matrix_path is not None:
                if not os.path.exists(design_matrix_path):
                    raise UserWarning("The design matrix file does not exists!")

                if not os.path.isfile(design_matrix_path):
                    raise UserWarning("The design matrix is not a file!")

            data = pandas.DataFrame()

            for index, case in enumerate(cases):
                case = case.strip()

                if not self.ert().getEnkfFsManager().caseExists(case):
                    raise UserWarning("The case '%s' does not exist!" % case)

                if not self.ert().getEnkfFsManager().caseHasData(case):
                    raise UserWarning("The case '%s' does not have any data!" % case)

                if infer_iteration:
                    iteration_number = self.inferIterationNumber(case)
                else:
                    iteration_number = index

                case_data = GenKwCollector.loadAllGenKwData(self.ert(), case)

                custom_kw_data = CustomKWCollector.loadAllCustomKWData(self.ert(), case)
                if not custom_kw_data.empty:
                    case_data = case_data.join(custom_kw_data, how='outer')

                if design_matrix_path is not None:
                    design_matrix_data = DesignMatrixReader.loadDesignMatrix(design_matrix_path)
                    if not design_matrix_data.empty:
                        case_data = case_data.join(design_matrix_data, how='outer')

                misfit_data = MisfitCollector.loadAllMisfitData(self.ert(), case)
                if not misfit_data.empty:
                    case_data = case_data.join(misfit_data, how='outer')

                summary_data = SummaryCollector.loadAllSummaryData(self.ert(), case)
                if not summary_data.empty:
                    case_data = case_data.join(summary_data, how='outer')
                else:
                    case_data["Date"] = None
                    case_data.set_index(["Date"], append=True, inplace=True)

                case_data["Iteration"] = iteration_number
                case_data["Case"] = case
                case_data.set_index(["Case", "Iteration"], append=True, inplace=True)

                data = pandas.concat([data, case_data])

            data = data.reorder_levels(["Realization", "Iteration", "Date", "Case"])
            data.to_csv(output_file)

            export_info = "Exported %d rows and %d columns to %s." % (len(data.index), len(data.columns), output_file)
            return export_info


        def getArguments(self, parent=None):
            description = "The CSV export requires some information before it starts:"
            dialog = CustomDialog("CSV Export", description, parent)

            default_csv_output_path = self.getDataKWValue("CSV_OUTPUT_PATH", default="output.csv")
            output_path_model = PathModel(default_csv_output_path)
            output_path_chooser = PathChooser(output_path_model)

            design_matrix_default = self.getDataKWValue("DESIGN_MATRIX_PATH", default="")
            design_matrix_path_model = PathModel(design_matrix_default, is_required=False, must_exist=True)
            design_matrix_path_chooser = PathChooser(design_matrix_path_model)

            list_edit = ListEditBox(self.getAllCaseList())

            infer_iteration_check = QCheckBox()
            infer_iteration_check.setChecked(True)
            infer_iteration_check.setToolTip(CSVExportJob.INFER_HELP)

            dialog.addLabeledOption("Output file path", output_path_chooser)
            dialog.addLabeledOption("Design Matrix path", design_matrix_path_chooser)
            dialog.addLabeledOption("List of cases to export", list_edit)
            dialog.addLabeledOption("Infer iteration number", infer_iteration_check)

            dialog.addButtons()

            success = dialog.showAndTell()

            if success:
                design_matrix_path = design_matrix_path_model.getPath()
                if design_matrix_path.strip() == "":
                    design_matrix_path = None

                case_list = ",".join(list_edit.getItems())

                return [output_path_model.getPath(), case_list, design_matrix_path, infer_iteration_check.isChecked()]

            raise CancelPluginException("User cancelled!")


        def getDataKWValue(self, name, default):
            data_kw = self.ert().getDataKW()
            if name in data_kw:
                return data_kw[data_kw.indexForKey(name)][1]
            return default

        def getAllCaseList(self):
            fs_manager = self.ert().getEnkfFsManager()
            all_case_list = fs_manager.getCaseList()
            all_case_list = [case for case in all_case_list if fs_manager.caseHasData(case)]
            return all_case_list          
          

.. rubric:: Footnotes

.. [csv_export] There are many solutions for CSV export; this plugin
                is an example which is implemented based on the
                internal ert datastructures, other - probably more
                widely used alterantives are based on an external
                process running through the simulation folders.
