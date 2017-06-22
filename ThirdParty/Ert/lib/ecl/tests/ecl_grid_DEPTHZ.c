/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_grid_DEPTHZ.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <ert/util/test_util.h>
#include <ert/ecl/ecl_grid.h>


double zfunc(double x , double y) {
  return cos(3*x)*sin(2*y)*sqrt(x*y);
}

double center_sum(const double * DV, int index) {
  double sum = DV[index] * 0.5;

  for (int i=0; i < index; i++)
    sum += DV[i];
  
  return sum;
}


void test_create() {
  ecl_grid_type * ecl_grid;
  int nx = 100;
  int ny = 100;
  int nz = 10;

  double * DXV = util_malloc( nx * sizeof * DXV );
  double * DYV = util_malloc( ny * sizeof * DYV );
  double * DZV = util_malloc( nz * sizeof * DZV );
  double * DEPTHZ = util_malloc( (nx + 1) * (ny + 1) * sizeof * DEPTHZ);

  for (int i=0; i < nx; i++)
    DXV[i] = 1.0 / nx;

  for (int j=0; j < ny; j++)
    DYV[j] = 1.0 / ny;

  for (int k=0; k < nz; k++)
    DZV[k] = 3.0 / nz;
  

  for (int j=0; j <= ny; j++) {
    double y = center_sum(DYV , j);
    for (int i=0; i <= nx; i++) {
      double x = center_sum(DXV , i);
      
      DEPTHZ[i + j*(nx + 1)] = zfunc( x,y );
    }
  }

  ecl_grid = ecl_grid_alloc_dxv_dyv_dzv_depthz( nx,ny,nz,DXV , DYV , DZV , DEPTHZ , NULL);
  
  for (int k=0; k < nz; k++) {
    double z0 = center_sum(DZV , k ) - 0.5*DZV[0];
    for (int j=0; j < ny; j++) {
      double y0 = center_sum(DYV , j );
      for (int i=0; i < nx; i++) {
        double x0 = center_sum(DXV , i );
        double xc,yc,zc;
        int g = ecl_grid_get_global_index3( ecl_grid , i , j , k );
        
        ecl_grid_get_xyz1( ecl_grid , g , &xc , &yc , &zc); 
        test_assert_double_equal( x0 , xc );
        test_assert_double_equal( y0 , yc );

        ecl_grid_get_cell_corner_xyz1( ecl_grid , g , 0 , &xc , &yc , &zc);
        test_assert_double_equal( z0 + zfunc(x0 , y0) , zc );

        ecl_grid_get_cell_corner_xyz1( ecl_grid , g , 4, &xc , &yc , &zc);
        test_assert_double_equal( z0 + zfunc(x0 , y0) + DZV[k] , zc );
      }
    }
  }


  free( DXV );
  free( DYV );
  free( DZV );
  free( DEPTHZ );
  ecl_grid_free( ecl_grid );
}


void test_compare() {
  int nx = 10;
  int ny = 15;
  int nz = 5;
  int V = nx*ny*nz;

  double dx = 10;
  double dy = 15;
  double dz = 10;
  double z0 = 0;


  ecl_grid_type * grid1;
  ecl_grid_type * grid2;

  {
    double * DX = util_malloc( V * sizeof * DX );
    double * DY = util_malloc( V * sizeof * DY );
    double * DZ = util_malloc( V * sizeof * DZ );
    double * TOPS = util_malloc( V * sizeof * TOPS );

    for (int i = 0; i < V; i++) {
      DX[i] = dx;
      DY[i] = dy;
      DZ[i] = dz;
    }

    for (int i = 0; i < nx*ny; i++) {
      TOPS[i] = z0;
    }

    for (int k=1; k < nz; k++) {
      for (int i = 0; i < nx*ny; i++) {
        int g2 = k*nx*ny + i;
        int g1 = (k- 1)*nx*ny + i;
        TOPS[g2] = TOPS[g1] + DZ[g2];
      }
    }


    grid1 = ecl_grid_alloc_dx_dy_dz_tops( nx , ny , nz , DX , DY , DZ , TOPS , NULL );
    free( DX );
    free( DY );
    free( DZ );
    free( TOPS );
  }

  {
    double * DXV = util_malloc( nx * sizeof * DXV );
    double * DYV = util_malloc( ny * sizeof * DYV );
    double * DZV = util_malloc( nz * sizeof * DZV );
    double * DEPTHZ = util_malloc( (nx + 1)*(ny + 1) * sizeof * DEPTHZ);

    for (int i = 0; i < nx; i++) 
      DXV[i] = dx;

    for (int i = 0; i < ny; i++) 
      DYV[i] = dy;

    for (int i = 0; i < nz; i++) 
      DZV[i] = dz;
    
    for (int i = 0; i < (nx + 1)*(ny+ 1); i++) 
      DEPTHZ[i] = z0;

    
    grid2 = ecl_grid_alloc_dxv_dyv_dzv_depthz( nx , ny , nz , DXV , DYV , DZV , DEPTHZ , NULL );
    free( DXV );
    free( DYV );
    free( DZV );
    free( DEPTHZ );
  }
  
  test_assert_true( ecl_grid_compare( grid1 , grid2 , true , true , true));
  ecl_grid_free( grid1 );
  ecl_grid_free( grid2 );
}



int main(int argc , char ** argv) {
  test_create();
  test_compare();
  exit(0);
}
