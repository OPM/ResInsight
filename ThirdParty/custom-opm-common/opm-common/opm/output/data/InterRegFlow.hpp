/*
  Copyright (c) 2022 Equinor ASA

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

#ifndef OPM_OUTPUT_DATA_INTERREGFLOW_HPP
#define OPM_OUTPUT_DATA_INTERREGFLOW_HPP

#include <algorithm>
#include <cassert>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace Opm { namespace data {

    /// Intermediary Protocol to Linearise Per-Connection Flow Rates Into Subrange.
    ///
    /// \tparam RandIt Random access iterator type of subrange.
    template <typename RandIt>
    class InterRegFlow
    {
    public:
        /// Subrange element type.
        ///
        /// Typically \c double or \c float.
        using ElmT = std::remove_cv_t<
            std::remove_reference_t<
                typename std::iterator_traits<RandIt>::value_type
            >>;

        /// Surface component flow type.
        enum class Component : char {
            Oil, Gas, Water, Disgas, Vapoil,

            // Must be last enumerator
            NumComponents,
        };

        /// Surface flow direction.
        enum class Direction : char {
            Positive, Negative,
        };

        /// Connection Flow Rates
        class FlowRates
        {
        public:
            /// Constructor.
            FlowRates()
            {
                this->rate_.fill(ElmT{});
            }

            /// Element access.
            ///
            /// \param[in] i Component ID for specific element.
            ///
            /// \return Read/Write reference to internal flow rate element.
            ElmT& operator[](const Component i)
            {
                return this->rate_[this->index(i)];
            }

            friend class InterRegFlow;

        private:
            /// Internal storage.
            std::array<ElmT, static_cast<std::size_t>(Component::NumComponents)> rate_{};

            /// Convert component ID into linear index.
            ///
            /// \param[in] i Component ID for specific element.
            ///
            /// \return Linear index to internal flow rate element identified by \p i.
            std::size_t index(const Component i) const
            {
                return static_cast<std::size_t>(i);
            }
        };

        /// Constructor.
        ///
        /// \param[in] begin Beginning of subrange viewd by this instance.
        /// \param[in] end One-past-the-end of subrange viewed by this instance.
        explicit InterRegFlow(RandIt begin, RandIt end)
            : elements_(begin, end)
        {}

        /// Deleted copy constructor.
        InterRegFlow(const InterRegFlow&) = delete;

        /// Move constructor.
        ///
        /// Invalidates source range.
        ///
        /// \param[in,out] rhs Source range.
        InterRegFlow(InterRegFlow&& rhs)
            : elements_(rhs.elements_.first, rhs.elements_.second)
        {
            rhs.elements_.second = rhs.elements_.first; // rhs -> empty
        }

        /// Assignment operator.
        ///
        /// \param[in] rhs Source range.
        ///
        /// \return \code *this \endcode.
        InterRegFlow& operator=(const InterRegFlow& rhs)
        {
            this->copyIn(rhs);

            return *this;
        }

        /// Move assignment operator.
        ///
        /// Invalidates source range.
        ///
        /// \param[in,out] rhs Source range.
        ///
        /// \return \code *this \endcode.
        InterRegFlow& operator=(InterRegFlow&& rhs)
        {
            if (! this->isValid()) {
                this->elements_ = rhs.elements_;
            }
            else {
                this->copyIn(rhs);
            }

            rhs.elements_.second = rhs.elements_.first; // rhs -> empty

            return *this;
        }

        /// Addition operator
        ///
        /// \tparam OtherRandIt Random access iterator, possibly different
        ///    from \c RandIt.
        ///
        /// \param[in] rhs Source subrange.
        ///
        /// \return \code *this \endcode.
        template <typename OtherRandIt>
        std::enable_if_t<
            std::is_convertible_v<typename InterRegFlow<OtherRandIt>::ElmT, ElmT>,
        InterRegFlow&> operator+=(const InterRegFlow<OtherRandIt>& rhs)
        {
            std::transform(this->begin(),
                           this->end(),
                           rhs  .begin(),
                           this->begin(),
                           std::plus<>{});

            return *this;
        }

        /// Assignment operator from different, but compatible, subrange.
        ///
        /// Enables assigning into a subrange of \code vector<double>
        /// \endcode from a subrange backed by \code array<float,N> \endcode
        /// of compatible size.
        ///
        /// \tparam OtherRandIt Random access iterator different from \c RandIt.
        ///
        /// \param[in] rhs Source subrange.
        ///
        /// \return \code *this \endcode.
        template <typename OtherRandIt>
        std::enable_if_t<
            !std::is_same_v<RandIt, OtherRandIt> &&
            std::is_convertible_v<typename InterRegFlow<OtherRandIt>::ElmT, ElmT>,
        InterRegFlow&> operator=(const InterRegFlow<OtherRandIt>& rhs)
        {
            this->copyIn(rhs.begin(), rhs.end());

            return *this;
        }

        /// Accumulate connection contribution into subrange.
        ///
        /// \param[in] sign Flow rate sign--e.g., to flip direction if
        ///    needed in calling context.
        ///
        /// \param[in] q Connection flow rates.
        void addFlow(const ElmT sign, const FlowRates& q)
        {
            assert (this->isValid());

            const auto numComp = static_cast<std::size_t>(Component::NumComponents);

            for (auto component = 0*numComp; component < numComp; ++component) {
                this->add(sign * q.rate_[component], component);
            }
        }

        /// Buffer size (number of elements)
        ///
        /// Storage buffer backing the \c InterRegFlow object must have at
        /// least \code InterRegFlow::bufferSize() \endcode contiguous elements.
        constexpr static std::size_t bufferSize() noexcept
        {
            return InterRegFlow::index(Component::NumComponents, Direction::Positive);
        }

        /// Total accumulated flow rate of particular surface rate component
        /// for this region.
        ///
        /// \param[in] component Component ID for specific element.
        ///
        /// \return Flow rate.
        constexpr ElmT flow(const Component component) const noexcept
        {
            // Add components since Positive and Negative are stored as
            // signed quantities.  In other words flow(x, Negative) <= 0
            // while flow(x, Positive) >= 0).
            return this->flow(component, Direction::Positive)
                +  this->flow(component, Direction::Negative);
        }

        /// Accumulated flow rate for this region par of particular surface
        /// rate component in particular direction.
        ///
        /// Flow from source to destination is \c Positive while from from
        /// destination to source is \c Negative.  Numerical value in the \c
        /// Positive direction is non-negative while the numerical value in
        /// the \c Negative direction is non-positive.
        ///
        /// \param[in] component Component ID for specific element.
        ///
        /// \param[in] direction Flow direction.
        ///
        /// \return Component flow rate in specified direction.
        constexpr ElmT flow(const Component component,
                            const Direction direction) const noexcept
        {
            return *(this->elements_.first + InterRegFlow::index(component, direction));
        }

        /// Predicate for whether or not this \c InterRegFlow object is
        /// backed by an empty range.
        constexpr bool empty() const noexcept
        {
            return this->begin() == this->end();
        }

        /// Predicate for whether or not this \c InterRegFlow object is
        /// backed by a valid range (size \code InterRegFlow::bufferSize()
        /// \endcode).
        constexpr bool isValid() const noexcept
        {
            using sz_t = decltype(InterRegFlow::bufferSize());

            const auto& [begin, end] = this->elements_;

            return static_cast<sz_t>(std::distance(begin, end))
                == InterRegFlow::bufferSize();
        }

        /// Iterator to beginning of subrange.
        RandIt begin() const noexcept
        {
            return this->elements_.first;
        }

        /// Iterator to one-past-end of subrange.
        RandIt end() const noexcept
        {
            return this->elements_.second;
        }

    private:
        /// Element subrange.
        std::pair<RandIt, RandIt> elements_;

        /// Convert directional component ID into linear index.
        ///
        /// Overload for already converted component index.
        ///
        /// \param[in] component Component index for specific element.
        /// \param[in] direction Flow direction.
        ///
        /// \return Linear index of ordered pair of component ID and flow
        /// direction.
        constexpr static std::size_t
        index(const std::size_t component, const Direction direction)
        {
            return 2*component + (direction == Direction::Negative);
        }

        /// Convert directional component ID into linear index.
        ///
        /// \param[in] component Component ID for specific element.
        /// \param[in] direction Flow direction.
        ///
        /// \return Linear index of ordered pair of component ID and flow
        /// direction.
        constexpr static std::size_t
        index(const Component component, const Direction direction)
        {
            return InterRegFlow::index(static_cast<std::size_t>(component), direction);
        }

        /// Accumulate component flow rate into linearised subrange.
        ///
        /// \param[in] rate Component flow rate.
        /// \param[in] component Component index.
        void add(const ElmT rate, const std::size_t component)
        {
            const auto direction = std::signbit(rate)
                ? Direction::Negative : Direction::Positive;

            auto* rateVec = &*this->elements_.first;
            rateVec[InterRegFlow::index(component, direction)] += rate;
        }

        /// Backend for assignment operator
        ///
        /// \param[in] rhs Source subrange.
        void copyIn(const InterRegFlow& rhs)
        {
            if (this->elements_ != rhs.elements_) {
                this->copyIn(rhs.elements_.first, rhs.elements_.second);
            }
        }

        /// Backend for assignment operator
        ///
        /// Activated for compatible ranges.
        ///
        /// \param[in] begin Beginning of source subrange.
        /// \param[in] end One-past-end of source subrange.
        template <typename OtherRandIt>
        void copyIn(OtherRandIt begin, OtherRandIt end)
        {
            std::copy(begin, end, this->elements_.first);
        }
    };

}} // namespace Opm::data

#endif // OPM_OUTPUT_DATA_INTERREGFLOW_HPP
