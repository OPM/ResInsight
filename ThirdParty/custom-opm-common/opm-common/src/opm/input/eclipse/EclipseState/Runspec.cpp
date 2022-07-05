/*
  Copyright 2016  Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/F.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <fmt/format.h>

namespace {
    Opm::Phases inferActivePhases(const Opm::Deck& deck)
    {
        return {
            deck.hasKeyword<Opm::ParserKeywords::OIL>(),
            deck.hasKeyword<Opm::ParserKeywords::GAS>(),
            deck.hasKeyword<Opm::ParserKeywords::WATER>(),
            deck.hasKeyword<Opm::ParserKeywords::SOLVENT>(),
            deck.hasKeyword<Opm::ParserKeywords::POLYMER>(),
            deck.hasKeyword<Opm::ParserKeywords::THERMAL>(),
            deck.hasKeyword<Opm::ParserKeywords::POLYMW>(),
            deck.hasKeyword<Opm::ParserKeywords::FOAM>(),
            deck.hasKeyword<Opm::ParserKeywords::BRINE>(),
            deck.hasKeyword<Opm::ParserKeywords::PVTSOL>()
        };
    }

    Opm::SatFuncControls::ThreePhaseOilKrModel
    inferThreePhaseOilKrModel(const Opm::Deck& deck)
    {
        using KroModel = Opm::SatFuncControls::ThreePhaseOilKrModel;

        if (deck.hasKeyword<Opm::ParserKeywords::STONE1>())
            return KroModel::Stone1;

        if (deck.hasKeyword<Opm::ParserKeywords::STONE>() ||
            deck.hasKeyword<Opm::ParserKeywords::STONE2>())
            return KroModel::Stone2;

        return KroModel::Default;
    }

    Opm::SatFuncControls::KeywordFamily
    inferKeywordFamily(const Opm::Deck& deck)
    {
        const auto phases = inferActivePhases(deck);
        const auto wat    = phases.active(Opm::Phase::WATER);
        const auto oil    = phases.active(Opm::Phase::OIL);
        const auto gas    = phases.active(Opm::Phase::GAS);

        const auto threeP = gas && oil && wat;
        const auto twoP = (!gas && oil && wat) || (gas && oil && !wat);

        const auto family1 =       // SGOF/SLGOF and/or SWOF
            (gas && (deck.hasKeyword<Opm::ParserKeywords::SGOF>() ||
                     deck.hasKeyword<Opm::ParserKeywords::SLGOF>())) ||
            (wat && deck.hasKeyword<Opm::ParserKeywords::SWOF>()) ||
            (wat && deck.hasKeyword<Opm::ParserKeywords::SWOFLET>());

        // note: we allow for SOF2 to be part of family1 for threeP +
        // solvent simulations.

        const auto family2 =      // SGFN, SOF{2,3}, SWFN
            (gas && deck.hasKeyword<Opm::ParserKeywords::SGFN>()) ||
            (oil && ((threeP && deck.hasKeyword<Opm::ParserKeywords::SOF3>()) ||
                     (twoP && deck.hasKeyword<Opm::ParserKeywords::SOF2>()))) ||
            (wat && deck.hasKeyword<Opm::ParserKeywords::SWFN>());

        if (family1)
            return Opm::SatFuncControls::KeywordFamily::Family_I;

        if (family2)
            return Opm::SatFuncControls::KeywordFamily::Family_II;

        return Opm::SatFuncControls::KeywordFamily::Undefined;
    }


std::time_t create_start_time(const Opm::Deck& deck) {
    if (deck.hasKeyword("START")) {
        const auto& keyword = deck["START"].back();
        return Opm::TimeService::timeFromEclipse(keyword.getRecord(0));
    } else
        // The default start date is not specified in the Eclipse
        // reference manual. We hence just assume it is same as for
        // the START keyword for Eclipse E100, i.e., January 1st,
        // 1983...
        return Opm::TimeService::mkdate(1983, 1, 1);
}

}

namespace Opm {

Phase get_phase( const std::string& str ) {
    if( str == "OIL" ) return Phase::OIL;
    if( str == "GAS" ) return Phase::GAS;
    if( str == "WAT" ) return Phase::WATER;
    if( str == "WATER" )   return Phase::WATER;
    if( str == "SOLVENT" ) return Phase::SOLVENT;
    if( str == "POLYMER" ) return Phase::POLYMER;
    if( str == "ENERGY" ) return Phase::ENERGY;
    if( str == "POLYMW" ) return Phase::POLYMW;
    if( str == "FOAM" ) return Phase::FOAM;
    if( str == "BRINE" ) return Phase::BRINE;
    if( str == "ZFRACTION" ) return Phase::ZFRACTION;

    throw std::invalid_argument( "Unknown phase '" + str + "'" );
}

std::ostream& operator<<( std::ostream& stream, const Phase& p ) {
    switch( p ) {
        case Phase::OIL:     return stream << "OIL";
        case Phase::GAS:     return stream << "GAS";
        case Phase::WATER:   return stream << "WATER";
        case Phase::SOLVENT: return stream << "SOLVENT";
        case Phase::POLYMER: return stream << "POLYMER";
        case Phase::ENERGY:  return stream << "ENERGY";
        case Phase::POLYMW:  return stream << "POLYMW";
        case Phase::FOAM:    return stream << "FOAM";
        case Phase::BRINE:   return stream << "BRINE";
        case Phase::ZFRACTION:    return stream << "ZFRACTION";

    }

    return stream;
}

using un = std::underlying_type< Phase >::type;

Phases::Phases( bool oil, bool gas, bool wat, bool sol, bool pol, bool energy, bool polymw, bool foam, bool brine, bool zfraction) noexcept :
    bits( (oil ? (1 << static_cast< un >( Phase::OIL ) )     : 0) |
          (gas ? (1 << static_cast< un >( Phase::GAS ) )     : 0) |
          (wat ? (1 << static_cast< un >( Phase::WATER ) )   : 0) |
          (sol ? (1 << static_cast< un >( Phase::SOLVENT ) ) : 0) |
          (pol ? (1 << static_cast< un >( Phase::POLYMER ) ) : 0) |
          (energy ? (1 << static_cast< un >( Phase::ENERGY ) ) : 0) |
          (polymw ? (1 << static_cast< un >( Phase::POLYMW ) ) : 0) |
          (foam ? (1 << static_cast< un >( Phase::FOAM ) ) : 0) |
          (brine ? (1 << static_cast< un >( Phase::BRINE ) ) : 0) |
          (zfraction ? (1 << static_cast< un >( Phase::ZFRACTION ) ) : 0) )

{}

Phases Phases::serializeObject()
{
    return Phases(true, true, true, false, true, false, true, false);
}

bool Phases::active( Phase p ) const noexcept {
    return this->bits[ static_cast< int >( p ) ];
}

size_t Phases::size() const noexcept {
    return this->bits.count();
}

bool Phases::operator==(const Phases& data) const {
    return bits == data.bits;
}

Welldims::Welldims(const Deck& deck)
{
    using WD = ParserKeywords::WELLDIMS;
    if (deck.hasKeyword<WD>()) {
        const auto& keyword = deck.get<WD>().front();
        const auto& wd = keyword.getRecord(0);

        this->nCWMax = wd.getItem<WD::MAXCONN>().get<int>(0);
        this->nWGMax = wd.getItem<WD::MAX_GROUPSIZE>().get<int>(0);

        // This is the E100 definition.  E300 instead uses
        //
        //   Max{ "MAXGROUPS", "MAXWELLS" }
        //
        // i.e., the maximum of item 1 and item 4 here.
        this->nGMax = wd.getItem<WD::MAXGROUPS>().get<int>(0);
        this->nWMax = wd.getItem<WD::MAXWELLS>().get<int>(0);

        // maximum number of well lists pr well
        this->nWlistPrWellMax = wd.getItem<WD::MAX_WELLIST_PR_WELL>().get<int>(0);
        //maximum number of dynamic well lists
        this->nDynWlistMax = wd.getItem<WD::MAX_DYNAMIC_WELLIST>().get<int>(0);


        this->m_location = keyword.location();
    }
}

Welldims Welldims::serializeObject()
{
    Welldims result;
    result.nWMax = 1;
    result.nCWMax = 2;
    result.nWGMax = 3;
    result.nGMax = 4;
    result.nWlistPrWellMax = 5;
    result.nDynWlistMax = 6;
    result.m_location = KeywordLocation::serializeObject();
    return result;
}

WellSegmentDims::WellSegmentDims() :
    nSegWellMax( ParserKeywords::WSEGDIMS::NSWLMX::defaultValue ),
    nSegmentMax( ParserKeywords::WSEGDIMS::NSEGMX::defaultValue ),
    nLatBranchMax( ParserKeywords::WSEGDIMS::NLBRMX::defaultValue )
{}

WellSegmentDims::WellSegmentDims(const Deck& deck) : WellSegmentDims()
{
    if (deck.hasKeyword("WSEGDIMS")) {
        const auto& wsd = deck["WSEGDIMS"][0].getRecord(0);

        this->nSegWellMax   = wsd.getItem("NSWLMX").get<int>(0);
        this->nSegmentMax   = wsd.getItem("NSEGMX").get<int>(0);
        this->nLatBranchMax = wsd.getItem("NLBRMX").get<int>(0);
    }
}

WellSegmentDims WellSegmentDims::serializeObject()
{
    WellSegmentDims result;
    result.nSegWellMax = 1;
    result.nSegmentMax = 2;
    result.nLatBranchMax = 3;

    return result;
}

bool WellSegmentDims::operator==(const WellSegmentDims& data) const
{
    return this->maxSegmentedWells() == data.maxSegmentedWells() &&
           this->maxSegmentsPerWell() == data.maxSegmentsPerWell() &&
           this->maxLateralBranchesPerWell() == data.maxLateralBranchesPerWell();
}

NetworkDims::NetworkDims() :
    nMaxNoNodes( 0 ),
    nMaxNoBranches( 0 ),
    nMaxNoBranchesConToNode( ParserKeywords::NETWORK::NBCMAX::defaultValue )
{}

NetworkDims::NetworkDims(const Deck& deck) : NetworkDims()
{
    if (deck.hasKeyword("NETWORK")) {
        const auto& wsd = deck["NETWORK"][0].getRecord(0);

        this->nMaxNoNodes   = wsd.getItem("NODMAX").get<int>(0);
        this->nMaxNoBranches   = wsd.getItem("NBRMAX").get<int>(0);
        this->nMaxNoBranchesConToNode = wsd.getItem("NBCMAX").get<int>(0);
    }
}

bool NetworkDims::active() const {
    return this->nMaxNoNodes > 0;
}


NetworkDims NetworkDims::serializeObject()
{
    NetworkDims result;
    result.nMaxNoNodes = 1;
    result.nMaxNoBranches = 2;
    result.nMaxNoBranchesConToNode = 3;

    return result;
}

bool NetworkDims::operator==(const NetworkDims& data) const
{
    return this->maxNONodes() == data.maxNONodes() &&
           this->maxNoBranches() == data.maxNoBranches() &&
           this->maxNoBranchesConToNode() == data.maxNoBranchesConToNode();
}

AquiferDimensions::AquiferDimensions()
    : maxNumAnalyticAquifers   { ParserKeywords::AQUDIMS::NANAQU::defaultValue }
    , maxNumAnalyticAquiferConn{ ParserKeywords::AQUDIMS::NCAMAX::defaultValue }
{}

AquiferDimensions::AquiferDimensions(const Deck& deck)
    : AquiferDimensions{}
{
    using AD = ParserKeywords::AQUDIMS;

    if (deck.hasKeyword<AD>()) {
        const auto& keyword = deck.get<AD>().front();
        const auto& ad = keyword.getRecord(0);

        this->maxNumAnalyticAquifers    = ad.getItem<AD::NANAQU>().get<int>(0);
        this->maxNumAnalyticAquiferConn = ad.getItem<AD::NCAMAX>().get<int>(0);
    }
}

AquiferDimensions AquiferDimensions::serializeObject()
{
    auto dim = AquiferDimensions{};

    dim.maxNumAnalyticAquifers = 3;
    dim.maxNumAnalyticAquiferConn = 10;

    return dim;
}

bool operator==(const AquiferDimensions& lhs, const AquiferDimensions& rhs)
{
    return (lhs.maxAnalyticAquifers() == rhs.maxAnalyticAquifers())
        && (lhs.maxAnalyticAquiferConnections() == rhs.maxAnalyticAquiferConnections());
}

EclHysterConfig::EclHysterConfig(const Opm::Deck& deck)
    {

        if (!deck.hasKeyword("SATOPTS"))
            return;

        const auto& satoptsItem = deck["SATOPTS"].back().getRecord(0).getItem(0);
        for (unsigned i = 0; i < satoptsItem.data_size(); ++i) {
            std::string satoptsValue = satoptsItem.get< std::string >(0);
            std::transform(satoptsValue.begin(),
                           satoptsValue.end(),
                           satoptsValue.begin(),
                           ::toupper);

            if (satoptsValue == "HYSTER")
                activeHyst = true;
        }

        // check for the (deprecated) HYST keyword
        if (deck.hasKeyword("HYST"))
            activeHyst = true;

        if (!activeHyst)
	      return;

        if (!deck.hasKeyword("EHYSTR"))
            throw std::runtime_error("Enabling hysteresis via the HYST parameter for SATOPTS requires the "
                                     "presence of the EHYSTR keyword");
	    /*!
	* \brief Set the type of the hysteresis model which is used for relative permeability.
	*
	* -1: relperm hysteresis is disabled
	* 0: use the Carlson model for relative permeability hysteresis of the non-wetting
	*    phase and the drainage curve for the relperm of the wetting phase
	* 1: use the Carlson model for relative permeability hysteresis of the non-wetting
	*    phase and the imbibition curve for the relperm of the wetting phase
	*/
        const auto& ehystrKeyword = deck["EHYSTR"].back();
        if (deck.hasKeyword("NOHYKR"))
            krHystMod = -1;
        else {
            krHystMod = ehystrKeyword.getRecord(0).getItem("relative_perm_hyst").get<int>(0);

            if (krHystMod != 0 && krHystMod != 1)
                throw std::runtime_error(
                    "Only the Carlson relative permeability hysteresis models (indicated by '0' or "
                    "'1' for the second item of the 'EHYSTR' keyword) are supported");
        }

        // this is slightly screwed: it is possible to specify contradicting hysteresis
        // models with HYPC/NOHYPC and the fifth item of EHYSTR. Let's ignore that for
        // now.
            /*!
	* \brief Return the type of the hysteresis model which is used for capillary pressure.
	*
	* -1: capillary pressure hysteresis is disabled
	* 0: use the Killough model for capillary pressure hysteresis
	*/
        std::string whereFlag =
            ehystrKeyword.getRecord(0).getItem("limiting_hyst_flag").getTrimmedString(0);
        if (deck.hasKeyword("NOHYPC") || whereFlag == "KR")
            pcHystMod = -1;
        else {
            // if capillary pressure hysteresis is enabled, Eclipse always uses the
            // Killough model
            pcHystMod = 0;

            throw std::runtime_error("Capillary pressure hysteresis is not supported yet");
        }
    }

EclHysterConfig EclHysterConfig::serializeObject()
{
    EclHysterConfig result;
    result.activeHyst = true;
    result.pcHystMod = 1;
    result.krHystMod = 2;

    return result;
}

bool EclHysterConfig::active() const
    { return activeHyst; }

int EclHysterConfig::pcHysteresisModel() const
    { return pcHystMod; }

int EclHysterConfig::krHysteresisModel() const
    { return krHystMod; }

bool EclHysterConfig::operator==(const EclHysterConfig& data) const {
    return this->active() == data.active() &&
           this->pcHysteresisModel() == data.pcHysteresisModel() &&
           this->krHysteresisModel() == data.krHysteresisModel();
}

SatFuncControls::SatFuncControls()
    : tolcrit(ParserKeywords::TOLCRIT::VALUE::defaultValue)
{}

SatFuncControls::SatFuncControls(const Deck& deck)
    : SatFuncControls()
{
    using TolCrit = ParserKeywords::TOLCRIT;

    if (deck.hasKeyword<TolCrit>()) {
        // SIDouble doesn't perform any unit conversions here since
        // TOLCRIT is a pure scalar (Dimension = 1).
        this->tolcrit = deck.get<TolCrit>().front().getRecord(0)
            .getItem<TolCrit::VALUE>().getSIDouble(0);
    }

    this->krmodel = inferThreePhaseOilKrModel(deck);
    this->satfunc_family = inferKeywordFamily(deck);
}

SatFuncControls::SatFuncControls(const double tolcritArg,
                                 const ThreePhaseOilKrModel model,
                                 const KeywordFamily family)
    : tolcrit(tolcritArg)
    , krmodel(model)
    , satfunc_family(family)
{}

SatFuncControls SatFuncControls::serializeObject()
{
    return SatFuncControls {
        1.0, ThreePhaseOilKrModel::Stone2, KeywordFamily::Family_I
    };
}

bool SatFuncControls::operator==(const SatFuncControls& rhs) const
{
    return (this->minimumRelpermMobilityThreshold() == rhs.minimumRelpermMobilityThreshold())
        && (this->krModel() == rhs.krModel())
        && (this->family() == rhs.family());
}

Nupcol::Nupcol() :
    Nupcol(ParserKeywords::MINNPCOL::VALUE::defaultValue)
{
}

Nupcol::Nupcol(int min_value) :
    min_nupcol(min_value)
{
    this->update( ParserKeywords::NUPCOL::NUM_ITER::defaultValue );
}

void Nupcol::update(int value) {
    if (value < this->min_nupcol && this->min_nupcol == ParserKeywords::MINNPCOL::VALUE::defaultValue)
        OpmLog::note(fmt::format("Minimum NUPCOL value is {} - see keyword MINNPCOL to adjust the minimum value", this->min_nupcol));
    this->nupcol_value = std::max(value, this->min_nupcol);
}

Nupcol Nupcol::serializeObject() {
    Nupcol nc;
    nc.update(123);
    return nc;
}

int Nupcol::value() const {
    return this->nupcol_value;
}

bool Nupcol::operator==(const Nupcol& data) const {
    return this->min_nupcol == data.min_nupcol &&
           this->nupcol_value == data.nupcol_value;
}


bool Tracers::operator==(const Tracers& other) const {
    return
        this->m_oil_tracers == other.m_oil_tracers &&
        this->m_water_tracers == other.m_water_tracers &&
        this->m_gas_tracers == other.m_gas_tracers &&
        this->m_env_tracers == other.m_env_tracers &&
        this->diffusion_control == other.diffusion_control &&
        this->max_iter == other.max_iter &&
        this->min_iter == other.min_iter;
}

Tracers Tracers::serializeObject() {
    Tracers tracers;
    tracers.m_oil_tracers = 123;
    tracers.m_gas_tracers = 77;
    tracers.max_iter = 11;
    return tracers;
}

int Tracers::water_tracers() const {
    return this->m_water_tracers;
}


Tracers::Tracers(const Deck& deck) {
    using TR = ParserKeywords::TRACERS;

    if (deck.hasKeyword<TR>()) {
        const auto& keyword = deck.get<TR>().back();
        const auto& record = keyword[0];
        this->m_oil_tracers = record.getItem<TR::MAX_OIL_TRACERS>().get<int>(0);
        this->m_water_tracers = record.getItem<TR::MAX_WATER_TRACERS>().get<int>(0);
        this->m_gas_tracers = record.getItem<TR::MAX_GAS_TRACERS>().get<int>(0);
        this->m_env_tracers = record.getItem<TR::MAX_ENV_TRACERS>().get<int>(0);
        this->max_iter = record.getItem<TR::MAX_ITER>().get<int>(0);
        this->min_iter = record.getItem<TR::MIN_ITER>().get<int>(0);

        const auto& diff_control = record.getItem<TR::NUMERIC_DIFF>().get<std::string>(0);
        this->diffusion_control = (diff_control == "DIFF" || diff_control == "SPECIAL");
    } else {
        this->m_oil_tracers = TR::MAX_OIL_TRACERS::defaultValue;
        this->m_water_tracers = TR::MAX_WATER_TRACERS::defaultValue;
        this->m_gas_tracers = TR::MAX_GAS_TRACERS::defaultValue;
        this->m_env_tracers = TR::MAX_ENV_TRACERS::defaultValue;
        this->diffusion_control = false;
        this->max_iter = TR::MAX_ITER::defaultValue;
        this->min_iter = TR::MIN_ITER::defaultValue;
    }
}

const Tracers& Runspec::tracers() const {
    return this->m_tracers;
}

Runspec::Runspec( const Deck& deck )
    : m_start_time( create_start_time(deck) )
    , active_phases( inferActivePhases(deck) )
    , m_tabdims( deck )
    , m_regdims( deck )
    , endscale( deck )
    , welldims( deck )
    , wsegdims( deck )
    , netwrkdims( deck )
    , aquiferdims( deck )
    , udq_params( deck )
    , hystpar( deck )
    , m_actdims( deck )
    , m_sfuncctrl( deck )
    , m_nupcol( )
    , m_tracers( deck )
    , m_co2storage (false)
    , m_micp (false)
{
    if (DeckSection::hasRUNSPEC(deck)) {
        const RUNSPECSection runspecSection{deck};
        if (runspecSection.hasKeyword<ParserKeywords::MINNPCOL>()) {
            const auto& min_item = runspecSection.get<ParserKeywords::MINNPCOL>().back().getRecord(0).getItem<ParserKeywords::MINNPCOL::VALUE>();
            auto min_value = min_item.get<int>(0);
            this->m_nupcol = Nupcol(min_value);
        }

        using NC = ParserKeywords::NUPCOL;
        if (runspecSection.hasKeyword<NC>()) {
            const auto& item = runspecSection.get<NC>().back()[0].getItem<NC::NUM_ITER>();
            if (item.defaultApplied(0)) {
                std::string msg = fmt::format("OPM Flow uses {} as default NUPCOL value", NC::NUM_ITER::defaultValue);
                OpmLog::note(msg);
            }
            auto deck_nupcol = item.get<int>(0);
            this->m_nupcol.update(deck_nupcol);
        }

        if (runspecSection.hasKeyword<ParserKeywords::CO2STORE>() ||
                runspecSection.hasKeyword<ParserKeywords::CO2STOR>()) {
            m_co2storage = true;
            std::string msg = "The CO2 storage option is given. PVT properties from the Brine-CO2 system is used \n"
                              "See the OPM manual for details on the used models.";
            OpmLog::note(msg);
        }

        if (runspecSection.hasKeyword<ParserKeywords::MICP>()) {
            m_micp = true;
            std::string msg = "The MICP option is given. Single phase (WATER) + 3 transported components + \n"
                              "3 solid phases are used. See https://doi.org/10.1016/j.ijggc.2021.103256 \n"
                              "for details on the used model.";
            OpmLog::note(msg);
        }
    }
}

Runspec Runspec::serializeObject()
{
    Runspec result;
    result.active_phases = Phases::serializeObject();
    result.m_tabdims = Tabdims::serializeObject();
    result.m_regdims = Regdims::serializeObject();
    result.endscale = EndpointScaling::serializeObject();
    result.welldims = Welldims::serializeObject();
    result.wsegdims = WellSegmentDims::serializeObject();
    result.aquiferdims = AquiferDimensions::serializeObject();
    result.udq_params = UDQParams::serializeObject();
    result.hystpar = EclHysterConfig::serializeObject();
    result.m_actdims = Actdims::serializeObject();
    result.m_sfuncctrl = SatFuncControls::serializeObject();
    result.m_nupcol = Nupcol::serializeObject();
    result.m_co2storage = true;
    result.m_micp = true;

    return result;
}

const Phases& Runspec::phases() const noexcept {
    return this->active_phases;
}

const Tabdims& Runspec::tabdims() const noexcept {
    return this->m_tabdims;
}

const Regdims& Runspec::regdims() const noexcept {
    return this->m_regdims;
}

const Actdims& Runspec::actdims() const noexcept {
    return this->m_actdims;
}

const EndpointScaling& Runspec::endpointScaling() const noexcept {
    return this->endscale;
}

const Welldims& Runspec::wellDimensions() const noexcept
{
    return this->welldims;
}

const WellSegmentDims& Runspec::wellSegmentDimensions() const noexcept
{
    return this->wsegdims;
}

const NetworkDims& Runspec::networkDimensions() const noexcept
{
    return this->netwrkdims;
}

const AquiferDimensions& Runspec::aquiferDimensions() const noexcept
{
    return this->aquiferdims;
}

const EclHysterConfig& Runspec::hysterPar() const noexcept
{
    return this->hystpar;
}

const SatFuncControls& Runspec::saturationFunctionControls() const noexcept
{
    return this->m_sfuncctrl;
}

const Nupcol& Runspec::nupcol() const noexcept
{
    return this->m_nupcol;
}

bool Runspec::co2Storage() const noexcept
{
    return this->m_co2storage;
}

bool Runspec::micp() const noexcept
{
    return this->m_micp;
}

std::time_t Runspec::start_time() const noexcept
{
    return this->m_start_time;
}


/*
  Returns an integer in the range 0...7 which can be used to indicate
  available phases in Eclipse restart and init files.
*/
int Runspec::eclPhaseMask( ) const noexcept {
    const int water = 1 << 2;
    const int oil   = 1 << 0;
    const int gas   = 1 << 1;

    return ( active_phases.active( Phase::WATER ) ? water : 0 )
         | ( active_phases.active( Phase::OIL ) ? oil : 0 )
         | ( active_phases.active( Phase::GAS ) ? gas : 0 );
}


const UDQParams& Runspec::udqParams() const noexcept {
    return this->udq_params;
}

bool Runspec::rst_cmp(const Runspec& full_spec, const Runspec& rst_spec) {
    return full_spec.phases() == rst_spec.phases() &&
        full_spec.tabdims() == rst_spec.tabdims() &&
        full_spec.regdims() == rst_spec.regdims() &&
        full_spec.endpointScaling() == rst_spec.endpointScaling() &&
        full_spec.wellSegmentDimensions() == rst_spec.wellSegmentDimensions() &&
        full_spec.aquiferDimensions() == rst_spec.aquiferDimensions() &&
        full_spec.hysterPar() == rst_spec.hysterPar() &&
        full_spec.actdims() == rst_spec.actdims() &&
        full_spec.saturationFunctionControls() == rst_spec.saturationFunctionControls() &&
        full_spec.m_nupcol == rst_spec.m_nupcol &&
        full_spec.m_co2storage == rst_spec.m_co2storage &&
        full_spec.m_micp == rst_spec.m_micp &&
        Welldims::rst_cmp(full_spec.wellDimensions(), rst_spec.wellDimensions());
}

bool Runspec::operator==(const Runspec& data) const {
    return this->phases() == data.phases() &&
           this->tabdims() == data.tabdims() &&
           this->regdims() == data.regdims() &&
           this->endpointScaling() == data.endpointScaling() &&
           this->wellDimensions() == data.wellDimensions() &&
           this->wellSegmentDimensions() == data.wellSegmentDimensions() &&
          (this->aquiferDimensions() == data.aquiferDimensions()) &&
           this->hysterPar() == data.hysterPar() &&
           this->actdims() == data.actdims() &&
           this->saturationFunctionControls() == data.saturationFunctionControls() &&
           this->m_nupcol == data.m_nupcol &&
           this->m_co2storage == data.m_co2storage &&
           this->m_micp == data.m_micp;
}


}
