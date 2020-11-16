/*
  Copyright 2014 Statoil ASA.

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
#include <cassert>
#include <iomanip>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/OpmLog/TimerLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>



namespace Opm {

TimerLog::TimerLog(const std::string& logFile) : StreamLog( logFile , StopTimer | StartTimer )
{
    m_work.precision(8);
    m_start = 0;
}

TimerLog::TimerLog(std::ostream& os) : StreamLog( os , StopTimer | StartTimer )
{
    m_work.precision(8);
    m_start = 0;
}



void TimerLog::addMessageUnconditionally(int64_t messageType, const std::string& msg ) {
    if (messageType == StopTimer) {
        clock_t stop = clock();
        double secondsElapsed = 1.0 * (m_start - stop) / CLOCKS_PER_SEC ;

        m_work.str("");
        m_work << std::fixed << msg << ": " << secondsElapsed << " seconds ";
        StreamLog::addMessageUnconditionally( messageType, m_work.str());
    } else {
        if (messageType == StartTimer)
            m_start = clock();
    }
}



} // namespace Opm
