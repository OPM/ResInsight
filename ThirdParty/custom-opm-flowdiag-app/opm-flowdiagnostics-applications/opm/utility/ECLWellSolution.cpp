/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016, 2017 Statoil ASA.

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
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl_well/well_const.h>
#include <cmath>
#include <stdexcept>
#include <sstream>

namespace Opm
{

    namespace {

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
            return ECLUnits::createUnitSystem(unit_code)->reservoirRate();
        }




        // Return input string with spaces stripped of the right end.
        std::string trimSpacesRight(const std::string& s)
        {
            return std::string(s.begin(), s.begin() + s.find_last_not_of(' ') + 1);
        }




        // Constants not provided by ert.
        enum { XWEL_RESV_INDEX = 4 };
        enum { IWEL_TYPE_PRODUCER = 1 };


    } // anonymous namespace




    ECLWellSolution::ECLWellSolution(const double rate_threshold,
                                     const bool disallow_crossflow)
        : rate_threshold_(rate_threshold)
        , disallow_crossflow_(disallow_crossflow)
    {
    }





    std::vector<ECLWellSolution::WellData>
    ECLWellSolution::solution(const ECLRestartData& restart,
                              const std::vector<std::string>& grids) const
    {
        // Note: this function expects to be called with the correct restart
        // block--e.g., a report step--selected in the caller.

        // Read well data for global grid.
        std::vector<WellData> all_wd{};
        for (const auto& gridName : grids) {
            std::vector<WellData> wd = readWellData(restart, gridName);
            // Append to set of all well data.
            all_wd.insert(all_wd.end(), wd.begin(), wd.end());
        }
        return all_wd;
    }




    std::vector<ECLWellSolution::WellData>
    ECLWellSolution::readWellData(const ECLRestartData& restart,
                                  const std::string&    gridName) const
    {
        // Check if result set provides complete set of well solution data.
        if (! (restart.haveKeywordData(ZWEL_KW, gridName) &&
               restart.haveKeywordData(IWEL_KW, gridName) &&
               restart.haveKeywordData("XWEL" , gridName) &&
               restart.haveKeywordData(ICON_KW, gridName) &&
               restart.haveKeywordData("XCON" , gridName)))
        {
            // Not all requisite keywords present in this grid.  Can't
            // create a well solution.
            return {};
        }

        // Read header, return if trivial.
        INTEHEAD ih(restart.keywordData<int>(INTEHEAD_KW, gridName));
        if (ih.nwell == 0) {
            return {};
        }
        const double qr_unit = resRateUnit(ih.unit);

        // Load well topology and flow rates.
        auto zwel = restart.keywordData<std::string>(ZWEL_KW, gridName);
        auto iwel = restart.keywordData<int>        (IWEL_KW, gridName);
        auto xwel = restart.keywordData<double>     ("XWEL" , gridName);
        auto icon = restart.keywordData<int>        (ICON_KW, gridName);
        auto xcon = restart.keywordData<double>     ("XCON" , gridName);

        // Create well data.
        std::vector<WellData> wd_vec;
        wd_vec.reserve(ih.nwell);
        for (int well = 0; well < ih.nwell; ++well) {
            // Skip if total rate below threshold (for wells that are
            // shut or stopped for example).
            const double well_reservoir_inflow_rate = -unit::convert::from(xwel[well * ih.nxwel + XWEL_RESV_INDEX], qr_unit);
            if (std::fabs(well_reservoir_inflow_rate) < rate_threshold_) {
                continue;
            }
            // Otherwise: add data for this well.
            WellData wd;
            wd.name = trimSpacesRight(zwel[well * ih.nzwel]);
            const bool is_producer = (iwel[well * ih.niwel + IWEL_TYPE_INDEX] == IWEL_TYPE_PRODUCER);
            wd.is_injector_well = !is_producer;
            const int ncon = iwel[well * ih.niwel + IWEL_CONNECTIONS_INDEX];
            wd.completions.reserve(ncon);
            for (int comp_index = 0; comp_index < ncon; ++comp_index) {
                const int icon_offset = (well*ih.ncwma + comp_index) * ih.nicon;
                const int xcon_offset = (well*ih.ncwma + comp_index) * ih.nxcon;
                WellData::Completion completion;
                // Note: subtracting 1 from indices (Fortran -> C convention).
                completion.gridName = gridName;
                completion.ijk = { icon[icon_offset + ICON_I_INDEX] - 1,
                                   icon[icon_offset + ICON_J_INDEX] - 1,
                                   icon[icon_offset + ICON_K_INDEX] - 1 };
                // Note: taking the negative input, to get inflow rate.
                completion.reservoir_inflow_rate = -unit::convert::from(xcon[xcon_offset + XCON_QR_INDEX], qr_unit);
                if (disallow_crossflow_) {
                    // Add completion only if not cross-flowing (injecting producer or producing injector).
                    if ((completion.reservoir_inflow_rate < 0.0) == is_producer) {
                        wd.completions.push_back(completion);
                    }
                } else {
                    // Always add completion.
                    wd.completions.push_back(completion);
                }
            }
            wd_vec.push_back(wd);
        }
        return wd_vec;
    }




} // namespace Opm
