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
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <opm/common/OpmLog/LogBackend.hpp>
#include <opm/common/OpmLog/Logger.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

namespace Opm {

    Logger::Logger()
        : m_globalMask(0),
          m_enabledTypes(0)
    {
        addMessageType( Log::MessageType::Debug , "debug");
        addMessageType( Log::MessageType::Info , "info");
        addMessageType( Log::MessageType::Warning , "warning");
        addMessageType( Log::MessageType::Error , "error");
        addMessageType( Log::MessageType::Problem , "problem");
        addMessageType( Log::MessageType::Bug , "bug");
        addMessageType( Log::MessageType::Note , "note");
    }

    void Logger::addTaggedMessage(int64_t messageType, const std::string& tag, const std::string& message) const {
        if ((m_enabledTypes & messageType) == 0)
            throw std::invalid_argument("Tried to issue message with unrecognized message ID");

        if (m_globalMask & messageType) {
            for (auto iter : m_backends) {
                LogBackend& backend = *(iter.second);
                backend.addTaggedMessage( messageType, tag, message );
            }
        }
    }

    void Logger::addMessage(int64_t messageType , const std::string& message) const {
        addTaggedMessage(messageType, "", message);
    }


    void Logger::updateGlobalMask( int64_t mask ) {
        m_globalMask |= mask;
    }


    bool Logger::hasBackend(const std::string& name) {
        if (m_backends.find( name ) == m_backends.end())
            return false;
        else
            return true;
    }

    void Logger::removeAllBackends() {
        m_backends.clear();
        m_globalMask = 0;
    }

    bool Logger::removeBackend(const std::string& name) {
        size_t eraseCount = m_backends.erase( name );
        if (eraseCount == 1)
            return true;
        else
            return false;
    }


    void Logger::addBackend(const std::string& name , std::shared_ptr<LogBackend> backend) {
        updateGlobalMask( backend->getMask() );
        m_backends[ name ] = backend;
    }


    int64_t Logger::enabledMessageTypes() const {
        return m_enabledTypes;
    }

    //static: 
    bool Logger::enabledMessageType( int64_t enabledTypes , int64_t messageType) {
        if (Log::isPower2( messageType)) {
            if ((messageType & enabledTypes) == 0)
                return false;
            else
                return true;
        } else
            throw std::invalid_argument("The message type id must be ~ 2^n");
    }


    //static:
    bool Logger::enabledDefaultMessageType( int64_t messageType) {
        return enabledMessageType( Log::DefaultMessageTypes , messageType );
    }

    bool Logger::enabledMessageType( int64_t messageType) const {
        return enabledMessageType( m_enabledTypes , messageType );
    }


    void Logger::addMessageType( int64_t messageType , const std::string& /* prefix */) {
        if (Log::isPower2( messageType)) {
            m_enabledTypes |= messageType;
        } else
            throw std::invalid_argument("The message type id must be ~ 2^n");
    }

}
