/*
  Copyright 2020 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/io/eclipse/rst/well.hpp>
#include <opm/io/eclipse/rst/state.hpp>

#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/doubhead.hpp>


namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm {
namespace RestartIO {

RstState::RstState(const ::Opm::UnitSystem& unit_system_,
                   const std::vector<int>& intehead,
                   const std::vector<bool>& logihead,
                   const std::vector<double>& doubhead):
    unit_system(unit_system_),
    header(unit_system_, intehead, logihead, doubhead)
{
    this->load_tuning(intehead, doubhead);
}

RstState::RstState(const ::Opm::UnitSystem& unit_system_,
                   const std::vector<int>& intehead,
                   const std::vector<bool>& logihead,
                   const std::vector<double>& doubhead,
                   const std::vector<std::string>& zgrp,
                   const std::vector<int>& igrp,
                   const std::vector<float>& sgrp,
                   const std::vector<double>& xgrp,
                   const std::vector<std::string>& zwel,
                   const std::vector<int>& iwel,
                   const std::vector<float>& swel,
                   const std::vector<double>& xwel,
                   const std::vector<int>& icon,
                   const std::vector<float>& scon,
                   const std::vector<double>& xcon):
    RstState(unit_system_, intehead, logihead, doubhead)
{
    this->add_groups(zgrp, igrp, sgrp, xgrp);

    for (int iw = 0; iw < this->header.num_wells; iw++) {
        std::size_t zwel_offset = iw * this->header.nzwelz;
        std::size_t iwel_offset = iw * this->header.niwelz;
        std::size_t swel_offset = iw * this->header.nswelz;
        std::size_t xwel_offset = iw * this->header.nxwelz;
        std::size_t icon_offset = iw * this->header.niconz * this->header.ncwmax;
        std::size_t scon_offset = iw * this->header.nsconz * this->header.ncwmax;
        std::size_t xcon_offset = iw * this->header.nxconz * this->header.ncwmax;
        int group_index = iwel[ iwel_offset + VI::IWell::Group ] - 1;
        const std::string group = this->groups[group_index].name;

        this->wells.emplace_back(this->unit_system,
                                 this->header,
                                 group,
                                 zwel.data() + zwel_offset,
                                 iwel.data() + iwel_offset,
                                 swel.data() + swel_offset,
                                 xwel.data() + xwel_offset,
                                 icon.data() + icon_offset,
                                 scon.data() + scon_offset,
                                 xcon.data() + xcon_offset);

        if (this->wells.back().msw_index)
            throw std::logic_error("MSW data not accounted for in this constructor");
    }
}

RstState::RstState(const ::Opm::UnitSystem& unit_system_,
                   const std::vector<int>& intehead,
                   const std::vector<bool>& logihead,
                   const std::vector<double>& doubhead,
                   const std::vector<std::string>& zgrp,
                   const std::vector<int>& igrp,
                   const std::vector<float>& sgrp,
                   const std::vector<double>& xgrp,
                   const std::vector<std::string>& zwel,
                   const std::vector<int>& iwel,
                   const std::vector<float>& swel,
                   const std::vector<double>& xwel,
                   const std::vector<int>& icon,
                   const std::vector<float>& scon,
                   const std::vector<double>& xcon,
                   const std::vector<int>& iseg,
                   const std::vector<double>& rseg) :
    RstState(unit_system_, intehead, logihead, doubhead)
{
    this->add_groups(zgrp, igrp, sgrp, xgrp);

    for (int iw = 0; iw < this->header.num_wells; iw++) {
        std::size_t zwel_offset = iw * this->header.nzwelz;
        std::size_t iwel_offset = iw * this->header.niwelz;
        std::size_t swel_offset = iw * this->header.nswelz;
        std::size_t xwel_offset = iw * this->header.nxwelz;
        std::size_t icon_offset = iw * this->header.niconz * this->header.ncwmax;
        std::size_t scon_offset = iw * this->header.nsconz * this->header.ncwmax;
        std::size_t xcon_offset = iw * this->header.nxconz * this->header.ncwmax;
        int group_index = iwel[ iwel_offset + VI::IWell::Group ] - 1;
        const std::string group = this->groups[group_index].name;

        this->wells.emplace_back(this->unit_system,
                                 this->header,
                                 group,
                                 zwel.data() + zwel_offset,
                                 iwel.data() + iwel_offset,
                                 swel.data() + swel_offset,
                                 xwel.data() + xwel_offset,
                                 icon.data() + icon_offset,
                                 scon.data() + scon_offset,
                                 xcon.data() + xcon_offset,
                                 iseg,
                                 rseg);
    }
}

void RstState::load_tuning(const std::vector<int>& intehead,
                           const std::vector<double>& doubhead)
{
    using M  = ::Opm::UnitSystem::measure;

    this->tuning.NEWTMX  = intehead[ VI::intehead::NEWTMX ];
    this->tuning.NEWTMN  = intehead[ VI::intehead::NEWTMN ];
    this->tuning.LITMAX  = intehead[ VI::intehead::LITMAX ];
    this->tuning.LITMIN  = intehead[ VI::intehead::LITMIN ];
    this->tuning.MXWSIT  = intehead[ VI::intehead::MXWSIT ];
    this->tuning.MXWPIT  = intehead[ VI::intehead::MXWPIT ];

    tuning.TSINIT = this->unit_system.to_si(M::time, doubhead[VI::doubhead::TsInit]);
    tuning.TSMAXZ = this->unit_system.to_si(M::time, doubhead[VI::doubhead::TsMaxz]);
    tuning.TSMINZ = this->unit_system.to_si(M::time, doubhead[VI::doubhead::TsMinz]);
    tuning.TSMCHP = this->unit_system.to_si(M::time, doubhead[VI::doubhead::TsMchp]);
    tuning.TSFMAX = doubhead[VI::doubhead::TsFMax];
    tuning.TSFMIN = doubhead[VI::doubhead::TsFMin];
    tuning.TSFCNV = doubhead[VI::doubhead::TsFcnv];
    tuning.THRUPT = doubhead[VI::doubhead::ThrUPT];
    tuning.TFDIFF = doubhead[VI::doubhead::TfDiff];
    tuning.TRGTTE = doubhead[VI::doubhead::TrgTTE];
    tuning.TRGCNV = doubhead[VI::doubhead::TrgCNV];
    tuning.TRGMBE = doubhead[VI::doubhead::TrgMBE];
    tuning.TRGLCV = doubhead[VI::doubhead::TrgLCV];
    tuning.XXXTTE = doubhead[VI::doubhead::XxxTTE];
    tuning.XXXCNV = doubhead[VI::doubhead::XxxCNV];
    tuning.XXXMBE = doubhead[VI::doubhead::XxxMBE];
    tuning.XXXLCV = doubhead[VI::doubhead::XxxLCV];
    tuning.XXXWFL = doubhead[VI::doubhead::XxxWFL];
    tuning.TRGFIP = doubhead[VI::doubhead::TrgFIP];
    tuning.TRGSFT = doubhead[VI::doubhead::TrgSFT];
    tuning.TRGDPR = doubhead[VI::doubhead::TrgDPR];
    tuning.XXXDPR = doubhead[VI::doubhead::XxxDPR];
    tuning.DDPLIM = doubhead[VI::doubhead::DdpLim];
    tuning.DDSLIM = doubhead[VI::doubhead::DdsLim];
}

void RstState::add_groups(const std::vector<std::string>& zgrp,
                          const std::vector<int>& igrp,
                          const std::vector<float>& sgrp,
                          const std::vector<double>& xgrp)
{
    for (int ig=0; ig < this->header.ngroup; ig++) {
        std::size_t zgrp_offset = ig * this->header.nzgrpz;
        std::size_t igrp_offset = ig * this->header.nigrpz;
        std::size_t sgrp_offset = ig * this->header.nsgrpz;
        std::size_t xgrp_offset = ig * this->header.nxgrpz;

        this->groups.emplace_back(this->unit_system,
                                  this->header,
                                  zgrp.data() + zgrp_offset,
                                  igrp.data() + igrp_offset,
                                  sgrp.data() + sgrp_offset,
                                  xgrp.data() + xgrp_offset);
    }
}

const RstWell& RstState::get_well(const std::string& wname) const {
    const auto well_iter = std::find_if(this->wells.begin(),
                                        this->wells.end(),
                                        [&wname] (const auto& well) {
                                            return well.name == wname;
                                        });
    if (well_iter == this->wells.end())
        throw std::out_of_range("No such well: " + wname);

    return *well_iter;
}

RstState RstState::load(EclIO::ERst& rst_file, int report_step) {
    rst_file.loadReportStepNumber(report_step);
    const auto& intehead = rst_file.getRestartData<int>("INTEHEAD", report_step, 0);
    const auto& logihead = rst_file.getRestartData<bool>("LOGIHEAD", report_step, 0);
    const auto& doubhead = rst_file.getRestartData<double>("DOUBHEAD", report_step, 0);

    auto unit_id = intehead[VI::intehead::UNIT];
    ::Opm::UnitSystem unit_system(unit_id);

    if (intehead[VI::intehead::NWELLS] != 0) {
        const auto& zgrp = rst_file.getRestartData<std::string>("ZGRP", report_step, 0);
        const auto& igrp = rst_file.getRestartData<int>("IGRP", report_step, 0);
        const auto& sgrp = rst_file.getRestartData<float>("SGRP", report_step, 0);
        const auto& xgrp = rst_file.getRestartData<double>("XGRP", report_step, 0);

        const auto& zwel = rst_file.getRestartData<std::string>("ZWEL", report_step, 0);
        const auto& iwel = rst_file.getRestartData<int>("IWEL", report_step, 0);
        const auto& swel = rst_file.getRestartData<float>("SWEL", report_step, 0);
        const auto& xwel = rst_file.getRestartData<double>("XWEL", report_step, 0);

        const auto& icon = rst_file.getRestartData<int>("ICON", report_step, 0);
        const auto& scon = rst_file.getRestartData<float>("SCON", report_step, 0);
        const auto& xcon = rst_file.getRestartData<double>("XCON", report_step, 0);


        if (rst_file.hasKey("ISEG")) {
            const auto& iseg = rst_file.getRestartData<int>("ISEG", report_step, 0);
            const auto& rseg = rst_file.getRestartData<double>("RSEG", report_step, 0);

            return RstState(unit_system,
                            intehead, logihead, doubhead,
                            zgrp, igrp, sgrp, xgrp,
                            zwel, iwel, swel, xwel,
                            icon, scon, xcon,
                            iseg, rseg);
        } else
            return RstState(unit_system,
                            intehead, logihead, doubhead,
                            zgrp, igrp, sgrp, xgrp,
                            zwel, iwel, swel, xwel,
                            icon, scon, xcon);
    } else
        return RstState(unit_system, intehead, logihead, doubhead);
}

}
}


