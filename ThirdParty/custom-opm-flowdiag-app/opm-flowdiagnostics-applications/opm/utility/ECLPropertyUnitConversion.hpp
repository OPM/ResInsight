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

#ifndef OPM_ECLPROPERTYUNITCONVERSION_HEADER_INCLUDED
#define OPM_ECLPROPERTYUNITCONVERSION_HEADER_INCLUDED

#include <memory>
#include <vector>

namespace Opm {

    class ECLRestartData;
    class ECLInitFileData;

    namespace ECLUnits {

        struct UnitSystem;

        /// Construct a Unit System object corresponding to the main grid's
        /// unit convention of a Restart result set.
        ///
        /// \param[in] rstrt ECL Restart data set from which to extract unit
        ///    system convention (e.g., METRIC or FIELD).
        ///
        /// \return Unit System object corresponding to the main grid's
        ///    system convention of the currently selected restart step.
        std::unique_ptr<const UnitSystem>
        serialisedUnitConventions(const ECLRestartData& rstrt);

        /// Construct a Unit System object corresponding to the main grid's
        /// unit convention of an INIT file result set.
        ///
        /// \param[in] init ECL INIT file data set from which to extract
        ///    unit system convention (e.g., METRIC or FIELD).
        ///
        /// \return Unit System object corresponding to the main grid's
        ///    system convention of the INIT file result set.
        std::unique_ptr<const UnitSystem>
        serialisedUnitConventions(const ECLInitFileData& init);

        /// Construct a Unit System object corresponding to the module's
        /// internal unit system convention (strict SI).
        ///
        /// \return Unit System object corresponding to the module's
        ///    internal unit system convention (i.e., strict SI).
        std::unique_ptr<const UnitSystem> internalUnitConventions();

        /// Construct a Unit System object corresponding to ECLIPSE's
        /// "METRIC" unit system convention.
        ///
        /// \return Unit System object corresponding to ECLIPSE's
        ///    "METRIC" unit system convention.
        std::unique_ptr<const UnitSystem> metricUnitConventions();

        /// Construct a Unit System object corresponding to ECLIPSE's
        /// "FIELD" unit system convention.
        ///
        /// \return Unit System object corresponding to ECLIPSE's
        ///    "FIELD" unit system convention.
        std::unique_ptr<const UnitSystem> fieldUnitConventions();

        /// Construct a Unit System object corresponding to ECLIPSE's
        /// "LAB" unit system convention.
        ///
        /// \return Unit System object corresponding to ECLIPSE's
        ///    "LAB" unit system convention.
        std::unique_ptr<const UnitSystem> labUnitConventions();

        /// Construct a Unit System object corresponding to ECLIPSE's
        /// "PVT-M" unit system convention.
        ///
        /// \return Unit System object corresponding to ECLIPSE's
        ///    "PVT-M" unit system convention.
        std::unique_ptr<const UnitSystem> pvtmUnitConventions();

        /// Facility for converting the units of measure of selected
        /// physcial quanties between user-specified systems of unit
        /// convention.
        struct Convert
        {
            /// Representation of a physical quantity
            ///
            /// Base class.  Resource management and operations common to
            /// all particular/specific physical quantities.
            class PhysicalQuantity
            {
            public:
                /// Default constructor.
                PhysicalQuantity();

                /// Destructor.
                virtual ~PhysicalQuantity();

                /// Copy constructor.
                ///
                /// \param[in] rhs Existing physical quantity.
                PhysicalQuantity(const PhysicalQuantity& rhs);

                /// Move constructor.
                ///
                /// Subsumes the implementation of an existing Physical
                /// Quantity.
                ///
                /// \param[in] rhs Existing physical quantity.  Does not
                ///    have a valid implementation when the constructor
                ///    completes.
                PhysicalQuantity(PhysicalQuantity&& rhs);

                /// Assignment operator
                ///
                /// \param[in] rhs Existing Physical Quantity.
                ///
                /// \return \code *this \endcode.
                PhysicalQuantity& operator=(const PhysicalQuantity& rhs);

                /// Move assignment operator.
                ///
                /// Subsumes the implementation of an existing object.
                ///
                /// \param[in] rhs Existing Physical Quantity.  Does not
                ///    have a valid implementation when the assignment
                ///    completes.
                ///
                /// \return \code *this \endcode.
                PhysicalQuantity& operator=(PhysicalQuantity&& rhs);

                /// Specify collection of units of measure of the inputs.
                ///
                /// \param[in] usys Collection of units of measure of inputs.
                ///
                /// \return \code *this \endcode.
                PhysicalQuantity& from(const UnitSystem& usys);

                /// Specify collection of units of measure of the output.
                ///
                /// \param[in] usys Collection of units of measure of outputs.
                ///
                /// \return \code *this \endcode.
                PhysicalQuantity& to(const UnitSystem& usys);

                /// Convert a physical quantity from its input unit of
                /// measure to its output unit of measure.
                ///
                /// Must be overridden in derived classes.
                ///
                /// \param[in,out] x On input, a sequence of values of a
                ///    physical quantities assumed to be defined in the
                ///    input unit specified through \code from(usys)
                ///    \endcode.  On output, the same seqeuence of values
                ///    converted to the output unit of measure specified
                ///    through \code to(usys) \endcode.
                virtual void appliedTo(std::vector<double>& x) const = 0;

            protected:
                /// Retrieve input unit system.
                ///
                /// Exists for the benefit of derived classes.
                ///
                /// \return Input unit system.  Null if not specified by
                ///    caller.
                const UnitSystem* from() const;

                /// Retrieve output unit system.
                ///
                /// Exists for the benefit of derived classes.
                ///
                /// \return Output unit system.  Null if not specified by
                ///    caller.
                const UnitSystem* to() const;

            private:
                /// Implementation class.
                class Impl;

                /// Pointer to implementation.
                std::unique_ptr<Impl> pImpl_;
            };

            /// Facility for converting pressure values between
            /// user-selected units of measure.
            class Pressure : public PhysicalQuantity {
            public:
                /// Convert a sequence of pressure values from its input
                /// unit of measure to its output unit of measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Convert LGR-1's oil pressure values on restart
                ///    step 123 from serialised format on disk (unified
                ///    restart) to internal units of measure (SI).
                /// \code
                ///   const auto rstrt  = ECLRestartData{ "Case.UNRST" };
                ///   const auto native = serialisedUnitConventions(rstrt);
                ///   const auto si     = internalUnitConvention();
                ///
                ///   rstrt.selectReportStep(123);
                ///   auto press = rstrt.keywordData<double>("PRESSURE", "LGR-1");
                ///   Convert::Pressure().from(*native).to(*si).appliedTo(press);
                /// \endcode
                ///
                /// \param[in,out] press On input, a sequence of pressure
                ///    values assumed to be defined in the input unit
                ///    specified through \code from(usys) \endcode.  On
                ///    output, the same seqeuence of pressure values
                ///    converted to the output unit of measure specified
                ///    through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& press) const override;
            };

            /// Facility for converting viscosity values between
            /// user-selected units of measure.
            class Viscosity : public PhysicalQuantity {
            public:
                /// Convert a sequence of fluid phase viscosity values from
                /// its input unit of measure to its output unit of measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Compute the dynamic gas viscosity value in cell
                ///    27182 on restart step 10 and report it in LAB units.
                /// \code
                ///   const auto rset  = ResultSet("Case.EGRID");
                ///   const auto init  = ECLInitFileData{ rset.initFile() };
                ///   const auto G     = ECLGraph.load(rset.gridFile(), init);
                ///   const auto rstrt = ECLRestartData{ rset.restartFile(10) };
                ///   const auto si    = internalUnitConvention();
                ///   const auto lab   = labUnitConvention();
                ///
                ///   rstrt.selectReportStep(10);
                ///   const auto press = G.linearisedCellData<double>
                ///       (rstrt, "PRESSURE", &UnitSystem::pressure);
                ///
                ///   const auto rv = G.linearisedCellData<double>
                ///       (rstrt, "RV", &UnitSystem::vaporisedOilGasRat);
                ///
                ///   const auto pvtCC = ECLPvtCurveCollection(G, init);
                ///
                ///   auto mu = pvtCC.getDynamicProperty(RawCurve::Viscosity,
                ///       ECLPhaseIndex::Vapour, 27182,
                ///       std::vector<double>{ press[27182] },
                ///       std::vector<double>{ rv.empty() ? 0.0 : rv[27182] });
                ///
                ///   Convert::Viscosity().to(*lab).from(*si).appliedTo(mu);
                /// \endcode
                ///
                /// \param[in,out] mu On input, a sequence of viscosity
                ///    values assumed to be defined in the input unit
                ///    specified through \code from(usys) \endcode.  On
                ///    output, the same seqeuence of viscosity values
                ///    converted to the output unit of measure specified
                ///    through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& mu) const override;
            };

            /// Facility for converting gas phase formation volume factor
            /// values between user-selected units of measure.
            class GasFVF : public PhysicalQuantity {
            public:
                /// Convert a sequence of formation volume factor values for
                /// the gas/vapour phase from its input unit of measure to
                /// its output unit of measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Compute the dynamic gas formation volume factor
                ///    value in cell 57721 on restart step 314 and report it
                ///    in Field units.
                /// \code
                ///   const auto rset  = ResultSet("Case.EGRID");
                ///   const auto init  = ECLInitFileData{ rset.initFile() };
                ///   const auto G     = ECLGraph.load(rset.gridFile(), init);
                ///   const auto rstrt = ECLRestartData{ rset.restartFile(10) };
                ///   const auto si    = internalUnitConvention();
                ///   const auto field = fieldUnitConvention();
                ///
                ///   rstrt.selectReportStep(314);
                ///   const auto press = G.linearisedCellData<double>
                ///       (rstrt, "PRESSURE", &UnitSystem::pressure);
                ///
                ///   const auto rv = G.linearisedCellData<double>
                ///       (rstrt, "RV", &UnitSystem::vaporisedOilGasRat);
                ///
                ///   const auto pvtCC = ECLPvtCurveCollection(G, init);
                ///
                ///   auto Bg = pvtCC.getDynamicProperty(RawCurve::FVF,
                ///       ECLPhaseIndex::Vapour, 57721,
                ///       std::vector<double>{ press[57721] },
                ///       std::vector<double>{ rv.empty() ? 0.0 : rv[57721] });
                ///
                ///   Convert::GasFVF().to(*field).from(*si).appliedTo(Bg);
                /// \endcode
                ///
                /// \param[in,out] Bg On input, a sequence of formation
                ///    volume factor values for the gas phase assumed to be
                ///    defined in the input unit specified through \code
                ///    from(usys) \endcode.  On output, the same seqeuence
                ///    of gas FVF values converted to the output unit of
                ///    measure specified through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& Bg) const override;
            };

            /// Facility for converting oil phase formation volume factor
            /// values between user-selected units of measure.
            class OilFVF : public PhysicalQuantity {
            public:
                /// Convert a sequence of formation volume factor values for
                /// the oil/liquid phase from its input unit of measure to
                /// its output unit of measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Extract the dynamic oil formation volume factor
                ///    curve in cell 355,113 in native units (convention of
                ///    serialised result set).
                /// \code
                ///   const auto rset   = ResultSet("Case.EGRID");
                ///   const auto init   = ECLInitFileData{ rset.initFile() };
                ///   const auto G      = ECLGraph.load(rset.gridFile(), init);
                ///   const auto si     = internalUnitConvention();
                ///   const auto native = serialisedUnitConventions(init);
                ///
                ///   const auto pvtCC = ECLPvtCurveCollection(G, init);
                ///
                ///   auto BoCurves = pvtCC.getPvtCurve(RawCurve::FVF,
                ///       ECLPhaseIndex::Liquid, 355113);
                ///
                ///   const auto cvrtPress =
                ///       Convert::Pressure().to(*native).from(*si);
                ///   const auto cvrtBo =
                ///       Convert::OilFVF().to(*native).from(*si);
                ///
                ///   for (auto& curve : BoCurves) {
                ///       cvrtPress.appliedTo(curve.first);
                ///       cvrtBo   .appliedTo(curve.second);
                ///   }
                /// \endcode
                ///
                /// \param[in,out] Bo On input, a sequence of formation
                ///    volume factor values for the oil phase assumed to be
                ///    defined in the input unit specified through \code
                ///    from(usys) \endcode.  On output, the same seqeuence
                ///    of oil FVF values converted to the output unit of
                ///    measure specified through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& Bo) const override;
            };

            /// Facility for converting dissolved gas/oil ratio (Rs) values
            /// between user-selected units of measure.
            class DissolvedGasOilRatio : public PhysicalQuantity {
            public:
                /// Convert a sequence of dissolved gas/oil ratio values
                /// from its input unit of measure to its output unit of
                /// measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Extract the saturated state curve for oil in
                ///    cell 14142 in PVT-M units.
                /// \code
                ///   const auto rset  = ResultSet("Case.EGRID");
                ///   const auto init  = ECLInitFileData{ rset.initFile() };
                ///   const auto G     = ECLGraph.load(rset.gridFile(), init);
                ///   const auto si    = internalUnitConvention();
                ///   const auto pvt_m = pvtmUnitConventions();
                ///
                ///   const auto pvtCC = ECLPvtCurveCollection(G, init);
                ///
                ///   const auto satState =
                ///       pvtCC.getPvtCurve(RawCurve::SaturatedState,
                ///           ECLPhaseIndex::Liquid, 14142);
                ///
                ///   Convert::DissolvedGasOilRatio().to(*pvt_m).from(*si)
                ///       .appliedTo(satState.first);
                ///
                ///   Convert::Pressure().to(*pvt_m).from(*si)
                ///       .appliedTo(satState.second);
                /// \endcode
                ///
                /// \param[in,out] Rs On input, a sequence of dissolved
                ///    gas/oil ratio values assumed to be defined in the
                ///    input unit specified through \code from(usys)
                ///    \endcode.  On output, the same seqeuence of oil Rs
                ///    values converted to the output unit of measure
                ///    specified through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& Rs) const override;
            };

            /// Facility for converting vaporised oil/gas ratio (Rv) values
            /// between user-selected units of measure.
            class VaporisedOilGasRatio : public PhysicalQuantity {
            public:
                /// Convert a sequence of vaporised oil/gas ratio values
                /// from its input unit of measure to its output unit of
                /// measure.
                ///
                /// Will throw an exception of type \code
                /// std::invalid_argument \endcode unless both of the input
                /// and output unit system conventions have been previously
                /// specified.
                ///
                /// Example: Extract the saturated state curve for gas in
                ///    cell 161803 in Metric units.
                /// \code
                ///   const auto rset   = ResultSet("Case.EGRID");
                ///   const auto init   = ECLInitFileData{ rset.initFile() };
                ///   const auto G      = ECLGraph.load(rset.gridFile(), init);
                ///   const auto si     = internalUnitConvention();
                ///   const auto metric = metricUnitConventions();
                ///
                ///   const auto pvtCC = ECLPvtCurveCollection(G, init);
                ///
                ///   const auto satState =
                ///       pvtCC.getPvtCurve(RawCurve::SaturatedState,
                ///           ECLPhaseIndex::Vapour, 161803);
                ///
                ///   Convert::Pressure().to(*metric).from(*si)
                ///       .appliedTo(satState.first);
                ///
                ///   Convert::VaporisedOilGasRatio().to(*metric).from(*si)
                ///       .appliedTo(satState.second);
                /// \endcode
                ///
                /// \param[in,out] Rv On input, a sequence of vapourised
                ///    oil/gas ratio values assumed to be defined in the
                ///    input unit specified through \code from(usys)
                ///    \endcode.  On output, the same seqeuence of gas Rv
                ///    values converted to the output unit of measure
                ///    specified through \code to(usys) \endcode.
                void appliedTo(std::vector<double>& Rv) const override;
            };
        };

    } // namespace ECLUnits
} // namespace Opm

#endif  // OPM_ECLPROPERTYUNITCONVERSION_HEADER_INCLUDED
