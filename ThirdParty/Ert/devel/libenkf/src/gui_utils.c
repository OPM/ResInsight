/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gui_utils.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/


int get_plot_data(main , node , iens , key_index, state_enum state , time_t * x_time, double * y) {
  member_config = enkf_main_iget_member_config(main, iens);
  stop_time = member_config_get_last_restart_nr(member_config);
  
  int count = 0;
  for(int step = 0; step <= stop_time; step++) {
    if (enkf_fs_has_node(fs, config_node, step, iens, state)) {
      double sim_days = member_config_iget_sim_days(member_config, step, fs)
        time_t sim_time = member_config_iget_sim_time(member_config, step, fs)
        
        enkf_fs_fread_node(fs, node, step, iens, state)
        double value;
      if (enkf_node_user_get( node , key_index , &value))
        x_time[count] = sim_time;
      x_days[count] = sim_days;
      y[count] = y;
      count++;
    }
  }
}
}

