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

#ifndef OPM_ECLENDPOINTSCALING_HEADER_INCLUDED
#define OPM_ECLENDPOINTSCALING_HEADER_INCLUDED

#include <opm/utility/ECLPhaseIndex.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Opm {

    class ECLGraph;
    class ECLInitFileData;

} // namespace Opm

namespace Opm { namespace SatFunc {

    /// Protocol for computing scaled saturation values.
    class EPSEvalInterface
    {
    public:
        /// Static (unscaled) end-points of a particular, tabulated
        /// saturation function.
        struct TableEndPoints {
            /// Critical or connate/minimum saturation, depending on
            /// subsequent function evaluation context.
            ///
            /// Use critical saturation when computing look-up values for
            /// relative permeability and connate/minimum saturation when
            /// computing look-up values for capillary pressure.
            double low;

            /// Displacing saturation (3-pt option only).
            double disp;

            /// Maximum (high) saturation point.
            double high;
        };

        /// Associate a saturation value to a specific cell.
        struct SaturationAssoc {
            /// Cell to which to connect a saturation value.
            std::vector<int>::size_type cell;

            /// Saturation value.
            double sat;
        };

        /// Convenience type alias.
        using SaturationPoints = std::vector<SaturationAssoc>;

        /// Derive scaled saturations--inputs to subsequent evaluation of
        /// saturation functions--corresponding to a sequence of unscaled
        /// saturation values.
        ///
        /// \param[in] tep Static end points that identify the saturation
        ///    scaling intervals of a particular tabulated saturation
        ///    function.
        ///
        /// \param[in] sp Sequence of saturation points.
        ///
        /// \return Sequence of scaled saturation values in order of the
        ///    input sequence.  In particular the \c i-th element of this
        ///    result is the scaled version of \code sp[i].sat \endcode.
        virtual std::vector<double>
        eval(const TableEndPoints&   tep,
             const SaturationPoints& sp) const = 0;

        virtual std::unique_ptr<EPSEvalInterface> clone() const = 0;

        /// Destructor.  Must be virtual.
        virtual ~EPSEvalInterface();
    };

    /// Implementation of ECLIPSE's standard, two-point, saturation scaling
    /// option.
    class TwoPointScaling : public EPSEvalInterface
    {
    public:
        /// Constructor.
        ///
        /// Typically set up to define the end-point scaling of all active
        /// cells in a model, but could alternatively be used as a means to
        /// computing the effective saturation function of a single cell.
        ///
        /// \param[in] smin Left end points for a set of cells.
        ///
        /// \param[in] smax Right end points for a set of cells.
        TwoPointScaling(std::vector<double> smin,
                        std::vector<double> smax);

        /// Destructor.
        ~TwoPointScaling();

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing object.
        TwoPointScaling(const TwoPointScaling& rhs);

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing object.
        TwoPointScaling(TwoPointScaling&& rhs);

        /// Assignment operator.
        ///
        /// Replaces current implementation with a copy of existing object's
        /// implementation details.
        ///
        /// \param[in] rhs Existing object.
        ///
        /// \return \code *this \endcode.
        TwoPointScaling&
        operator=(const TwoPointScaling& rhs);

        /// Move assingment operator.
        ///
        /// Subsumes existing object's implementation details and uses those
        /// to replace the current implementation.
        ///
        /// \return \code *this \endcode.
        TwoPointScaling&
        operator=(TwoPointScaling&& rhs);

        /// Derive scaled saturations using the two-point scaling definition
        /// from a sequence of unscaled saturation values.
        ///
        /// \param[in] tep Static end points that identify the saturation
        ///    scaling intervals of a particular tabulated saturation
        ///    function.  The evaluation procedure considers only \code
        ///    tep.low \endcode and \code tep.high \endcode.  The value of
        ///    \code tep.disp \endcode is never read.
        ///
        /// \param[in] sp Sequence of saturation points.  The maximum cell
        ///    index (\code sp[i].cell \endcode) must be strictly less than
        ///    the size of the input arrays that define the current
        ///    saturation regions.
        ///
        /// \return Sequence of scaled saturation values in order of the
        ///    input sequence.  In particular the \c i-th element of this
        ///    result is the scaled version of \code sp[i].sat \endcode.
        virtual std::vector<double>
        eval(const TableEndPoints&   tep,
             const SaturationPoints& sp) const override;

        virtual std::unique_ptr<EPSEvalInterface> clone() const override;

    private:
        /// Implementation class.
        class Impl;

        /// Pimpl idiom.
        std::unique_ptr<Impl> pImpl_;
    };

    /// Implementation of ECLIPSE's alternative, three-point, saturation
    /// scaling option.
    class ThreePointScaling : public EPSEvalInterface
    {
    public:
        /// Constructor.
        ///
        /// Typically set up to define the end-point scaling of all active
        /// cells in a model, but could alternatively be used as a means to
        /// computing the effective saturation function of a single cell.
        ///
        /// \param[in] smin Left end points for a set of cells.
        ///
        /// \param[in] sdisp Intermediate--displacing saturation--end points
        ///    for a set of cells.
        ///
        /// \param[in] smax Right end points for a set of cells.
        ThreePointScaling(std::vector<double> smin,
                          std::vector<double> sdisp,
                          std::vector<double> smax);

        /// Destructor.
        ~ThreePointScaling();

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing object.
        ThreePointScaling(const ThreePointScaling& rhs);

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing object.
        ThreePointScaling(ThreePointScaling&& rhs);

        /// Assignment operator.
        ///
        /// Replaces current implementation with a copy of existing object's
        /// implementation details.
        ///
        /// \param[in] rhs Existing object.
        ///
        /// \return \code *this \endcode.
        ThreePointScaling&
        operator=(const ThreePointScaling& rhs);

        /// Move assingment operator.
        ///
        /// Subsumes existing object's implementation details and uses those
        /// to replace the current implementation.
        ///
        /// \return \code *this \endcode.
        ThreePointScaling&
        operator=(ThreePointScaling&& rhs);

        /// Derive scaled saturations using the three-point scaling
        /// definition from a sequence of unscaled saturation values.
        ///
        /// \param[in] tep Static end points that identify the saturation
        ///    scaling intervals of a particular tabulated saturation
        ///    function.  The evaluation procedure considers only \code
        ///    tep.low \endcode and \code tep.high \endcode.  The value of
        ///    \code tep.disp \endcode is never read.
        ///
        /// \param[in] sp Sequence of saturation points.  The maximum cell
        ///    index (\code sp[i].cell \endcode) must be strictly less than
        ///    the size of the input arrays that define the current
        ///    saturation regions.
        ///
        /// \return Sequence of scaled saturation values in order of the
        ///    input sequence.  In particular the \c i-th element of this
        ///    result is the scaled version of \code sp[i].sat \endcode.
        virtual std::vector<double>
        eval(const TableEndPoints&   tep,
             const SaturationPoints& sp) const override;

        virtual std::unique_ptr<EPSEvalInterface> clone() const override;

    private:
        /// Implementation class.
        class Impl;

        /// Pimpl idiom.
        std::unique_ptr<Impl> pImpl_;
    };

    /// Named constructors for enabling saturation end-point scaling from an
    /// ECL result set (see class \c ECLInitFileData).
    struct CreateEPS
    {
        /// Category of function for which to create an EPS evaluator.
        enum class FunctionCategory {
            /// This EPS is for relative permeability.  Possibly uses
            /// three-point (alternative) formulation.
            Relperm,

            /// This EPS is for capillary pressure.  Two-point option only.
            CapPress,
        };

        /// Categories of saturation function systems for which to create an
        /// EPS evaluator.
        enum class SubSystem {
            /// Create an EPS for a curve in the Oil-Water (sub-) system.
            OilWater,

            /// Create an EPS for a curve in the Oil-Gas (sub-) system.
            OilGas,
        };

        /// Set of options that uniquely define a single EPS operation.
        struct EPSOptions {
            /// Whether or not to employ the alternative (i.e., 3-pt) scaling
            /// procedure.  Only applicable to FunctionCategory::Relperm and
            /// ignored in the case of FunctionCategory::CapPress.
            bool use3PtScaling;

            /// Curve-type for which to create an EPS.
            FunctionCategory curve;

            /// Part of global fluid system for which to create an EPS.
            SubSystem subSys;

            /// Phase for whose \c curve in which \c subSys to create an
            /// EPS.
            ///
            /// Example: Create a standard (two-point) EPS for the relative
            ///    permeability of oil in the oil-gas subsystem of an
            ///    oil-gas-water active phase system.
            ///
            /// \code
            ///   auto opt = EPSOptions{};
            ///
            ///   opt.use3PtScaling = false;
            ///   opt.curve         = FunctionCategory::Relperm;
            ///   opt.subSys        = SubSystem::OilGas;
            ///   opt.thisPh        = ECLPhaseIndex::Oil;
            ///
            ///   auto eps = CreateEPS::fromECLOutput(G, init, opt);
            /// \endcode
            ::Opm::ECLPhaseIndex thisPh;
        };

        /// Collection of raw saturation table end points.
        struct RawTableEndPoints {
            /// Collection of connate (minimum) saturation end points.
            struct Connate {
                /// Connate oil saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> oil;

                /// Connate gas saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> gas;

                /// Connate water saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> water;
            };

            /// Collection of critical saturations.  Used in deriving scaled
            /// displacing saturations in the alternative (three-point)
            /// scaling procedure.
            struct Critical {
                /// Critical oil saturation in 2p OG system from total set
                /// of tabulated saturation functions.
                std::vector<double> oil_in_gas;

                /// Critical oil saturation in 2p OW system from total set
                /// of tabulated saturation functions.
                std::vector<double> oil_in_water;

                /// Critical gas saturation in 2p OG or 3p OGW system from
                /// total set of tabulated saturation functions.
                std::vector<double> gas;

                /// Critical water saturation in 2p OW or 3p OGW system from
                /// total set of tabulated saturation functions.
                std::vector<double> water;
            };

            /// Collection of maximum saturation end points.
            struct Maximum {
                /// Maximum oil saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> oil;

                /// Maximum gas saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> gas;

                /// Maximum water saturation for each table in total set of
                /// tabulated saturation functions.
                std::vector<double> water;
            };

            /// Minimum saturation end points for all tabulated saturation
            /// functions.
            Connate conn;

            /// Critical saturations for all tabulated saturation functions.
            Critical crit;

            /// Maximum saturation end points for all tabulated saturation
            /// functions.
            Maximum smax;
        };

        /// Construct an EPS evaluator from a particular ECL result set.
        ///
        /// \param[in] G Connected topology of current model's active cells.
        ///    Needed to linearise table end-points that may be distributed
        ///    on local grids to all of the model's active cells (\code
        ///    member function G.rawLinearisedCellData() \endcode).
        ///
        /// \param[in] init Container of tabulated saturation functions and
        ///    saturation table end points for all active cells.
        ///
        /// \param[in] opt Options that identify a particular end-point
        ///    scaling behaviour of a particular saturation function curve.
        ///
        /// \return EPS evaluator for the particular curve defined by the
        ///    input options.
        static std::unique_ptr<EPSEvalInterface>
        fromECLOutput(const ECLGraph&        G,
                      const ECLInitFileData& init,
                      const EPSOptions&      opt);

        /// Extract table end points relevant to a particular EPS evaluator
        /// from raw tabulated saturation functions.
        ///
        /// \param[in] ep Collection of all raw table saturation end points
        ///    for all tabulated saturation functions.  Typically computed
        ///    by direct calls to the \code connateSat() \endcode, \code
        ///    criticalSat() \endcode, and \code maximumSat() \endcode of
        ///    the currently active \code Opm::SatFuncInterpolant \code
        ///    objects.
        ///
        /// \param[in] opt Options that identify a particular end-point
        ///    scaling behaviour of a particular saturation function curve.
        ///
        /// \return Subset of the input end points in a format intended for
        ///    passing as the first argument of member function \code eval()
        ///    \endcode of the \code EPSEvalInterface \endcode that
        ///    corresponds to the input options.
        static std::vector<EPSEvalInterface::TableEndPoints>
        unscaledEndPoints(const RawTableEndPoints& ep,
                          const EPSOptions&        opt);
    };
}} // namespace Opm::SatFunc

#endif // OPM_ECLENDPOINTSCALING_HEADER_INCLUDED
