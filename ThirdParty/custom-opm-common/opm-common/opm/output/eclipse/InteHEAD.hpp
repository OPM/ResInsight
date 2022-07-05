/*
  Copyright 2021 Equinor ASA.
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

class EclipseGrid;
class EclipseState;
class UnitSystem;
class Phases;
}

namespace Opm { namespace RestartIO {

    class InteHEAD
    {
    public:
        struct WellTableDim {
            int numWells;
            int maxPerf;
            int maxWellInGroup;
            int maxGroupInField;
            int maxWellsInField;
            int mxwlstprwel;
            int mxdynwlst;
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

        struct RockOpts {
            int ttyp;
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
            int wseg_mx_rst;
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


     struct ActiveNetwork {
         int actnetwrk;
     };

     struct NetworkDims {
            int noactnod;
            int noactbr;
            int nodmax;
            int nbrmax;
            int nibran;
            int nrbran;
            int ninode;
            int nrnode;
            int nznode;
            int ninobr;
        };

     struct NetBalanceDims {
            int maxNoIterationsNBC;
            int maxNoIterationsTHP;
        };

        struct AquiferDims {
            // Number of active analytic aquifers (# unique aquifer IDs)
            int numAquifers {0};

            // Declared maximum number of analytic aquifers in model
            // (AQUDIMS(5))
            int maxNumAquifers {0};

            // Declared maximum number of connections in any analytic
            // aquifer (AQUDIMS(6))
            int maxNumAquiferConn {0};

            // Maximum number of *active* connections in any analytic aquifer
            int maxNumActiveAquiferConn {0};

            // Maximum aquifer ID across all of the model's analytic aquifers.
            int maxAquiferID {0};

            // Number of numeric aquifer records (lines of AQUNUM data)
            int numNumericAquiferRecords {0};

            // Number of data elements per aquifer in IAAQ array.
            int numIntAquiferElem {18};

            // Number of data elements per aquifer in SAAQ array.
            int numRealAquiferElem {24};

            // Number of data elements per aquifer in XAAQ array.
            int numDoubAquiferElem {10};

            // Number of data elements in IAQN array per numeric aquifer record.
            int numNumericAquiferIntElem {10};

            // Number of data elements in RAQN array per numeric aquifer record.
            int numNumericAquiferDoubleElem {13};

            // Number of data elements per coonnection in ICAQ array.
            int numIntConnElem {7};

            // Number of data elements per connecetion in SCAQ array.
            int numRealConnElem {2};

            // Number of data elements per connection in ACAQ array.
            int numDoubConnElem {4};
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
        InteHEAD& aquiferDimensions(const AquiferDims& aqudims);

        InteHEAD& calendarDate(const TimePoint& date);
        InteHEAD& activePhases(const Phases& phases);

        InteHEAD& params_NWELZ(const int niwelz, const int nswelz, const int nxwelz, const int nzwelz);
        InteHEAD& params_NCON(const int niconz, const int nsconz, const int nxconz);
        InteHEAD& params_GRPZ(const std::array<int, 4>& grpz);
        InteHEAD& params_NGCTRL(const int gct);

        InteHEAD& stepParam(const int tstep, const int report_step);
        InteHEAD& tuningParam(const TuningPar& tunpar);
        InteHEAD& variousParam(const int version, const int iprog);
        InteHEAD& wellSegDimensions(const WellSegDims& wsdim);
        InteHEAD& activeNetwork(const ActiveNetwork& actntwrk);
        InteHEAD& networkDimensions(const NetworkDims& nwdim);
        InteHEAD& netBalanceData(const NetBalanceDims& nwbaldim);
        InteHEAD& regionDimensions(const RegDims& rdim);
        InteHEAD& rockOpts(const RockOpts& rckop);
        InteHEAD& ngroups(const Group& gr);
        InteHEAD& udqParam_1(const UdqParam& udqpar);
        InteHEAD& actionParam(const ActionParam& act_par);
        InteHEAD& variousUDQ_ACTIONXParam();
        InteHEAD& nominatedPhaseGuideRate(GuideRateNominatedPhase nphase);
        InteHEAD& whistControlMode(int mode);
        InteHEAD& liftOptParam(int in_enc);

        static int numRsegElem(const Opm::Phases& phase);
        const std::vector<int>& data() const
        {
            return this->data_;
        }

    private:
        std::vector<int> data_;
    };

    InteHEAD::TimePoint
    getSimulationTimePoint(const std::time_t start,
                           const double      elapsed);

    InteHEAD::AquiferDims
    inferAquiferDimensions(const EclipseState& es);
}} // Opm::RestartIO

#endif // OPM_INTEHEAD_HEADER_INCLUDED
