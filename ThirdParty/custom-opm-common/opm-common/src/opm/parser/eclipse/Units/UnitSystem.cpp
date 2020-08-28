/*
  Copyright 2013 Statoil ASA.

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


#include <iostream>
#include <stdexcept>

#include <opm/common/utility/String.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <vector>
#include <limits>


namespace Opm {

namespace {
    /*
     * It is VERY important that the measure enum has the same order as the
     * metric and field arrays. C++ does not support designated initializers, so
     * this cannot be done in a declaration-order independent matter.
     */

    // =================================================================
    // METRIC Unit Conventions

    static const double from_metric_offset[] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        Metric::TemperatureOffset,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

    static const double to_metric[] = {
        1,
        1 / Metric::Length,
        1 / Metric::Time,
        1 / Metric::Density,
        1 / Metric::Pressure,
        1 / Metric::AbsoluteTemperature,
        1 / Metric::Temperature,
        1 / Metric::Viscosity,
        1 / Metric::Permeability,
        1 / Metric::LiquidSurfaceVolume,
        1 / Metric::GasSurfaceVolume,
        1 / Metric::ReservoirVolume,
        1 / Metric::GeomVolume,
        1 / ( Metric::LiquidSurfaceVolume / Metric::Time ),
        1 / ( Metric::GasSurfaceVolume / Metric::Time ),
        1 / ( Metric::ReservoirVolume / Metric::Time ),
        1 / ( Metric::GeomVolume / Metric::Time ),
        1 / Metric::Transmissibility,
        1 / (Metric::Permeability * Metric::Length),
        1 / Metric::Mass,
        1 / ( Metric::Mass / Metric::Time ),
        1, /* gas-oil ratio */
        1, /* oil-gas ratio */
        1, /* water cut */
        1, /* gas formation volume factor */
        1, /* oil formation volume factor */
        1, /* water formation volume factor */
        1, /* gas inverse formation volume factor */
        1, /* oil inverse formation volume factor */
        1, /* water inverse formation volume factor */
        1 / (Metric::LiquidSurfaceVolume / Metric::Time / Metric::Pressure),
        1 / (Metric::GasSurfaceVolume / Metric::Time / Metric::Pressure),
        1 / Metric::Energy,
        1 / (Metric::Pressure / Opm::unit::square(Metric::GeomVolume / Metric::Time)),
    };

    static const double from_metric[] = {
        1,
        Metric::Length,
        Metric::Time,
        Metric::Density,
        Metric::Pressure,
        Metric::AbsoluteTemperature,
        Metric::Temperature,
        Metric::Viscosity,
        Metric::Permeability,
        Metric::LiquidSurfaceVolume,
        Metric::GasSurfaceVolume,
        Metric::ReservoirVolume,
        Metric::GeomVolume,
        Metric::LiquidSurfaceVolume / Metric::Time,
        Metric::GasSurfaceVolume / Metric::Time,
        Metric::ReservoirVolume / Metric::Time,
        Metric::GeomVolume / Metric::Time,
        Metric::Transmissibility,
        Metric::Permeability * Metric::Length,
        Metric::Mass,
        Metric::Mass / Metric::Time,
        1, /* gas-oil ratio */
        1, /* oil-gas ratio */
        1, /* water cut */
        1, /* gas formation volume factor */
        1, /* oil formation volume factor */
        1, /* water formation volume factor */
        1, /* gas inverse formation volume factor */
        1, /* oil inverse formation volume factor */
        1, /* water inverse formation volume factor */
        Metric::LiquidSurfaceVolume / Metric::Time / Metric::Pressure,
        Metric::GasSurfaceVolume / Metric::Time / Metric::Pressure,
        Metric::Energy,
        Metric::Pressure / Opm::unit::square(Metric::GeomVolume / Metric::Time),
    };

    static constexpr const char* metric_names[static_cast<int>(UnitSystem::measure::_count)] = {
        "",
        "M",
        "DAYS",
        "KG/M3",
        "BARSA",
        "K",
        "C",
        "CP",
        "MD",
        "SM3",
        "SM3",
        "RM3",
        "SM3",  // Should possibly be RM3
        "SM3/DAY",
        "SM3/DAY",
        "RM3/DAY",
        "SM3/DAY",  // Should possibly be RM3/DAY
        "CPR3/DAY/BARS",
        "MDM",
        "KG",
        "KG/DAY",
        "SM3/SM3",
        "SM3/SM3",
        "SM3/SM3",
        "RM3/SM3", /* gas formation volume factor */
        "RM3/SM3", /* oil formation volume factor */
        "RM3/SM3", /* water formation volume factor */
        "SM3/RM3", /* gas inverse formation volume factor */
        "SM3/RM3", /* oil inverse formation volume factor */
        "SM3/RM3", /* water inverse formation volume factor */
        "SM3/DAY/BARS",
        "SM3/DAY/BARS",
        "KJ", /* energy */
        "BARS/(RM3/DAY)2", /* ICD strength parameter */
    };

    static_assert(
        metric_names[static_cast<int>(UnitSystem::measure::_count) - 1] != nullptr,
        "Name missing from ::metric_names"
    );

    // =================================================================
    // FIELD Unit Conventions

    static const double from_field_offset[] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        Field::TemperatureOffset,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

    static const double to_field[] = {
        1,
        1 / Field::Length,
        1 / Field::Time,
        1 / Field::Density,
        1 / Field::Pressure,
        1 / Field::AbsoluteTemperature,
        1 / Field::Temperature,
        1 / Field::Viscosity,
        1 / Field::Permeability,
        1 / Field::LiquidSurfaceVolume,
        1 / Field::GasSurfaceVolume,
        1 / Field::ReservoirVolume,
        1 / Field::GeomVolume,
        1 / ( Field::LiquidSurfaceVolume / Field::Time ),
        1 / ( Field::GasSurfaceVolume / Field::Time ),
        1 / ( Field::ReservoirVolume / Field::Time ),
        1 / ( Field::GeomVolume / Field::Time ),
        1 / Field::Transmissibility,
        1 / (Field::Permeability * Field::Length),
        1 / Field::Mass,
        1 / ( Field::Mass / Field::Time ),
        1 / ( Field::GasSurfaceVolume / Field::LiquidSurfaceVolume ), /* gas-oil ratio */
        1 / ( Field::LiquidSurfaceVolume / Field::GasSurfaceVolume ), /* oil-gas ratio */
        1, /* water cut */
        1 / (Field::ReservoirVolume / Field::GasSurfaceVolume), /* gas formation volume factor */
        1, /* oil formation volume factor */
        1, /* water formation volume factor */
        1 / (Field::GasSurfaceVolume / Field::ReservoirVolume), /* gas inverse formation volume factor */
        1, /* oil inverse formation volume factor */
        1, /* water inverse formation volume factor */
        1 / (Field::LiquidSurfaceVolume / Field::Time / Field::Pressure),
        1 / (Field::GasSurfaceVolume / Field::Time / Field::Pressure),
        1 / Field::Energy,
        1 / (Field::Pressure / Opm::unit::square(Field::GeomVolume / Field::Time)),
    };

    static const double from_field[] = {
         1,
         Field::Length,
         Field::Time,
         Field::Density,
         Field::Pressure,
         Field::AbsoluteTemperature,
         Field::Temperature,
         Field::Viscosity,
         Field::Permeability,
         Field::LiquidSurfaceVolume,
         Field::GasSurfaceVolume,
         Field::ReservoirVolume,
         Field::GeomVolume,
         Field::LiquidSurfaceVolume / Field::Time,
         Field::GasSurfaceVolume / Field::Time,
         Field::ReservoirVolume / Field::Time,
         Field::GeomVolume / Field::Time,
         Field::Transmissibility,
         Field::Permeability * Field::Length,
         Field::Mass,
         Field::Mass / Field::Time,
         Field::GasSurfaceVolume / Field::LiquidSurfaceVolume, /* gas-oil ratio */
         Field::LiquidSurfaceVolume / Field::GasSurfaceVolume, /* oil-gas ratio */
         1, /* water cut */
         Field::ReservoirVolume / Field::GasSurfaceVolume, /* gas formation volume factor */
         1, /* oil formation volume factor */
         1, /* water formation volume factor */
         Field::GasSurfaceVolume / Field::ReservoirVolume, /* gas inverse formation volume factor */
         1, /* oil inverse formation volume factor */
         1, /* water inverse formation volume factor */
         Field::LiquidSurfaceVolume / Field::Time / Field::Pressure,
         Field::GasSurfaceVolume / Field::Time / Field::Pressure,
         Field::Energy,
         Field::Pressure / Opm::unit::square(Field::GeomVolume / Field::Time),
    };

    static constexpr const char* field_names[static_cast<int>(UnitSystem::measure::_count)] = {
        "",
        "FT",
        "DAYS",
        "LB/FT3",
        "PSIA",
        "R",
        "F",
        "CP",
        "MD",
        "STB",
        "MSCF",
        "RB",
        "FT3",       // Should possibly be RFT3
        "STB/DAY",
        "MSCF/DAY",
        "RB/DAY",
        "FT3/DAY",   // Should possibly be RFT3/DAY
        "CPRB/DAY/PSI",
        "MDFT",
        "LB",
        "LB/DAY",
        "MSCF/STB",
        "STB/MSCF",
        "STB/STB",
        "RB/MSCF", /* gas formation volume factor */
        "RB/STB", /* oil formation volume factor */
        "RB/STB", /* water formation volume factor */
        "MSCF/RB", /* gas inverse formation volume factor */
        "STB/RB", /* oil inverse formation volume factor */
        "STB/RB", /* water inverse formation volume factor */
        "STB/DAY/PSIA",
        "MSCF/DAY/PSIA",
        "BTU", /* energy */
        "PSI/(RFT3/DAY)2", /* ICD strength parameter */
    };

    static_assert(
        field_names[static_cast<int>(UnitSystem::measure::_count) - 1] != nullptr,
        "Name missing from ::field_names"
    );

    // =================================================================
    // LAB Unit Conventions

    static const double from_lab_offset[] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        Lab::TemperatureOffset,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

    static const double to_lab[] = {
        1,
        1 / Lab::Length,
        1 / Lab::Time,
        1 / Lab::Density,
        1 / Lab::Pressure,
        1 / Lab::AbsoluteTemperature,
        1 / Lab::Temperature,
        1 / Lab::Viscosity,
        1 / Lab::Permeability,
        1 / Lab::LiquidSurfaceVolume,
        1 / Lab::GasSurfaceVolume,
        1 / Lab::ReservoirVolume,
        1 / Lab::GeomVolume,
        1 / ( Lab::LiquidSurfaceVolume / Lab::Time ),
        1 / ( Lab::GasSurfaceVolume / Lab::Time ),
        1 / ( Lab::ReservoirVolume / Lab::Time ),
        1 / ( Lab::GeomVolume / Lab::Time ),
        1 / Lab::Transmissibility,
        1 / (Lab::Permeability * Lab::Length),
        1 / Lab::Mass,
        1 / ( Lab::Mass / Lab::Time ),
        1 / Lab::GasDissolutionFactor, /* gas-oil ratio */
        1 / Lab::OilDissolutionFactor, /* oil-gas ratio */
        1, /* water cut */
        1, /* gas formation volume factor */
        1, /* oil formation volume factor */
        1, /* water formation volume factor */
        1, /* gas inverse formation volume factor */
        1, /* oil inverse formation volume factor */
        1, /* water inverse formation volume factor */
        1 / (Lab::LiquidSurfaceVolume / Lab::Time / Lab::Pressure),
        1 / (Lab::GasSurfaceVolume / Lab::Time / Lab::Pressure),
        1 / Lab::Energy,
        1 / (Lab::Pressure / Opm::unit::square(Lab::GeomVolume / Lab::Time)),
    };

    static const double from_lab[] = {
        1,
        Lab::Length,
        Lab::Time,
        Lab::Density,
        Lab::Pressure,
        Lab::AbsoluteTemperature,
        Lab::Temperature,
        Lab::Viscosity,
        Lab::Permeability,
        Lab::LiquidSurfaceVolume,
        Lab::GasSurfaceVolume,
        Lab::ReservoirVolume,
        Lab::GeomVolume,
        Lab::LiquidSurfaceVolume / Lab::Time,
        Lab::GasSurfaceVolume / Lab::Time,
        Lab::ReservoirVolume / Lab::Time,
        Lab::GeomVolume / Lab::Time,
        Lab::Transmissibility,
        Lab::Permeability * Lab::Length,
        Lab::Mass,
        Lab::Mass / Lab::Time,
        Lab::GasDissolutionFactor,  /* gas-oil ratio */
        Lab::OilDissolutionFactor,  /* oil-gas ratio */
        1, /* water cut */
        1, /* gas formation volume factor */
        1, /* oil formation volume factor */
        1, /* water formation volume factor */
        1, /* gas inverse formation volume factor */
        1, /* oil inverse formation volume factor */
        1, /* water inverse formation volume factor */
        Lab::LiquidSurfaceVolume / Lab::Time / Lab::Pressure,
        Lab::GasSurfaceVolume / Lab::Time / Lab::Pressure,
        Lab::Energy,
        Lab::Pressure / Opm::unit::square(Lab::GeomVolume / Lab::Time),
    };

    static constexpr const char* lab_names[static_cast<int>(UnitSystem::measure::_count)] = {
        "",
        "CM",
        "HRS",
        "G/CC",
        "ATM",
        "K",
        "C",
        "CP",
        "MD",
        "SCC",
        "SCC",
        "RCC",
        "SCC",      // Should possibly be RCC
        "SCC/HR",
        "SCC/HR",
        "RCC/HR",
        "SCC/HR",   // Should possibly be RCC/HR
        "CPRCC/HR/ATM",
        "MDCC",
        "G",
        "G/HR",
        "SCC/SCC",
        "SCC/SCC",
        "SCC/SCC",
        "RCC/SCC", /* gas formation volume factor */
        "RCC/SCC", /* oil formation volume factor */
        "RCC/SCC", /* water formation volume factor */
        "SCC/RCC", /* gas formation volume factor */
        "SCC/RCC", /* oil inverse formation volume factor */
        "SCC/RCC", /* water inverse formation volume factor */
        "SCC/HR/ATM",
        "SCC/HR/ATM",
        "J", /* energy */
        "ATM/(RCC/H)2", /* ICD strength parameter */
    };

    static_assert(
        lab_names[static_cast<int>(UnitSystem::measure::_count) - 1] != nullptr,
        "Name missing from ::lab_names"
    );

    // =================================================================
    // PVT-M Unit Conventions

    static const double from_pvt_m_offset[] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        PVT_M::TemperatureOffset,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

    static const double to_pvt_m[] = {
        1,
        1 / PVT_M::Length,
        1 / PVT_M::Time,
        1 / PVT_M::Density,
        1 / PVT_M::Pressure,
        1 / PVT_M::AbsoluteTemperature,
        1 / PVT_M::Temperature,
        1 / PVT_M::Viscosity,
        1 / PVT_M::Permeability,
        1 / PVT_M::LiquidSurfaceVolume,
        1 / PVT_M::GasSurfaceVolume,
        1 / PVT_M::ReservoirVolume,
        1 / PVT_M::GeomVolume,
        1 / ( PVT_M::LiquidSurfaceVolume / PVT_M::Time ),
        1 / ( PVT_M::GasSurfaceVolume / PVT_M::Time ),
        1 / ( PVT_M::ReservoirVolume / PVT_M::Time ),
        1 / ( PVT_M::GeomVolume / PVT_M::Time ),
        1 / PVT_M::Transmissibility,
        1 / (PVT_M::Permeability * PVT_M::Length),
        1 / PVT_M::Mass,
        1 / ( PVT_M::Mass / PVT_M::Time ),
        1 / (PVT_M::GasSurfaceVolume / PVT_M::LiquidSurfaceVolume), // Rs
        1 / (PVT_M::LiquidSurfaceVolume / PVT_M::GasSurfaceVolume), // Rv
        1, /* water cut */
        1 / (PVT_M::ReservoirVolume / PVT_M::GasSurfaceVolume), /* Bg */
        1 / (PVT_M::ReservoirVolume / PVT_M::LiquidSurfaceVolume), /* Bo */
        1 / (PVT_M::ReservoirVolume / PVT_M::LiquidSurfaceVolume), /* Bw */
        1 / (PVT_M::GasSurfaceVolume / PVT_M::ReservoirVolume), /* 1/Bg */
        1 / (PVT_M::LiquidSurfaceVolume / PVT_M::ReservoirVolume), /* 1/Bo */
        1 / (PVT_M::LiquidSurfaceVolume / PVT_M::ReservoirVolume), /* 1/Bw */
        1 / (PVT_M::LiquidSurfaceVolume / PVT_M::Time / PVT_M::Pressure),
        1 / (PVT_M::GasSurfaceVolume / PVT_M::Time / PVT_M::Pressure),
        1 / PVT_M::Energy,
        1 / (PVT_M::Pressure / Opm::unit::square(PVT_M::GeomVolume / PVT_M::Time)),
    };

    static const double from_pvt_m[] = {
        1,
        PVT_M::Length,
        PVT_M::Time,
        PVT_M::Density,
        PVT_M::Pressure,
        PVT_M::AbsoluteTemperature,
        PVT_M::Temperature,
        PVT_M::Viscosity,
        PVT_M::Permeability,
        PVT_M::LiquidSurfaceVolume,
        PVT_M::GasSurfaceVolume,
        PVT_M::ReservoirVolume,
        PVT_M::GeomVolume,
        PVT_M::LiquidSurfaceVolume / PVT_M::Time,
        PVT_M::GasSurfaceVolume / PVT_M::Time,
        PVT_M::ReservoirVolume / PVT_M::Time,
        PVT_M::GeomVolume / PVT_M::Time,
        PVT_M::Transmissibility,
        PVT_M::Permeability * PVT_M::Length,
        PVT_M::Mass,
        PVT_M::Mass / PVT_M::Time,
        PVT_M::GasSurfaceVolume / PVT_M::LiquidSurfaceVolume, // Rs
        PVT_M::LiquidSurfaceVolume / PVT_M::GasSurfaceVolume, // Rv
        1, /* water cut */
        PVT_M::ReservoirVolume / PVT_M::GasSurfaceVolume, /* Bg */
        PVT_M::ReservoirVolume / PVT_M::LiquidSurfaceVolume, /* Bo */
        PVT_M::ReservoirVolume / PVT_M::LiquidSurfaceVolume, /* Bw */
        PVT_M::GasSurfaceVolume / PVT_M::ReservoirVolume, /* 1/Bg */
        PVT_M::LiquidSurfaceVolume / PVT_M::ReservoirVolume, /* 1/Bo */
        PVT_M::LiquidSurfaceVolume / PVT_M::ReservoirVolume, /* 1/Bw */
        PVT_M::LiquidSurfaceVolume / PVT_M::Time / PVT_M::Pressure,
        PVT_M::GasSurfaceVolume / PVT_M::Time / PVT_M::Pressure,
        PVT_M::Energy,
        PVT_M::Pressure / Opm::unit::square(PVT_M::GeomVolume / PVT_M::Time),
    };

    static constexpr const char* pvt_m_names[static_cast<int>(UnitSystem::measure::_count)] = {
        "",
        "M",
        "DAYS",
        "KG/M3",
        "ATM",
        "K",
        "C",
        "CP",
        "MD",
        "SM3",
        "SM3",
        "RM3",
        "SM3",         // Should possibly be RM3
        "SM3/DAY",
        "SM3/DAY",
        "RM3/DAY",
        "SM3/DAY",     // Should possibly be SM3/DAY
        "CPR3/DAY/ATM",
        "MDM",
        "KG",
        "KG/DAY",
        "SM3/SM3",
        "SM3/SM3",
        "SM3/SM3",
        "RM3/SM3", /* gas formation volume factor */
        "RM3/SM3", /* oil formation volume factor */
        "RM3/SM3", /* water formation volume factor */
        "SM3/RM3", /* gas inverse formation volume factor */
        "SM3/RM3", /* oil inverse formation volume factor */
        "SM3/RM3", /* water inverse formation volume factor */
        "SM3/DAY/ATM",
        "SM3/DAY/ATM",
        "KJ" /* energy */,
        "ATM/(RM3/DAY)2" /* ICD strength parameter */,
    };

    static_assert(
        pvt_m_names[static_cast<int>(UnitSystem::measure::_count) - 1] != nullptr,
        "Name missing from ::pvt_m_names"
    );

    // =================================================================
    // INPUT Unit Conventions

    static const double from_input_offset[] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

    static const double to_input[] = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
    };

    static const double from_input[] = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
    };

    static constexpr const char* input_names[static_cast<int>(UnitSystem::measure::_count)] = {
        "",
        "M",
        "DAY",
        "KG/M3",
        "BARSA",
        "K",
        "C",
        "CP",
        "MD",
        "SM3",
        "SM3",
        "RM3",
        "SM3",
        "SM3/DAY",
        "SM3/DAY",
        "RM3/DAY",
        "SM3/DAY",
        "CPR3/DAY/BARS",
        "MDM",
        "KG",
        "KG/DAY",
        "SM3/SM3",
        "SM3/SM3",
        "SM3/SM3",
        "RM3/SM3", /* gas formation volume factor */
        "RM3/SM3", /* oil formation volume factor */
        "RM3/SM3", /* water formation volume factor */
        "SM3/RM3", /* gas inverse formation volume factor */
        "SM3/RM3", /* oil inverse formation volume factor */
        "SM3/RM3", /* water inverse formation volume factor */
        "SM3/DAY/BARS",
        "SM3/DAY/BARS",
        "KJ", /* energy */
        "BARS/(RM3/DAY)2", /* ICD strength parameter */
    };

    static_assert(
        input_names[static_cast<int>(UnitSystem::measure::_count) - 1] != nullptr,
        "Name missing from ::input_names"
    );

} // namespace Anonymous

    UnitSystem::UnitSystem(const UnitType unit) :
        m_unittype( unit )
    {
        init();
    }

    UnitSystem UnitSystem::serializeObject()
    {
        return UnitSystem(UnitType::UNIT_TYPE_METRIC);
    }

    void UnitSystem::initINPUT() {
        this->m_name = "Input";
        this->measure_table_from_si = to_input;
        this->measure_table_to_si = from_input;
        this->measure_table_to_si_offset = from_input_offset;
        this->unit_name_table = input_names;

        this->addDimension("1"         , 1.0);
        this->addDimension("Unit"         , 1.0);
        this->addDimension("Pressure"  , 1.0);
        this->addDimension("Temperature", 1.0);
        this->addDimension("AbsoluteTemperature", 1.0, 0.0);
        this->addDimension("Length"    , 1.0);
        this->addDimension("Time"      , 1.0);
        this->addDimension("Mass"         , 1.0);
        this->addDimension("Permeability", 1.0);
        this->addDimension("Transmissibility", 1.0);
        this->addDimension("GasDissolutionFactor", 1.0);
        this->addDimension("OilDissolutionFactor", 1.0);
        this->addDimension("LiquidSurfaceVolume", 1.0);
        this->addDimension("GasSurfaceVolume" , 1.0);
        this->addDimension("ReservoirVolume", 1.0);
        this->addDimension("GeometricVolume", 1.0 );
        this->addDimension("Density"   , 1.0);
        this->addDimension("PolymerDensity", 1.0);
        this->addDimension("FoamDensity", 1.0);
        this->addDimension("FoamSurfactantConcentration", 1.0);
        this->addDimension("Salinity", 1.0);
        this->addDimension("Viscosity" , 1.0);
        this->addDimension("Timestep"  , 1.0);
        this->addDimension("SurfaceTension"  , 1.0);
        this->addDimension("Energy", 1.0);
        this->addDimension("ContextDependent", 1.0);
    }


    void UnitSystem::initPVT_M() {
        this->m_name = "PVT-M";
        this->measure_table_from_si = to_pvt_m;
        this->measure_table_to_si = from_pvt_m;
        this->measure_table_to_si_offset = from_pvt_m_offset;
        this->unit_name_table = pvt_m_names;

        this->addDimension("1"         , 1.0);
        this->addDimension("Unit"         , 1.0);
        this->addDimension("Pressure"  , PVT_M::Pressure );
        this->addDimension("Temperature", PVT_M::Temperature, PVT_M::TemperatureOffset);
        this->addDimension("AbsoluteTemperature", PVT_M::AbsoluteTemperature);
        this->addDimension("Length"    , PVT_M::Length);
        this->addDimension("Time"      , PVT_M::Time );
        this->addDimension("Mass"         , PVT_M::Mass );
        this->addDimension("Permeability", PVT_M::Permeability );
        this->addDimension("Transmissibility", PVT_M::Transmissibility );
        this->addDimension("GasDissolutionFactor", PVT_M::GasDissolutionFactor);
        this->addDimension("OilDissolutionFactor", PVT_M::OilDissolutionFactor);
        this->addDimension("LiquidSurfaceVolume", PVT_M::LiquidSurfaceVolume );
        this->addDimension("GasSurfaceVolume" , PVT_M::GasSurfaceVolume );
        this->addDimension("ReservoirVolume", PVT_M::ReservoirVolume );
        this->addDimension("GeometricVolume", PVT_M::GeomVolume );
        this->addDimension("Density"   , PVT_M::Density );
        this->addDimension("PolymerDensity", PVT_M::PolymerDensity);
        this->addDimension("FoamDensity", PVT_M::FoamDensity);
        this->addDimension("FoamSurfactantConcentration", PVT_M::FoamSurfactantConcentration);
        this->addDimension("Salinity", PVT_M::Salinity);
        this->addDimension("Viscosity" , PVT_M::Viscosity);
        this->addDimension("Timestep"  , PVT_M::Timestep);
        this->addDimension("SurfaceTension"  , PVT_M::SurfaceTension);
        this->addDimension("Energy", PVT_M::Energy);
        this->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
    }


    void UnitSystem::initLAB() {
        this->m_name = "Lab";
        this->measure_table_from_si = to_lab;
        this->measure_table_to_si = from_lab;
        this->measure_table_to_si_offset = from_lab_offset;
        this->unit_name_table = lab_names;

        this->addDimension("1"    , 1.0);
        this->addDimension("Unit"         , 1.0);
        this->addDimension("Pressure", Lab::Pressure );
        this->addDimension("Temperature", Lab::Temperature, Lab::TemperatureOffset);
        this->addDimension("AbsoluteTemperature", Lab::AbsoluteTemperature);
        this->addDimension("Length", Lab::Length);
        this->addDimension("Time" , Lab::Time);
        this->addDimension("Mass", Lab::Mass);
        this->addDimension("Permeability", Lab::Permeability );
        this->addDimension("Transmissibility", Lab::Transmissibility );
        this->addDimension("GasDissolutionFactor" , Lab::GasDissolutionFactor);
        this->addDimension("OilDissolutionFactor", Lab::OilDissolutionFactor);
        this->addDimension("LiquidSurfaceVolume", Lab::LiquidSurfaceVolume );
        this->addDimension("GasSurfaceVolume", Lab::GasSurfaceVolume );
        this->addDimension("ReservoirVolume", Lab::ReservoirVolume );
        this->addDimension("GeometricVolume", Lab::GeomVolume );
        this->addDimension("Density", Lab::Density );
        this->addDimension("PolymerDensity", Lab::PolymerDensity);
        this->addDimension("FoamDensity", Lab::FoamDensity);
        this->addDimension("FoamSurfactantConcentration", Lab::FoamSurfactantConcentration);
        this->addDimension("Salinity", Lab::Salinity);
        this->addDimension("Viscosity", Lab::Viscosity);
        this->addDimension("Timestep", Lab::Timestep);
        this->addDimension("SurfaceTension"  , Lab::SurfaceTension);
        this->addDimension("Energy", Lab::Energy);
        this->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
    }



    void UnitSystem::initMETRIC() {
        this->m_name = "Metric";
        this->measure_table_from_si = to_metric;
        this->measure_table_to_si = from_metric;
        this->measure_table_to_si_offset = from_metric_offset;
        this->unit_name_table = metric_names;

        this->addDimension("1"         , 1.0);
        this->addDimension("Unit"         , 1.0);
        this->addDimension("Pressure"  , Metric::Pressure );
        this->addDimension("Temperature", Metric::Temperature, Metric::TemperatureOffset);
        this->addDimension("AbsoluteTemperature", Metric::AbsoluteTemperature);
        this->addDimension("Length"    , Metric::Length);
        this->addDimension("Time"      , Metric::Time );
        this->addDimension("Mass"         , Metric::Mass );
        this->addDimension("Permeability", Metric::Permeability );
        this->addDimension("Transmissibility", Metric::Transmissibility );
        this->addDimension("GasDissolutionFactor", Metric::GasDissolutionFactor);
        this->addDimension("OilDissolutionFactor", Metric::OilDissolutionFactor);
        this->addDimension("LiquidSurfaceVolume", Metric::LiquidSurfaceVolume );
        this->addDimension("GasSurfaceVolume" , Metric::GasSurfaceVolume );
        this->addDimension("ReservoirVolume", Metric::ReservoirVolume );
        this->addDimension("GeometricVolume", Metric::GeomVolume );
        this->addDimension("Density"   , Metric::Density );
        this->addDimension("PolymerDensity", Metric::PolymerDensity);
        this->addDimension("FoamDensity", Metric::FoamDensity);
        this->addDimension("FoamSurfactantConcentration", Metric::FoamSurfactantConcentration);
        this->addDimension("Salinity", Metric::Salinity);
        this->addDimension("Viscosity" , Metric::Viscosity);
        this->addDimension("Timestep"  , Metric::Timestep);
        this->addDimension("SurfaceTension"  , Metric::SurfaceTension);
        this->addDimension("Energy", Metric::Energy);
        this->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
    }

    void UnitSystem::initFIELD() {
        m_name = "Field";
        this->measure_table_from_si = to_field;
        this->measure_table_to_si = from_field;
        this->measure_table_to_si_offset = from_field_offset;
        this->unit_name_table = field_names;

        this->addDimension("1"    , 1.0);
        this->addDimension("Unit"         , 1.0);
        this->addDimension("Pressure", Field::Pressure );
        this->addDimension("Temperature", Field::Temperature, Field::TemperatureOffset);
        this->addDimension("AbsoluteTemperature", Field::AbsoluteTemperature);
        this->addDimension("Length", Field::Length);
        this->addDimension("Time" , Field::Time);
        this->addDimension("Mass", Field::Mass);
        this->addDimension("Permeability", Field::Permeability );
        this->addDimension("Transmissibility", Field::Transmissibility );
        this->addDimension("GasDissolutionFactor" , Field::GasDissolutionFactor);
        this->addDimension("OilDissolutionFactor", Field::OilDissolutionFactor);
        this->addDimension("LiquidSurfaceVolume", Field::LiquidSurfaceVolume );
        this->addDimension("GasSurfaceVolume", Field::GasSurfaceVolume );
        this->addDimension("ReservoirVolume", Field::ReservoirVolume );
        this->addDimension("GeometricVolume", Field::GeomVolume );
        this->addDimension("Density", Field::Density );
        this->addDimension("PolymerDensity", Field::PolymerDensity);
        this->addDimension("FoamDensity", Field::FoamDensity);
        this->addDimension("FoamSurfactantConcentration", Field::FoamSurfactantConcentration);
        this->addDimension("Salinity", Field::Salinity);
        this->addDimension("Viscosity", Field::Viscosity);
        this->addDimension("Timestep", Field::Timestep);
        this->addDimension("SurfaceTension"  , Field::SurfaceTension);
        this->addDimension("Energy", Field::Energy);
        this->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
    }


    namespace {

    int to_ecl_id(UnitSystem::UnitType unit_type) {
        if (unit_type == UnitSystem::UnitType::UNIT_TYPE_METRIC)
            return 1;

        if (unit_type == UnitSystem::UnitType::UNIT_TYPE_FIELD)
            return 2;

        if (unit_type == UnitSystem::UnitType::UNIT_TYPE_LAB)
            return 3;

        if (unit_type == UnitSystem::UnitType::UNIT_TYPE_PVT_M)
            return 4;

        throw std::invalid_argument("The nonstandard unit system does not have a corresponding ecl id");
    }

    UnitSystem::UnitType from_ecl_id(int ecl_id) {
        if (ecl_id == 1)
            return UnitSystem::UnitType::UNIT_TYPE_METRIC;

        if (ecl_id == 2)
            return UnitSystem::UnitType::UNIT_TYPE_FIELD;

        if (ecl_id == 3)
            return UnitSystem::UnitType::UNIT_TYPE_LAB;

        if (ecl_id == 4)
            return UnitSystem::UnitType::UNIT_TYPE_PVT_M;

        throw std::invalid_argument("The integer value: " + std::to_string(ecl_id) + " is not recogmized as a valid Eclipse unit ID");
    }

    UnitSystem::UnitType fromDeckName(const std::string &deck_name) {
      if (deck_name == "FIELD")
        return UnitSystem::UnitType::UNIT_TYPE_FIELD;

      if (deck_name == "METRIC")
        return UnitSystem::UnitType::UNIT_TYPE_METRIC;

      if (deck_name == "LAB")
        return UnitSystem::UnitType::UNIT_TYPE_LAB;

      if (deck_name == "PVT-M")
        return UnitSystem::UnitType::UNIT_TYPE_PVT_M;

      throw std::invalid_argument("Unit string: " + deck_name +
                                  " not recognized ");
    }
    }

    bool UnitSystem::valid_name(const std::string &deck_name) {
      if (deck_name == "FIELD")
        return true;

      if (deck_name == "METRIC")
        return true;

      if (deck_name == "LAB")
        return true;

      if (deck_name == "PVT-M")
        return true;

      return false;
    }

    std::string UnitSystem::deck_name() const {
        switch( this->m_unittype) {
        case(UnitType::UNIT_TYPE_METRIC): return "METRIC";
        case(UnitType::UNIT_TYPE_FIELD):  return "FIELD";
        case(UnitType::UNIT_TYPE_LAB):    return "LAB";
        case(UnitType::UNIT_TYPE_PVT_M):  return "PVT-M";
        default:
            throw std::invalid_argument("No valid deckname could be inferred");
        }
    }


    UnitSystem::UnitSystem(const std::string& deck_name) :
        UnitSystem( fromDeckName(deck_name) )
    {
    }


    UnitSystem::UnitSystem(int ecl_id) :
        UnitSystem(from_ecl_id(ecl_id))
    {}


    int UnitSystem::ecl_id() const {
        return to_ecl_id( this->m_unittype );
    }


    bool UnitSystem::hasDimension(const std::string& dimension) const {
        return (m_dimensions.find( dimension ) != m_dimensions.end());
    }


    const Dimension& UnitSystem::getNewDimension(const std::string& dimension) {
        if( !hasDimension( dimension ) )
            this->addDimension( dimension, this->parse( dimension ) );

        return getDimension( dimension );
    }


    const Dimension& UnitSystem::getDimension(const std::string& dimension) const {
        auto iter = this->m_dimensions.find(dimension);
        if (iter == this->m_dimensions.end())
            throw std::out_of_range("The dimension: '" + dimension + "' was not recognized");
        this->m_use_count++;
        return iter->second;
    }

    Dimension UnitSystem::getDimension(measure m) const {
        double si_factor = this->measure_table_to_si[ static_cast< int >( m ) ];
        double si_offset = this->measure_table_to_si_offset[ static_cast<int>( m ) ];
        return Dimension(si_factor, si_offset);
    }


    std::size_t UnitSystem::use_count() const {
        return this->m_use_count;
    }

    void UnitSystem::addDimension(const std::string& dimension , const Dimension& dim) {
        this->m_dimensions[ dimension ] = std::move(dim);
    }

    void UnitSystem::addDimension(const std::string& dimension , double SIfactor, double SIoffset) {
        this->addDimension(dimension, Dimension(SIfactor, SIoffset));
    }

    const std::string& UnitSystem::getName() const {
        return m_name;
    }

    UnitSystem::UnitType UnitSystem::getType() const {
        return m_unittype;
    }


    Dimension UnitSystem::parseFactor(const std::string& dimension) const {
        std::vector<std::string> dimensionList = split_string(dimension, '*');

        double SIfactor = 1.0;
        for( const auto& x : dimensionList ) {
            auto dim = this->getDimension( x );

            // all constituing dimension must be compositable. The
            // only exception is if there is the "composite" dimension
            // consists of exactly a single atomic dimension...
            if (dimensionList.size() > 1 && !dim.isCompositable())
                throw std::invalid_argument("Composite dimensions currently cannot require a conversion offset");

            SIfactor *= dim.getSIScaling();
        }
        return Dimension( SIfactor );
    }

    Dimension UnitSystem::parse(const std::string& dimension) const {
        const size_t divCount = std::count( dimension.begin() , dimension.end() , '/' );

        if( divCount > 1 )
                throw std::invalid_argument("Dimension string can only have one division sign '/'");

        const bool haveDivisor = divCount == 1;
        if( !haveDivisor ) return this->parseFactor( dimension );

        std::vector<std::string> parts = split_string(dimension, '/');
        Dimension dividend = this->parseFactor( parts[0] );
        Dimension divisor = this->parseFactor( parts[1] );

        if (dividend.getSIOffset() != 0.0 || divisor.getSIOffset() != 0.0)
            throw std::invalid_argument("Composite dimensions cannot currently require a conversion offset");

        return Dimension( dividend.getSIScaling() / divisor.getSIScaling() );
    }


    bool UnitSystem::equal(const UnitSystem& other) const {
        return *this == other;
    }

    bool UnitSystem::operator==( const UnitSystem& rhs ) const {
        return this->m_name == rhs.m_name
            && this->m_unittype == rhs.m_unittype
            && this->m_dimensions.size() == rhs.m_dimensions.size()
            && std::equal( this->m_dimensions.begin(),
                           this->m_dimensions.end(),
                           rhs.m_dimensions.begin() )
            && this->measure_table_to_si_offset == rhs.measure_table_to_si_offset
            && this->measure_table_from_si == rhs.measure_table_from_si
            && this->measure_table_to_si == rhs.measure_table_to_si
            && this->unit_name_table == rhs.unit_name_table;
    }

    bool UnitSystem::operator!=( const UnitSystem& rhs ) const {
        return !( *this == rhs );
    }

    double UnitSystem::from_si( measure m, double val ) const {
        return
            this->measure_table_from_si[ static_cast< int >( m ) ]
            * (val - this->measure_table_to_si_offset[ static_cast< int >( m ) ]);
    }

    double UnitSystem::to_si( measure m, double val ) const {
        return
            this->measure_table_to_si[ static_cast< int >( m ) ]*val
            + this->measure_table_to_si_offset[ static_cast< int >( m ) ];
    }

    void UnitSystem::from_si( measure m, std::vector<double>& data ) const {
        double factor = this->measure_table_from_si[ static_cast< int >( m ) ];
        double offset = this->measure_table_to_si_offset[ static_cast< int >( m ) ];
        auto scale = [=](double x) { return (x - offset) * factor; };
        std::transform( data.begin() , data.end() , data.begin() , scale);
    }


    void UnitSystem::to_si( measure m, std::vector<double>& data) const {
        double factor = this->measure_table_to_si[ static_cast< int >( m ) ];
        double offset = this->measure_table_to_si_offset[ static_cast< int >( m ) ];
        auto scale = [=](double x) { return x * factor + offset; };
        std::transform( data.begin() , data.end() , data.begin() , scale);
    }

    const char* UnitSystem::name( measure m ) const {
        return this->unit_name_table[ static_cast< int >( m ) ];
    }


    UnitSystem UnitSystem::newMETRIC() {
        UnitSystem system( UnitType::UNIT_TYPE_METRIC );
        return system;
    }

    UnitSystem UnitSystem::newFIELD() {
        UnitSystem system( UnitType::UNIT_TYPE_FIELD );
        return system;
    }


    UnitSystem UnitSystem::newLAB() {
        UnitSystem system( UnitType::UNIT_TYPE_LAB );
        return system;
    }

    UnitSystem UnitSystem::newPVT_M() {
        UnitSystem system( UnitType::UNIT_TYPE_PVT_M );
        return system;
    }

    UnitSystem UnitSystem::newINPUT() {
        UnitSystem system( UnitType::UNIT_TYPE_INPUT );
        return system;
     }

    void UnitSystem::init()
    {
        switch(m_unittype) {
            case(UnitType::UNIT_TYPE_METRIC):
                this->initMETRIC();
                break;

            case(UnitType::UNIT_TYPE_FIELD):
                this->initFIELD();
                break;

            case(UnitType::UNIT_TYPE_LAB):
                this->initLAB();
                break;

            case(UnitType::UNIT_TYPE_PVT_M):
                this->initPVT_M();
                break;

            case(UnitType::UNIT_TYPE_INPUT):
                this->initINPUT();
                break;

            default:
                throw std::runtime_error("Tried to construct UnitSystem with unknown unit family.");
                break;
        };
    }

}
