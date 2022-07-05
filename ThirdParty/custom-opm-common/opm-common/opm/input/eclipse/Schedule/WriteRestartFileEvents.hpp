/*
  Copyright 2021 Equinor ASA.

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

#ifndef OPM_WRITE_RESTART_FILE_EVENTS_HPP
#define OPM_WRITE_RESTART_FILE_EVENTS_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Opm {

class WriteRestartFileEvents
{
public:
    void resize(const std::size_t numReportSteps);
    void clearRemainingEvents(const std::size_t start);
    void addRestartOutput(const std::size_t reportStep);

    bool writeRestartFile(const std::size_t reportStep) const;

    std::optional<std::size_t>
    lastRestartEventBefore(const std::size_t reportStep) const;

    bool operator==(const WriteRestartFileEvents& that) const;

    static WriteRestartFileEvents serializeObject();

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->write_restart_file_);
    }

private:
    std::vector<std::uint_fast64_t> write_restart_file_{};
};

}

#endif // OPM_WRITE_RESTART_FILE_EVENTS_HPP
