/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_MESSAGEFORMATTER_HEADER_INCLUDED
#define OPM_MESSAGEFORMATTER_HEADER_INCLUDED

#include <opm/common/OpmLog/LogUtil.hpp>
#include <string>

namespace Opm
{


    /// Abstract interface for message formatting classes.
    class MessageFormatterInterface
    {
    public:
        /// Virtual destructor to enable inheritance.
        virtual ~MessageFormatterInterface() = default;
        /// Should return a possibly modified/decorated version of the
        /// input string, the formatting applied depending on the
        /// message_flag.
        virtual std::string format(const int64_t message_flag, const std::string& message) = 0;
    };





    /// A simple formatter capable of adding message prefixes and colors.
    class SimpleMessageFormatter : public MessageFormatterInterface
    {
    public:
        /// Constructor controlling formatting to be applied.
        SimpleMessageFormatter(const bool use_prefix, const bool use_color_coding)
            : use_color_coding_(use_color_coding)
        {
            if (use_prefix) {
                prefix_flag_ = Log::DefaultMessageTypes;
            }
        }


        SimpleMessageFormatter(const int64_t prefix_flag, const bool use_color_coding)
            : use_color_coding_(use_color_coding),
              prefix_flag_(prefix_flag)
        {
        }


        explicit SimpleMessageFormatter(const bool use_color_coding)
            : use_color_coding_(use_color_coding)
        {
            prefix_flag_ = Log::MessageType::Warning + Log::MessageType::Error 
                         + Log::MessageType::Problem + Log::MessageType::Bug;
        }
        /// Returns a copy of the input string with a flag-dependant
        /// prefix (if use_prefix) and the entire message in a
        /// flag-dependent color (if use_color_coding).
        virtual std::string format(const int64_t message_flag, const std::string& message) override
        {
            std::string msg = message;
            if (message_flag & prefix_flag_) {
                msg = Log::prefixMessage(message_flag, msg);
            }
            if (use_color_coding_) {
                msg = Log::colorCodeMessage(message_flag, msg);
            }
            return msg;
        }
    private:
        bool use_color_coding_ = false;
        int64_t prefix_flag_ = 0;
    };


} // namespace Opm

#endif // OPM_MESSAGEFORMATTER_HEADER_INCLUDED
