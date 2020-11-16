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

#include <opm/common/OpmLog/LogBackend.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

namespace Opm {

    LogBackend::LogBackend( int64_t mask ) :
        m_mask(mask)
    {
    }

    LogBackend::~LogBackend()
    {
    }

    void LogBackend::setMessageFormatter(std::shared_ptr<MessageFormatterInterface> formatter)
    {
        m_formatter = formatter;
    }

    void LogBackend::setMessageLimiter(std::shared_ptr<MessageLimiter> limiter)
    {
        m_limiter = limiter;
    }

    void LogBackend::addMessage(int64_t messageFlag, const std::string& message)
    {
        // Forward the call to the tagged version.
        addTaggedMessage(messageFlag, "", message);
    }

    void LogBackend::addTaggedMessage(int64_t messageType, const std::string& messageTag, const std::string& message) {
        if (includeMessage( messageType, messageTag )) {
            addMessageUnconditionally(messageType, message);
        }
    }

    int64_t LogBackend::getMask() const
    {
        return m_mask;
    }

    bool LogBackend::includeMessage(int64_t messageFlag, const std::string& messageTag)
    {
        // Check mask.
        const bool included = ((messageFlag & m_mask) == messageFlag) && (messageFlag > 0);
        if (!included) {
            return false;
        }

        // Use the message limiter (if any).
        MessageLimiter::Response res = m_limiter
            ? m_limiter->handleMessageLimits(messageTag, messageFlag)
            : MessageLimiter::Response::PrintMessage;
        if (res == MessageLimiter::Response::JustOverTagLimit) {
            // Special case: add a message to this backend about limit being reached.
            std::string msg = "Message limit reached for message tag: " + messageTag;
            addMessageUnconditionally(messageFlag, msg);
        }
        if (res == MessageLimiter::Response::JustOverCategoryLimit) {
            // Special case: add a message to this backend about limit being reached.
            std::string prefix = Log::prefixMessage(messageFlag, "");
            std::string msg = "Message limit reached for message category: " + prefix.substr(0, prefix.size()-2);
            addMessageUnconditionally(messageFlag, msg);
        }

        return res == MessageLimiter::Response::PrintMessage;
    }

    std::string LogBackend::formatMessage(int64_t messageFlag, const std::string& message)
    {
        if (m_formatter) {
            return m_formatter->format(messageFlag, message);
        } else {
            return message;
        }
    }


}
