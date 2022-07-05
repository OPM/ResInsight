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

#ifndef OPM_RUNSPEC_HPP
#define OPM_RUNSPEC_HPP

#include <iosfwd>
#include <string>
#include <optional>

#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Tabdims.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Regdims.hpp>
#include <opm/input/eclipse/EclipseState/EndpointScaling.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>
#include <opm/input/eclipse/Schedule/Action/Actdims.hpp>

namespace Opm {
class Deck;


enum class Phase {
    OIL     = 0,
    GAS     = 1,
    WATER   = 2,
    SOLVENT = 3,
    POLYMER = 4,
    ENERGY  = 5,
    POLYMW  = 6,
    FOAM  = 7,
    BRINE = 8,
    ZFRACTION  = 9

    // If you add more entries to this enum, remember to update NUM_PHASES_IN_ENUM below.
};

constexpr int NUM_PHASES_IN_ENUM = static_cast<int>(Phase::ZFRACTION) + 1;  // Used to get correct size of the bitset in class Phases.

Phase get_phase( const std::string& );
std::ostream& operator<<( std::ostream&, const Phase& );

class Phases {
    public:
        Phases() noexcept = default;
        Phases( bool oil, bool gas, bool wat, bool solvent = false, bool polymer = false, bool energy = false,
                bool polymw = false, bool foam = false, bool brine = false, bool zfraction = false ) noexcept;

        static Phases serializeObject();

        bool active( Phase ) const noexcept;
        size_t size() const noexcept;

        bool operator==(const Phases& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            if (serializer.isSerializing())
                serializer(bits.to_ulong());
            else {
              unsigned long Bits = 0;
              serializer(Bits);
              bits = std::bitset<NUM_PHASES_IN_ENUM>(Bits);
            }
        }

    private:
        std::bitset< NUM_PHASES_IN_ENUM > bits;
};


class Welldims {
public:
    Welldims() = default;
    explicit Welldims(const Deck& deck);

    static Welldims serializeObject();

    int maxConnPerWell() const
    {
        return this->nCWMax;
    }

    int maxWellsPerGroup() const
    {
        return this->nWGMax;
    }

    int maxGroupsInField() const
    {
        return this->nGMax;
    }

    int maxWellsInField() const
    {
        return this->nWMax;
    }

    int maxWellListsPrWell() const
    {
        return this->nWlistPrWellMax;
    }

    int maxDynamicWellLists() const
    {
        return this->nDynWlistMax;
    }

    const std::optional<KeywordLocation>& location() const {
        return this->m_location;
    }

    static bool rst_cmp(const Welldims& full_dims, const Welldims& rst_dims) {
        return full_dims.maxConnPerWell() == rst_dims.maxConnPerWell() &&
            full_dims.maxWellsPerGroup() == rst_dims.maxWellsPerGroup() &&
            full_dims.maxGroupsInField() == rst_dims.maxGroupsInField() &&
            full_dims.maxWellsInField() == rst_dims.maxWellsInField() &&
            full_dims.maxWellListsPrWell() == rst_dims.maxWellListsPrWell() &&
            full_dims.maxDynamicWellLists() == rst_dims.maxDynamicWellLists();
    }

    bool operator==(const Welldims& data) const {
        return this->location() == data.location() &&
            rst_cmp(*this, data);
    }


    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(nWMax);
        serializer(nCWMax);
        serializer(nWGMax);
        serializer(nGMax);
        serializer(nWlistPrWellMax);
        serializer(nDynWlistMax);
        serializer(m_location);
    }

private:
    int nWMax  { 0 };
    int nCWMax { 0 };
    int nWGMax { 0 };
    int nGMax  { 0 };
    int nWlistPrWellMax  { 1 };
    int nDynWlistMax  { 1 };
    std::optional<KeywordLocation> m_location;
};

class WellSegmentDims {
public:
    WellSegmentDims();
    explicit WellSegmentDims(const Deck& deck);

    static WellSegmentDims serializeObject();


    int maxSegmentedWells() const
    {
        return this->nSegWellMax;
    }

    int maxSegmentsPerWell() const
    {
        return this->nSegmentMax;
    }

    int maxLateralBranchesPerWell() const
    {
        return this->nLatBranchMax;
    }

    bool operator==(const WellSegmentDims& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(nSegWellMax);
        serializer(nSegmentMax);
        serializer(nLatBranchMax);
    }

private:
    int nSegWellMax;
    int nSegmentMax;
    int nLatBranchMax;
};

class NetworkDims {
public:
    NetworkDims();
    explicit NetworkDims(const Deck& deck);

    static NetworkDims serializeObject();

    int maxNONodes() const
    {
        return this->nMaxNoNodes;
    }

    int maxNoBranches() const
    {
        return this->nMaxNoBranches;
    }

    int maxNoBranchesConToNode() const
    {
        return this->nMaxNoBranchesConToNode;
    }

    bool active() const;

    bool operator==(const NetworkDims& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(nMaxNoNodes);
        serializer(nMaxNoBranches);
        serializer(nMaxNoBranchesConToNode);
    }

private:
    int nMaxNoNodes;
    int nMaxNoBranches;
    int nMaxNoBranchesConToNode;
};

class AquiferDimensions {
public:
    AquiferDimensions();
    explicit AquiferDimensions(const Deck& deck);

    static AquiferDimensions serializeObject();

    int maxAnalyticAquifers() const
    {
        return this->maxNumAnalyticAquifers;
    }

    int maxAnalyticAquiferConnections() const
    {
        return this->maxNumAnalyticAquiferConn;
    }

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->maxNumAnalyticAquifers);
        serializer(this->maxNumAnalyticAquiferConn);
    }

private:
    int maxNumAnalyticAquifers;
    int maxNumAnalyticAquiferConn;
};

bool operator==(const AquiferDimensions& lhs, const AquiferDimensions& rhs);

class EclHysterConfig
{
public:
    EclHysterConfig() = default;
    explicit EclHysterConfig(const Deck& deck);

    static EclHysterConfig serializeObject();

    /*!
     * \brief Specify whether hysteresis is enabled or not.
     */
    //void setActive(bool yesno);

    /*!
     * \brief Returns whether hysteresis is enabled (active).
     */
    bool active() const;

    /*!
     * \brief Return the type of the hysteresis model which is used for capillary pressure.
     *
     * -1: capillary pressure hysteresis is disabled
     * 0: use the Killough model for capillary pressure hysteresis
     */
    int pcHysteresisModel() const;

    /*!
     * \brief Return the type of the hysteresis model which is used for relative permeability.
     *
     * -1: relperm hysteresis is disabled
     * 0: use the Carlson model for relative permeability hysteresis
     */
    int krHysteresisModel() const;

    bool operator==(const EclHysterConfig& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(activeHyst);
        serializer(pcHystMod);
        serializer(krHystMod);
    }

private:
    // enable hysteresis at all
    bool activeHyst  { false };

    // the capillary pressure and the relperm hysteresis models to be used
    int pcHystMod { 0 };
    int krHystMod { 0 };
};

class SatFuncControls {
public:
    enum class ThreePhaseOilKrModel {
        Default,
        Stone1,
        Stone2
    };

    enum class KeywordFamily {
        Family_I,               // SGOF, SWOF, SLGOF
        Family_II,              // SGFN, SOF{2,3}, SWFN
        Undefined,
    };

    SatFuncControls();
    explicit SatFuncControls(const Deck& deck);
    explicit SatFuncControls(const double tolcritArg,
                             const ThreePhaseOilKrModel model,
                             const KeywordFamily family);

    static SatFuncControls serializeObject();

    double minimumRelpermMobilityThreshold() const
    {
        return this->tolcrit;
    }

    ThreePhaseOilKrModel krModel() const
    {
        return this->krmodel;
    }

    KeywordFamily family() const
    {
        return this->satfunc_family;
    }

    bool operator==(const SatFuncControls& rhs) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(tolcrit);
        serializer(krmodel);
        serializer(satfunc_family);
    }

private:
    double tolcrit;
    ThreePhaseOilKrModel krmodel = ThreePhaseOilKrModel::Default;
    KeywordFamily satfunc_family = KeywordFamily::Undefined;
};


class Nupcol {
public:
    Nupcol();
    explicit Nupcol(int min_value);
    void update(int value);
    int value() const;

    static Nupcol serializeObject();
    bool operator==(const Nupcol& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer(this->nupcol_value);
        serializer(this->min_nupcol);
    }

private:
    int min_nupcol;
    int nupcol_value;
};


class Tracers {
public:

    Tracers() = default;

    explicit Tracers(const Deck& );
    int water_tracers() const;

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer(this->m_oil_tracers);
        serializer(this->m_water_tracers);
        serializer(this->m_gas_tracers);
        serializer(this->m_env_tracers);
        serializer(this->diffusion_control);
        serializer(this->max_iter);
        serializer(this->min_iter);
    }

    static Tracers serializeObject();
    bool operator==(const Tracers& data) const;

private:
    int m_oil_tracers;
    int m_water_tracers;
    int m_gas_tracers;
    int m_env_tracers;
    bool diffusion_control;
    int max_iter;
    int min_iter;
    // The TRACERS keyword has some additional options which seem quite arcane,
    // for now not included here.
};


class Runspec {
public:
    Runspec() = default;
    explicit Runspec( const Deck& );

    static Runspec serializeObject();

    std::time_t start_time() const noexcept;
    const UDQParams& udqParams() const noexcept;
    const Phases& phases() const noexcept;
    const Tabdims&  tabdims() const noexcept;
    const Regdims&  regdims() const noexcept;
    const EndpointScaling& endpointScaling() const noexcept;
    const Welldims& wellDimensions() const noexcept;
    const WellSegmentDims& wellSegmentDimensions() const noexcept;
    const NetworkDims& networkDimensions() const noexcept;
    const AquiferDimensions& aquiferDimensions() const noexcept;
    int eclPhaseMask( ) const noexcept;
    const EclHysterConfig& hysterPar() const noexcept;
    const Actdims& actdims() const noexcept;
    const SatFuncControls& saturationFunctionControls() const noexcept;
    const Nupcol& nupcol() const noexcept;
    const Tracers& tracers() const;
    bool co2Storage() const noexcept;
    bool micp() const noexcept;

    bool operator==(const Runspec& data) const;
    static bool rst_cmp(const Runspec& full_state, const Runspec& rst_state);

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->m_start_time);
        active_phases.serializeOp(serializer);
        m_tabdims.serializeOp(serializer);
        m_regdims.serializeOp(serializer);
        endscale.serializeOp(serializer);
        welldims.serializeOp(serializer);
        wsegdims.serializeOp(serializer);
        netwrkdims.serializeOp(serializer);
        aquiferdims.serializeOp(serializer);
        udq_params.serializeOp(serializer);
        hystpar.serializeOp(serializer);
        m_actdims.serializeOp(serializer);
        m_sfuncctrl.serializeOp(serializer);
        m_nupcol.serializeOp(serializer);
        serializer(m_co2storage);
        serializer(m_micp);
    }

private:
    std::time_t m_start_time;
    Phases active_phases;
    Tabdims m_tabdims;
    Regdims m_regdims;
    EndpointScaling endscale;
    Welldims welldims;
    WellSegmentDims wsegdims;
    NetworkDims netwrkdims;
    AquiferDimensions aquiferdims;
    UDQParams udq_params;
    EclHysterConfig hystpar;
    Actdims m_actdims;
    SatFuncControls m_sfuncctrl;
    Nupcol m_nupcol;
    Tracers m_tracers;
    bool m_co2storage;
    bool m_micp;
};


}

#endif // OPM_RUNSPEC_HPP
