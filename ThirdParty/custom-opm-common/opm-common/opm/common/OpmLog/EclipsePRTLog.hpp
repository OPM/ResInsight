/*
  Copyright 2016 Statoil ASA.

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

#ifndef ECLIPSEPRTLOG_H
#define ECLIPSEPRTLOG_H

#include <map>
#include <string>
#include <opm/common/OpmLog/StreamLog.hpp>

namespace Opm {

class EclipsePRTLog : public StreamLog {

public:
    using StreamLog::StreamLog;

    void addTaggedMessage(int64_t messageType, const std::string& messageTag, const std::string& message);

    size_t numMessages(int64_t messageType) const;

    ~EclipsePRTLog();

private:
    std::map<int64_t, size_t> m_count;
};
}
#endif // ECLIPSEPRTLOG_H
