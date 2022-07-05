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

#include <opm/io/eclipse/rst/state.hpp>

#include <opm/io/eclipse/RestartFileView.hpp>

#include <opm/io/eclipse/rst/aquifer.hpp>
#include <opm/io/eclipse/rst/action.hpp>
#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/io/eclipse/rst/group.hpp>
#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/network.hpp>
#include <opm/io/eclipse/rst/udq.hpp>
#include <opm/io/eclipse/rst/well.hpp>

#include <opm/common/utility/String.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Schedule/Action/Actdims.hpp>
#include <opm/input/eclipse/Schedule/Action/Condition.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/output/eclipse/UDQDims.hpp>
#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/doubhead.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>

namespace {
    std::string
    udq_define(const std::vector<std::string>& zudl,
               const std::size_t               udq_index)
    {
        auto zudl_begin = zudl.begin();
        auto zudl_end = zudl.begin();
        std::advance( zudl_begin, (udq_index + 0) * Opm::UDQDims::entriesPerZUDL() );
        std::advance( zudl_end  , (udq_index + 1) * Opm::UDQDims::entriesPerZUDL() );

        auto define = std::accumulate(zudl_begin, zudl_end, std::string{}, std::plus<>());
        if (!define.empty() && (define[0] == '~')) {
            define[0] = '-';
        }

        return define;
    }

    Opm::UDQUpdate udq_update(const std::vector<int>& iudq,
                              const std::size_t       udq_index)
    {
        return Opm::UDQ::updateType(iudq[udq_index * Opm::UDQDims::entriesPerIUDQ()]);
    }

}

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm { namespace RestartIO {

RstState::RstState(std::shared_ptr<EclIO::RestartFileView> rstView,
                   const Runspec& runspec,
                   const ::Opm::EclipseGrid*               grid)
    : unit_system(rstView->intehead()[VI::intehead::UNIT])
    , header(runspec, unit_system, rstView->intehead(), rstView->logihead(), rstView->doubhead())
    , aquifers(rstView, grid, unit_system)
    , network(rstView, unit_system)
{
    this->load_tuning(rstView->intehead(), rstView->doubhead());
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
    auto load_group = [this, &zgrp, &igrp, &sgrp, &xgrp](const int ig)
    {
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
    };

    // Load active named/user-defined groups.
    for (int ig=0; ig < this->header.ngroup; ig++)
        load_group(ig);

    // Load FIELD group from zero-based window index NGMAX in the *GRP
    // arrays.  Needed to reconstruct any field-wide constraints (e.g.,
    // GCONINJE and/or GCONPROD applied to the FIELD itself).
    //
    // Recall that 'max_groups_in_field' is really NGMAX + 1 here as FIELD
    // is also included in this value in the restart file.  Subtract one to
    // get the actual NGMAX value.
    load_group(this->header.max_groups_in_field - 1);
}

void RstState::add_wells(const std::vector<std::string>& zwel,
                         const std::vector<int>& iwel,
                         const std::vector<float>& swel,
                         const std::vector<double>& xwel,
                         const std::vector<int>& icon,
                         const std::vector<float>& scon,
                         const std::vector<double>& xcon) {

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

void RstState::add_msw(const std::vector<std::string>& zwel,
                       const std::vector<int>& iwel,
                       const std::vector<float>& swel,
                       const std::vector<double>& xwel,
                       const std::vector<int>& icon,
                       const std::vector<float>& scon,
                       const std::vector<double>& xcon,
                       const std::vector<int>& iseg,
                       const std::vector<double>& rseg) {

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

void RstState::add_udqs(const std::vector<int>& iudq,
                        const std::vector<std::string>& zudn,
                        const std::vector<std::string>& zudl,
                        const std::vector<double>& dudw,
                        const std::vector<double>& dudg,
                        const std::vector<double>& dudf)
{
    auto well_var  = 0*this->header.num_wells;
    auto group_var = 0*this->header.ngroup;
    auto field_var = 0*this->header.num_udq();

    for (auto udq_index = 0*this->header.num_udq(); udq_index < this->header.num_udq(); ++udq_index) {
        const auto& name = zudn[udq_index*UDQDims::entriesPerZUDN() + 0];
        const auto& unit = zudn[udq_index*UDQDims::entriesPerZUDN() + 1];

        const auto define = udq_define(zudl, udq_index);
        auto& udq = define.empty()
            ? this->udqs.emplace_back(name, unit)
            : this->udqs.emplace_back(name, unit, define, udq_update(iudq, udq_index));

        if (udq.var_type == UDQVarType::WELL_VAR) {
            for (std::size_t well_index = 0; well_index < this->wells.size(); well_index++) {
                auto well_value = dudw[ well_var * this->header.max_wells_in_field + well_index ];
                if (well_value == UDQ::restart_default)
                    continue;

                const auto& well_name = this->wells[well_index].name;
                udq.add_value(well_name, well_value);
            }

            ++well_var;
        }

        if (udq.var_type == UDQVarType::GROUP_VAR) {
            for (std::size_t group_index = 0; group_index < this->groups.size(); group_index++) {
                auto group_value = dudg[ group_var * this->header.max_groups_in_field + group_index ];
                if (group_value == UDQ::restart_default)
                    continue;

                const auto& group_name = this->groups[group_index].name;
                udq.add_value(group_name, group_value);
            }

            ++group_var;
        }

        if (udq.var_type == UDQVarType::FIELD_VAR) {
            auto field_value = dudf[ field_var ];
            if (field_value != UDQ::restart_default)
                udq.add_value(field_value);

            ++field_var;
        }
    }
}

std::string oper_string(Action::Logical logic) {
    if (logic == Action::Logical::AND)
        return "AND";

    if (logic == Action::Logical::OR)
        return "OR";

    return "";
}


void RstState::add_actions(const Parser& parser,
                           const Runspec& runspec,
                           std::time_t sim_time,
                           const std::vector<std::string>& zact,
                           const std::vector<int>& iact,
                           const std::vector<float>& sact,
                           const std::vector<std::string>& zacn,
                           const std::vector<int>& iacn,
                           const std::vector<double>& sacn,
                           const std::vector<std::string>& zlact)
{
    const auto& actdims = runspec.actdims();
    auto zact_action_size  = RestartIO::Helpers::entriesPerZACT();
    auto iact_action_size  = RestartIO::Helpers::entriesPerIACT();
    auto sact_action_size  = RestartIO::Helpers::entriesPerSACT();
    auto zacn_action_size  = RestartIO::Helpers::entriesPerZACN(actdims);
    auto iacn_action_size  = RestartIO::Helpers::entriesPerIACN(actdims);
    auto sacn_action_size  = RestartIO::Helpers::entriesPerSACN(actdims);
    auto zlact_action_size = zlact.size() / this->header.num_action;

    auto zacn_cond_size = 13;
    auto iacn_cond_size = 26;
    auto sacn_cond_size = 16;

    for (std::size_t index=0; index < static_cast<std::size_t>(this->header.num_action); index++) {
        std::vector<RstAction::Condition> conditions;
        for (std::size_t icond = 0; icond < actdims.max_conditions(); icond++) {
            const auto zacn_offset = index * zacn_action_size + icond * zacn_cond_size;
            const auto iacn_offset = index * iacn_action_size + icond * iacn_cond_size;
            const auto sacn_offset = index * sacn_action_size + icond * sacn_cond_size;

            if (RstAction::Condition::valid(&zacn[zacn_offset], &iacn[iacn_offset]))
                conditions.emplace_back(&zacn[zacn_offset], &iacn[iacn_offset], &sacn[sacn_offset]);
        }

        const auto& name = zact[index * zact_action_size + 0];
        const auto& max_run = iact[index * iact_action_size + 5];
        const auto& run_count = iact[index * iact_action_size + 2] - 1;
        const auto& min_wait = this->unit_system.to_si(UnitSystem::measure::time, sact[index * sact_action_size + 3]);
        const auto& last_run_elapsed = this->unit_system.to_si(UnitSystem::measure::time, sact[index * sact_action_size + 4]);

        auto last_run_time = TimeService::advance( runspec.start_time(), last_run_elapsed );
        this->actions.emplace_back(name, max_run, run_count, min_wait, sim_time, last_run_time, conditions );


        std::string action_deck;
        auto zlact_offset = index * zlact_action_size;
        while (true) {
            std::string line;
            for (std::size_t item_index = 0; item_index < actdims.line_size(); item_index++)
                line += zlact[zlact_offset + item_index];

            line = trim_copy(line);
            if (line.empty())
                continue;

            if (line == "ENDACTIO")
                break;

            action_deck += line + "\n";
            zlact_offset += actdims.line_size();
        }
        const auto& deck = parser.parseString( action_deck );
        for (auto keyword : deck)
            this->actions.back().keywords.push_back(std::move(keyword));
    }
}

void RstState::add_wlist(const std::vector<std::string>& zwls,
                         const std::vector<int>& iwls)
{
    for (auto well_index = 0*this->header.num_wells; well_index < this->header.num_wells; well_index++) {
        const auto zwls_offset = this->header.max_wlist * well_index;
        const auto iwls_offset = this->header.max_wlist * well_index;
        const auto& well_name = this->wells[well_index].name;

        for (auto wlist_index = 0*this->header.max_wlist; wlist_index < this->header.max_wlist; wlist_index++) {
            const auto well_order = iwls[iwls_offset + wlist_index];
            if (well_order < 1) {
                continue;
            }

            auto& wlist = this->wlists[zwls[zwls_offset + wlist_index]];
            if (wlist.size() < static_cast<std::vector<std::string>::size_type>(well_order)) {
                wlist.resize(well_order);
            }

            wlist[well_order - 1] = well_name;
        }
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

RstState RstState::load(std::shared_ptr<EclIO::RestartFileView> rstView,
                        const Runspec& runspec,
                        const Parser& parser,
                        const ::Opm::EclipseGrid*               grid)
{
    RstState state(rstView, runspec, grid);

    // At minimum we need any applicable constraint data for FIELD.  Load
    // groups unconditionally.
    {
        const auto& zgrp = rstView->getKeyword<std::string>("ZGRP");
        const auto& igrp = rstView->getKeyword<int>("IGRP");
        const auto& sgrp = rstView->getKeyword<float>("SGRP");
        const auto& xgrp = rstView->getKeyword<double>("XGRP");
        state.add_groups(zgrp, igrp, sgrp, xgrp);
    }

    if (state.header.num_wells > 0) {
        const auto& zwel = rstView->getKeyword<std::string>("ZWEL");
        const auto& iwel = rstView->getKeyword<int>("IWEL");
        const auto& swel = rstView->getKeyword<float>("SWEL");
        const auto& xwel = rstView->getKeyword<double>("XWEL");

        const auto& icon = rstView->getKeyword<int>("ICON");
        const auto& scon = rstView->getKeyword<float>("SCON");
        const auto& xcon = rstView->getKeyword<double>("XCON");

        if (rstView->hasKeyword<int>("ISEG")) {
            const auto& iseg = rstView->getKeyword<int>("ISEG");
            const auto& rseg = rstView->getKeyword<double>("RSEG");

            state.add_msw(zwel, iwel, swel, xwel,
                          icon, scon, xcon,
                          iseg, rseg);
        } else
            state.add_wells(zwel, iwel, swel, xwel,
                            icon, scon, xcon);

        if (rstView->hasKeyword<int>("IWLS")) {
            const auto& iwls = rstView->getKeyword<int>("IWLS");
            const auto& zwls = rstView->getKeyword<std::string>("ZWLS");

            state.add_wlist(zwls, iwls);
        }
    }

    if (state.header.num_udq() > 0) {
        const auto& iudq = rstView->getKeyword<int>("IUDQ");
        const auto& zudn = rstView->getKeyword<std::string>("ZUDN");
        const auto& zudl = rstView->getKeyword<std::string>("ZUDL");

        const auto& dudw = state.header.nwell_udq  > 0 ? rstView->getKeyword<double>("DUDW") : std::vector<double>{};
        const auto& dudg = state.header.ngroup_udq > 0 ? rstView->getKeyword<double>("DUDG") : std::vector<double>{};
        const auto& dudf = state.header.nfield_udq > 0 ? rstView->getKeyword<double>("DUDF") : std::vector<double>{};

        state.add_udqs(iudq, zudn, zudl, dudw, dudg, dudf);

        if (rstView->hasKeyword<int>("IUAD")) {
            const auto& iuad = rstView->getKeyword<int>("IUAD");
            const auto& iuap = rstView->getKeyword<int>("IUAP");
            const auto& igph = rstView->getKeyword<int>("IGPH");
            state.udq_active = RstUDQActive(iuad, iuap, igph);
        }
    }

    if (state.header.num_action > 0) {
        const auto& zact = rstView->getKeyword<std::string>("ZACT");
        const auto& iact = rstView->getKeyword<int>("IACT");
        const auto& sact = rstView->getKeyword<float>("SACT");
        const auto& zacn = rstView->getKeyword<std::string>("ZACN");
        const auto& iacn = rstView->getKeyword<int>("IACN");
        const auto& sacn = rstView->getKeyword<double>("SACN");
        const auto& zlact= rstView->getKeyword<std::string>("ZLACT");
        state.add_actions(parser, runspec, state.header.sim_time(), zact, iact, sact, zacn, iacn, sacn, zlact);
    }


    return state;
}

}} // namespace Opm::RestartIO
