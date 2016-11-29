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
#ifndef OPM_TIMERLOG_HPP
#define OPM_TIMERLOG_HPP

#include <time.h>

#include <memory>
#include <sstream>
#include <string>

#include <opm/common/OpmLog/StreamLog.hpp>

/*
  This class is a simple demonstration of how the logging framework
  can be used to create a simple very special case logging facility. 
*/

namespace Opm {

class TimerLog : public StreamLog {
public:
    static const int64_t StartTimer = 4096;
    static const int64_t StopTimer  = 8192;

    TimerLog(const std::string& logFile);
    TimerLog(std::ostream& os);

    void addTaggedMessage(int64_t messageFlag,
                          const std::string& messageTag,
                          const std::string& message) override;

    void clear();
    ~TimerLog() {};
private:
    clock_t m_start;
    std::ostringstream m_work;
};

typedef std::shared_ptr<TimerLog> TimerLogPtr;
typedef std::shared_ptr<const TimerLog> TimerLogConstPtr;
} // namespace Opm

#endif

