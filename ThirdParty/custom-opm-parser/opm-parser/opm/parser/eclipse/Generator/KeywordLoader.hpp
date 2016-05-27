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
#include <memory>
#include <string>
#include <vector>

namespace boost {
    namespace filesystem {
        class path;
    }
}

namespace Opm {

    class ParserKeyword;

    class KeywordLoader {

    public:
        KeywordLoader(bool verbose);
        size_t size() const;
        bool hasKeyword(const std::string& keyword) const;
        std::shared_ptr<const ParserKeyword> getKeyword(const std::string& keyword) const;
        std::string getJsonFile(const std::string& keyword) const;
        size_t loadKeywordDirectory(const std::string& pathname);
        size_t loadKeywordDirectory(boost::filesystem::path& path);
        void loadKeyword(const std::string& filename);
        void loadKeyword(boost::filesystem::path& path);

        static std::vector<std::string> sortSubdirectories( const std::string& directory );
        size_t loadMultipleKeywordDirectories(const std::string& directory);

        std::map<std::string , std::shared_ptr<ParserKeyword> >::const_iterator keyword_begin( ) const;
        std::map<std::string , std::shared_ptr<ParserKeyword> >::const_iterator keyword_end( ) const;
    private:
        void addKeyword(std::shared_ptr<ParserKeyword> keyword , const std::string& jsonFile);

        bool m_verbose;
        std::map<std::string , std::shared_ptr<ParserKeyword> > m_keywords;
        std::map<std::string , std::string > m_jsonFile;
    };
}

#endif
