/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_sum_data.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <string.h>
#include <math.h>

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <string>

#include <ert/util/util.h>
#include <ert/util/vector.hpp>
#include <ert/util/time_t_vector.hpp>
#include <ert/util/int_vector.hpp>
#include <ert/util/stringlist.hpp>
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_smspec.hpp>
#include <ert/ecl/ecl_sum_data.hpp>
#include <ert/ecl/ecl_sum_tstep.hpp>
#include <ert/ecl/smspec_node.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_endian_flip.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_sum_vector.hpp>

#include "detail/ecl/ecl_sum_file_data.hpp"

/*
  This file implements the sruct ecl_sum_data which manages the actual simulated
  values from a summary file, including all time-related information. In the
  case of restarted simulations the different summary cases will be internalized
  as separate ecl_sum_file_data instances. The ecl_sum_file_data class is an
  internal implemenation detail which is not exported. More details about the
  internal storage of summary data can be found in that file.

  For this file the implementation mainly consists of maintaining an ordered
  list of ecl_sum_file_data instances and dispatching method calls to the right
  ecl_sum_file_data instance.
*/

namespace {

/*
  The class CaseIndex and the struct IndexNode are used to maintain a list of
  the ecl_sum_file_data instances, and lookup the correct one based one various
  time related arguments.
*/

  struct IndexNode {
    IndexNode(int d, int o, int l) {
      this->data_index = d;
      this->offset = o;
      this->length = l;
    }

    int end() const {
      return this->offset + this->length;
    }

    int data_index;
    int offset;
    int length;
    int report1;
    int report2;
    time_t time1;
    time_t time2;
    double days1;
    double days2;
    std::vector<int> params_map;
  };


  class CaseIndex {
  public:

    IndexNode& add(int length) {
      int offset = 0;
      int data_index = this->index.size();

      if (!this->index.empty())
        offset = this->index.back().end();

      this->index.emplace_back(data_index, offset, length);
      return this->index.back();
    }

/*
  The lookup_time() and lookup_report() methods will lookup which file_data
  instance corresponds to the time/report argument. The methods will return two
  pointers to file_data instances, if the argument is inside one file_data
  instance the pointers will be equal - otherwise they will point to the
  file_data instance before and after the argument:

  File 1                     File 2
  |------|-----|------|      |----|----------|---|
      /|\                /|\
       |                  |
       |                  |
       A                  B

  For time A the lookup_time function will return <file1,file1> whereas for time
  B the function will return <file1,file2>.
 */

    std::pair<const IndexNode*, const IndexNode *> lookup_time(time_t sim_time) const {
      auto iter = this->index.begin();
      auto next = this->index.begin();
      if (sim_time < iter->time1)
        throw std::invalid_argument("Simulation time out of range");

      ++next;
      while (true) {
        double t1 = iter->time1;
        double t2 = iter->time2;


        if (sim_time>= t1) {
          if (sim_time <= t2)
            return std::make_pair<const IndexNode*, const IndexNode*>(&(*iter), &(*iter));

          if (next == this->index.end())
            throw std::invalid_argument("Simulation days out of range");

          if (sim_time < next->time1)
            return std::make_pair<const IndexNode*, const IndexNode*>(&(*iter),&(*next));
        }
        ++next;
        ++iter;
      }
    }


    std::pair<const IndexNode*, const IndexNode *> lookup_days(double days) const {
      auto iter = this->index.begin();
      auto next = this->index.begin();
      if (days < iter->days1)
        throw std::invalid_argument("Simulation days out of range");

      ++next;
      while (true) {
        double d1 = iter->days1;
        double d2 = iter->days2;


        if (days >= d1) {
          if (days <= d2)
            return std::make_pair<const IndexNode*, const IndexNode*>(&(*iter), &(*iter));

          if (next == this->index.end())
            throw std::invalid_argument("Simulation days out of range");

          if (days < next->days1)
            return std::make_pair<const IndexNode*, const IndexNode*>(&(*iter),&(*next));
        }
        ++next;
        ++iter;
      }
    }

    const IndexNode& lookup(int internal_index) const {
      for (const auto& node : this->index)
        if (internal_index >= node.offset && internal_index < node.end())
          return node;

      throw std::invalid_argument("Internal error when looking up index: " + std::to_string(internal_index));
    }


    const IndexNode& lookup_report(int report) const {
      for (const auto& node : this->index)
        if (node.report1 <= report && node.report2 >= report)
          return node;

      throw std::invalid_argument("Internal error when looking up report: " + std::to_string(report));
    }

    /*
      This will check that we have a datafile which report range covers the
      report argument, in adition there can be 'holes' in the series - that must
      be checked by actually querying the data_file object.
    */

    bool has_report(int report) const {
      for (const auto& node : this->index)
        if (node.report1 <= report && node.report2 >= report)
          return true;

      return false;
    }

    IndexNode& back() {
      return this->index.back();
    }

    void clear() {
      this->index.clear();
    }

    int length() const {
      return this->index.back().end();
    }

    std::vector<IndexNode>::const_iterator begin() const {
      return this->index.begin();
    }

    std::vector<IndexNode>::const_iterator end() const {
      return this->index.end();
    }

  private:
    std::vector<IndexNode> index;
  };

}



struct ecl_sum_data_struct {
  const ecl_smspec_type  * smspec;
  std::vector<ecl::ecl_sum_file_data*> data_files;              // List of ecl_sum_file_data instances
  CaseIndex              index;
};


static void ecl_sum_data_build_index( ecl_sum_data_type * self );
static double ecl_sum_data_iget_sim_seconds( const ecl_sum_data_type * data , int internal_index );



/*****************************************************************/

 void ecl_sum_data_free( ecl_sum_data_type * data ) {
  if (!data)
    throw std::invalid_argument(__func__ + std::string(": invalid delete") );

  if (data->data_files.size() > 0)
    delete data->data_files.back();

  delete data;
}


ecl_sum_data_type * ecl_sum_data_alloc(ecl_smspec_type * smspec) {
  ecl_sum_data_type * data =  new ecl_sum_data_type();
  data->smspec = smspec;
  return data;
}


void ecl_sum_data_reset_self_map( ecl_sum_data_type * data ) {
  ecl_sum_data_build_index(data);
}


static void ecl_sum_data_append_file_data( ecl_sum_data_type * sum_data, ecl::ecl_sum_file_data * file_data) {
  sum_data->data_files.push_back( file_data );
}



/**
   This function will take a report as input , and update the two
   pointers ministep1 and ministep2 with the range of the report step
   (in terms of ministeps).

   Calling this function with report_step == 2 for the example
   documented at the top of the file will yield: *ministep1 = 3 and
   *ministep2 = 7. If you are only interested in one of the limits you
   can pass in NULL for the other limit, i.e.

      xxx(data , report_step , NULL , &ministep2);

   to get the last step.

   If the supplied report_step is invalid the function will set both
   return values to -1 (the return value from safe_iget). In that case
   it is the responsability of the calling scope to check the return
   values, alternatively one can use the query function
   ecl_sum_data_has_report_step() first.
*/




static double ecl_sum_data_iget_sim_seconds( const ecl_sum_data_type * data , int internal_index ) {
  const auto index_node = data->index.lookup(internal_index);
  const auto data_file = data->data_files[index_node.data_index];
  return data_file->iget_sim_seconds( internal_index - index_node.offset );
}


double ecl_sum_data_iget_sim_days( const ecl_sum_data_type * data , int internal_index ) {
  const auto index_node = data->index.lookup(internal_index);
  const auto data_file = data->data_files[index_node.data_index];
  return data_file->iget_sim_days( internal_index - index_node.offset );
}





ecl_sum_data_type * ecl_sum_data_alloc_writer( ecl_smspec_type * smspec ) {
  ecl_sum_data_type * data = ecl_sum_data_alloc( smspec );
  ecl::ecl_sum_file_data * file_data = new ecl::ecl_sum_file_data( smspec );
  ecl_sum_data_append_file_data( data, file_data );
  ecl_sum_data_build_index(data);
  return data;
}


static void ecl_sum_data_fwrite_unified( const ecl_sum_data_type * data , const char * ecl_case , bool fmt_case ) {
  char * filename = ecl_util_alloc_filename( NULL , ecl_case , ECL_UNIFIED_SUMMARY_FILE , fmt_case , 0 );
  fortio_type * fortio = fortio_open_writer( filename , fmt_case , ECL_ENDIAN_FLIP );

  for (size_t index = 0; index < data->data_files.size(); index++)
    data->data_files[index]->fwrite_unified( fortio );

  fortio_fclose( fortio );
  free( filename );
}


static void ecl_sum_data_fwrite_multiple( const ecl_sum_data_type * data , const char * ecl_case , bool fmt_case ) {

  for (size_t index = 0; index < data->data_files.size(); index++)
    data->data_files[index]->fwrite_multiple(ecl_case, fmt_case);

}


void ecl_sum_data_fwrite( const ecl_sum_data_type * data , const char * ecl_case , bool fmt_case , bool unified) {
  if (unified)
    ecl_sum_data_fwrite_unified( data , ecl_case , fmt_case );
  else
    ecl_sum_data_fwrite_multiple( data , ecl_case , fmt_case );
}


bool ecl_sum_data_can_write(const ecl_sum_data_type * data) {
  bool can_write = true;
  for (const auto& file_ptr : data->data_files)
    can_write &= file_ptr->can_write();

  return can_write;
}

time_t ecl_sum_data_get_sim_end(const ecl_sum_data_type * data ) {
  const auto& file_data = data->data_files.back();
  return file_data->get_sim_end();
}

time_t ecl_sum_data_get_data_start( const ecl_sum_data_type * data ) {
  const auto& file_data = data->data_files[0];
  return file_data->get_data_start();
}

double ecl_sum_data_get_first_day( const ecl_sum_data_type * data) {
  const auto& file_data = data->data_files[0];
  return file_data->get_days_start();
}

/**
   Returns the number of simulations days from the start of the
   simulation (irrespective of whether the that summary data has
   actually been loaded) to the last loaded simulation step.
*/

double ecl_sum_data_get_sim_length( const ecl_sum_data_type * data ) {
  const auto& file_data = data->data_files.back();
  return file_data->get_sim_length();
}







/**
   The check_sim_time() and check_sim_days() routines check if you
   have summary data for the requested date/days value. In the case of
   a restarted case, where the original case is missing - this will
   return false if the input values are in the region after simulation
   start with no data.
*/

bool ecl_sum_data_check_sim_time( const ecl_sum_data_type * data , time_t sim_time) {
  if (sim_time < ecl_sum_data_get_data_start(data))
    return false;

  if (sim_time > ecl_sum_data_get_sim_end(data))
    return false;

  return true;
}


bool ecl_sum_data_check_sim_days( const ecl_sum_data_type * data , double sim_days) {
  return sim_days >= ecl_sum_data_get_first_day(data) && sim_days <= ecl_sum_data_get_sim_length(data);
}




/**
   This function will return the ministep corresponding to a time_t
   instance 'sim_time'. The function will fail hard if the time_t is
   before the simulation start, or after the end of the
   simulation. Check with

       ecl_smspec_get_start_time() and ecl_sum_data_get_sim_end()

   first.

   See the documentation about report steps, ministeps and rates at
   the top of this file for how the sim_time relates to to the
   returned ministep_nr.

   The indices used in this function are the internal indices, and not
   ministep numbers. Observe that if there are holes in the
   time-domain, i.e. if RPTONLY has been used, the function can return
   a ministep index which does NOT cover the input time:

     The 'X' should represent report times - the dashed lines
     represent the temporal extent of two ministeps. Outside the '--'
     area we do not have any results. The two ministeps we actually
     have are M15 and M25, i.e. there is a hole.


      X      .      +-----X            +----X
            /|\        M15               M25
             |
             |

     When asking for the ministep number at the location of the arrow,
     the function will return '15', i.e. the valid ministep following
     the sim_time. Of course - the ideal situation is if the time
     sequence has no holes.
*/


static int ecl_sum_data_get_index_from_sim_time( const ecl_sum_data_type * data , time_t sim_time) {
  if (!ecl_sum_data_check_sim_time(data, sim_time)) {
    time_t start_time = ecl_sum_data_get_data_start(data);
    time_t end_time = ecl_sum_data_get_sim_end(data);

    fprintf(stderr , "Simulation start: "); util_fprintf_date_utc( ecl_smspec_get_start_time( data->smspec ) , stderr );
    fprintf(stderr , "Data start......: "); util_fprintf_date_utc( start_time , stderr );
    fprintf(stderr , "Simulation end .: "); util_fprintf_date_utc( end_time , stderr );
    fprintf(stderr , "Requested date .: "); util_fprintf_date_utc( sim_time , stderr );
    util_abort("%s: invalid time_t instance:%d  interval:  [%d,%d]\n",__func__, sim_time , start_time, end_time);
  }

  /*
     The moment we have passed the intial test we MUST find a valid
     ministep index, however care should be taken that there can
     perfectly well be 'holes' in the time domain, because of e.g. the
     RPTONLY keyword.
  */

  int low_index = 0;
  int high_index = ecl_sum_data_get_length(data) - 1;

  // perform binary search
  while (low_index+1 < high_index) {
    int center_index = (low_index + high_index) / 2;
    const time_t center_time = ecl_sum_data_iget_sim_time(data, center_index);

    if (sim_time > center_time)
      low_index = center_index;
    else
      high_index = center_index;
  }

  return sim_time <= ecl_sum_data_iget_sim_time(data, low_index) ? low_index : high_index;
}

int ecl_sum_data_get_index_from_sim_days( const ecl_sum_data_type * data , double sim_days) {
  time_t sim_time = ecl_smspec_get_start_time( data->smspec );
  util_inplace_forward_days_utc( &sim_time , sim_days );
  return ecl_sum_data_get_index_from_sim_time(data , sim_time );
}


/**
   This function will take a true time 'sim_time' as input. The
   ministep indices bracketing this sim_time are identified, and the
   corresponding weights are calculated.

   The actual value we are interested in can then be computed with the
   ecl_sum_data_interp_get() function:


   int    param_index;
   time_t sim_time;
   {
      int    ministep1 , ministep2;
      double weight1   , weight2;

      ecl_sum_data_init_interp_from_sim_time( data , sim_time , &ministep1 , &ministep2 , &weight1 , &weight2);
      return ecl_sum_data_interp_get( data , ministep1 , ministep2 , weight1 , weight2 , param_index );
   }


   For further explanation (in particular for which keywords the
   function should be used), consult documentation at the top of this
   file.
*/

void ecl_sum_data_init_interp_from_sim_time(const ecl_sum_data_type* data,
                                            time_t sim_time,
                                            int* index1,
                                            int* index2,
                                            double* weight1,
                                            double* weight2) {
  int idx = ecl_sum_data_get_index_from_sim_time(data, sim_time);

  // if sim_time is first date, idx=0 and then we cannot interpolate, so we give
  // weight 1 to index1=index2=0.
  if (idx == 0) {
    *index1 = 0;
    *index2 = 0;
    *weight1 = 1;
    *weight2 = 0;
    return;
  }

  time_t sim_time1 = ecl_sum_data_iget_sim_time(data, idx-1);
  time_t sim_time2 = ecl_sum_data_iget_sim_time(data, idx);

  *index1 = idx-1;
  *index2 = idx;

  // weights the interpolation each of the ministeps according to distance from sim_time
  double time_diff  = sim_time2 - sim_time1;
  double time_dist1 =  (sim_time - sim_time1);
  double time_dist2 = -(sim_time - sim_time2);

  *weight1 = time_dist2 / time_diff;
  *weight2 = time_dist1 / time_diff;
}



void ecl_sum_data_init_interp_from_sim_days( const ecl_sum_data_type * data , double sim_days, int *step1, int *step2 , double * weight1 , double *weight2) {
  time_t sim_time = ecl_smspec_get_start_time( data->smspec );
  util_inplace_forward_days_utc( &sim_time , sim_days );
  ecl_sum_data_init_interp_from_sim_time( data , sim_time , step1 , step2 , weight1 , weight2);
}


double_vector_type * ecl_sum_data_alloc_seconds_solution(const ecl_sum_data_type * data, const smspec_node_type * node, double cmp_value, bool rates_clamp_lower) {
  double_vector_type * solution = double_vector_alloc(0, 0);
  const int param_index = smspec_node_get_params_index(node);
  const int size = ecl_sum_data_get_length(data);

  if (size <= 1)
    return solution;

  for (int index = 0; index < size; ++index) {
    int prev_index    = util_int_max(0, index-1);
    double value      = ecl_sum_data_iget(data, index, param_index);
    double prev_value = ecl_sum_data_iget(data, prev_index, param_index);

    // cmp_value in interval value (closed) and prev_value (open)
    bool contained = (value == cmp_value);
    contained |= (util_double_min(prev_value, value) < cmp_value) &&
            (cmp_value < util_double_max(prev_value, value));

    if (!contained)
      continue;

    double prev_time = ecl_sum_data_iget_sim_seconds(data, prev_index);
    double time      = ecl_sum_data_iget_sim_seconds(data, index);

    if (smspec_node_is_rate(node)) {
      double_vector_append(solution, rates_clamp_lower ? prev_time + 1 : time);
    } else {
      double slope = (value - prev_value) / (time - prev_time);
      double seconds = (cmp_value - prev_value)/slope + prev_time;
      double_vector_append(solution, seconds);
    }
  }
  return solution;
}


static void ecl_sum_data_build_index( ecl_sum_data_type * self ) {
  std::sort(self->data_files.begin(), self->data_files.end(),
            [](const ecl::ecl_sum_file_data* case1,
               const ecl::ecl_sum_file_data* case2)
            {
              return case1->get_data_start() < case2->get_data_start();
            });

  self->index.clear();
  for (size_t i=0; i < self->data_files.size(); i++) {
    const auto& data = self->data_files[i];
    bool main_case = (i == (self->data_files.size() - 1));
    time_t next_start;

    if (main_case)
      self->index.add(data->length());
    else {
      const auto& next = self->data_files[i+1];
      next_start = next->get_data_start();
      self->index.add( data->length_before(next_start));
    }

    auto & node = self->index.back();
    if (node.length > 0) {
      node.report1 = data->first_report();

      if (main_case)
        node.report2 = data->last_report();
      else
        node.report2 = data->report_before( next_start );

      node.time1   = data->get_data_start();
      node.time2   = data->get_sim_end();
      node.days1   = data->get_days_start();
      node.days2   = data->get_sim_length();
      {
        int * tmp_map = ecl_smspec_alloc_mapping( self->smspec , data->smspec() );
        node.params_map.assign(tmp_map, tmp_map + ecl_smspec_get_params_size(self->smspec));
        free( tmp_map );
      }
    }
  }

}



/*
  This function is meant to be called in write mode; and will create a
  new and empty tstep which is appended to the current data. The tstep
  will also be returned, so the calling scope can call
  ecl_sum_tstep_iset() to set elements in the tstep.
*/

ecl_sum_tstep_type * ecl_sum_data_add_new_tstep( ecl_sum_data_type * data , int report_step , double sim_seconds) {
  ecl::ecl_sum_file_data * file_data = data->data_files.back();
  ecl_sum_tstep_type * tstep = file_data->add_new_tstep( report_step, sim_seconds );
  ecl_sum_data_build_index( data );
  return tstep;
}


int * ecl_sum_data_alloc_param_mapping( int * current_param_mapping, int * old_param_mapping, size_t size) {
  int * new_param_mapping = (int*)util_malloc( size * sizeof * new_param_mapping );
  for (size_t i = 0; i < size; i++) {
    if (current_param_mapping[i] >= 0)
      new_param_mapping[i] = old_param_mapping[ current_param_mapping[i] ];
    else
      new_param_mapping[i] = -1;
  }
  return new_param_mapping;
}


void ecl_sum_data_add_case(ecl_sum_data_type * self, const ecl_sum_data_type * other) {
  for (auto other_file : other->data_files)
    self->data_files.push_back( other_file );

  ecl_sum_data_build_index(self);
}


/*
  Observe that this can be called several times (but not with the same
  data - that will die).

  Warning: The index information of the ecl_sum_data instance has
  __NOT__ been updated when leaving this function. That is done with a
  call to ecl_sum_data_build_index().
*/

bool ecl_sum_data_fread(ecl_sum_data_type * data , const stringlist_type * filelist, bool lazy_load, int file_options) {
  ecl::ecl_sum_file_data * file_data = new ecl::ecl_sum_file_data( data->smspec );
  if (file_data->fread( filelist, lazy_load, file_options)) {
    ecl_sum_data_append_file_data( data, file_data );
    ecl_sum_data_build_index(data);
    return true;
  }
  return false;
}









/**
   If the variable @include_restart is true the function will query
   the smspec object for restart information, and load summary
   information from case(s) which this case was restarted from (this
   only really applies to predictions where the basename has been
   (manually) changed from the historical part.
*/

ecl_sum_data_type * ecl_sum_data_fread_alloc( ecl_smspec_type * smspec , const stringlist_type * filelist , bool include_restart, bool lazy_load, int file_options) {
  ecl_sum_data_type * data = ecl_sum_data_alloc( smspec );
  ecl_sum_data_fread( data , filelist, lazy_load, file_options );

  /*****************************************************************/
  /* OK - now we have loaded all the data. Must sort the internal
     storage vector, and build up various internal indexing vectors;
     this is done in a sepearate function.
  */
  ecl_sum_data_build_index( data );
  return data;
}


void ecl_sum_data_summarize(const ecl_sum_data_type * data , FILE * stream) {
  fprintf(stream , "REPORT         INDEX              DATE                 DAYS\n");
  fprintf(stream , "---------------------------------------------------------------\n");
  {
    int index;
    for (index = 0; index < ecl_sum_data_get_length(data); index++) {
      time_t sim_time = ecl_sum_data_iget_sim_time(data, index);
      int report_step = ecl_sum_data_iget_report_step(data, index);
      double days = ecl_sum_data_iget_sim_days(data, index);

      int day,month,year;
      ecl_util_set_date_values( sim_time, &day, &month , &year);
      fprintf(stream , "%04d          %6d               %02d/%02d/%4d           %7.2f \n", report_step , index , day,month,year, days);
    }
  }
  fprintf(stream , "---------------------------------------------------------------\n");
}



/*****************************************************************/


bool ecl_sum_data_has_report_step(const ecl_sum_data_type * data , int report_step ) {
  if (!data->index.has_report(report_step))
    return false;

  const auto& index_node = data->index.lookup_report(report_step);
  const auto& file_data = data->data_files[index_node.data_index];
  return file_data->has_report(report_step);
}



/**
   Returns the last index included in report step @report_step.
   Observe that if the dataset does not include @report_step at all,
   the function will return INVALID_MINISTEP_NR; this must be checked for in the
   calling scope.
*/

int ecl_sum_data_iget_report_end( const ecl_sum_data_type * data , int report_step ) {
  const auto& index_node = data->index.lookup_report(report_step);
  const auto& file_data = data->data_files[index_node.data_index];
  auto range = file_data->report_range(report_step);
  return range.second;
}






int ecl_sum_data_iget_report_step(const ecl_sum_data_type * data , int internal_index) {
  const auto& index_node = data->index.lookup(internal_index);
  const auto& file_data = data->data_files[index_node.data_index];
  return file_data->iget_report(internal_index - index_node.offset);
}



/**
    This will look up a value based on an internal index. The internal
    index will ALWAYS run in the interval [0,num_ministep), without
    any holes.
*/


double ecl_sum_data_iget( const ecl_sum_data_type * data , int time_index , int params_index ) {
  const auto& index_node = data->index.lookup( time_index );
  ecl::ecl_sum_file_data * file_data = data->data_files[index_node.data_index];
  const auto& params_map = index_node.params_map;
  if (params_map[params_index] >= 0)
    return file_data->iget( time_index - index_node.offset, params_map[params_index] );
  else {
    const smspec_node_type * smspec_node = ecl_smspec_iget_node(data->smspec, params_index);
    return smspec_node_get_default(smspec_node);
  }
}




/**
   This function will form a weight average of the two ministeps
   @ministep1 and @ministep2. The weights and the ministep indices
   should (typically) be determined by the

      ecl_sum_data_init_interp_from_sim_xxx()

   functions. The function will typically the last function called
   when we seek a reservoir state variable at an intermediate time
   between two ministeps.
*/

static double ecl_sum_data_interp_get(const ecl_sum_data_type * data , int time_index1 , int time_index2 , double weight1 , double weight2 , int params_index) {
  return ecl_sum_data_iget(data, time_index1, params_index) * weight1 + ecl_sum_data_iget(data, time_index2, params_index) * weight2;
}


static double ecl_sum_data_vector_iget(const ecl_sum_data_type * data,  time_t sim_time, int params_index, bool is_rate,
                                       int time_index1 , int time_index2 , double weight1 , double weight2 ) {

  double value = 0.0;
  if (is_rate) {
    int time_index = ecl_sum_data_get_index_from_sim_time(data, sim_time);
    // uses step function since it is a rate
    value = ecl_sum_data_iget(data, time_index, params_index);
  } else {
    // uses interpolation between timesteps
    value = ecl_sum_data_interp_get(data, time_index1, time_index2, weight1, weight2, params_index);
  }
  return value;
}

void ecl_sum_data_fwrite_interp_csv_line(const ecl_sum_data_type * data, time_t sim_time, const ecl_sum_vector_type * keylist, FILE *fp){
  int num_keywords = ecl_sum_vector_get_size(keylist);
  double weight1, weight2;
  int    time_index1, time_index2;

  ecl_sum_data_init_interp_from_sim_time(data, sim_time, &time_index1, &time_index2, &weight1, &weight2);

  for (int i = 0; i < num_keywords; i++) {
    if (ecl_sum_vector_iget_valid(keylist, i)) {
      int params_index = ecl_sum_vector_iget_param_index(keylist, i);
      bool is_rate = ecl_sum_vector_iget_is_rate(keylist, i);
      double value = ecl_sum_data_vector_iget( data, sim_time, params_index , is_rate, time_index1, time_index2, weight1, weight2);

      if (i == 0)
        fprintf(fp, "%f", value);
      else
        fprintf(fp, ",%f", value);
    } else {
      if (i == 0)
        fputs("", fp);
      else
        fputs(",", fp);
    }
  }
}


/*
  If the keylist contains invalid indices the corresponding element in the
  results vector will *not* be updated; i.e. it is smart to initialize the
  results vector with an invalid-value marker before calling this function:

  double_vector_type * results = double_vector_alloc( ecl_sum_vector_get_size(keys), NAN);
  ecl_sum_data_get_interp_vector( data, sim_time, keys, results);

*/

void ecl_sum_data_get_interp_vector( const ecl_sum_data_type * data , time_t sim_time, const ecl_sum_vector_type * keylist, double_vector_type * results){
  int num_keywords = ecl_sum_vector_get_size(keylist);
  double weight1, weight2;
  int    time_index1, time_index2;

  ecl_sum_data_init_interp_from_sim_time(data, sim_time, &time_index1, &time_index2, &weight1, &weight2);
  double_vector_reset( results );
  for (int i = 0; i < num_keywords; i++) {
    if (ecl_sum_vector_iget_valid(keylist, i)) {
      int params_index = ecl_sum_vector_iget_param_index(keylist, i);
      bool is_rate = ecl_sum_vector_iget_is_rate(keylist, i);
      double value = ecl_sum_data_vector_iget( data, sim_time, params_index, is_rate, time_index1, time_index2, weight1, weight2);
      double_vector_iset( results, i , value );
    }
  }
}

double ecl_sum_data_get_from_sim_time( const ecl_sum_data_type * data , time_t sim_time , const smspec_node_type * smspec_node) {
  int params_index = smspec_node_get_params_index( smspec_node );
  if (smspec_node_is_rate( smspec_node )) {
    /*
      In general the mapping from sim_time to index is based on half
      open intervals, which are closed in the upper end:

          []<------------]<--------------]<-----------]
          t0             t1             t2           t3

       However - as indicated on the figure above there is a zero
       measure point right at the start which corresponds to
       time_index == 0; this is to ensure that there is correspondance
       with the ECLIPSE results if you ask for a value interpolated to
       the starting time.
    */
    int time_index = ecl_sum_data_get_index_from_sim_time( data , sim_time );
    return ecl_sum_data_iget( data , time_index , params_index);
  } else {
    /* Interpolated lookup based on two (hopefully) consecutive ministeps. */
    double weight1 , weight2;
    int    time_index1 , time_index2;


    ecl_sum_data_init_interp_from_sim_time( data , sim_time , &time_index1 , &time_index2 , &weight1 , &weight2);
    return ecl_sum_data_interp_get( data , time_index1 , time_index2 , weight1 , weight2 , params_index);
  }
}


int ecl_sum_data_get_report_step_from_days(const ecl_sum_data_type * data , double sim_days) {
  if ((sim_days < ecl_sum_data_get_first_day(data)) || (sim_days > ecl_sum_data_get_sim_length(data)))
    return -1;
  else {
    auto files = data->index.lookup_days(sim_days);
    if (files.first != files.second)
      return -1;

    const auto& data_file = data->data_files[files.first->data_index];
    return data_file->report_step_from_days(sim_days);
  }
}

/**
   Will go through the data and find the report step which EXACTLY
   matches the input sim_time. If no report step matches exactly the
   function will return -1.

   Observe that by default the report steps consist of half-open time
   intervals like this: (t1, t2]. However the first report step
   (i.e. report step 1, is a fully inclusive interval: [t0 , t1] where
   t0 is the simulation start time. That is not implemented here;
   meaning that if you supply the start time as @sim_time argument you
   will get -1 and not 0 as you might expect.

   It would certainly be possible to detect the start_time input
   argument and special case the return, but the opposite would be
   'impossible' - you would never get anything sensible out when using
   report_step == 0 as input to one of the functions expecting
   report_step input.
*/


int ecl_sum_data_get_report_step_from_time(const ecl_sum_data_type * data , time_t sim_time) {
  if (!ecl_sum_data_check_sim_time(data , sim_time))
    return -1;
  else {
    auto files = data->index.lookup_time(sim_time);
    if (files.first != files.second)
      return -1;

    const auto& data_file = data->data_files[files.first->data_index];
    return data_file->report_step_from_time(sim_time);
  }
}

double ecl_sum_data_time2days( const ecl_sum_data_type * data , time_t sim_time) {
  time_t start_time = ecl_smspec_get_start_time( data->smspec );
  return util_difftime_days( start_time , sim_time );
}

double ecl_sum_data_get_from_sim_days( const ecl_sum_data_type * data , double sim_days , const smspec_node_type * smspec_node) {
  time_t sim_time = ecl_smspec_get_start_time( data->smspec );
  util_inplace_forward_days_utc( &sim_time , sim_days );
  return ecl_sum_data_get_from_sim_time( data , sim_time , smspec_node );
}


time_t ecl_sum_data_iget_sim_time(const ecl_sum_data_type * data, int ministep_index) {
  const auto& index_node = data->index.lookup( ministep_index );
  const auto data_file = data->data_files[index_node.data_index];
  return data_file->iget_sim_time(ministep_index - index_node.offset);
}


time_t ecl_sum_data_get_report_time( const ecl_sum_data_type * data , int report_step) {
  if (report_step == 0)
    return ecl_smspec_get_start_time( data->smspec );
  else {
    int internal_index = ecl_sum_data_iget_report_end( data , report_step );
    return ecl_sum_data_iget_sim_time(data, internal_index);
  }
}



int ecl_sum_data_get_first_report_step( const ecl_sum_data_type * data ) {
  const auto& data_file = data->data_files[0];
  return data_file->first_report();
}


int ecl_sum_data_get_last_report_step( const ecl_sum_data_type * data ) {
  const auto& data_file = data->data_files.back();
  return data_file->last_report();
}


/*****************************************************************/
/* High level vector routines */




static void ecl_sum_data_init_time_vector__(const ecl_sum_data_type * data, time_t * output_data, bool report_only) {
  int offset = 0;
  for (const auto& index_node : data->index) {
    const auto& data_file = data->data_files[index_node.data_index];

    if (report_only)
      offset += data_file->get_time_report(index_node.length, &output_data[offset]);
    else {
      data_file->get_time(index_node.length, &output_data[offset]);
      offset += index_node.length;
    }
  }
}


void ecl_sum_data_init_time_vector(const ecl_sum_data_type * data, time_t * output_data) {
  ecl_sum_data_init_time_vector__(data, output_data, false);
}



time_t_vector_type *  ecl_sum_data_alloc_time_vector( const ecl_sum_data_type * data , bool report_only) {
  std::vector<time_t> output_data;
  if (report_only)
    output_data.resize( 1 + ecl_sum_data_get_last_report_step(data) - ecl_sum_data_get_first_report_step(data));
  else
    output_data.resize( ecl_sum_data_get_length(data) );

  ecl_sum_data_init_time_vector__(data, output_data.data(), report_only);
  time_t_vector_type * time_vector = time_t_vector_alloc(output_data.size(),0);
  {
    time_t * tmp_data = time_t_vector_get_ptr( time_vector );
    memcpy(tmp_data, output_data.data(), output_data.size() * sizeof(time_t));
  }
  return time_vector;
}




static void ecl_sum_data_init_double_vector__(const ecl_sum_data_type * data, int main_params_index, double * output_data, bool report_only) {
  int offset = 0;
  for (const auto& index_node : data->index) {
    const auto& data_file = data->data_files[index_node.data_index];
    const auto& params_map = index_node.params_map;
    int params_index = params_map[main_params_index];


    if (report_only) {
      const smspec_node_type * smspec_node = ecl_smspec_iget_node(data->smspec, main_params_index);
      double default_value = smspec_node_get_default(smspec_node);
      offset += data_file->get_data_report(params_index, index_node.length, &output_data[offset], default_value);
    } else {

      if (params_index >= 0)
        data_file->get_data(params_index, index_node.length, &output_data[offset]);
      else {
        const smspec_node_type * smspec_node = ecl_smspec_iget_node(data->smspec, main_params_index);
        for (int i=0; i < index_node.length; i++)
          output_data[offset + i] = smspec_node_get_default(smspec_node);
      }
      offset += index_node.length;
    }
  }
}


void ecl_sum_data_init_datetime64_vector(const ecl_sum_data_type * data, int64_t * output_data, int multiplier) {
  for (int i = 0; i < ecl_sum_data_get_length(data); i++)
    output_data[i] = ecl_sum_data_iget_sim_time(data, i) * multiplier;
}



void ecl_sum_data_init_double_vector(const ecl_sum_data_type * data, int params_index, double * output_data) {
  ecl_sum_data_init_double_vector__(data, params_index, output_data, false);
}


double_vector_type * ecl_sum_data_alloc_data_vector( const ecl_sum_data_type * data , int params_index , bool report_only) {
  std::vector<double> output_data;
  if (report_only)
    output_data.resize( 1 + ecl_sum_data_get_last_report_step(data) - ecl_sum_data_get_first_report_step(data));
  else
    output_data.resize( ecl_sum_data_get_length(data) );

  ecl_sum_data_init_double_vector__(data, params_index, output_data.data(), report_only);
  double_vector_type * data_vector = double_vector_alloc(output_data.size(), 0);
  {
    double * tmp_data = double_vector_get_ptr( data_vector );
    memcpy(tmp_data, output_data.data(), output_data.size() * sizeof(double));
  }
  return data_vector;
}


void ecl_sum_data_init_double_vector_interp(const ecl_sum_data_type * data,
                                            const smspec_node_type * smspec_node,
                                            const time_t_vector_type * time_points,
                                            double * output_data) {
  bool is_rate = smspec_node_is_rate(smspec_node);
  int params_index = smspec_node_get_params_index(smspec_node);
  time_t start_time = ecl_sum_data_get_data_start(data);
  time_t end_time   = ecl_sum_data_get_sim_end(data);
  double start_value = 0;
  double end_value = 0;

  if (!is_rate) {
    start_value = ecl_sum_data_iget_first_value(data, params_index);
    end_value = ecl_sum_data_iget_last_value(data, params_index);
  }

  for (int time_index=0; time_index < time_t_vector_size(time_points); time_index++) {
    time_t sim_time = time_t_vector_iget( time_points, time_index);
    double value;
    if (sim_time < start_time)
      value = start_value;

    else if (sim_time > end_time)
      value = end_value;

    else {
      int time_index1, time_index2;
      double weight1, weight2;
      ecl_sum_data_init_interp_from_sim_time(data, sim_time, &time_index1, &time_index2, &weight1, &weight2);
      value = ecl_sum_data_vector_iget( data,
                                        sim_time,
                                        params_index,
                                        is_rate,
                                        time_index1,
                                        time_index2,
                                        weight1,
                                        weight2);
    }

    output_data[time_index] = value;
  }
}




void ecl_sum_data_init_double_frame(const ecl_sum_data_type * data, const ecl_sum_vector_type * keywords, double *output_data) {
  int time_stride = ecl_sum_vector_get_size(keywords);
  int key_stride = 1;
  for (int time_index=0; time_index < ecl_sum_data_get_length(data); time_index++) {
    for (int key_index = 0; key_index < ecl_sum_vector_get_size(keywords); key_index++) {
      int param_index = ecl_sum_vector_iget_param_index(keywords, key_index);
      int data_index = key_index*key_stride + time_index * time_stride;

      output_data[data_index] = ecl_sum_data_iget(data, time_index, param_index);
    }
  }
}


void ecl_sum_data_init_double_frame_interp(const ecl_sum_data_type * data,
                                           const ecl_sum_vector_type * keywords,
                                           const time_t_vector_type * time_points,
                                           double * output_data) {
  int num_keywords = ecl_sum_vector_get_size(keywords);
  int time_stride = num_keywords;
  int key_stride = 1;
  time_t start_time = ecl_sum_data_get_data_start(data);
  time_t end_time   = ecl_sum_data_get_sim_end(data);


  for (int time_index=0; time_index < time_t_vector_size(time_points); time_index++) {
    time_t sim_time = time_t_vector_iget( time_points, time_index);
    if (sim_time < start_time) {
      for (int key_index = 0; key_index < num_keywords; key_index++) {
        int param_index = ecl_sum_vector_iget_param_index(keywords, key_index);
        int data_index = key_index*key_stride + time_index*time_stride;
        bool is_rate = ecl_sum_vector_iget_is_rate(keywords, key_index);
        if (is_rate)
          output_data[data_index] = 0;
        else
          output_data[data_index] = ecl_sum_data_iget_first_value(data, param_index);
      }
    } else if (sim_time > end_time) {
      for (int key_index = 0; key_index < num_keywords; key_index++) {
        int param_index = ecl_sum_vector_iget_param_index(keywords, key_index);
        int data_index = key_index*key_stride + time_index*time_stride;
        bool is_rate = ecl_sum_vector_iget_is_rate(keywords, key_index);
        if (is_rate)
          output_data[data_index] = 0;
        else
          output_data[data_index] = ecl_sum_data_iget_last_value(data, param_index);
      }
    } else {
      double weight1, weight2;
      int    time_index1, time_index2;

      ecl_sum_data_init_interp_from_sim_time(data, sim_time, &time_index1, &time_index2, &weight1, &weight2);

      for (int key_index = 0; key_index < num_keywords; key_index++) {
        int param_index = ecl_sum_vector_iget_param_index(keywords, key_index);
        int data_index = key_index*key_stride + time_index*time_stride;
        bool is_rate = ecl_sum_vector_iget_is_rate(keywords, key_index);
        double value = ecl_sum_data_vector_iget( data, sim_time, param_index , is_rate, time_index1, time_index2, weight1, weight2);
        output_data[data_index] = value;
      }
    }
  }
}

/**
   This function will return the total number of ministeps in the
   current ecl_sum_data instance; but observe that actual series of
   ministeps can have non-zero offset and also "holes" in the series.
*/

int ecl_sum_data_get_length( const ecl_sum_data_type * data ) {
  return data->index.length();
}

static bool ecl_sum_data_report_step_equal__( const ecl_sum_data_type * data1 , const ecl_sum_data_type * data2, bool strict) {
  if (data1->data_files.size() != data2->data_files.size())
    return false;

  for (size_t i=0; i < data1->data_files.size(); i++) {
    const auto& data_file1 = data1->data_files[i];
    const auto& data_file2 = data2->data_files[i];

    if (!data_file1->report_step_equal(*data_file2, strict))
      return false;
  }

  return true;
}


bool ecl_sum_data_report_step_compatible( const ecl_sum_data_type * data1 , const ecl_sum_data_type * data2) {
  return ecl_sum_data_report_step_equal__(data1, data2, false);
}

bool ecl_sum_data_report_step_equal( const ecl_sum_data_type * data1 , const ecl_sum_data_type * data2) {
  return ecl_sum_data_report_step_equal__(data1, data2, true);
}


double ecl_sum_data_iget_last_value(const ecl_sum_data_type * data, int param_index) {
  return ecl_sum_data_iget(data, ecl_sum_data_get_length(data)-1, param_index);
}

double ecl_sum_data_get_last_value(const ecl_sum_data_type * data, int param_index) {
  return ecl_sum_data_iget_last_value(data, param_index);
}

double ecl_sum_data_iget_first_value(const ecl_sum_data_type * data, int param_index) {
  return ecl_sum_data_iget(data, 0, param_index);
}
