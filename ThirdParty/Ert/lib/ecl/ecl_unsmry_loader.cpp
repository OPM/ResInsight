#include <cmath>
#include <string>
#include <iostream>

#include <ert/util/int_vector.hpp>

#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file.hpp>

#include "detail/ecl/ecl_unsmry_loader.hpp"


namespace ecl {

unsmry_loader::unsmry_loader(const ecl_smspec_type * smspec, const std::string& filename) :
  size(ecl_smspec_get_params_size(smspec)),
  time_index(ecl_smspec_get_time_index(smspec)),
  time_seconds(ecl_smspec_get_time_seconds(smspec)),
  sim_start(ecl_smspec_get_start_time(smspec))
{
  int options = 0;
  ecl_file_type * file = ecl_file_open(filename.c_str(), options);
  if (!file)
    throw std::bad_alloc();

  if (!ecl_file_has_kw(file, PARAMS_KW)) {
    ecl_file_close(file);
    throw std::bad_alloc();
  }

  if (ecl_file_get_num_named_kw(file, PARAMS_KW) != ecl_file_get_num_named_kw(file, MINISTEP_KW)) {
    ecl_file_close(file);
    throw std::bad_alloc();
  }

  this->date_index = {{ ecl_smspec_get_date_day_index(smspec),
                        ecl_smspec_get_date_month_index(smspec),
                        ecl_smspec_get_date_year_index(smspec) }};
  this->file = ecl_file_open(filename.c_str(), 0);
  this->file_view = ecl_file_get_global_view( this->file );
  this->m_length = ecl_file_view_get_num_named_kw(this->file_view, PARAMS_KW);
}


unsmry_loader::~unsmry_loader() {
  ecl_file_close(file);
}


int unsmry_loader::length() const {
  return this->m_length;
}


std::vector<double> unsmry_loader::get_vector(int pos) const {
  if (pos >= size)
     throw std::invalid_argument("unsmry_loader::get_vector: argument 'pos' mst be less than size of PARAMS.");

  std::vector<double> data(this->length());
  int_vector_type * index_map = int_vector_alloc( 1 , pos);
  char buffer[4];

  for (int index = 0; index < this->length(); index++) {
    ecl_file_view_index_fload_kw(file_view, PARAMS_KW, index, index_map, buffer);
    float * data_value = (float*) buffer;
    data[index] = *data_value;
  }
  int_vector_free( index_map );
  return data;
}


// This is horribly inefficient
double unsmry_loader::iget(int time_index, int params_index) const {
  int_vector_type * index_map = int_vector_alloc( 1 , params_index);
  float value;
  ecl_file_view_index_fload_kw(this->file_view, PARAMS_KW, time_index, index_map, (char *) &value);
  int_vector_free(index_map);
  return value;
}



time_t unsmry_loader::iget_sim_time(int time_index) const {
  if (this->time_index >= 0) {
    double sim_seconds = this->iget_sim_seconds(time_index);
    time_t sim_time = this->sim_start;
    util_inplace_forward_seconds_utc( &sim_time ,  sim_seconds ) ;
    return sim_time;
  } else {
    int_vector_type * index_map = int_vector_alloc(3,0);
    int_vector_iset(index_map, 0, this->date_index[0]);
    int_vector_iset(index_map, 1, this->date_index[1]);
    int_vector_iset(index_map, 2, this->date_index[2]);

    float values[3];
    ecl_file_view_index_fload_kw(this->file_view, PARAMS_KW, time_index, index_map, (char *) &values);
    int_vector_free(index_map);

    return ecl_util_make_date(util_roundf( values[0] ),
                              util_roundf( values[1] ),
                              util_roundf( values[2] ));
  }
}

double unsmry_loader::iget_sim_seconds(int time_index) const {
  if (this->time_index >= 0) {
    double raw_time = this->iget(time_index, this->time_index);
    return raw_time * this->time_seconds;
  } else {
    time_t sim_time = this->iget_sim_time(time_index);
    return util_difftime_seconds(this->sim_start, sim_time);
  }
}

std::vector<int> unsmry_loader::report_steps(int offset) const {
  std::vector<int> report_steps;
  int current_step = offset;
  for (int i=0; i < ecl_file_view_get_size(this->file_view); i++) {
    const ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw(this->file_view, i);
    if (util_string_equal(SEQHDR_KW, ecl_file_kw_get_header(file_kw)))
      current_step++;

    if (util_string_equal(PARAMS_KW, ecl_file_kw_get_header(file_kw)))
      report_steps.push_back(current_step);
  }
  return report_steps;
}

std::vector<time_t> unsmry_loader::sim_time() const {
  if (this->time_index >= 0) {
    const std::vector<double> sim_seconds = this->sim_seconds();
    std::vector<time_t> st(this->length(), this->sim_start);

    for (size_t i=0; i < st.size(); i++)
      util_inplace_forward_seconds_utc(&st[i], sim_seconds[i]);

    return st;

  } else {
    const auto day   = this->get_vector(this->date_index[0]);
    const auto month = this->get_vector(this->date_index[1]);
    const auto year  = this->get_vector(this->date_index[2]);
    std::vector<time_t> st(this->length());

    for (size_t i=0; i < st.size(); i++)
      st[i] = ecl_util_make_date(util_round(day[i]),
                                 util_round(month[i]),
                                 util_round(year[i]));

    return st;
  }
}


std::vector<double> unsmry_loader::sim_seconds() const {
  if (this->time_index >= 0) {
    std::vector<double> seconds = this->get_vector(this->time_index);
    for (size_t i=0; i < seconds.size(); i++)
      seconds[i] *= this->time_seconds;

    return seconds;
  } else {
    std::vector<time_t> st = this->sim_time();
    std::vector<double> seconds(st.size());

    for (size_t i=0; i < st.size(); i++)
      seconds[i] = util_difftime_seconds(this->sim_start, st[i]);

    return seconds;
  }
}

}
