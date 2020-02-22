#include <string>
#include <array>
#include <vector>
#include <map>

#include <ert/ecl/ecl_smspec.hpp>
#include <ert/ecl/ecl_file.hpp>

namespace ecl {

class unsmry_loader {
public:
  unsmry_loader(const ecl_smspec_type * smspec, const std::string& filename, int file_options);
  ~unsmry_loader();

  std::vector<double> get_vector(int pos) const;
  std::vector<double> sim_seconds() const;
  std::vector<time_t> sim_time() const;
  int length() const;

  time_t iget_sim_time(int time_index) const;
  double iget_sim_seconds(int time_index) const;
  std::vector<int> report_steps(int offset) const;
  double iget(int time_index, int params_index) const;

private:
  int size;           //Number of entries in the smspec index
  int time_index;
  int time_seconds;
  time_t sim_start;
  int m_length;      //Number of PARAMS in the UNSMRY file

  std::array<int,3>    date_index;
  ecl_file_type      * file;
  ecl_file_view_type * file_view;
};







}
