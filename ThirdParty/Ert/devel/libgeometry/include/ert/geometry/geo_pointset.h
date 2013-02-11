/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'geo_pointset.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GEO_POINTSET_H__
#define __GEO_POINTSET_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct geo_pointset_struct geo_pointset_type;


  geo_pointset_type * geo_pointset_alloc( bool external_z );
  void                geo_pointset_free( geo_pointset_type * pointset );
  void                geo_pointset_add_xy( geo_pointset_type * pointset , double x , double y);
  void                geo_pointset_add_xyz( geo_pointset_type * pointset , double x , double y, double z);
  int                 geo_pointset_get_size( const geo_pointset_type * pointset );
  void                geo_pointset_iget_xy( const geo_pointset_type * pointset , int index , double * x , double * y);
  const double      * geo_pointset_get_zcoord( const geo_pointset_type * pointset );
  
#ifdef __cplusplus
}
#endif
#endif
