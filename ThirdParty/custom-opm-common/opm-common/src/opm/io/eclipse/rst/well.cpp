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

#include <opm/common/utility/String.hpp>

#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/io/eclipse/rst/well.hpp>

#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/msw.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>


namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm {
namespace RestartIO {

constexpr int def_ecl_phase = 1;
constexpr int def_pvt_table = 0;

using M  = ::Opm::UnitSystem::measure;

double swel_value(float raw_value) {
    const auto infty = 1.0e+20f;
    if (std::abs(raw_value) == infty)
        return 0;
    else
        return raw_value;
}

RstWell::RstWell(const ::Opm::UnitSystem& unit_system,
                 const RstHeader& header,
                 const std::string& group_arg,
                 const std::string* zwel,
                 const int * iwel,
                 const float * swel,
                 const double * xwel,
                 const int * icon,
                 const float * scon,
                 const double * xcon) :
    name(rtrim_copy(zwel[0])),
    group(group_arg),
    ij(                                                              {iwel[VI::IWell::IHead] - 1, iwel[VI::IWell::JHead] - 1}),
    k1k2(                                                            std::make_pair(iwel[VI::IWell::FirstK] - 1, iwel[VI::IWell::LastK] - 1)),
    wtype(                                                           iwel[VI::IWell::WType], def_ecl_phase),
    well_status(                                                     iwel[VI::IWell::Status]),
    active_control(                                                  iwel[VI::IWell::ActWCtrl]),
    vfp_table(                                                       iwel[VI::IWell::VFPTab]),
    allow_xflow(                                                     iwel[VI::IWell::XFlow] == 1),
    preferred_phase(                                                 iwel[VI::IWell::PreferredPhase]),
    hist_requested_control(                                          iwel[VI::IWell::HistReqWCtrl]),
    msw_index(                                                       iwel[VI::IWell::MsWID]),
    completion_ordering(                                             iwel[VI::IWell::CompOrd]),
    pvt_table(                                                       def_pvt_table),
    msw_pressure_drop_model(                                         iwel[VI::IWell::MSW_PlossMod]),
    // The values orat_target -> bhp_target_flow will be used in UDA values. The
    // UDA values are responsible for unit conversion and raw values are
    // internalized here.
    orat_target(                                                     swel_value(swel[VI::SWell::OilRateTarget])),
    wrat_target(                                                     swel_value(swel[VI::SWell::WatRateTarget])),
    grat_target(                                                     swel_value(swel[VI::SWell::GasRateTarget])),
    lrat_target(                                                     swel_value(swel[VI::SWell::LiqRateTarget])),
    resv_target(                                                     swel_value(swel[VI::SWell::ResVRateTarget])),
    thp_target(                                                      swel_value(swel[VI::SWell::THPTarget])),
    bhp_target_float(                                                swel[VI::SWell::BHPTarget]),
    hist_lrat_target(    unit_system.to_si(M::liquid_surface_rate,   swel[VI::SWell::HistLiqRateTarget])),
    hist_grat_target(    unit_system.to_si(M::gas_surface_rate,      swel[VI::SWell::HistGasRateTarget])),
    hist_bhp_target(     unit_system.to_si(M::pressure,              swel[VI::SWell::HistBHPTarget])),
    datum_depth(         unit_system.to_si(M::length,                swel[VI::SWell::DatumDepth])),
    drainage_radius(     unit_system.to_si(M::length,                swel_value(swel[VI::SWell::DrainageRadius]))),
    efficiency_factor(   unit_system.to_si(M::identity,              swel[VI::SWell::EfficiencyFactor1])),
    alq_value(                                                       swel[VI::SWell::Alq_value]),
    oil_rate(            unit_system.to_si(M::liquid_surface_rate,   xwel[VI::XWell::OilPrRate])),
    water_rate(          unit_system.to_si(M::liquid_surface_rate,   xwel[VI::XWell::WatPrRate])),
    gas_rate(            unit_system.to_si(M::gas_surface_rate,      xwel[VI::XWell::GasPrRate])),
    liquid_rate(         unit_system.to_si(M::rate,                  xwel[VI::XWell::LiqPrRate])),
    void_rate(           unit_system.to_si(M::rate,                  xwel[VI::XWell::VoidPrRate])),
    thp(                 unit_system.to_si(M::pressure,              xwel[VI::XWell::TubHeadPr])),
    flow_bhp(            unit_system.to_si(M::pressure,              xwel[VI::XWell::FlowBHP])),
    wct(                 unit_system.to_si(M::water_cut,             xwel[VI::XWell::WatCut])),
    gor(                 unit_system.to_si(M::gas_oil_ratio,         xwel[VI::XWell::GORatio])),
    oil_total(           unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::OilPrTotal])),
    water_total(         unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::WatPrTotal])),
    gas_total(           unit_system.to_si(M::gas_surface_volume,    xwel[VI::XWell::GasPrTotal])),
    void_total(          unit_system.to_si(M::volume,                xwel[VI::XWell::VoidPrTotal])),
    water_inj_total(     unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::WatInjTotal])),
    gas_inj_total(       unit_system.to_si(M::gas_surface_volume,    xwel[VI::XWell::GasInjTotal])),
    void_inj_total(      unit_system.to_si(M::volume,                xwel[VI::XWell::VoidInjTotal])),
    gas_fvf(                                                         xwel[VI::XWell::GasFVF]),
    bhp_target_double(   unit_system.to_si(M::pressure,              xwel[VI::XWell::BHPTarget])),
    hist_oil_total(      unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::HistOilPrTotal])),
    hist_wat_total(      unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::HistWatPrTotal])),
    hist_gas_total(      unit_system.to_si(M::gas_surface_volume,    xwel[VI::XWell::HistGasPrTotal])),
    hist_water_inj_total(unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::HistWatInjTotal])),
    hist_gas_inj_total(  unit_system.to_si(M::gas_surface_volume,    xwel[VI::XWell::HistGasInjTotal])),
    water_void_rate(     unit_system.to_si(M::liquid_surface_volume, xwel[VI::XWell::WatVoidPrRate])),
    gas_void_rate(       unit_system.to_si(M::gas_surface_volume,    xwel[VI::XWell::GasVoidPrRate]))
{
    for (int ic = 0; ic < iwel[VI::IWell::NConn]; ic++) {
        std::size_t icon_offset = ic * header.niconz;
        std::size_t scon_offset = ic * header.nsconz;
        std::size_t xcon_offset = ic * header.nxconz;
        this->connections.emplace_back( unit_system, ic, header.nsconz, icon + icon_offset, scon + scon_offset, xcon + xcon_offset);
    }
}



RstWell::RstWell(const ::Opm::UnitSystem& unit_system,
                 const RstHeader& header,
                 const std::string& group_arg,
                 const std::string* zwel,
                 const int * iwel,
                 const float * swel,
                 const double * xwel,
                 const int * icon,
                 const float * scon,
                 const double * xcon,
                 const std::vector<int>& iseg,
                 const std::vector<double>& rseg) :
    RstWell(unit_system, header, group_arg, zwel, iwel, swel, xwel, icon, scon, xcon)
{

    if (this->msw_index) {
        std::unordered_map<int, std::size_t> segment_map;
        for (int is=0; is < header.nsegmx; is++) {
            std::size_t iseg_offset = header.nisegz * (is + (this->msw_index - 1) * header.nsegmx);
            std::size_t rseg_offset = header.nrsegz * (is + (this->msw_index - 1) * header.nsegmx);
            auto other_segment_number = iseg[iseg_offset + VI::ISeg::SegNo];
            if (other_segment_number != 0) {
                auto segment_number = is + 1;
                segment_map.insert({segment_number, this->segments.size()});
                this->segments.emplace_back( unit_system, segment_number, iseg.data() + iseg_offset, rseg.data() + rseg_offset);
            }
        }

        for (auto& segment : this->segments) {
            if (segment.outlet_segment != 0) {
                auto& outlet_segment = this->segments[ segment_map[segment.outlet_segment] ];
                outlet_segment.inflow_segments.push_back(segment.segment);
            }
        }
    }
}

const RstSegment RstWell::segment(int segment_number) const {
    const auto& iter = std::find_if(this->segments.begin(), this->segments.end(), [segment_number](const RstSegment& segment) { return segment.segment == segment_number; });
    if (iter == this->segments.end())
        throw std::invalid_argument("No such segment");

    return *iter;
}

}
}
