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

#ifndef OPM_MESSAGELIMITER_HEADER_INCLUDED
#define OPM_MESSAGELIMITER_HEADER_INCLUDED

#include <string>
#include <unordered_map>

namespace Opm
{


    /// Handles limiting the number of messages with the same tag.
    class MessageLimiter
    {
    public:
        /// Used to indicate no message number limit.
        enum { NoLimit = -1 };

        /// Default constructor, no limit to the number of messages.
        MessageLimiter()
            : message_limit_(NoLimit)
        {
        }

        /// Construct with given limit to number of messages with the
        /// same tag.
        ///
        /// Negative limits (including NoLimit) are interpreted as
        /// NoLimit, but the default constructor is the preferred way
        /// to obtain that behaviour.
        explicit MessageLimiter(const int message_limit)
            : message_limit_(message_limit < 0 ? NoLimit : message_limit)
        {
        }

        /// The message limit (same for all tags).
        int messageLimit() const
        {
            return message_limit_;
        }

        /// Used for handleMessageTag() return type (see that
        /// function).
        enum class Response
        {
            PrintMessage, JustOverLimit, OverLimit
        };

        /// If a tag is empty, there is no message limit or for that
        /// tag (count <= limit), respond PrintMessage.
        /// If (count == limit + 1), respond JustOverLimit.
        /// If (count > limit + 1), respond OverLimit.
        Response handleMessageTag(const std::string& tag)
        {
            if (tag.empty() || message_limit_ == NoLimit) {
                return Response::PrintMessage;
            } else {
                // See if tag already encountered.
                auto it = tag_counts_.find(tag);
                if (it != tag_counts_.end()) {
                    // Already encountered this tag. Increment its count.
                    const int count = ++it->second;
                    return countBasedResponse(count);
                } else {
                    // First encounter of this tag. Insert 1.
                    tag_counts_.insert({tag, 1});
                    return countBasedResponse(1);
                }
            }
        }

    private:
        Response countBasedResponse(const int count)
        {
            if (count <= message_limit_) {
                return Response::PrintMessage;
            } else if (count == message_limit_ + 1) {
                return Response::JustOverLimit;
            } else {
                return Response::OverLimit;
            }
        }


        int message_limit_;
        std::unordered_map<std::string, int> tag_counts_;
    };



} // namespace Opm

#endif // OPM_MESSAGELIMITER_HEADER_INCLUDED
