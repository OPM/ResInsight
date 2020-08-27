/*
  Copyright 2014 Andreas Lauser

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
#include <stdexcept>
#include <sstream>
#include <cassert>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/OpmLog/CounterLog.hpp>



namespace Opm {

CounterLog::CounterLog(int64_t messageTypes) : LogBackend(messageTypes)
{ }

CounterLog::CounterLog() : LogBackend(Log::DefaultMessageTypes)
{ }


size_t CounterLog::numMessages(int64_t messageType) const {
    if (Log::isPower2( messageType )) {
        auto iter = m_count.find( messageType );
        if (iter == m_count.end())
            return 0;
        else
            return (*iter).second;
    } else
        throw std::invalid_argument("The messageType ID must be 2^n");
}



void CounterLog::addMessageUnconditionally(int64_t messageType, const std::string& ) {
    m_count[messageType]++;
}


void CounterLog::clear()
{
    m_count.clear();
}



} // namespace Opm
