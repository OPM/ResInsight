/*
  Copyright (c) 2018 Equinor ASA
  Copyright (c) 2018 Statoil ASA

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

#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/DoubHEAD.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/GuideRateConfig.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <chrono>
#include <cstddef>
#include <vector>

namespace {
    Opm::RestartIO::DoubHEAD::TimeStamp
    computeTimeStamp(const ::Opm::Schedule& sched,
                     const double           elapsed)
    {
        return {
            std::chrono::system_clock::from_time_t(sched.getStartTime()),
            std::chrono::duration<
                double, std::chrono::seconds::period>{ elapsed },
        };
    }

    double getTimeConv(const ::Opm::UnitSystem& us)
    {
        switch (us.getType()) {
        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC:
            return static_cast<double>(Opm::Metric::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD:
            return static_cast<double>(Opm::Field::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_LAB:
            return static_cast<double>(Opm::Lab::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_PVT_M:
            return static_cast<double>(Opm::PVT_M::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_INPUT:
            throw std::invalid_argument {
                "Cannot Run Simulation With Non-Standard Units"
            };
        }

        return static_cast<double>(Opm::Metric::Time);
    }
    
    
    Opm::RestartIO::DoubHEAD::guideRate
    computeGuideRate(const ::Opm::Schedule& sched,
                     const std::size_t    lookup_step)
    {
            double a = 0.;
            double b = 0.;
            double c = 0.;
            double d = 0.;
            double e = 0.;
            double f = 0.;
            double delay = 0.;
            double damping_fact = 0.;
            
            const auto& guideCFG = sched.guideRateConfig(lookup_step);
            if (guideCFG.has_model()) {
                const auto& guideRateModel = guideCFG.model();
                
                a = guideRateModel.getA();
                b = guideRateModel.getB();
                c = guideRateModel.getC();
                d = guideRateModel.getD();
                e = guideRateModel.getE();
                f = guideRateModel.getF();
                delay = guideRateModel.update_delay();
                damping_fact = guideRateModel.damping_factor();
            }
            return {
                a,
                b,
                c,
                d,
                e,
                f,
                delay,
                damping_fact
            };
    }
} // Anonymous

// #####################################################################
// Public Interface (createDoubHead()) Below Separator
// ---------------------------------------------------------------------

std::vector<double>
Opm::RestartIO::Helpers::
createDoubHead(const EclipseState& es,
               const Schedule&     sched,
               const std::size_t   lookup_step,
               const double        simTime,
               const double        nextTimeStep)
{
    const auto& usys  = es.getDeckUnitSystem();
    const auto& rspec  = es.runspec();
    const auto  tconv = getTimeConv(usys);

    auto dh = DoubHEAD{}
        .tuningParameters(sched.getTuning(lookup_step), tconv)
        .timeStamp       (computeTimeStamp(sched, simTime))
        .drsdt           (sched, lookup_step, tconv)
        .udq_param(rspec.udqParams())
        .guide_rate_param(computeGuideRate(sched, lookup_step))
        ;

    if (nextTimeStep > 0.0) {
        using M = ::Opm::UnitSystem::measure;

        dh.nextStep(usys.from_si(M::time, nextTimeStep));
    }

    return dh.data();
}
