/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <opm/utility/ECLWellSolution.hpp>
#include <opm/core/utility/Units.hpp>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl_well/well_const.h>
#include <stdexcept>
#include <sstream>

namespace Opm
{

    namespace {



        /// RAII class using the ERT block selection stack mechanism
        /// to select a report step in a restart file.
        struct SelectReportBlock
        {
            /// \param[in] file         ecl file to select block in.
            /// \paran[in] report_step  sequence number of block to choose
            SelectReportBlock(ecl_file_type* file, const int report_step)
                : file_(file)
            {
                if (!ecl_file_has_report_step(file_, report_step)) {
                    throw std::invalid_argument("Report step " + std::to_string(report_step) + " not found.");
                }
                ecl_file_push_block(file_);
                ecl_file_select_global(file_);
                ecl_file_select_rstblock_report_step(file_, report_step);
            }
            ~SelectReportBlock()
            {
                ecl_file_pop_block(file_);
            }
            ecl_file_type* file_;
        };




        /// RAII class using the ERT block selection stack mechanism
        /// to select an LGR sub-block.
        struct SubSelectLGRBlock
        {
            /// \param[in] file       ecl file to select LGR block in.
            /// \paran[in] lgr_index  sequence number of block to choose
            SubSelectLGRBlock(ecl_file_type* file, const int lgr_index)
                : file_(file)
            {
                ecl_file_push_block(file_);
                ecl_file_subselect_block(file_, LGR_KW, lgr_index);
            }
            ~SubSelectLGRBlock()
            {
                ecl_file_pop_block(file_);
            }
            ecl_file_type* file_;
        };




        /// Simple ecl file loading function.
        ERT::ert_unique_ptr<ecl_file_type, ecl_file_close>
        load(const boost::filesystem::path& filename)
        {
            // Read-only, keep open between requests
            const auto open_flags = 0;
            using FilePtr = ERT::ert_unique_ptr<ecl_file_type, ecl_file_close>;
            FilePtr file(ecl_file_open(filename.generic_string().c_str(), open_flags));
            if (!file) {
                std::ostringstream os;
                os << "Failed to load ECL File object from '"
                   << filename.generic_string() << '\'';
                throw std::invalid_argument(os.str());
            }
            return file;
        }




        // ---------   Restart file keywords.   ---------

        // Note:
        // This struct is more complete (containing more fields)
        // than currently needed, but we should expect that more
        // fields could be needed in the future.
        struct INTEHEAD
        {

            // Unit codes used in INTEHEAD
            enum {
                Metric = 1,
                Field = 2,
                Lab = 3
            };

            explicit INTEHEAD(const std::vector<int>& v)
                : unit   (v[INTEHEAD_UNIT_INDEX])
                , nx     (v[INTEHEAD_NX_INDEX])
                , ny     (v[INTEHEAD_NY_INDEX])
                , nz     (v[INTEHEAD_NZ_INDEX])
                , nactive(v[INTEHEAD_NACTIVE_INDEX])
                , iphs   (v[INTEHEAD_PHASE_INDEX])
                , nwell  (v[INTEHEAD_NWELLS_INDEX])
                , ncwma  (v[INTEHEAD_NCWMAX_INDEX])
                , nwgmax (v[INTEHEAD_NWGMAX_INDEX])
                , ngmaxz (v[INTEHEAD_NGMAXZ_INDEX])
                , niwel  (v[INTEHEAD_NIWELZ_INDEX])
                , nswel  (v[INTEHEAD_NSWELZ_INDEX])
                , nxwel  (v[INTEHEAD_NXWELZ_INDEX])
                , nzwel  (v[INTEHEAD_NZWELZ_INDEX])
                , nicon  (v[INTEHEAD_NICONZ_INDEX])
                , nscon  (v[INTEHEAD_NSCONZ_INDEX])
                , nxcon  (v[INTEHEAD_NXCONZ_INDEX])
                , nigrpz (v[INTEHEAD_NIGRPZ_INDEX])
                , iday   (v[INTEHEAD_DAY_INDEX])
                , imon   (v[INTEHEAD_MONTH_INDEX])
                , iyear  (v[INTEHEAD_YEAR_INDEX])
                , iprog  (v[INTEHEAD_IPROG_INDEX])
            {
            }

            int unit;      // Unit system. 1:metric, 2:field, 3:lab.
            int nx;        // Cartesian size i-direction.
            int ny;        // Cartesian size j-direction.
            int nz;        // Cartesian size k-direction.
            int nactive;   // Number of active cells.
            int iphs;      // Phase. 1:o, 2:w, 3:ow, 4:g, 5:og, 6:wg, 7:owg.
            int nwell;     // Number of wells.
            int ncwma;     // Maximum number of completions per well.
            int nwgmax;    // Maximum number of wells in any group.
            int ngmaxz;    // Maximum number of groups in field.
            int niwel;     // Number of elements pr. well in the IWEL array.
            int nswel;     // Number of elements pr. well in the SWEL array.
            int nxwel;     // Number of elements pr. well in the XWEL array.
            int nzwel;     // Number of 8 character words pr. well in ZWEL.
            int nicon;     // Number of elements pr completion in the ICON array.
            int nscon;     // Number of elements pr completion in the SCON array.
            int nxcon;     // Number of elements pr completion in the XCON array.
            int nigrpz;    // Number of elements pr group in the IGRP array.
            int iday;      // Report day.
            int imon;      // Report month.
            int iyear;     // Report year.
            int iprog;     // Eclipse program type. 100, 300 or 500.
        };




        // Reservoir rate units from code used in INTEHEAD.
        double resRateUnit(const int unit_code)
        {
            using prefix::centi;
            using namespace unit;

            switch (unit_code) {
            case INTEHEAD::Metric: return cubic(meter)/day;        // m^3/day
            case INTEHEAD::Field:  return stb/day;                 // stb/day
            case INTEHEAD::Lab:    return cubic(centi*meter)/hour; // (cm)^3/h
            default: throw std::runtime_error("Unknown unit code from INTEHEAD: " + std::to_string(unit_code));
            }
        }




        // Return input string with spaces stripped of the right end.
        std::string trimSpacesRight(const std::string& s)
        {
            return std::string(s.begin(), s.begin() + s.find_last_not_of(' ') + 1);
        }


    } // anonymous namespace




    ECLWellSolution::ECLWellSolution(const boost::filesystem::path& restart_filename)
        : restart_(load(restart_filename))
    {
    }





    std::vector<ECLWellSolution::WellData>
    ECLWellSolution::solution(const int report_step,
                              const int num_grids) const
    {
        SelectReportBlock select(restart_.get(), report_step);
        {
            // Read well data for global grid.
            std::vector<WellData> all_wd = readWellData(0);
            for (int grid_index = 1; grid_index < num_grids; ++grid_index) {
                const int lgr_index = grid_index - 1;
                SubSelectLGRBlock subselect(restart_.get(), lgr_index);
                {
                    // Read well data for LGR grid.
                    std::vector<WellData> wd = readWellData(grid_index);
                    // Append to set of all well data.
                    all_wd.insert(all_wd.end(), wd.begin(), wd.end());
                }
            }
            return all_wd;
        }
    }




    ecl_kw_type*
    ECLWellSolution::getKeyword(const std::string& fieldname) const
    {
        const int local_occurrence = 0; // This should be correct for all the well-related keywords.
        if (ecl_file_get_num_named_kw(restart_.get(), fieldname.c_str()) == 0) {
            throw std::runtime_error("Could not find field " + fieldname);
        }
        return ecl_file_iget_named_kw(restart_.get(), fieldname.c_str(), local_occurrence);
    }




    std::vector<double>
    ECLWellSolution::loadDoubleField(const std::string& fieldname) const
    {
        ecl_kw_type* keyword = getKeyword(fieldname);
        std::vector<double> field_data;
        field_data.resize(ecl_kw_get_size(keyword));
        ecl_kw_get_data_as_double(keyword, field_data.data());
        return field_data;
    }




    std::vector<int>
    ECLWellSolution::loadIntField(const std::string& fieldname) const
    {
        ecl_kw_type* keyword = getKeyword(fieldname);
        std::vector<int> field_data;
        field_data.resize(ecl_kw_get_size(keyword));
        ecl_kw_get_memcpy_int_data(keyword, field_data.data());
        return field_data;
    }




    std::vector<std::string>
    ECLWellSolution::loadStringField(const std::string& fieldname) const
    {
        ecl_kw_type* keyword = getKeyword(fieldname);
        std::vector<std::string> field_data;
        const int size = ecl_kw_get_size(keyword);
        field_data.resize(size);
        for (int pos = 0; pos < size; ++pos) {
            field_data[pos] = ecl_kw_iget_char_ptr(keyword, pos);
        }
        return field_data;
    }




    std::vector<ECLWellSolution::WellData>
    ECLWellSolution::readWellData(const int grid_index) const
    {
        // Note: this function is expected to be called in a context
        // where the correct restart block and grid subblock has already
        // been selected using the ert block mechanisms.

        // Read header, return if trivial.
        INTEHEAD ih(loadIntField(INTEHEAD_KW));
        if (ih.nwell == 0) {
            return {};
        }
        const double qr_unit = resRateUnit(ih.unit);

        // Read necessary keywords.
        auto zwel = loadStringField(ZWEL_KW);
        auto iwel = loadIntField(IWEL_KW);
        auto icon = loadIntField(ICON_KW);
        auto xcon = loadDoubleField("XCON");

        // Create well data.
        std::vector<WellData> wd(ih.nwell);
        for (int well = 0; well < ih.nwell; ++well) {
            wd[well].name = trimSpacesRight(zwel[well * ih.nzwel]);
            const int ncon = iwel[well * ih.niwel + IWEL_CONNECTIONS_INDEX];
            wd[well].completions.resize(ncon);
            for (int comp_index = 0; comp_index < ncon; ++comp_index) {
                const int icon_offset = (well*ih.ncwma + comp_index) * ih.nicon;
                const int xcon_offset = (well*ih.ncwma + comp_index) * ih.nxcon;
                auto& completion = wd[well].completions[comp_index];
                // Note: subtracting 1 from indices (Fortran -> C convention).
                completion.grid_index = grid_index;
                completion.ijk = { icon[icon_offset + ICON_I_INDEX] - 1,
                                   icon[icon_offset + ICON_J_INDEX] - 1,
                                   icon[icon_offset + ICON_K_INDEX] - 1 };
                // Note: taking the negative input, to get inflow rate.
                completion.reservoir_inflow_rate = -unit::convert::from(xcon[xcon_offset + XCON_QR_INDEX], qr_unit);
            }
        }
        return wd;
    }




} // namespace Opm
