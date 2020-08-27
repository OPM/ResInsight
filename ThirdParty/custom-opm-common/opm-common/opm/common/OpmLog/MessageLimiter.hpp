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

#include <opm/common/OpmLog/LogUtil.hpp>
#include <cassert>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>


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
            : MessageLimiter(NoLimit)
        {
        }

        /// Construct with given limit to number of messages with the
        /// same tag.
        ///
        /// Negative limits (including NoLimit) are interpreted as
        /// NoLimit, but the default constructor is the preferred way
        /// to obtain that behaviour.
        explicit MessageLimiter(const int tag_limit)
            : tag_limit_(tag_limit < 0 ? NoLimit : tag_limit),
              category_limits_({{Log::MessageType::Debug, NoLimit},
		                {Log::MessageType::Note, NoLimit},
                                {Log::MessageType::Info, NoLimit},
                                {Log::MessageType::Warning, NoLimit},
                                {Log::MessageType::Error, NoLimit},
                                {Log::MessageType::Problem, NoLimit},
                                {Log::MessageType::Bug, NoLimit}})
        {
        }

        MessageLimiter(const int tag_limit, const std::map<int64_t, int>& category_limits)
            : tag_limit_(tag_limit < 0 ? NoLimit : tag_limit),
              category_limits_(category_limits)
        {
            // Must ensure NoLimit for categories that are not
            // explicitly specified in the input.
            for (auto category : { Log::MessageType::Debug,
		                   Log::MessageType::Note,
                                   Log::MessageType::Info,
                                   Log::MessageType::Warning,
                                   Log::MessageType::Error,
                                   Log::MessageType::Problem,
                                   Log::MessageType::Bug }) {
                if (category_limits_.find(category) == category_limits_.end()) {
                    category_limits_[category] = NoLimit;
                }
            }
        }

        /// The tag message limit (same for all tags).
        int tagMessageLimit() const
        {
            return tag_limit_;
        }

        /// The category message limits.
        const std::map<int64_t, int>& categoryMessageLimits() const
        {
            return category_limits_;
        }

        /// The category message counts.
        const std::map<int64_t, int>& categoryMessageCounts() const
        {
            return category_counts_;
        }

        /// Used for handleMessageLimits() return type (see that
        /// function).
        enum class Response
        {
            PrintMessage, JustOverTagLimit, JustOverCategoryLimit, OverTagLimit, OverCategoryLimit
        };

        /// If (tag count == tag limit + 1) for the passed tag, respond JustOverTagLimit.
        /// If (tag count > tag limit + 1), respond OverTagLimit.
        /// If a tag is empty, there is no tag message limit or for that tag
        /// (tag count <= tag limit), consider the category limits:
        /// If (category count == category limit + 1) for the passed messageMask, respond JustOverCategoryLimit.
        /// If (category count > category limit + 1), respond OverCategoryLimit.
        /// If (category count <= category limit), or there is no limit for that category, respond PrintMessage.
        Response handleMessageLimits(const std::string& tag, const int64_t messageMask)
        {
            Response res = Response::PrintMessage;

            // Deal with tag limits.
            if (!tag.empty() && tag_limit_ != NoLimit) {
                // See if tag already encountered.
                auto it = tag_counts_.find(tag);
                if (it != tag_counts_.end()) {
                    // Already encountered this tag. Increment its count.
                    const int count = ++it->second;
                    res = countBasedResponseTag(count);
                } else {
                    // First encounter of this tag. Insert 1.
                    tag_counts_.insert({tag, 1});
                    res = countBasedResponseTag(1);
                }
            }

            // If tag count reached the limit, the message is not counted
            // towards the category limits.
            if (res == Response::PrintMessage) {
                // We are *not* above the tag limit, consider category limit.
                const int count = ++category_counts_[messageMask];
                if (category_limits_[messageMask] != NoLimit) {
                    res = countBasedResponseCategory(count, messageMask);
                }
            }

            return res;
        }

    private:
        Response countBasedResponseTag(const int count) const
        {
            if (count <= tag_limit_) {
                return Response::PrintMessage;
            } else if (count == tag_limit_ + 1) {
                return Response::JustOverTagLimit;
            } else {
                return Response::OverTagLimit;
            }
        }


        Response countBasedResponseCategory(const int count, const int64_t messageMask) const
        {
            const int limit = category_limits_.at(messageMask);
            if (count <= limit) {
                return Response::PrintMessage;
            } else if (count == limit + 1) {
                return Response::JustOverCategoryLimit;
            } else {
                return Response::OverCategoryLimit;
            }
        }

        int tag_limit_;
        std::unordered_map<std::string, int> tag_counts_;
        std::map<int64_t, int> category_limits_;
        std::map<int64_t, int> category_counts_ = {{Log::MessageType::Note, 0},
                                                   {Log::MessageType::Info, 0},
                                                   {Log::MessageType::Warning, 0},
                                                   {Log::MessageType::Error, 0},
                                                   {Log::MessageType::Problem, 0},
                                                   {Log::MessageType::Bug, 0}};
    };



} // namespace Opm

#endif // OPM_MESSAGELIMITER_HEADER_INCLUDED
