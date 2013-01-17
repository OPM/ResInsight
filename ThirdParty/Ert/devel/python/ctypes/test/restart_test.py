#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'restart_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

file = ecl.EclFile( "data/eclipse/case/ECLIPSE.UNRST" )

bl = ecl.EclFile.restart_block( "data/eclipse/case/ECLIPSE.UNRST" , dtime = datetime.datetime( 2001 , 6 , 1 ))


for (step,date) in zip( file.report_steps , file.report_dates):
    print step,date

kw = file["SWAT"][4]
if file.has_report_step( 4 ):
    print "Has report 4"

if file.has_sim_time( datetime.datetime( 2001 , 6 , 1 )):
    print "Har 1. juni 2001"

if ecl.EclFile.contains_report_step( "data/eclipse/case/ECLIPSE.UNRST" , 4 ):
    print "OK"

if ecl.EclFile.contains_sim_time( "data/eclipse/case/ECLIPSE.UNRST" , datetime.datetime( 2002 , 6 , 7) ):
    print "OK"

file.select_restart_section( report_step = 40 )
file.select_restart_section( sim_time = datetime.datetime( 2002 , 6 , 1) )
print file.headers


f2 = ecl.EclFile( "data/eclipse/case/ECLIPSE.UNRST")
print f2["SWAT"][10]

print ecl.EclFile.file_report_steps( "data/eclipse/case/ECLIPSE.UNRST" )

midF = ecl.EclFile( "midgard/MD7_BA11.UNRST" )
for index in range(len(midF["SEQNUM"])):
    midF.select_block( "SEQNUM" , index )
    print midF["SEQNUM"][0][0]
    
