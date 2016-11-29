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

#ifndef OPM_MESSAGES_HPP
#define OPM_MESSAGES_HPP

#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

namespace Opm {

    class Messages {
    public:

        /*
           This constructor will create a new Messages object which is
           a copy of the input argument, and then all items explicitly
           set in the record are modified.
        */
        Messages(const Messages& message , const DeckRecord& record);
        
        Messages(const DeckRecord& record);

    private:
        int m_message_print_limit;
        int m_comment_print_limit;
        int m_warning_print_limit;
        int m_problem_print_limit;
        int m_error_print_limit;
        int m_bug_print_limit;

        int m_message_stop_limit;
        int m_comment_stop_limit;
        int m_warning_stop_limit;
        int m_problem_stop_limit;
        int m_error_stop_limit;
        int m_bug_stop_limit;
    };
}

#endif
