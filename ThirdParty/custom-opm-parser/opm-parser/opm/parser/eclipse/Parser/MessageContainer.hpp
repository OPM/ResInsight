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

#ifndef MESSAGECONTAINER_H
#define MESSAGECONTAINER_H

#include <string>
#include <vector>
#include <memory>

namespace Opm {

    struct Location {
        Location() = default;
        Location( const std::string&, size_t );

        std::string filename;
        size_t lineno = 0;

        explicit operator bool() const {
            return lineno != 0;
        }
    };

    struct Message {
        enum type {
            Debug     = 1,
            Note      = 2,
            Info      = 3,
            Warning   = 4,
            Error     = 5,
            Problem   = 6,
            Bug       = 7
        };

        Message( type mt, const std::string& msg, Location&& loc ) :
            mtype( mt ), message( msg ), location( std::move( loc ) ) {}

        Message( type mt, const std::string& msg ) :
            mtype( mt ), message( msg ) {}


        type mtype;
        std::string message;
        Location location;
    };


    ///Message container is used to replace OpmLog functionalities.
    class MessageContainer {
    public:

        using const_iterator = std::vector< Message >::const_iterator;

        void error(const std::string& msg, const std::string& filename, const size_t lineno);
        void error(const std::string& msg);

        void bug(const std::string& msg, const std::string& filename, const size_t lineno);
        void bug(const std::string& msg);

        void warning(const std::string& msg, const std::string& filename, const size_t lineno);
        void warning(const std::string& msg);

        void info(const std::string& msg, const std::string& filename, const size_t lineno);
        void info(const std::string& msg);

        void debug(const std::string& msg, const std::string& filename, const size_t lineno);
        void debug(const std::string& msg);

        void problem(const std::string& msg, const std::string& filename, const size_t lineno);
        void problem(const std::string& msg);

        void note(const std::string& msg, const std::string& filename, const size_t lineno);
        void note(const std::string& msg);

        void add( const Message& );
        void add( Message&& );
        
        void appendMessages(const MessageContainer& other);
        
        const_iterator begin() const;
        const_iterator end() const;

        std::size_t size() const;
        
    private:
        std::vector<Message> m_messages;
    };

} // namespace Opm

#endif // OPM_MESSAGECONTAINER_HEADER_INCLUDED
