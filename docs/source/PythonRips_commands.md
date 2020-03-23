
As the Python interface is growing release by release, we are investigating how to automate the building of reference documentation. This document is not complete, but will improve as the automation moves forward.
## Currently missing features

 - Description of enums
 - Description of return values/classes
 - Description of each object
## clone_view

Parameter | Type | Description
--------- | ---- | -----------
view_id | int | View Id

## close_project

Parameter | Type | Description
--------- | ---- | -----------

## compute_case_group_statistics

Parameter | Type | Description
--------- | ---- | -----------
case_group_id | int | Case Group ID
case_ids | List of str | Case IDs

## create_grid_case_group

Parameter | Type | Description
--------- | ---- | -----------
case_paths | List of str | List of Paths to Case Files

## create_lgr_for_completions

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
time_step | int | Time Step Index
well_path_names | List of str | Well Path Names
refinement_i | int | RefinementI
refinement_j | int | RefinementJ
refinement_k | int | RefinementK
split_type | str | SplitType

## create_multi_plot

Parameter | Type | Description
--------- | ---- | -----------
plots | List of str | Plots

## create_multiple_fractures

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
well_path_names | List of str | Well Path Names
min_dist_from_well_td | float | Min Distance From Well TD
max_fractures_per_well | int | Max Fractures per Well
template_id | int | Template ID
top_layer | int | Top Layer
base_layer | int | Base Layer
spacing | float | Spacing
action | str | Action

## create_saturation_pressure_plots

Parameter | Type | Description
--------- | ---- | -----------
case_ids | List of str | Case IDs

## create_statistics_case

Parameter | Type | Description
--------- | ---- | -----------
case_group_id | int | Case Group Id

## create_view

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case Id

## create_well_bore_stability_plot

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | GeoMech Case Id
well_path | str | Well Path
time_step | int | Time Step
wbs_parameters | str | WbsParameters

## export_contour_map_to_text

Parameter | Type | Description
--------- | ---- | -----------
export_file_name | str | 
export_local_coordinates | str | 
undefined_value_label | str | 
exclude_undefined_values | str | 
view_id | int | View Id

## export_flow_characteristics

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
time_steps | List of str | Selected Time Steps
injectors | List of str | Injectors
producers | List of str | Producers
file_name | str | Export File Name
minimum_communication | float | Minimum Communication
aquifer_cell_threshold | float | Aquifer Cell Threshold

## export_lgr_for_completions

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
time_step | int | Time Step Index
well_path_names | List of str | Well Path Names
refinement_i | int | RefinementI
refinement_j | int | RefinementJ
refinement_k | int | RefinementK
split_type | str | SplitType

## export_msw

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
well_path | str | Well Path Name
include_perforations | str | Include Perforations
include_fishbones | str | Include Fishbones
include_fractures | str | Include Fractures
file_split | str | File Split

## export_multi_case_snapshots

Parameter | Type | Description
--------- | ---- | -----------
grid_list_file | str | Grid List File

## export_property

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
time_step | int | Time Step Index
property | str | Property Name
eclipse_keyword | str | Eclipse Keyword
undefined_value | float | Undefined Value
export_file | str | Export FileName

## export_property_in_views

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
view_ids | List of str | View IDs
view_names | List of str | View Names
undefined_value | float | Undefined Value

## export_sim_well_fracture_completions

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
view_id | int | View ID
view_name | str | View Name
time_step | int | Time Step Index
simulation_well_names | List of str | Simulation Well Names
file_split | str | File Split
compdat_export | str | Compdat Export

## export_snapshots

Parameter | Type | Description
--------- | ---- | -----------
type | str | Type
prefix | str | Prefix
case_id | int | Case Id
view_id | int | View Id
export_folder | str | Export Folder
plot_output_format | str | Output Format

## export_visible_cells

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
view_id | int | View ID
view_name | str | View Name
export_keyword | str | Export Keyword
visible_active_cells_value | int | Visible Active Cells Value
hidden_active_cells_value | int | Hidden Active Cells Value
inactive_cells_value | int | Inactive Cells Value

## export_well_log_plot_data

Parameter | Type | Description
--------- | ---- | -----------
export_format | str | 
view_id | int | 
export_folder | str | 
file_prefix | str | 
export_tvd_rkb | str | 
capitalize_file_names | str | 
resample_interval | float | 
convert_curve_units | str | 

## export_well_path_completions

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
time_step | int | Time Step Index
well_path_names | List of str | Well Path Names
file_split | str | File Split
compdat_export | str | Compdat Export
combination_mode | str | Combination Mode
use_ntg_horizontally | str | Use NTG Horizontally
include_perforations | str | Include Perforations
include_fishbones | str | Include Fishbones
include_fractures | str | Include Fractures
exclude_main_bore_for_fishbones | str | Exclude Main Bore for Fishbones
perform_trans_scaling | str | Perform Transmissibility Scaling
trans_scaling_time_step | int | Transmissibility Scaling Pressure Time Step
trans_scaling_wbhp_from_summary | str | Transmissibility Scaling WBHP from summary
trans_scaling_wbhp | float | Transmissibility Scaling Constant WBHP Value

## export_well_paths

Parameter | Type | Description
--------- | ---- | -----------
well_path_names | List of str | Well Path Names
md_step_size | float | MD Step Size

## import_formation_names

Parameter | Type | Description
--------- | ---- | -----------
formation_files | List of str | 
apply_to_case_id | int | 

## import_well_log_files

Parameter | Type | Description
--------- | ---- | -----------
well_log_folder | str | 
well_log_files | List of str | 

## import_well_paths

Parameter | Type | Description
--------- | ---- | -----------
well_path_folder | str | 
well_path_files | List of str | 

## load_case

Parameter | Type | Description
--------- | ---- | -----------
path | str | Path to Case File

## open_project

Parameter | Type | Description
--------- | ---- | -----------
path | str | Path

## replace_case

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
new_grid_file | str | New Grid File

## replace_multiple_cases

Parameter | Type | Description
--------- | ---- | -----------

## replace_source_cases

Parameter | Type | Description
--------- | ---- | -----------
case_group_id | int | Case Group ID
grid_list_file | str | Grid List File

## run_octave_script

Parameter | Type | Description
--------- | ---- | -----------
path | str | Path
case_ids | List of str | Case IDs

## save_project

Parameter | Type | Description
--------- | ---- | -----------
file_path | str | 

## save_project_as

Parameter | Type | Description
--------- | ---- | -----------
file_path | str | 

## scale_fracture_template

Parameter | Type | Description
--------- | ---- | -----------
id | int | Id
half_length | float | HalfLengthScaleFactor
height | float | HeightScaleFactor
d_factor | float | DFactorScaleFactor
conductivity | float | ConductivityScaleFactor
width | float | WidthScaleFactor

## set_export_folder

Parameter | Type | Description
--------- | ---- | -----------
type | str | Type
path | str | Path
create_folder | str | Create Folder

## set_fracture_containment

Parameter | Type | Description
--------- | ---- | -----------
id | int | Id
top_layer | int | TopLayer
base_layer | int | BaseLayer

## set_main_window_size

Parameter | Type | Description
--------- | ---- | -----------
height | int | Height
width | int | Width

## set_plot_window_size

Parameter | Type | Description
--------- | ---- | -----------
height | int | Height
width | int | Width

## set_start_dir

Parameter | Type | Description
--------- | ---- | -----------
path | str | Path

## set_time_step

Parameter | Type | Description
--------- | ---- | -----------
case_id | int | Case ID
view_id | int | View ID
time_step | int | Time Step Index

