/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RST_NETWORK_HPP
#define RST_NETWORK_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Opm {
    class UnitSystem;
} // namespace Opm

namespace Opm { namespace EclIO {
    class RestartFileView;
}} // namespace Opm::EclIO

namespace Opm { namespace RestartIO {

    class RstNetwork
    {
    public:
        /// Single branch in extended network model.
        struct Branch
        {
            /// Downtree node.  Index into 'nodes' array.
            int down{-1};

            /// Uptree node.  Index into 'nodes' array.
            int up{-1};

            /// One-based VFP table ID.
            int vfp{-1};
        };

        /// Single node in extended network model.
        struct Node
        {
            /// Name of network node.
            std::string name{};

            /// Fixed pressure for terminal node.  Nullopt if not terminal.
            std::optional<double> terminal_pressure{};

            /// Group whose rate target the choking mechanism attempts to
            /// match.  Nullopt if this node does not act as a choke or if
            /// choking is disabled.
            std::optional<std::string> as_choke{};

            /// Whether or not to include lift gas of subordinate wells as
            /// part of the produced gas entering the network at this node.
            bool add_lift_gas{false};
        };

        explicit RstNetwork(std::shared_ptr<EclIO::RestartFileView> rstView,
                            const UnitSystem&                       usys);

        bool isActive() const;

        const std::vector<Branch>& branches() const
        {
            return this->branches_;
        }

        const std::vector<Node>& nodes() const
        {
            return this->nodes_;
        }

    private:
        std::vector<Branch> branches_{};
        std::vector<Node> nodes_{};
    };

}} // namespace Opm::RestartIO

#endif // RST_NETWORK_HPP
