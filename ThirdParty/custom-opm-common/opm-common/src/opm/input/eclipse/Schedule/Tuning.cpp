/*
  Copyright 2015 Statoil ASA.

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

#include <opm/input/eclipse/Schedule/Tuning.hpp>

#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>

namespace Opm {


NextStep::NextStep(double value, bool every_report)
    : next_tstep(value)
    , persist(every_report)
{}

bool NextStep::operator==(const NextStep& other) const {
    return this->next_tstep == other.next_tstep &&
           this->persist == other.persist;
}

double NextStep::value() const {
    return this->next_tstep;
}

bool NextStep::every_report() const {
    return this->persist;
}





Tuning::Tuning() {
    using TuningKw = ParserKeywords::TUNING;
    using WsegIterKW = ParserKeywords::WSEGITER;

    // Record1
    TSINIT = TuningKw::TSINIT::defaultValue * Metric::Time;
    TSMAXZ = TuningKw::TSMAXZ::defaultValue * Metric::Time;
    TSMINZ = TuningKw::TSMINZ::defaultValue * Metric::Time;
    TSMCHP = TuningKw::TSMCHP::defaultValue * Metric::Time;
    TSFMAX = TuningKw::TSFMAX::defaultValue;
    TSFMIN = TuningKw::TSFMIN::defaultValue;
    TFDIFF = TuningKw::TFDIFF::defaultValue;
    TSFCNV = TuningKw::TSFCNV::defaultValue;
    THRUPT = TuningKw::THRUPT::defaultValue;

    // Record 2
    TRGTTE = TuningKw::TRGTTE::defaultValue;
    TRGCNV = TuningKw::TRGCNV::defaultValue;
    TRGMBE = TuningKw::TRGMBE::defaultValue;
    TRGLCV = TuningKw::TRGLCV::defaultValue;
    XXXTTE = TuningKw::XXXTTE::defaultValue;
    XXXCNV = TuningKw::XXXCNV::defaultValue;
    XXXMBE = TuningKw::XXXMBE::defaultValue;
    XXXLCV = TuningKw::XXXLCV::defaultValue;
    XXXWFL = TuningKw::XXXWFL::defaultValue;
    TRGFIP = TuningKw::TRGFIP::defaultValue;
    THIONX = TuningKw::THIONX::defaultValue;
    TRWGHT = TuningKw::TRWGHT::defaultValue;

    // Record 3
    NEWTMX = TuningKw::NEWTMX::defaultValue;
    NEWTMN = TuningKw::NEWTMN::defaultValue;
    LITMAX = TuningKw::LITMAX::defaultValue;
    LITMIN = TuningKw::LITMIN::defaultValue;
    MXWSIT = TuningKw::MXWSIT::defaultValue;
    MXWPIT = TuningKw::MXWPIT::defaultValue;
    DDPLIM = TuningKw::DDPLIM::defaultValue * Metric::Pressure;
    DDSLIM = TuningKw::DDSLIM::defaultValue;
    TRGDPR = TuningKw::TRGDPR::defaultValue * Metric::Pressure;
    XXXDPR = 0.0 * Metric::Pressure;

    WSEG_MAX_RESTART = WsegIterKW::MAX_TIMES_REDUCED::defaultValue;
    WSEG_REDUCTION_FACTOR = WsegIterKW::REDUCTION_FACTOR::defaultValue;
    WSEG_INCREASE_FACTOR = WsegIterKW::INCREASING_FACTOR::defaultValue;
}


Tuning Tuning::serializeObject() {
    Tuning result;
    result.TSINIT = 1.0;
    result.TSMAXZ = 2.0;
    result.TSMINZ = 3.0;
    result.TSMCHP = 4.0;
    result.TSFMAX = 5.0;
    result.TSFMIN = 6.0;
    result.TFDIFF = 7.0;
    result.TSFCNV = 8.0;
    result.THRUPT = 9.0;
    result.TMAXWC = 10.0;
    result.TMAXWC_has_value = true;

    result.TRGTTE = 11.0;
    result.TRGCNV = 12.0;
    result.TRGMBE = 13.0;
    result.TRGLCV = 14.0;
    result.XXXTTE = 15.0;
    result.XXXCNV = 16.0;
    result.XXXMBE = 17.0;
    result.XXXLCV = 18.0;
    result.XXXWFL = 19.0;
    result.TRGFIP = 20.0;
    result.TRGSFT = 21.0;
    result.TRGSFT_has_value = true;
    result.THIONX = 22.0;
    result.TRWGHT = 23.0;

    result.NEWTMX = 24;
    result.NEWTMN = 25;
    result.LITMAX = 26;
    result.LITMIN = 27;
    result.MXWSIT = 28;
    result.MXWPIT = 29;
    result.DDPLIM = 30.0;
    result.DDSLIM = 31.0;
    result.TRGDPR = 32.0;
    result.XXXDPR = 33.0;
    result.XXXDPR_has_value = true;

    return result;
}

bool Tuning::operator==(const Tuning& data) const {
    return TSINIT == data.TSINIT &&
           TSMAXZ == data.TSMAXZ &&
           TSMINZ == data.TSMINZ &&
           TSMCHP == data.TSMCHP &&
           TSFMAX == data.TSFMAX &&
           TSFMIN == data.TSFMIN &&
           TSFCNV == data.TSFCNV &&
           TFDIFF == data.TFDIFF &&
           THRUPT == data.THRUPT &&
           TMAXWC == data.TMAXWC &&
           TMAXWC_has_value == data.TMAXWC_has_value &&
           TRGTTE == data.TRGTTE &&
           TRGCNV == data.TRGCNV &&
           TRGMBE == data.TRGMBE &&
           TRGLCV == data.TRGLCV &&
           XXXTTE == data.XXXTTE &&
           XXXCNV == data.XXXCNV &&
           XXXMBE == data.XXXMBE &&
           XXXLCV == data.XXXLCV &&
           XXXWFL == data.XXXWFL &&
           TRGFIP == data.TRGFIP &&
           TRGSFT == data.TRGSFT &&
           TRGSFT_has_value == data.TRGSFT_has_value &&
           THIONX == data.THIONX &&
           TRWGHT == data.TRWGHT &&
           NEWTMX == data.NEWTMX &&
           NEWTMN == data.NEWTMN &&
           LITMAX == data.LITMAX &&
           LITMIN == data.LITMIN &&
           MXWSIT == data.MXWSIT &&
           MXWPIT == data.MXWPIT &&
           DDPLIM == data.DDPLIM &&
           DDSLIM == data.DDSLIM &&
           TRGDPR == data.TRGDPR &&
           XXXDPR == data.XXXDPR &&
           XXXDPR_has_value == data.XXXDPR_has_value &&
           WSEG_MAX_RESTART == data.WSEG_MAX_RESTART &&
           WSEG_REDUCTION_FACTOR == data.WSEG_REDUCTION_FACTOR &&
           WSEG_INCREASE_FACTOR == data.WSEG_INCREASE_FACTOR;
}

}
