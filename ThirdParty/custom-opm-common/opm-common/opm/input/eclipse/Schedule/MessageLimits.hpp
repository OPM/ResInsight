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

namespace Opm {

    class Deck;
    class DeckKeyword;

    class MessageLimits {
    public:
        MessageLimits();
        explicit MessageLimits(const Deck& deck);

        static MessageLimits serializeObject();

        ///Get all the value from MESSAGES keyword.
        int getMessagePrintLimit() const;
        int getCommentPrintLimit() const;
        int getWarningPrintLimit() const;
        int getProblemPrintLimit() const;
        int getErrorPrintLimit() const;
        int getBugPrintLimit() const;
        void setMessagePrintLimit(int value);
        void setCommentPrintLimit(int value);
        void setWarningPrintLimit(int value);
        void setProblemPrintLimit(int value);
        void setErrorPrintLimit(int value);
        void setBugPrintLimit(int value);

        int getMessageStopLimit() const;
        int getCommentStopLimit() const;
        int getWarningStopLimit() const;
        int getProblemStopLimit() const;
        int getErrorStopLimit() const;
        int getBugStopLimit() const;
        void setMessageStopLimit(int value);
        void setCommentStopLimit(int value);
        void setWarningStopLimit(int value);
        void setProblemStopLimit(int value);
        void setErrorStopLimit(int value);
        void setBugStopLimit(int value);

        bool operator==(const MessageLimits& data) const;
        void update(const DeckKeyword& keyword);

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(message_print_limit);
            serializer(comment_print_limit);
            serializer(warning_print_limit);
            serializer(problem_print_limit);
            serializer(error_print_limit);
            serializer(bug_print_limit);
            serializer(message_stop_limit);
            serializer(comment_stop_limit);
            serializer(warning_stop_limit);
            serializer(problem_stop_limit);
            serializer(error_stop_limit);
            serializer(bug_stop_limit);
        }

    private:
        int message_print_limit;
        int comment_print_limit;
        int warning_print_limit;
        int problem_print_limit;
        int error_print_limit;
        int bug_print_limit;
        int message_stop_limit;
        int comment_stop_limit;
        int warning_stop_limit;
        int problem_stop_limit;
        int error_stop_limit;
        int bug_stop_limit;
    };
}

#endif
