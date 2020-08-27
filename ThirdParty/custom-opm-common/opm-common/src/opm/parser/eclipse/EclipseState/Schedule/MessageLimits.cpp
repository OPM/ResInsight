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

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MessageLimits.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

namespace Opm {





    MessageLimits::MessageLimits( const TimeMap& timemap ) :
        limits( timemap , MLimits())
    { }


    MessageLimits MessageLimits::serializeObject()
    {
       MessageLimits result;
       result.limits = {{MLimits::serializeObject()}, 2};

        return result;
    }


    int MessageLimits::getMessagePrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.message_print_limit;
    }

    int MessageLimits::getCommentPrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.comment_print_limit;
    }

    int MessageLimits::getWarningPrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.warning_print_limit;
    }

    int MessageLimits::getProblemPrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.problem_print_limit;
    }

    int MessageLimits::getErrorPrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.error_print_limit;
    }

    int MessageLimits::getBugPrintLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.bug_print_limit;
    }

    /*-----------------------------------------------------------------*/

    int MessageLimits::getMessageStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.message_stop_limit;
    }

    int MessageLimits::getCommentStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.comment_stop_limit;
    }

    int MessageLimits::getWarningStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.warning_stop_limit;
    }

    int MessageLimits::getProblemStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.problem_stop_limit;
    }

    int MessageLimits::getErrorStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.error_stop_limit;
    }

    int MessageLimits::getBugStopLimit(size_t timestep) const
    {
        const auto& mlimit = limits.get( timestep );
        return mlimit.bug_stop_limit;
    }

    /*-----------------------------------------------------------------*/

    void MessageLimits::update(size_t timestep, const MLimits& value) {
        if (timestep == 0)
            limits.updateInitial( value );
        else
            limits.update( timestep , value );
    }

    void MessageLimits::setMessagePrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.message_print_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setCommentPrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.comment_print_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setWarningPrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.warning_print_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setProblemPrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.problem_print_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setErrorPrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.error_print_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setBugPrintLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.bug_print_limit = value;
        this->update( timestep , mlimit );
    }

    /*-----------------------------------------------------------------*/

    void MessageLimits::setMessageStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.message_stop_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setCommentStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.comment_stop_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setWarningStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.warning_stop_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setProblemStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.problem_stop_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setErrorStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.error_stop_limit = value;
        this->update( timestep , mlimit );
    }

    void MessageLimits::setBugStopLimit(size_t timestep, int value)
    {
        auto mlimit = limits.get( timestep );
        mlimit.bug_stop_limit = value;
        this->update( timestep , mlimit );
    }

    bool MessageLimits::operator==(const MessageLimits& data) const
    {
        return limits == data.limits;
    }


}


