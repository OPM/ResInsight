#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'grav_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import datetime
import ert.ecl.ecl as ecl


# 1: We need the name of the GRID/EGRID file and the init file. Pass
#    these two filenames to the EclGrav() constructor.
grid      = ecl.EclGrid( "data/eclipse/grav/TROLL.EGRID" )
init_file = ecl.EclFile( "data/eclipse/grav/TROLL.INIT" )
grav = ecl.EclGrav( grid , init_file )


# 2: We load the restart files for the times we are interested in,
#    this can be done in two different ways:
#    
#    a) In the case of non unified restart files you can just use the
#       EclFile() constructor to load the whole file:
#
#              restart1 = ecl.EclFile("ECLIPSE.X0078")
#
#    b) You can use the ecl.EclFile.restart_block() method to load
#       only one block from a unified restart file. In that case you
#       must use 'report_step = nnn' to specifiy which report_step you
#       are interested in. Alternatively you can use 'dtime =
#       datetime( year , month , day)' to specify which block are
#       interested in.
# 
#          restart1 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , report_step = 88)
#          restart2 = ecl.EclFile.restart_block( "ECLIPSE.UNRST" , dtime = datetime.datetime( 2008 , 12 , 1) )

restart1  = ecl.EclFile.restart_block("data/eclipse/grav/TROLL.UNRST" , report_step = 117 )
restart2  = ecl.EclFile.restart_block("data/eclipse/grav/TROLL.UNRST" , report_step = 199 )


# 3. Add the surveys - as loaded from restart files. Give them a
#    sensible name as the first argument. You must add at least two
#    surveys, but you can add as many as you like.

grav.add_survey_PORMOD("PORMOD" , restart1 )
grav.add_survey_RPORV("RPORV"   , restart1 )

grav.new_std_density( ecl.ECL_GAS_PHASE   , 0.77840  )
grav.new_std_density( ecl.ECL_WATER_PHASE , 1045 )
grav.add_survey_FIP("FIP" , restart1 )

# 4: Load the list of stations from file - this can of course be done
#    any way you want.
stations = []
fileH = open("data/eclipse/grav/gravity_stations_2002" , "r")
for line in fileH.readlines():
    tmp = line.split()
    name = tmp[0]
    pos = (float( tmp[1]) , float( tmp[2] ) , float( tmp[3] ))
    stations.append( (name , pos) )
fileH.close()


# 5. Evaluate the gravitational response for all the stations.
for (name, pos) in stations:
    print "%-5s: %8.3f  %8.3f" % (name , grav.eval( "PORMOD" , "FIP" , pos) , grav.eval( "RPORV" , None , pos) )
