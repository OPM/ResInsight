/*
  Copyright 2016 Statoil ASA.

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

#include <functional>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Schedule/MessageLimits.hpp>

namespace Opm {

    MessageLimits::MessageLimits()
    {
        message_print_limit = ParserKeywords::MESSAGES::MESSAGE_PRINT_LIMIT::defaultValue;
        comment_print_limit = ParserKeywords::MESSAGES::COMMENT_PRINT_LIMIT::defaultValue;
        warning_print_limit = ParserKeywords::MESSAGES::WARNING_PRINT_LIMIT::defaultValue;
        problem_print_limit = ParserKeywords::MESSAGES::PROBLEM_PRINT_LIMIT::defaultValue;
        error_print_limit   = ParserKeywords::MESSAGES::ERROR_PRINT_LIMIT::defaultValue;
        bug_print_limit     = ParserKeywords::MESSAGES::BUG_PRINT_LIMIT::defaultValue;
        message_stop_limit  = ParserKeywords::MESSAGES::MESSAGE_STOP_LIMIT::defaultValue;
        comment_stop_limit  = ParserKeywords::MESSAGES::COMMENT_STOP_LIMIT::defaultValue;
        warning_stop_limit  = ParserKeywords::MESSAGES::WARNING_STOP_LIMIT::defaultValue;
        problem_stop_limit  = ParserKeywords::MESSAGES::PROBLEM_STOP_LIMIT::defaultValue;
        error_stop_limit    = ParserKeywords::MESSAGES::ERROR_STOP_LIMIT::defaultValue;
        bug_stop_limit      = ParserKeywords::MESSAGES::BUG_STOP_LIMIT::defaultValue;
    }

    MessageLimits::MessageLimits(const Deck& deck ) :
        MessageLimits()
    {
        for (const auto& keyword : deck) {
            if (keyword.name() == "MESSAGES")
                this->update(keyword);

            if (keyword.name() == "SCHEDULE")
                break;
        }
    }


    MessageLimits MessageLimits::serializeObject()
    {
       MessageLimits result;
       result.message_print_limit = 12345;
       return result;
    }


    int MessageLimits::getMessagePrintLimit() const
    {
        return this->message_print_limit;
    }

    int MessageLimits::getCommentPrintLimit() const
    {
        return this->comment_print_limit;
    }

    int MessageLimits::getWarningPrintLimit() const
    {
        return this->warning_print_limit;
    }

    int MessageLimits::getProblemPrintLimit() const
    {
        return this->problem_print_limit;
    }

    int MessageLimits::getErrorPrintLimit() const
    {
        return this->error_print_limit;
    }

    int MessageLimits::getBugPrintLimit() const
    {
        return this->bug_print_limit;
    }

    /*-----------------------------------------------------------------*/

    int MessageLimits::getMessageStopLimit() const
    {
        return this->message_stop_limit;
    }

    int MessageLimits::getCommentStopLimit() const
    {
        return this->comment_stop_limit;
    }

    int MessageLimits::getWarningStopLimit() const
    {
        return this->warning_stop_limit;
    }

    int MessageLimits::getProblemStopLimit() const
    {
        return this->problem_stop_limit;
    }

    int MessageLimits::getErrorStopLimit() const
    {
        return this->error_stop_limit;
    }

    int MessageLimits::getBugStopLimit() const
    {
        return this->bug_stop_limit;
    }

    /*-----------------------------------------------------------------*/

    void MessageLimits::setMessagePrintLimit(int value)
    {
        this->message_print_limit = value;
    }

    void MessageLimits::setCommentPrintLimit(int value)
    {
        this->comment_print_limit = value;
    }

    void MessageLimits::setWarningPrintLimit(int value)
    {
        this->warning_print_limit = value;
    }

    void MessageLimits::setProblemPrintLimit(int value)
    {
        this->problem_print_limit = value;
    }

    void MessageLimits::setErrorPrintLimit(int value)
    {
        this->error_print_limit = value;
    }

    void MessageLimits::setBugPrintLimit(int value)
    {
        this->bug_print_limit = value;
    }

    /*-----------------------------------------------------------------*/

    void MessageLimits::setMessageStopLimit(int value)
    {
        this->message_stop_limit = value;
    }

    void MessageLimits::setCommentStopLimit(int value)
    {
        this->comment_stop_limit = value;
    }

    void MessageLimits::setWarningStopLimit(int value)
    {
        this->warning_stop_limit = value;
    }

    void MessageLimits::setProblemStopLimit(int value)
    {
        this->problem_stop_limit = value;
    }

    void MessageLimits::setErrorStopLimit(int value)
    {
        this->error_stop_limit = value;
    }

    void MessageLimits::setBugStopLimit(int value)
    {
        this->bug_stop_limit = value;
    }

    bool MessageLimits::operator==(const MessageLimits& other) const
    {
        return  ((this->message_print_limit == other.message_print_limit) &&
                 (this->comment_print_limit == other.comment_print_limit) &&
                 (this->warning_print_limit == other.warning_print_limit) &&
                 (this->problem_print_limit == other.problem_print_limit) &&
                 (this->error_print_limit   == other.error_print_limit  ) &&
                 (this->bug_print_limit     == other.bug_print_limit    ) &&
                 (this->message_stop_limit  == other.message_stop_limit ) &&
                 (this->comment_stop_limit  == other.comment_stop_limit ) &&
                 (this->warning_stop_limit  == other.warning_stop_limit ) &&
                 (this->problem_stop_limit  == other.problem_stop_limit ) &&
                 (this->error_stop_limit    == other.error_stop_limit   ) &&
                 (this->bug_stop_limit      == other.bug_stop_limit     ));
    }

    void MessageLimits::update(const DeckKeyword& keyword) {
        const auto& record = keyword.getRecord(0);
        using  set_limit_fptr = decltype( std::mem_fn( &MessageLimits::setMessagePrintLimit ) );
        static const std::pair<std::string , set_limit_fptr> setters[] = {
            {"MESSAGE_PRINT_LIMIT" , std::mem_fn( &MessageLimits::setMessagePrintLimit )},
            {"COMMENT_PRINT_LIMIT" , std::mem_fn( &MessageLimits::setCommentPrintLimit )},
            {"WARNING_PRINT_LIMIT" , std::mem_fn( &MessageLimits::setWarningPrintLimit )},
            {"PROBLEM_PRINT_LIMIT" , std::mem_fn( &MessageLimits::setProblemPrintLimit )},
            {"ERROR_PRINT_LIMIT"   , std::mem_fn( &MessageLimits::setErrorPrintLimit   )},
            {"BUG_PRINT_LIMIT"     , std::mem_fn( &MessageLimits::setBugPrintLimit     )},
            {"MESSAGE_STOP_LIMIT"  , std::mem_fn( &MessageLimits::setMessageStopLimit  )},
            {"COMMENT_STOP_LIMIT"  , std::mem_fn( &MessageLimits::setCommentStopLimit  )},
            {"WARNING_STOP_LIMIT"  , std::mem_fn( &MessageLimits::setWarningStopLimit  )},
            {"PROBLEM_STOP_LIMIT"  , std::mem_fn( &MessageLimits::setProblemStopLimit  )},
            {"ERROR_STOP_LIMIT"    , std::mem_fn( &MessageLimits::setErrorStopLimit    )},
            {"BUG_STOP_LIMIT"      , std::mem_fn( &MessageLimits::setBugStopLimit      )},
        };

        for (const auto& pair : setters) {
            const auto& item = record.getItem( pair.first );
            if (!item.defaultApplied(0)) {
                const set_limit_fptr& fptr = pair.second;
                int value = item.get<int>(0);
                fptr( *this, value );
            }
        }
    }

}


