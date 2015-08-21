/*
  This file is included from the ecl_kw.c file during compilation. It
  contains functions which are not really related to the ecl_kw as a
  datastructure, but rather use an ecl_kw instance in a function.
*/


/*
  This is an extremely special-case function written for the region
  creation code. Given a completed ecl_kw regions keyword, the purpose
  of this function is to "detect and correct" uninitialized cells with
  value 0. This function is purely heuristic:

   1. It only considers cells which are active in the grid, i.e. where
      actnum[] != 0.

   2. It will scan the four neighbours in the xy plane, if all
      neighbours agree on region value this value will be applied;
      otherwise the value will not be changed. Neighbouring cells with
      value zero are not considered when comparing.
*/


void ecl_kw_fix_uninitialized(ecl_kw_type * ecl_kw , int nx , int ny , int nz, const int * actnum) {
  int i,j,k;
  int * data = ecl_kw_get_ptr( ecl_kw );

  int_vector_type * undetermined1 = int_vector_alloc(0,0);
  int_vector_type * undetermined2 = int_vector_alloc(0,0);

  for (k=0; k < nz; k++)  {
    int_vector_reset( undetermined1 );
    for (j=0; j < ny; j++) {
      for (i=0; i < nx; i++) {
        int g0 = i + j * nx + k* nx*ny;

        if (data[g0] == 0 && actnum[g0])
          int_vector_append( undetermined1 , g0 );
      }
    }


    while (true) {
      int index;
      bool finished = true;

      int_vector_reset( undetermined2 );
      for (index = 0; index < int_vector_size( undetermined1 ); index++) {
        int g0 = int_vector_iget( undetermined1 , index );
        int j = (g0  - k * nx*ny) / nx;
        int i =  g0  - k * nx*ny - j * nx;

        if (data[g0] == 0 && actnum[g0]) {
          int n1 = 0;
          int n2 = 0;
          int n3 = 0;
          int n4 = 0;

          if (i > 0) {
            int g1 = g0 - 1;
            if (actnum[g1])
              n1 = data[g1];
          }

          if (i < (nx - 1)) {
            int g2 = g0 + 1;
            if (actnum[g2])
              n2 = data[g2];
          }

          if (j > 0) {
            int g3 = g0 - nx;
            if (actnum[g3])
              n3 = data[g3];
          }

          if (j < (ny - 1)) {
            int g4 = g0 + nx;
            if (actnum[g4])
              n4 = data[g4];
          }

          {
            int new_value = 0;

            if (n1)
              new_value = n1;

            if (n2) {
              if (new_value == 0)
                new_value = n2;
              else if (new_value != n2)
                new_value = -1;
            }

            if (n3) {
              if (new_value == 0)
                new_value = n3;
              else if (new_value != n3)
                new_value = -1;
            }

            if (n4) {
              if (new_value == 0)
                new_value = n4;
              else if (new_value != n4)
                new_value = -1;
            }

            if (new_value > 0) {
              data[g0] = new_value;
              finished = false;
            }
          }
          if ((n1 + n2 + n3 + n4) == 0)
            int_vector_append( undetermined2 , g0 );
        }
      }
      {
        int_vector_type * tmp = undetermined2;
        undetermined2 = undetermined1;
        undetermined1 = tmp;
      }
      if (finished || (int_vector_size( undetermined1) == 0))
        break;
    }
  }
  int_vector_free( undetermined1 );
  int_vector_free( undetermined2 );
}
