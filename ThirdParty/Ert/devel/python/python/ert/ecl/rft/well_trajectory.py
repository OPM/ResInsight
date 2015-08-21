#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'well_trajectory.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import sys
import os
from collections import namedtuple

TrajectoryPoint = namedtuple("TrajectoryPoint", "utm_x utm_y measured_depth true_vertical_depth zone")

class WellTrajectory:

    def __init__(self , filename):
        if os.path.isfile(filename):
            self.points = []
            with open(filename) as fileH:
                for line in fileH.readlines():
                    line = line.partition("--")[0]
                    line = line.strip()
                    if line:
                        point = line.split()
                        if len(point) < 4 or len(point) > 5:
                            raise UserWarning("Trajectory data file not on correct format: \"utm_x utm_y md tvd [zone]\" - zone is optional")
                            
                        try:
                            utm_x = float(point[0])
                            utm_y = float(point[1])
                            md = float(point[2])
                            tvd = float(point[3])
                            if len(point) > 4:
                                zone = point[4]
                            else:
                                zone = None
                        except ValueError:
                            raise UserWarning("Error: Failed to extract data from line %s\n" % line)
                            
                        self.points.append(TrajectoryPoint(utm_x , utm_y , md , tvd , zone))
                
        else:
            raise IOError("File not found:%s" % filename)
            
    
    def __len__(self):
        return len(self.points)

        
    def __getitem__(self , index):
        if index < 0:
            index += len(self)

        return self.points[index]
