/*
  Copyright 2017 Statoil ASA.

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

#include <opm/utility/ECLPropertyUnitConversion.hpp>

#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <opm/parser/eclipse/Units/Units.hpp>

#include <ert/ecl/ecl_kw_magic.h>

#include <exception>
#include <stdexcept>

namespace detail {
    template <class ResultSet>
    std::unique_ptr<const Opm::ECLUnits::UnitSystem>
    serialisedUnitConventions(const ResultSet& rset)
    {
        // Use INTEHEAD from Main grid.  Reasonably safe.
        const auto& ih = rset.template keywordData<int>(INTEHEAD_KW);

        return ::Opm::ECLUnits::createUnitSystem(ih[ INTEHEAD_UNIT_INDEX ]);
    }

    struct SIUnits : public ::Opm::ECLUnits::UnitSystem
    {
    public:
        virtual std::unique_ptr<Opm::ECLUnits::UnitSystem>
        clone() const override
        {
            return std::unique_ptr<Opm::ECLUnits::UnitSystem> {
                new SIUnits(*this)
            };
        }

        virtual double density() const override
        {
            return 1.0;
        }

        virtual double depth() const override
        {
            return 1.0;
        }

        virtual double pressure() const override
        {
            return 1.0;
        }

        virtual double reservoirRate() const override
        {
            return 1.0;
        }

        virtual double reservoirVolume() const override
        {
            return 1.0;
        }

        virtual double surfaceVolumeGas() const override
        {
            return 1.0;
        }

        virtual double surfaceVolumeLiquid() const override
        {
            return 1.0;
        }

        virtual double time() const override
        {
            return 1.0;
        }

        virtual double transmissibility() const override
        {
            return 1.0;
        }

        virtual double viscosity() const override
        {
            return 1.0;
        }
    };
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::serialisedUnitConventions(const ECLRestartData& rstrt)
{
    return detail::serialisedUnitConventions(rstrt);
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::serialisedUnitConventions(const ECLInitFileData& init)
{
    return detail::serialisedUnitConventions(init);
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::internalUnitConventions()
{
    using UPtr = std::unique_ptr<const UnitSystem>;

    return UPtr{ new detail::SIUnits{} };
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::metricUnitConventions()
{
    return ::Opm::ECLUnits::createUnitSystem(1);
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::fieldUnitConventions()
{
    return ::Opm::ECLUnits::createUnitSystem(2);
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::labUnitConventions()
{
    return ::Opm::ECLUnits::createUnitSystem(3);
}

std::unique_ptr<const Opm::ECLUnits::UnitSystem>
Opm::ECLUnits::pvtmUnitConventions()
{
    return ::Opm::ECLUnits::createUnitSystem(4);
}

// =====================================================================
// Class Convert::PhysicalQuantity::Impl
// =====================================================================

namespace {
    std::unique_ptr<Opm::ECLUnits::UnitSystem>
    conditionalClone(const Opm::ECLUnits::UnitSystem* usys)
    {
        if (usys == nullptr) {
            return std::unique_ptr<Opm::ECLUnits::UnitSystem>{};
        }

        return usys->clone();
    }
}

class Opm::ECLUnits::Convert::PhysicalQuantity::Impl
{
public:
    Impl() {}

    Impl(const Impl& rhs)
        : from_(conditionalClone(rhs.from_.get()))
        , to_  (conditionalClone(rhs.to_  .get()))
    {}

    Impl(Impl&& rhs)
        : from_(std::move(rhs.from_))
        , to_  (std::move(rhs.to_))
    {}

    Impl& operator=(const Impl& rhs)
    {
        this->from_ = conditionalClone(rhs.from_.get());
        this->to_   = conditionalClone(rhs.to_  .get());

        return *this;
    }

    Impl& operator=(Impl&& rhs)
    {
        this->from_ = std::move(rhs.from_);
        this->to_   = std::move(rhs.to_);

        return *this;
    }

    void from(const UnitSystem& usys)
    {
        this->from_ = usys.clone();
    }

    void to(const UnitSystem& usys)
    {
        this->to_ = usys.clone();
    }

    const UnitSystem* from() const
    {
        return this->from_.get();
    }

    const UnitSystem* to() const
    {
        return this->to_.get();
    }

private:
    std::unique_ptr<UnitSystem> from_{ nullptr };
    std::unique_ptr<UnitSystem> to_  { nullptr };
};

// =====================================================================
// Class Convert::PhysicalQuantity
// =====================================================================

Opm::ECLUnits::Convert::PhysicalQuantity::PhysicalQuantity()
    : pImpl_(new Impl{})
{}

Opm::ECLUnits::Convert::PhysicalQuantity::~PhysicalQuantity()
{}

Opm::ECLUnits::Convert::PhysicalQuantity::
PhysicalQuantity(const PhysicalQuantity& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLUnits::Convert::PhysicalQuantity::
PhysicalQuantity(PhysicalQuantity&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLUnits::Convert::PhysicalQuantity&
Opm::ECLUnits::Convert::PhysicalQuantity::operator=(const PhysicalQuantity& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLUnits::Convert::PhysicalQuantity&
Opm::ECLUnits::Convert::PhysicalQuantity::operator=(PhysicalQuantity&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLUnits::Convert::PhysicalQuantity&
Opm::ECLUnits::Convert::PhysicalQuantity::from(const UnitSystem& usys)
{
    this->pImpl_->from(usys);

    return *this;
}

Opm::ECLUnits::Convert::PhysicalQuantity&
Opm::ECLUnits::Convert::PhysicalQuantity::to(const UnitSystem& usys)
{
    this->pImpl_->to(usys);

    return *this;
}

const Opm::ECLUnits::UnitSystem*
Opm::ECLUnits::Convert::PhysicalQuantity::from() const
{
    return this->pImpl_->from();
}

const Opm::ECLUnits::UnitSystem*
Opm::ECLUnits::Convert::PhysicalQuantity::to() const
{
    return this->pImpl_->to();
}

// =====================================================================

namespace {
    void rescaleVector(const double factor, std::vector<double>& x)
    {
        for (auto& xi : x) {
            xi *= factor;
        }
    }

    void validateUnitConventions(const std::string&               name,
                                 const Opm::ECLUnits::UnitSystem* from,
                                 const Opm::ECLUnits::UnitSystem* to)
    {
        if (from == nullptr) {
            throw std::invalid_argument {
                "Cannot Perform " + name + " Value Unit Conversion "
                "Without Known 'From' Unit System Convention"
            };
        }

        if (to == nullptr) {
            throw std::invalid_argument {
                "Cannot Perform " + name + " Value Unit Conversion "
                "Without Known 'To' Unit System Convention"
            };
        }
    }

    double calculateScaleFactor(const double from, const double to)
    {
        using namespace ::Opm::unit;

        // "return from / to", essentially.
        return convert::to(convert::from(1.0, from), to);
    }
} // namespace Anonymous

// =====================================================================
// class Convert::Pressure
// =====================================================================

void
Opm::ECLUnits::Convert::Pressure::appliedTo(std::vector<double>& press) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("Pressure", from, to);

    const auto scaling =
        calculateScaleFactor(from->pressure(), to->pressure());

    rescaleVector(scaling, press);
}

// =====================================================================
// class Convert::Viscosity
// =====================================================================

void
Opm::ECLUnits::Convert::Viscosity::appliedTo(std::vector<double>& mu) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("Viscosity", from, to);

    const auto scaling =
        calculateScaleFactor(from->viscosity(), to->viscosity());

    rescaleVector(scaling, mu);
}

// =====================================================================
// class Convert::GasFVF
// =====================================================================

void
Opm::ECLUnits::Convert::GasFVF::appliedTo(std::vector<double>& Bg) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("GasFVF", from, to);

    const auto scaling =
        calculateScaleFactor(from->reservoirVolume() / from->surfaceVolumeGas(),
                             to  ->reservoirVolume() / to  ->surfaceVolumeGas());

    rescaleVector(scaling, Bg);
}

// =====================================================================
// class Convert::OilFVF
// =====================================================================

void
Opm::ECLUnits::Convert::OilFVF::appliedTo(std::vector<double>& Bo) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("OilFVF", from, to);

    const auto scaling =
        calculateScaleFactor(from->reservoirVolume() / from->surfaceVolumeLiquid(),
                             to  ->reservoirVolume() / to  ->surfaceVolumeLiquid());

    rescaleVector(scaling, Bo);
}

// =====================================================================
// class Convert::DissolvedGasOilRatio
// =====================================================================

void
Opm::ECLUnits::Convert::DissolvedGasOilRatio::
appliedTo(std::vector<double>& Rs) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("DissolvedGasOilRatio", from, to);

    const auto scaling =
        calculateScaleFactor(from->dissolvedGasOilRat(),
                             to  ->dissolvedGasOilRat());

    rescaleVector(scaling, Rs);
}

// =====================================================================
// class Convert::VaporisedOilGasRatio
// =====================================================================

void
Opm::ECLUnits::Convert::VaporisedOilGasRatio::
appliedTo(std::vector<double>& Rv) const
{
    const auto* from = this->from();
    const auto* to   = this->to();

    validateUnitConventions("VaporisedOilGasRatio", from, to);

    const auto scaling =
        calculateScaleFactor(from->vaporisedOilGasRat(),
                             to  ->vaporisedOilGasRat());

    rescaleVector(scaling, Rv);
}
