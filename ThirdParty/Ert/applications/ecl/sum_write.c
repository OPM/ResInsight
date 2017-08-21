/*
   Copyright (C) 2012  Statoil ASA, Norway.
   The file 'sum_write' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/smspec_node.h>


/*
  The ECLIPSE summary data is stored in two different files:

   - A header file which contains metadata of the stored data; this
     header file has extension SMSPEC.

   - The actual data is stored as blocks of data where each block
     contains all the variables for one timestep (called MINISTEP in
     ECLIPSE lingo). The data can be stored in a unified summary file
     with all the data in one file, or alternatively several separate
     files - one for each report step.

     In the case of a unified file the extension is UNSMRY whereas the
     multiple summary files haver extension Snnnn.

  The format is quite primitive, and very much influenced by Fortran77
  arrays which are fixed-size-large-enough thank you. The header file
  is (mainly) based on three different ecl_kw vectors which together
  completely specify the variables. The example below consists of the
  following:

    - Variables: Field Oil Production Total (FOPT), Group Oil
      Production Rate (GOPR) in group P-North, the Block Pressure
      (BPR) in cell 5423 and the Well Gas Production Rate (WGPR) in
      the well GasW.

    - A total of two report steps (dates), where the first consist of
      two timesteps and the latter of three timesteps.



             CASE.SMSPEC                      CASE.S0001                         CASE.S0002
   ------------------------------       --------------------------       ---------------------------------------
   | KEYWORD |  WGNAMES  | NUMS |       | MINISTEP 1|  MINISTEP 2|       | MINISTEP 3|  MINISTEP 4|  MINISTEP 5|
   +----------------------------+       +------------------------+       +-------------------------------------+
   | TIME    |           |      |  -->  |           |            |  -->  |           |            |            |
   | FOPT    |           |      |  -->  |           |            |  -->  |           |            |            |
   | GOPR    | P-North   |      |  -->  |           |            |  -->  |           |            |            |
   | BPR     |           | 5423 |  -->  |           |            |  -->  |           |            |            |
   | WGPR    | GasW      |      |  -->  |           |            |  -->  |           |            |            |
   +----------------------------+       +------------------------+       +-------------------------------------+

                                        |<---                 --  CASE.UNSMRY --                           --->|

  Observe the following points:

   1. The variable name supplied in the KEYWORD array is significant;
      ECLIPSE will determine the type of variable by looking at the
      first letter of the keyword: "W" for well variables, "F" for
      field variables and so on. This is further documented in the
      ecl_smspec_identify_var_type() function.

   2. The KEYWORD vector in the SMSPEC file is relevant for all the
      entries, whereas the WGNAMES and NUMS entries should be
      considered as further qualifiers which apply selectively
      depending on the variable type - for instance both the WGNAMES
      and NUMS vectors are ignored in the case of field variable like
      FOPT.

   3. The actual data is organised in blocks of data, one continous
      block for each (simulator) timestep. These blocks are collected
      in report steps; when stored the data can be either lumped
      together in a unified file, or split in many different files.

   4. Each block of data is exactly as large as the header. This
      implies that:

        - The number of elements in the summary file must be known up
          front; it is not possible to add/remove variables during
          simulation.
        - All timesteps are equally large.

      Due to this fixed size nature an important element in the
      ecl_sum implementation is "the index of a variable" - i.e. the
      "FOPT" variable in the example above will be found at index 1 of
      every datablock.

   5. The header contains a "TIME" variable; to some extent this looks
      like just any other variable, but it must be present in the
      SMSPEC header. In the example above the first element in every
      data block is the current time (in days) for that datablock; if
      labunits is used the stored value is the elapsed time in hours.

   6. In the ecl_sum library the concept of a 'gen_key' is used quite
      extensively. The gen_key is a string combination of the KEYWORD,
      WGNAMES and NUMS values from the smspec header file. Only the
      relevant elements are combined when forming a gen_key; i.e. for
      the example above we will have:

        ------------------------------
        | KEYWORD |  WGNAMES  | NUMS |        General key
        +----------------------------+        ------------
        | TIME    |           |      |  -->   TIME
        | FOPT    |           |      |  -->   FOPT
        | GOPR    | P-North   |      |  -->   GOPR:P-North
        | BPR     |           | 5423 |  -->   BPR:5423 , BPR:i,j,k
        | WGPR    | GasW      |      |  -->   WGPR:GasW
        +----------------------------+

     Note the following:

      o The keys are joined with a string typically called
        'key_join_string' in the code; in the example above this is a
        ":" which is quite close to a standard. The gen_key values are
        never inverted, so the join string can be arbitrary.

      o For the block quantities, like the BPR in the example above
        the NUMS value is the cell number in (i,j,k) ordering:

                 NUMS = i + (j - 1)*nx + (k-1)*nx*ny

        The block related properties are also available as the general
        key BPR:i,j,k where the coordinates (i,j,k) are offset 1.

  Due to the fixed size nature of the summary file format described in
  point 4 above, the library is somewhat cumbersome and unflexible to
  use. Very briefly the usage pattern will be like:

    1. Create a ecl_sum instance and add variables to it with the
       ecl_sum_add_var() function.

    2. Use your simulator to step forward in time, and add timesteps
       with ecl_sum_add_tstep().

   Now - the important thing is that steps 1 and 2 two can not be
   interchanged, that will lead to crash and burn.
*/



int main( int argc , char ** argv) {
  time_t start_time = util_make_date_utc( 1,1,2010 );
  int nx = 10;
  int ny = 10;
  int nz = 10;


  smspec_node_type * wwct_wellx;
  smspec_node_type * wopr_wellx;
  vector_type      * blank_nodes = vector_alloc_new();


  /*
    We create a new summary case which will be used for writing. The
    arguments are:

      1: The case - this an ECLIPSE basename, with an optional leading
         path component. Can later be modified with ecl_sum_set_case().

      2: Should formatted files be used? Can be modified with
         ecl_sum_set_fmt_output().

      3: Character(s) to separate the items from the smspec header
         generate a gen_key value. The value ":" is close to a
         standard.

      4: Should the data be output as a unified file, or as a
         collection of files - one for each report step. Can be
         modified with ecl_sum_set_unified_output().

      5: The real-world start time of this simulation, as a unix
         time_t value.

      6-8: Grid dimensions.
  */
  bool time_in_days = true;
  ecl_sum_type * ecl_sum = ecl_sum_alloc_writer( "/tmp/CASE" , false , true , ":" , start_time , time_in_days , nx , ny , nz );


  /*
    We add the variables we wish to measure. Due to the rather
    inflexible nature of the format we must add all the variables we
    are interested in before we start adding data.

    The arguments to this function are:

      1. self / this

      2. The KEYWORD value for the variable we are considering; the
         function ecl_smspec_identify_var_type() will be called with
         this string - i.e. you must follow the ECLIPSE rules (see
         table 3.4 in the ECLIPSE file format reference manual).

      3. The WGNAME value for this variable. WGNAME is the well or
         group name; if the variable in question is neither a well nor
         a group variable you can just send in NULL. The WGNAME value
         can be changed runtime with the ecl_sum_update_wgname()
         function.

      4. The NUMS value for this variable.

      5. The unit for this variable.

      6. A defualt value for this variable.

    Observe that as an alternative to ecl_sum_add_var() you can use
    the combination:

       smspec_node_type * var = ecl_sum_add_blank_var( ecl_sum , DEFAULT_VALUE );
       .....
       ecl_sum_init_var( ecl_sum , var , keyword , wgname , num , unit );

    This is an alternative when e.g. the name of wells is not known in
    advance.
  */
  ecl_sum_add_var( ecl_sum , "FOPT" , NULL   , 0   , "Barrels" , 99.0 );
  ecl_sum_add_var( ecl_sum , "BPR"  , NULL   , 567 , "BARS"    , 0.0  );
  ecl_sum_add_var( ecl_sum , "WWCT" , "OP-1" , 0   , "(1)"     , 0.0  );
  ecl_sum_add_var( ecl_sum , "WOPR" , "OP-1" , 0   , "Barrels" , 0.0  );


  /*
    The return value from the ecl_sum_add_var() function is an
    smspec_node_type instance (implemented in file smspec_node.c);
    which is essentially a struct holding header information about
    this varible. There are several options on how to handle the
    return value from the ecl_sum_add_var() function; which affect how
    you can add data at a later stage:

       1. You can just ignore it - that is not very clean; in this
          case you must make an assumption of gen_key format at a
          later stage.

       2. You can use the smspec_node_get_gen_key1() or
          smspec_node_get_params_index() and hold on to the gen_key or
          params_index values. You will need these later.

       3. You can hold on to the complete smspec_node instance, and
          then later on call one of the smspec_node_get_params_index()
          or smspec_node_get_gen_key1() functions.

          If you wish to change the WGNAME value with
          ecl_sum_update_wgname() a later stage you must hold on to
          the smspec_node instance.

    ECLIPSE supports the 'dynamic' creation of wells, however you must
    specify up-front how many wells (max) you will have, and then the
    name will be specified as the wells 'pop up' in the Schedule
    file. In the current implementation this is supported by requering
    that you first add all wells/groups with ecl_sum_add_var(), and
    then you can subsequently update the WGNAME value later. This is
    illustrated with the two lines below where we add the WWCT and
    WOPR variables for a well without name, and then call
    ecl_sum_update_wganme() further down in the code.
  */

  wwct_wellx = ecl_sum_add_var( ecl_sum , "WWCT" , NULL , 0 , "(1)"     , 0.0);
  wopr_wellx = ecl_sum_add_var( ecl_sum , "WOPR" , NULL , 0 , "Barrels" , 0.0);

  /*
    Here we add a collection of ten variables which are not
    initialized. Before they can be actually used you must initialize
    them with:

       ecl_sum_init_var( ecl_sum , node , keyword , wgname , num , unit );

    If you do not init them at all they will appear in the SMSPEC file
    as WWCT variable of the DUMMY_WELL (i.e. they will be discarded in
    a subsequent load, but the will be there).
  */
  {
    int i;
    for (i=0; i < 10; i++) {
      smspec_node_type * blank_node = ecl_sum_add_blank_var( ecl_sum , i * 1.0 );
      vector_append_ref( blank_nodes , blank_node );
    }
  }



  {
    int num_dates = 10;
    int num_step = 10;
    double sim_days = 0;
    int report_step;
    int step;
    for (report_step = 0; report_step < num_dates; report_step++) {
      for (step = 0; step < num_step; step++) {
        /* Simulate .... */

        sim_days += 10;
        {
          /*
             Here we add a new timestep. The report steps are not
             added explicitly; the ecl_sum layer will just see if this
             is a new report step. Observe that report_step == 1 is
             the first permissible report step.

             The new time step is essentially a block of data with one
             element for each variable, and now we must set the
             various elements in the block. There are different ways
             to do this - depending on which metadata information we
             are holding on to from the ecl_sum_add_var() calls.

             Observe that one element for the number of simulation
             days has already been inserted in the timestep. The
             elements which are not set explicitly will have the
             default value given in the ecl_sum_add_var() call.
          */
          ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 1 , sim_days );


          /*
            We can just set a value by it's index using the
            ecl_sum_tstep_iset() function. The index value should come
            from the smspec_get_params_index() function.
          */
          ecl_sum_tstep_iset( tstep , 3 , 0.98);

          /*
            We can use the gen_key value to set the numerical
            values. The gen_key value should could from the
            smpec_get_gen_key1() function:
          */
          ecl_sum_tstep_set_from_key( tstep  , "WWCT:OP-1" , sim_days / 10);
          if (report_step >= 5)
            /*
              We can use the smspec_node value from the
              ecl_sum_add_var() function directly:
            */
            ecl_sum_tstep_set_from_node( tstep , wwct_wellx , sim_days );
        }
      }
    }
  }

  /*
    Suddenly someone calls in and tell us the name of the mystery
    well:
  */
  ecl_sum_update_wgname( ecl_sum , wwct_wellx , "OPX");
  ecl_sum_update_wgname( ecl_sum , wopr_wellx , "OPX");

  vector_free( blank_nodes ); // Only frees the container - the actual nodes are handled by the ecl_sum instance.
  ecl_sum_fwrite( ecl_sum );
  ecl_sum_free( ecl_sum );
}
