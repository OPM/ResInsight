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


#ifndef KEYWORD_LOADER_HPP
#define KEYWORD_LOADER_HPP

#include <map>
#include <string>
#include <vector>

#include <opm/input/eclipse/Parser/ParserKeyword.hpp>

namespace Opm {


    class KeywordLoader {

    public:
        KeywordLoader(const std::vector<std::string>& keyword_files, bool verbose);
        std::string getJsonFile(const std::string& keyword) const;

        std::map<char , std::vector<ParserKeyword> >::const_iterator begin( ) const;
        std::map<char , std::vector<ParserKeyword> >::const_iterator end( ) const;
    private:
        std::map<char, std::vector<ParserKeyword>> keywords;
        std::map<std::string , std::string > m_jsonFile;
    };
}

#endif
