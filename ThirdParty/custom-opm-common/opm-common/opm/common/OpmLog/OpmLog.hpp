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

#ifndef OPMLOG_HPP
#define OPMLOG_HPP

#include <memory>
#include <cstdint>

#include <opm/common/OpmLog/Logger.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

namespace Opm {

    class LogBackend;

/*
  The OpmLog class is a fully static class which manages a proper
  Logger instance.
*/


class OpmLog {

public:
    static void addMessage(int64_t messageFlag , const std::string& message);
    static void addTaggedMessage(int64_t messageFlag, const std::string& tag, const std::string& message);

    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void problem(const std::string& message);
    static void bug(const std::string& message);
    static void debug(const std::string& message);
    static void note(const std::string& message);

    static void info(const std::string& tag, const std::string& message);
    static void warning(const std::string& tag, const std::string& message);
    static void error(const std::string& tag, const std::string& message);
    static void problem(const std::string& tag, const std::string& message);
    static void bug(const std::string& tag, const std::string& message);
    static void debug(const std::string& tag, const std::string& message);
    static void note(const std::string& tag, const std::string& message);

    static bool hasBackend( const std::string& backendName );
    static void addBackend(const std::string& name , std::shared_ptr<LogBackend> backend);
    static bool removeBackend(const std::string& name);
    static void removeAllBackends();
    static bool enabledMessageType( int64_t messageType );
    static void addMessageType( int64_t messageType , const std::string& prefix);

    /// Create a basic logging setup that will send all log messages to standard output.
    ///
    /// By default category prefixes will be printed (i.e. Error: or
    /// Warning:), color coding will be used, and a maximum of 10
    /// messages with the same tag will be printed. These settings can
    /// be controlled by the function parameters.
    static void setupSimpleDefaultLogging(const bool use_prefix = true,
                                          const bool use_color_coding = true,
                                          const int message_limit = 10);

    template <class BackendType>
    static std::shared_ptr<BackendType> getBackend(const std::string& name) {
        auto logger = getLogger();
        return logger->getBackend<BackendType>(name);
    }

    template <class BackendType>
    static std::shared_ptr<BackendType> popBackend(const std::string& name) {
        auto logger = getLogger();
        return logger->popBackend<BackendType>(name);
    }


    static bool stdoutIsTerminal();

private:
    static std::shared_ptr<Logger> getLogger();
    static std::shared_ptr<Logger> m_logger;
};


}




#endif
