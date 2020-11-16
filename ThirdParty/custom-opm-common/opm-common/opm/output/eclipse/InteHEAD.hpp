/*
  Copyright 2016, 2017, 2018 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#ifndef OPM_INTEHEAD_HEADER_INCLUDED
#define OPM_INTEHEAD_HEADER_INCLUDED

#include <array>
#include <ctime>
#include <memory>
#include <vector>


namespace Opm {

class UnitSystem;

namespace RestartIO {

    class InteHEAD
    {
    public:
        struct WellTableDim {
            int numWells;
            int maxPerf;
            int maxWellInGroup;
            int maxGroupInField;
            int maxWellsInField;
        };

        struct WellSegDims {
            int nsegwl;
            int nswlmx;
            int nsegmx;
            int nlbrmx;
            int nisegz;
            int nrsegz;
            int nilbrz;
        };

        struct RegDims {
            int ntfip;
            int nmfipr;
            int nrfreg;
            int ntfreg;
            int nplmix;
        };

        struct TimePoint {
            int year;
            int month;          // 1..12
            int day;            // 1..31

            int hour;           // 0..23
            int minute;         // 0..59
            int second;         // 0..59

            int microseconds;   // 0..999999
        };

        struct Phases {
            int oil;
            int water;
            int gas;
        };

        struct TuningPar {
            int newtmx;
            int newtmn;
            int litmax;
            int litmin;
            int mxwsit;
            int mxwpit;
        };
	
	struct Group {
	  int ngroups;
	};
	
	struct UdqParam {
	  int    udqParam_1;
      int    no_wudqs;
      int    no_gudqs;
      int    no_fudqs;
      int    no_iuads;
      int    no_iuaps;
	};

    struct ActionParam {
      int   no_actions;
      int   max_no_sched_lines_per_action;
      int   max_no_conditions_per_action;
      int   max_no_characters_per_line;
     };
     
     struct GuideRateNominatedPhase {
      int   nominated_phase;
     };
     
        InteHEAD();
        ~InteHEAD() = default;

        InteHEAD(const InteHEAD& rhs) = default;
        InteHEAD(InteHEAD&& rhs) = default;

        InteHEAD& operator=(const InteHEAD& rhs) = default;
        InteHEAD& operator=(InteHEAD&& rhs) = default;

        InteHEAD& dimensions(const int nx, const int ny, const int nz);
        InteHEAD& dimensions(const std::array<int,3>& cartDims);
        InteHEAD& numActive(const int nactive);

        InteHEAD& unitConventions(const UnitSystem& usys);
        InteHEAD& wellTableDimensions(const WellTableDim& wtdim);
        InteHEAD& calendarDate(const TimePoint& date);
        InteHEAD& activePhases(const Phases& phases);
        InteHEAD& params_NWELZ(const int niwelz, const int nswelz, const int nxwelz, const int nzwelz);
        InteHEAD& params_NCON(const int niconz, const int nsconz, const int nxconz);
        InteHEAD& params_GRPZ(const std::array<int, 4>& grpz);
        InteHEAD& params_NGCTRL(const int gct);
        InteHEAD& params_NAAQZ(const int ncamax, const int niaaqz, const int nsaaqz, const int nxaaqz, const int nicaqz, const int nscaqz, const int nacaqz);
        InteHEAD& stepParam(const int tstep, const int report_step);
        InteHEAD& tuningParam(const TuningPar& tunpar);
        InteHEAD& variousParam(const int version, const int iprog);
        InteHEAD& wellSegDimensions(const WellSegDims& wsdim);
        InteHEAD& regionDimensions(const RegDims& rdim);
        InteHEAD& ngroups(const Group& gr);
        InteHEAD& udqParam_1(const UdqParam& udqpar);
        InteHEAD& actionParam(const ActionParam& act_par);
        InteHEAD& variousUDQ_ACTIONXParam();
        InteHEAD& nominatedPhaseGuideRate(GuideRateNominatedPhase nphase);
        InteHEAD& whistControlMode(int mode);

        const std::vector<int>& data() const
        {
            return this->data_;
        }

    private:
        std::vector<int> data_;
    };

    std::time_t makeUTCTime(const std::tm& timePoint);

    InteHEAD::TimePoint
    getSimulationTimePoint(const std::time_t start,
                           const double      elapsed);
}} // Opm::RestartIO

#endif // OPM_INTEHEAD_HEADER_INCLUDED
