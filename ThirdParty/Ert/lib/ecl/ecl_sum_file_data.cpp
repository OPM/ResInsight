#include <stdexcept>
#include <limits>
#include <algorithm>
#include <memory>

#include <ert/ecl/ecl_sum_tstep.hpp>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_endian_flip.h>

#include "detail/ecl/ecl_sum_file_data.hpp"
#include "detail/ecl/ecl_unsmry_loader.hpp"

/*
  This file implements the type ecl_sum_data_type. The data structure
  is involved with holding all the actual summary data (i.e. the
  PARAMS vectors in ECLIPSE speak), in addition the time-information
  with MINISTEPS / REPORT_STEPS and so on is implemented here.

  This file has no information about how to index into the PARAMS
  vector, i.e. at which location can the WWCT for well P6 be found,
  that is responsability of the ecl_smspec_type.

  The time direction in this system is implemented in terms of
  ministeps. There are some query / convert functons based on report
  steps.
*/


/*****************************************************************/
/*
   About ministeps and report steps.
   ---------------------------------

   A sequence of summary data will typically look like this:

   ------------------
   SEQHDR            \
   MINISTEP  0        |
   PARAMS    .....    |
   MINISTEP  1        |==> This is REPORT STEP 1, in file BASE.S00001
   PARAMS    .....    |
   MINISTEP  2        |
   PARAMS    .....   /
   ------------------
   SEQHDR            \
   MINISTEP  3        |
   PARAMS    .....    |
   MINISTEP  4        |
   PARAMS    .....    |
   MINISTEP  5        |==> This is REPORT STEP 2, in file BASE.S0002
   PARAMS    .....    |
   MINISTEP  6        |
   PARAMS    .....    |
   SEQHDR             |
   MINISTEP  7        |
   PARAMS    .....   /
   ------------------


   Observe the following:

     * The MINISTEP counter runs continously, and does not
       differentiate between unified files and not unified files.

     * When using multiple files we can read off the report number
       from the filename, for unified files this is IMPOSSIBLE, and we
       just have to assume that the first block corresponds to
       report_step 1 and then count afterwards.

     * When asking for a summary variable at a particular REPORT STEP
       (as we do in enkf) it is ambigous as to which ministep within
       the block one should use. The convention we have employed
       (which corresponds to the old RPTONLY based behaviour) is to
       use the last ministep in the block.

     * There is no BASE.SOOOO file

     * The report steps are halfopen intervals in the "wrong way":
       (....]




   About MINISTEP, REPORTSTEP, rates and continous sim_time/sim_days:
   ------------------------------------------------------------------

   For ECLIPSE summary files the smallest unit of time resolution is
   called the ministep - a ministep corresponds to a time step in the
   underlying partial differential equation, i.e. the length of the
   timesteps is controlled by the simulator itself - there is no finer
   temporal resolution.

   The user has told the simulator to store (i.e. save to file
   results) the results at reportsteps. A reportstep will typically
   consist of several ministeps. The timeline below shows a simulation
   consisting of two reportsteps:


                                                 S0001                                          S0002
   ||------|------|------------|------------------||----------------------|----------------------||
          M1     M2           M3                 M4                      M5                     M6

   The first reportstep consist of four ministeps, the second
   reportstep consits of only two ministeps. As a user you have no
   control over the length/number of ministeps apart from:

      1. Indirectly through the TUNING keywords.
      2. A ministep will always end at a report step.


   RPTONLY: In conjunction with enkf it has been customary to use the
   keyword RPTONLY. This is purely a storage directive, the effect is
   that only the ministep ending at the REPORT step is reported,
   i.e. in the case above we would get the ministeps [M4 , M6], where
   the ministeps M4 and M6 will be unchanged, and there will be many
   'holes' in the timeline.

   About truetime: The ministeps have a finite length; this implies
   that

     [rates]: The ministep value is NOT actually an instantaneous
        value, it is the total production during the ministepd period
        - divided by the length of the ministep. I.e. it is an average
        value. (I.e. the differential time element dt is actually quite
        looong).

     [state]: For state variables (this will include total production
        of various phases), the ministep value corresponds to the
        reservoir state at THE END OF THE MINISTEP.

   This difference between state variables and rates implies a
   difference in how continous time-variables (in the middle of a
   ministep) are reported, i.e.


   S0000                                                      S0001
   ||--------------|---------------|------------X-------------||
                  M1              M2           /|\            M3
                                                |
                                                |

   We have enteeed the sim_days/sim_time cooresponding to the location
   of 'X' on the timeline, i.e. in the middle of ministep M3. If we
   are interested in the rate at this time the function:

        ecl_sum_data_get_from_sim_time()

   will just return the M3 value, whereas if you are interested in
   e.g. pressure at this time the function will return a weighted
   average of the M2 and M3 values. Whether a variable in question is
   interpreted as a 'rate' is effectively determined by the
   ecl_smspec_set_rate_var() function in ecl_smspec.c.



   Indexing and _get() versus _iget()
   ----------------------------------
   As already mentionded the set of ministeps is not necessarrily a
   continous series, we can easily have a series of ministeps with
   "holes" in it, and the series can also start on a non-zero
   value. Internally all the ministeps are stored in a dense, zero
   offset vector instance; and we must be able to translate back and
   forth between ministep_nr and internal index.

   Partly due to EnKF heritage the MINISTEP nr has been the main
   method to access the time dimension of the data, i.e. all the
   functions like ecl_sum_get_general_var() expect the time direction
   to be given as a ministep; however it is also possible to get the
   data by giving an internal (not that internal ...) index. In
   ecl_sum_data.c the latter functions have _iget():


      ecl_sum_data_get_xxx : Expects the time direction given as a ministep_nr.
      ecl_sum_data_iget_xxx: Expects the time direction given as an internal index.

*/


namespace ecl {


ecl_sum_file_data::ecl_sum_file_data(const ecl_smspec_type * smspec) :
  ecl_smspec( smspec ),
  data( vector_alloc_new() )
{
}

ecl_sum_file_data::~ecl_sum_file_data() {
  vector_free( data );
}


int ecl_sum_file_data::length() const {
  if (this->loader)
    return this->loader->length();
  else
    return this->index.size();
}


int ecl_sum_file_data::length_before(time_t end_time) const {
  int offset = 0;
  while (true) {
    time_t itime = this->iget_sim_time(offset);
    if (itime >= end_time)
      return offset;

    offset += 1;
    if (offset == this->length())
      return offset;
  }
}


int ecl_sum_file_data::report_before(time_t end_time) const {
  if (end_time < this->first_report())
    throw std::invalid_argument("time argument before first report step");

  int r = this->first_report();
  int last_report = this->last_report();
  while (true) {
    if (r == last_report)
      return last_report;

    auto next_range = this->index.report_range(r + 1);
    if (this->iget_sim_time(next_range.first) > end_time)
      return r;

    r += 1;
  }
}


int  ecl_sum_file_data::first_report() const{
  const auto& node = this->index[0];
  return node.report_step;
}

int ecl_sum_file_data::last_report() const {
  const auto& node = this->index.back();
  return node.report_step;
}



time_t ecl_sum_file_data::get_data_start() const {
  const auto& node = this->index[0];
  return node.sim_time;
}


time_t ecl_sum_file_data::get_sim_end() const {
  const auto& node = this->index.back();
  return node.sim_time;
}

time_t ecl_sum_file_data::iget_sim_time(int time_index) const {
  const auto& node = this->index[time_index];
  return node.sim_time;
}

/*
  Will return the length of the simulation in whatever units were used in input.
*/
double ecl_sum_file_data::get_sim_length() const {
  const auto& node = this->index.back();
  return node.sim_seconds / ecl_smspec_get_time_seconds( this->ecl_smspec );
}

double ecl_sum_file_data::iget( int time_index , int params_index ) const {
  if (this->loader)
    return this->loader->iget(time_index, params_index);
  else {
    const ecl_sum_tstep_type * ministep_data = iget_ministep( time_index  );
    return ecl_sum_tstep_iget( ministep_data , params_index);
  }
}



void ecl_sum_file_data::append_tstep(ecl_sum_tstep_type * tstep) {
  /*
     Here the tstep is just appended naively, the vector will be
     sorted by ministep_nr before the data instance is returned.
  */

  vector_append_owned_ref( data , tstep , ecl_sum_tstep_free__);
}


/*
  This function is meant to be called in write mode; and will create a
  new and empty tstep which is appended to the current data. The tstep
  will also be returned, so the calling scope can call
  ecl_sum_tstep_iset() to set elements in the tstep.
*/

ecl_sum_tstep_type * ecl_sum_file_data::add_new_tstep( int report_step , double sim_seconds) {
  int ministep_nr = vector_get_size( data );
  ecl_sum_tstep_type * tstep = ecl_sum_tstep_alloc_new( report_step , ministep_nr , sim_seconds , ecl_smspec );
  ecl_sum_tstep_type * prev_tstep = NULL;

  if (vector_get_size( data ) > 0)
    prev_tstep = (ecl_sum_tstep_type*) vector_get_last( data );

  append_tstep( tstep );


  bool rebuild_index = true;
  /*
    In the simple case that we just add another timestep to the
    currently active report_step, we do a limited update of the
    index, otherwise we call ecl_sum_data_build_index() to get a
    full recalculation of the index.
  */
  if (!prev_tstep)
    goto exit;

  if (ecl_sum_tstep_get_report(prev_tstep) != ecl_sum_tstep_get_report( tstep ))
    goto exit;

  if (ecl_sum_tstep_get_sim_days( prev_tstep ) >= ecl_sum_tstep_get_sim_days( tstep ))
    goto exit;

  this->index.add(ecl_sum_tstep_get_sim_time(tstep), sim_seconds, report_step);
  rebuild_index = false;

exit:
  if (rebuild_index)
      this->build_index();

  return tstep;
}


ecl_sum_tstep_type * ecl_sum_file_data::iget_ministep( int internal_index ) const {
  return (ecl_sum_tstep_type*)vector_iget( data , internal_index );
}

double ecl_sum_file_data::iget_sim_days(int time_index ) const {
  const auto& node = this->index[time_index];
  return node.sim_seconds / 86400;
}

double ecl_sum_file_data::iget_sim_seconds(int time_index ) const {
  const auto& node = this->index[time_index];
  return node.sim_seconds;
}


static int cmp_ministep( const void * arg1 , const void * arg2) {
  const ecl_sum_tstep_type * ministep1 = ecl_sum_tstep_safe_cast_const( arg1 );
  const ecl_sum_tstep_type * ministep2 = ecl_sum_tstep_safe_cast_const( arg2 );

  time_t time1 = ecl_sum_tstep_get_sim_time( ministep1 );
  time_t time2 = ecl_sum_tstep_get_sim_time( ministep2 );

  if (time1 < time2)
    return -1;
  else if (time1 == time2)
    return 0;
  else
    return 1;
}



void ecl_sum_file_data::build_index( ) {
  this->index.clear();

  if (this->loader) {
    int offset = ecl_smspec_get_first_step(this->ecl_smspec) - 1;
    std::vector<int> report_steps = this->loader->report_steps(offset);
    std::vector<time_t> sim_time = this->loader->sim_time();
    std::vector<double> sim_seconds = this->loader->sim_seconds();

    for (int i=0; i < this->loader->length(); i++) {
      this->index.add(sim_time[i],
                      sim_seconds[i],
                      report_steps[i]);
    }
  } else {
    vector_sort( data , cmp_ministep );
    for (int internal_index = 0; internal_index < vector_get_size( data ); internal_index++) {
      const ecl_sum_tstep_type * ministep = iget_ministep( internal_index  );
      this->index.add(ecl_sum_tstep_get_sim_time(ministep),
                      ecl_sum_tstep_get_sim_seconds(ministep),
                      ecl_sum_tstep_get_report(ministep));
    }
  }
}

void ecl_sum_file_data::get_time(int length, time_t * data) {
  for (int time_index=0; time_index < length; time_index++)
    data[time_index] = this->iget_sim_time(time_index);
}


int ecl_sum_file_data::get_time_report(int end_index, time_t *data) {
  int offset = 0;

  for (int report_step = this->first_report(); report_step <= this->last_report(); report_step++) {
    const auto& range = this->report_range(report_step);
    int time_index = range.second;
    if (time_index >= end_index)
      break;

    data[offset] = this->iget_sim_time(time_index);

    offset += 1;
  }
  return offset;
}



void ecl_sum_file_data::get_data(int params_index, int length, double *data) {
  if (this->loader) {
    const auto tmp_data = loader->get_vector(params_index);
    memcpy(data, tmp_data.data(), length * sizeof data);
  } else {
    for (int time_index=0; time_index < length; time_index++)
      data[time_index] = this->iget(time_index, params_index);
  }
}


int ecl_sum_file_data::get_data_report(int params_index, int end_index, double *data, double default_value) {
  int offset = 0;

  for (int report_step = this->first_report(); report_step <= this->last_report(); report_step++) {
    int time_index = this->index.report_range(report_step).second;
    if (time_index >= end_index)
      break;

    if (params_index >= 0)
      data[offset] = this->iget(time_index, params_index);
    else
      data[offset] = default_value;

    offset += 1;
  }
  return offset;
}



bool ecl_sum_file_data::has_report(int report_step ) const {
  return this->index.has_report(report_step);
}


// ******************** Start writing ***************************************************


std::pair<int,int> ecl_sum_file_data::report_range(int report_step) const {
  return this->index.report_range(report_step);
}


void ecl_sum_file_data::fwrite_report( int report_step , fortio_type * fortio) const {
  {
    ecl_kw_type * seqhdr_kw = ecl_kw_alloc( SEQHDR_KW , SEQHDR_SIZE , ECL_INT );
    ecl_kw_iset_int( seqhdr_kw , 0 , 0 );
    ecl_kw_fwrite( seqhdr_kw , fortio );
    ecl_kw_free( seqhdr_kw );
  }

  {
    auto range = this->report_range( report_step );
    for (int index = range.first; index <= range.second; index++) {
      const ecl_sum_tstep_type * tstep = iget_ministep( index );
      ecl_sum_tstep_fwrite( tstep , ecl_smspec_get_index_map( ecl_smspec ) , fortio );
    }
  }
}


void ecl_sum_file_data::fwrite_unified( fortio_type * fortio ) const {
  if (this->length() == 0)
    return;

  for (int report_step = first_report(); report_step <= last_report(); report_step++) {
    if (has_report( report_step ))
      fwrite_report( report_step , fortio );
  }
}


void ecl_sum_file_data::fwrite_multiple( const char * ecl_case , bool fmt_case ) const {
  if (this->length() == 0)
    return;

  for (int report_step = this->first_report(); report_step <= this->last_report(); report_step++) {
    if (this->has_report( report_step )) {
      char * filename = ecl_util_alloc_filename( NULL , ecl_case , ECL_SUMMARY_FILE , fmt_case , report_step );
      fortio_type * fortio = fortio_open_writer( filename , fmt_case , ECL_ENDIAN_FLIP );

      fwrite_report( report_step , fortio );

      fortio_fclose( fortio );
      free( filename );
    }
  }
}


bool ecl_sum_file_data::can_write() const {
  if (this->loader)
    return false;

  return true;
}

double ecl_sum_file_data::get_days_start() const {
  const auto& node = this->index[0];
  return node.sim_seconds * 86400;
}


// ***************************** End writing *************************************

// **************************** Start Reading ************************************

bool ecl_sum_file_data::check_file( ecl_file_type * ecl_file ) {
  return ecl_file_has_kw( ecl_file , PARAMS_KW ) &&
    (ecl_file_get_num_named_kw( ecl_file , PARAMS_KW ) == ecl_file_get_num_named_kw( ecl_file , MINISTEP_KW));
}

/**
   Malformed/incomplete files:
   ----------------------------
   Observe that ECLIPSE works in the following way:

     1. At the start of a report step a summary data section
        containing only the 'SEQHDR' keyword is written - this is
        currently an 'invalid' summary section.

     2. ECLIPSE simulates as best it can.

     3. When the time step is complete data is written to the summary
        file.

   Now - if ECLIPSE goes down in flames during step 2 a malformed
   summary file will be left around, to handle this situation
   reasonably gracefully we check that the ecl_file instance has at
   least one "PARAMS" keyword.

   One ecl_file corresponds to one report_step (limited by SEQHDR); in
   the case of non unfied summary files these objects correspond to
   one BASE.Annnn or BASE.Snnnn file, in the case of unified files the
   calling routine will read the unified summary file partly.
*/

void ecl_sum_file_data::add_ecl_file(int report_step, const ecl_file_view_type * summary_view, const ecl_smspec_type * smspec) {

  int num_ministep  = ecl_file_view_get_num_named_kw( summary_view , PARAMS_KW);
  if (num_ministep > 0) {
    int ikw;

    for (ikw = 0; ikw < num_ministep; ikw++) {
      ecl_kw_type * ministep_kw = ecl_file_view_iget_named_kw( summary_view , MINISTEP_KW , ikw);
      ecl_kw_type * params_kw   = ecl_file_view_iget_named_kw( summary_view , PARAMS_KW   , ikw);

      {
        int ministep_nr = ecl_kw_iget_int( ministep_kw , 0 );
        ecl_sum_tstep_type * tstep = ecl_sum_tstep_alloc_from_file( report_step ,
                                                                    ministep_nr ,
                                                                    params_kw ,
                                                                    ecl_file_view_get_src_file( summary_view ),
                                                                    smspec );

        if (tstep)
            append_tstep( tstep );
      }
    }
  }
}


bool ecl_sum_file_data::fread(const stringlist_type * filelist, bool lazy_load) {
  if (stringlist_get_size( filelist ) == 0)
    return false;

  ecl_file_enum file_type = ecl_util_get_file_type( stringlist_iget( filelist , 0 ) , NULL , NULL);
  if ((stringlist_get_size( filelist ) > 1) && (file_type != ECL_SUMMARY_FILE))
    util_abort("%s: internal error - when calling with more than one file - you can not supply a unified file - come on?! \n",__func__);

  if (file_type == ECL_SUMMARY_FILE) {

    /* Not unified. */
    for (int filenr = 0; filenr < stringlist_get_size( filelist ); filenr++) {
      const char * data_file = stringlist_iget( filelist , filenr);
      ecl_file_enum file_type;
      int report_step;
      file_type = ecl_util_get_file_type( data_file , NULL , &report_step);
      if (file_type != ECL_SUMMARY_FILE)
        util_abort("%s: file:%s has wrong type \n",__func__ , data_file);
      {
        ecl_file_type * ecl_file = ecl_file_open( data_file , 0);
        if (ecl_file && check_file( ecl_file )) {
          add_ecl_file( report_step , ecl_file_get_global_view( ecl_file ) , ecl_smspec);
          ecl_file_close( ecl_file );
        }
      }
    }
  } else if (file_type == ECL_UNIFIED_SUMMARY_FILE) {
    if (lazy_load) {
      try {
        this->loader.reset( new unsmry_loader( this->ecl_smspec, stringlist_iget(filelist, 0)) );
      }
      catch(const std::bad_alloc& e)
      {
        return false;
      }
    } else {

      // Is this correct for a restarted chain of UNSMRY files? Looks like the
      // report step sequence will be restarted?
      ecl_file_type * ecl_file = ecl_file_open( stringlist_iget(filelist ,0 ) , 0);
      if (ecl_file && check_file( ecl_file )) {
        int first_report_step = ecl_smspec_get_first_step(this->ecl_smspec);
        int block_index = 0;
        while (true) {
          /*
            Observe that there is a number discrepancy between ECLIPSE
            and the ecl_file_select_smryblock() function. ECLIPSE
            starts counting report steps at 1; whereas the first
            SEQHDR block in the unified summary file is block zero (in
            ert counting).
        */
          ecl_file_view_type * summary_view = ecl_file_get_summary_view(ecl_file , block_index);
          if (summary_view) {
            add_ecl_file(block_index + first_report_step , summary_view , ecl_smspec);
            block_index++;
          } else break;
        }
        ecl_file_close( ecl_file );
      }
    }
  }

  build_index();
  return (length() > 0);
}

const ecl_smspec_type * ecl_sum_file_data::smspec() const {
  return this->ecl_smspec;
}


bool ecl_sum_file_data::report_step_equal( const ecl_sum_file_data& other, bool strict) const {
  if (strict && this->first_report() != other.first_report())
    return false;

  if (strict && (this->last_report() != other.last_report()))
    return false;

  int report_step = std::max(this->first_report(), other.first_report());
  int last_report = std::min(this->last_report(), other.last_report());
  while (true) {
    int time_index1 = this->report_range(report_step).second;
    int time_index2 = other.report_range(report_step).second;

    if ((time_index1 != INVALID_MINISTEP_NR) && (time_index2 != INVALID_MINISTEP_NR)) {
      time_t time1 = this->iget_sim_time(time_index1);
      time_t time2 = other.iget_sim_time(time_index2);

      if (time1 != time2)
        return false;

    } else if (time_index1 != time_index2) {
      if (strict)
        return false;
    }

    report_step++;
    if (report_step > last_report)
      break;
  }
  return true;
}


// ***************************** End reading *************************************

int ecl_sum_file_data::report_step_from_days(double sim_days) const {
  int report_step = this->first_report();
  double sim_seconds = sim_days * 86400;
  while (true) {
    const auto& range = this->index.report_range(report_step);
    if (range.second >= 0) {
      const auto& node = this->index[range.second];

      // Warning - this is a double == comparison!
      if (sim_seconds == node.sim_seconds)
        return report_step;

      report_step++;
      if (report_step > this->last_report())
        return -1;
    }
  }
}

int ecl_sum_file_data::report_step_from_time(time_t sim_time) const {
  int report_step = this->first_report();
  while (true) {
    const auto& range = this->index.report_range(report_step);
    if (range.second >= 0) {
      const auto& node = this->index[range.second];
      if (sim_time == node.sim_time)
        return report_step;

      report_step++;
      if (report_step > this->last_report())
        return -1;
    }
  }
}

int ecl_sum_file_data::iget_report(int time_index) const {
  const auto& index_node = this->index[time_index];
  return index_node.report_step;
}


} //end namespace


