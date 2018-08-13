#include <vector>
#include <memory>
#include <array>

#include <ert/util/vector.hpp>

#include <ert/ecl/ecl_smspec.hpp>
#include <ert/ecl/ecl_sum_tstep.hpp>
#include <ert/ecl/ecl_file.hpp>

namespace ecl {

#define INVALID_MINISTEP_NR -1
#define INVALID_TIME_T 0


struct IndexNode {

IndexNode(time_t sim_time, double sim_seconds, int report_step) :
  sim_time(sim_time),
  sim_seconds(sim_seconds),
  report_step(report_step)
{}

    time_t sim_time;
    double sim_seconds;
    int report_step;
};


class TimeIndex {
public:

  void add(time_t sim_time, double sim_seconds, int report_step) {
    int internal_index = static_cast<int>(this->nodes.size());
    this->nodes.emplace_back(sim_time, sim_seconds, report_step);

    /* Indexing internal_index - report_step */
    if (static_cast<int>(this->report_map.size()) <= report_step)
      this->report_map.resize( report_step + 1, std::pair<int,int>(std::numeric_limits<int>::max(), -1));

    auto& range = this->report_map[report_step];
    range.first = std::min(range.first, internal_index);
    range.second = std::max(range.second, internal_index);
  }


  bool has_report(int report_step) const {
    if (report_step >= static_cast<int>(this->report_map.size()))
      return false;

    const auto& range_pair = this->report_map[report_step];
    if (range_pair.second < 0)
      return false;

    return true;
  }


  void clear() {
    this->nodes.clear();
    this->report_map.clear();
  }

  const IndexNode& operator[](size_t index) const {
    return this->nodes[index];
  }

  const IndexNode& back() const {
    return this->nodes.back();
  }

  size_t size() const {
    return this->nodes.size();
  }

  std::pair<int,int>& report_range(int report_step) {
    return this->report_map[report_step];
  }

  const std::pair<int,int>& report_range(int report_step) const {
    return this->report_map[report_step];
  }

private:
  std::vector<IndexNode> nodes;
  std::vector<std::pair<int,int>> report_map;
};


class unsmry_loader;

class ecl_sum_file_data {

public:
  ecl_sum_file_data(const ecl_smspec_type * smspec);
  ~ecl_sum_file_data();
  const ecl_smspec_type * smspec() const;

  int                  length_before(time_t end_time) const;
  void                 get_time(int length, time_t *data);
  void                 get_data(int params_index, int length, double *data);
  int                  length() const;
  time_t               get_data_start() const;
  time_t               get_sim_end() const;
  double               iget( int time_index , int params_index ) const;
  time_t               iget_sim_time(int time_index ) const;
  double               iget_sim_days(int time_index ) const;
  double               iget_sim_seconds(int time_index ) const;
  ecl_sum_tstep_type * iget_ministep( int internal_index ) const;
  double               get_days_start() const;
  double               get_sim_length() const;

  std::pair<int,int>   report_range(int report_step) const;
  bool                 report_step_equal( const ecl_sum_file_data& other, bool strict) const;
  int                  report_before(time_t end_time) const;
  int                  get_time_report(int max_internal_index, time_t *data);
  int                  get_data_report(int params_index, int max_internal_index, double *data, double default_value);
  int                  first_report() const;
  int                  last_report() const;
  int                  iget_report(int time_index) const;
  bool                 has_report(int report_step ) const;
  int                  report_step_from_days(double sim_days) const;
  int                  report_step_from_time(time_t sim_time) const;

  ecl_sum_tstep_type * add_new_tstep(int report_step , double sim_seconds);
  bool                 can_write() const;
  void                 fwrite_unified( fortio_type * fortio ) const;
  void                 fwrite_multiple( const char * ecl_case , bool fmt_case ) const;
  bool                 fread(const stringlist_type * filelist, bool lazy_load);

private:
  const ecl_smspec_type         * ecl_smspec;

  TimeIndex                       index;
  vector_type                   * data;

  std::unique_ptr<ecl::unsmry_loader> loader;

  void                 append_tstep(ecl_sum_tstep_type * tstep);
  void                 build_index();
  void                 fwrite_report( int report_step , fortio_type * fortio) const;
  bool                 check_file( ecl_file_type * ecl_file );
  void                 add_ecl_file(int report_step, const ecl_file_view_type * summary_view, const ecl_smspec_type * smspec);
};




}
