#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_isosurf.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import os.path
import ert.ecl.ecl_util as ecl_util
import ert.ecl as ecl
import ert.util.tvector
from   ert.util.lookup_table import LookupTable


# This function will scan the vertical column specified by (i,j) and
# collect the pairs ( value(kw) , depth ) in a lookuptable. When the
# full column has been scanned it will lookup the value @level in the
# lookuptable and return the corresponding depth, based on linear
# interpolation.
#
# If @level is out of bounds - i.e. it is above the highest value
# found in the column, or below the lowest value, the behaviour will
# depend on whether we are searching for gas levels or water levels,
# if we aware searching for gas the behaviour is as follows:
# 
#  o If @level is above the found maximum level we return reservoir
#    top.
#  o If @level is below the found minimum level we return reservoir
#    bottom.
#
# When searching for water levels the behaviour os opposite.
#
# If the only one or zero active cells is founc in the column, the
# value None is returned.
#
# 
# Caveat: Non unique depth profile



def usage():
    print """
The ecl_isosurf script can be used to extract isosurfaces of the SWAT
or SGAS keywords from a restart file. When using the program you must
give commandline arguments about:

  o Which restart file you want to use, and in case of unified restart
    file which report_step you are interested in. The program will
    look for a EGRID/GRID file with the same basename.

  o Which phase you are interested in - i.e. SWAT or SGAS.

  o The name of an output file where the surface will be
    stored. Observe that this filename will be used a format
    specifier, hence a float placeholder (i.e. something like %g /
    %4.2f ) will be replaced with the level value when creating the
    output file - that way you can create several iso surface with one
    program invocation.

  o The iso levels you are interested in.

Example 1:

   ecl_isosurf   ECLIPSE.UNRST  25  SWAT   surfaces/swat_%4.2f   0.50 0.6 0.70 0.80 0.90

  This will look for a unified restart file "ECLIPSE.UNRST" in the
  current directory, and load solution data from report step 25 in
  that restart file. We will focus on water, i.e. the SWAT keyword,
  and the output surfaces will be stored in files surfaces/swat_0.50,
  surfaces/swat_0.60, surfaces/swat_0.70, ... the directory surfaces
  will be created if it does not exist.


Example 2:

   ecl_isosurf  /path/to/ECLIPSE.X0060  SGAS  sgas_iso  0.50

  In this case we give a non-unified restart filename, and then we
  should not include a report number. Observe that we assume that the
  grid is in the same directory as the restart file. In this case we
  are interested the gas, and only at the iso level 0.50. Since we are
  only interested in one level, we do not include any % formatting
  characters in the output filename.
"""
    sys.exit()

def iso_level( grid , kw , level , i , j ):
    depth_value = LookupTable()
    klist = range(grid.nz) 
    if kw.name == "SGAS":
        gas = True
    else:
        gas = False
    
    for k in klist:
        if grid.active( ijk = (i,j,k) ):
            depth_value.append( grid.grid_value( kw , i,j,k) , grid.depth( ijk = (i,j,k)))

    if depth_value.size > 1:
        if depth_value.arg_max > level and depth_value.arg_min < level:
            depth = depth_value.interp( level ) 
            k = grid.locate_depth( depth , i , j )
            return (depth , k)
        else:
            # The level we are searching is outside the [min,max]
            # range present in this column. We return values
            # corresponding to reservoir top / reservoir bottom
            # depending on the phase we are seaking, and whether we
            # are above the max or below the min value present.
            if gas:
                if depth_value.arg_max < level:
                    depth = grid.top( i , j)
                    k = 0
                else:
                    depth = grid.bottom( i , j )
                    k = grid.nz - 1
            else:
                if depth_value.arg_max < level:
                    depth = grid.bottom( i , j)
                    k = grid.nz - 1
                else:
                    depth = grid.top( i , j )
                    k = 0
            return (depth , k)
    else:
        # The column zero or one active cells, not enough to determine
        # a saturation-depth relationship.
        return (None , 0)


# Creates a iso surface on the level @level of the phase found in @kw. 

def write_surface( grid , kw , output_fmt , level ):
    if output_fmt.find("%") > -1:
        output_file = output_fmt % level
    else:
        output_file = output_fmt 

    (path , file) = os.path.split( output_file )
    if path:
        if not os.path.exists( path ):
            os.makedirs( path )
    print "Creating surface: %s" % output_file
    fileH = open( output_file , "w")
    for i in range(grid.nx):
        for j in range(grid.ny):
            (depth , k) = iso_level( grid , kw , level ,i,j)
            if depth:
                (utm_x , utm_y , tmp) = grid.get_xyz( ijk=(i,j,k) )
                fileH.write("%12.5f  %12.5f  %12.5f\n" % (utm_x , utm_y , depth))
    fileH.close()



# The arguments to the ecl_isosurf script must be given on the
# commandline; the full list of commandline arguments is then sent up
# here for parsing and loading of files. 
#
# The function will return a tuple of four elements:
#
#      (grid , kw , output_fmt , level_list)

def load_input( arglist ):
    if len(arglist) < 3:
        usage()

    input_file = arglist[0]
    file_type = ecl_util.get_file_type( input_file )
    if file_type == ecl_util.ECL_UNIFIED_RESTART_FILE:
        try:
            report_step = int( arglist[1] )
        except:
            usage()

        if os.path.exists( input_file ):
            ecl_file = ecl.EclFile.restart_block( input_file , report_step = report_step )
        else:
            usage()
        arg_offset = 2
    elif file_type == ecl_util.ECL_RESTART_FILE:
        if os.path.exists( input_file ):
            ecl_file = ecl.EclFile( input_file )
        else:
            usage()
        arg_offset = 1
    else:
        usage()
    
    (path_base , ext) = os.path.splitext( input_file )
    grid = ecl.EclGrid( path_base )
    if not grid:
        usage()

    phase = arglist[ arg_offset ].upper()
    if phase in ["SWAT" , "SGAS"]:
        kw = ecl_file.iget_named_kw( phase , 0 )
    else:
        sys.exit("Must give phase : SWAT / SGAS ")
        
    output_fmt = arglist[ arg_offset + 1 ]
    level_list = []
    for level in arglist[arg_offset + 2:]:
        level_list.append( float( level ) )
        
    return (grid , kw , output_fmt , level_list )


#################################################################
# Main program
#################################################################

(grid , kw , output_fmt , level_list) = load_input( sys.argv[1:] )

for level in level_list:
    write_surface( grid , kw , output_fmt , level )
