/*
  Copyright 2015 Statoil ASA.

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
#include <opm/common/OpmLog/StreamLog.hpp>

namespace Opm {


StreamLog::StreamLog(const std::string& logFile , int64_t messageMask, bool append) : LogBackend(messageMask)
{
    if (append) {
        m_ofstream.open( logFile.c_str() ,  std::ofstream::app );
    } else {
        m_ofstream.open( logFile.c_str() ,  std::ofstream::out );
    }
    if (!m_ofstream) {
        throw std::runtime_error("Failed opening file " + logFile + " for StreamLog.");
    }
    m_streamOwner = true;
    m_ostream = &m_ofstream;
}


StreamLog::StreamLog(std::ostream& os , int64_t messageMask) : LogBackend(messageMask)
{
    m_ostream = &os;
    m_streamOwner = false;
}


void StreamLog::close() {
    if (m_streamOwner && m_ofstream.is_open()) {
        m_ofstream.close();
        m_ostream = nullptr;
    }
}

void StreamLog::addMessageUnconditionally(int64_t messageType, const std::string& message)
{
    (*m_ostream) << formatMessage(messageType, message) << std::endl;
    if (m_ofstream.is_open()) {
        m_ofstream.flush();
    }
}


StreamLog::~StreamLog() {
    close();
}

} // namespace Opm
