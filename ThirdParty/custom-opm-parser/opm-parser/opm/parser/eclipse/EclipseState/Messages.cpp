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

#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/parser/eclipse/EclipseState/Messages.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>


namespace Opm {

    Messages::Messages(const Messages& messages, const DeckRecord& record) :
        m_message_print_limit( messages.m_message_print_limit ),
        m_comment_print_limit( messages.m_comment_print_limit ),
        m_warning_print_limit( messages.m_warning_print_limit ),
        m_problem_print_limit( messages.m_problem_print_limit ),
        m_error_print_limit( messages.m_error_print_limit ),
        m_bug_print_limit( messages.m_bug_print_limit ),
        m_message_stop_limit( messages.m_message_stop_limit ),
        m_comment_stop_limit( messages.m_comment_stop_limit ),
        m_warning_stop_limit( messages.m_warning_stop_limit ),
        m_problem_stop_limit( messages.m_problem_stop_limit ),
        m_error_stop_limit( messages.m_error_stop_limit ),
        m_bug_stop_limit( messages.m_bug_stop_limit )

    {
        if (!record.getItem<ParserKeywords::MESSAGES::MESSAGE_PRINT_LIMIT>().defaultApplied(0))
            m_message_print_limit = record.getItem<ParserKeywords::MESSAGES::MESSAGE_PRINT_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::COMMENT_PRINT_LIMIT>().defaultApplied(0))
            m_comment_print_limit = record.getItem<ParserKeywords::MESSAGES::COMMENT_PRINT_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::WARNING_PRINT_LIMIT>().defaultApplied(0))
            m_warning_print_limit = record.getItem<ParserKeywords::MESSAGES::WARNING_PRINT_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::PROBLEM_PRINT_LIMIT>().defaultApplied(0))
            m_problem_print_limit = record.getItem<ParserKeywords::MESSAGES::PROBLEM_PRINT_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::ERROR_PRINT_LIMIT>().defaultApplied(0))
            m_error_print_limit = record.getItem<ParserKeywords::MESSAGES::ERROR_PRINT_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::BUG_PRINT_LIMIT>().defaultApplied(0))
            m_bug_print_limit = record.getItem<ParserKeywords::MESSAGES::BUG_PRINT_LIMIT>().get< int >( 0 );

        /*****************************************************************/

        if (!record.getItem<ParserKeywords::MESSAGES::MESSAGE_STOP_LIMIT>().defaultApplied(0))
            m_message_stop_limit = record.getItem<ParserKeywords::MESSAGES::MESSAGE_STOP_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::COMMENT_STOP_LIMIT>().defaultApplied(0))
            m_comment_stop_limit = record.getItem<ParserKeywords::MESSAGES::COMMENT_STOP_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::WARNING_STOP_LIMIT>().defaultApplied(0))
            m_warning_stop_limit = record.getItem<ParserKeywords::MESSAGES::WARNING_STOP_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::PROBLEM_STOP_LIMIT>().defaultApplied(0))
            m_problem_stop_limit = record.getItem<ParserKeywords::MESSAGES::PROBLEM_STOP_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::ERROR_STOP_LIMIT>().defaultApplied(0))
            m_error_stop_limit = record.getItem<ParserKeywords::MESSAGES::ERROR_STOP_LIMIT>().get< int >( 0 );

        if (!record.getItem<ParserKeywords::MESSAGES::BUG_STOP_LIMIT>().defaultApplied(0))
            m_bug_stop_limit = record.getItem<ParserKeywords::MESSAGES::BUG_STOP_LIMIT>().get< int >( 0 );
    }

    Messages::Messages(const DeckRecord& record) :
        m_message_print_limit( record.getItem<ParserKeywords::MESSAGES::MESSAGE_PRINT_LIMIT>().get< int >( 0 )),
        m_comment_print_limit( record.getItem<ParserKeywords::MESSAGES::COMMENT_PRINT_LIMIT>().get< int >( 0 )),
        m_warning_print_limit( record.getItem<ParserKeywords::MESSAGES::WARNING_PRINT_LIMIT>().get< int >(0)),
        m_problem_print_limit( record.getItem<ParserKeywords::MESSAGES::PROBLEM_PRINT_LIMIT>().get< int >(0)),
        m_error_print_limit( record.getItem<ParserKeywords::MESSAGES::ERROR_PRINT_LIMIT>().get< int >(0)),
        m_bug_print_limit( record.getItem<ParserKeywords::MESSAGES::BUG_PRINT_LIMIT>().get< int >(0)),
        m_message_stop_limit( record.getItem<ParserKeywords::MESSAGES::MESSAGE_STOP_LIMIT>().get< int >(0)),
        m_comment_stop_limit( record.getItem<ParserKeywords::MESSAGES::COMMENT_STOP_LIMIT>().get< int >(0)),
        m_warning_stop_limit( record.getItem<ParserKeywords::MESSAGES::WARNING_STOP_LIMIT>().get< int >(0)),
        m_problem_stop_limit( record.getItem<ParserKeywords::MESSAGES::PROBLEM_STOP_LIMIT>().get< int >(0)),
        m_error_stop_limit( record.getItem<ParserKeywords::MESSAGES::ERROR_STOP_LIMIT>().get< int >(0)),
        m_bug_stop_limit( record.getItem<ParserKeywords::MESSAGES::BUG_STOP_LIMIT>().get< int >(0))
    { }

}


