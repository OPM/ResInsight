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


#ifndef KEYWORD_GENERATOR_HPP
#define KEYWORD_GENERATOR_HPP

#include <string>

namespace Opm {

    class KeywordLoader;

    class KeywordGenerator {

    public:
        KeywordGenerator(bool verbose);

        static void ensurePath( const std::string& file_name);
        static std::string endTest();
        static std::string startTest(const std::string& test_name);
        static std::string headerHeader( const std::string& );
        static void updateFile(const std::stringstream& newContent, const std::string& filename);

        void updateBuiltInHeader(const KeywordLoader& loader, const std::string& headerBuildPath, const std::string& headerPath) const;
        void updateInitSource(const KeywordLoader& loader, const std::string& sourceFile ) const;
        void updateKeywordSource(const KeywordLoader& loader, const std::string& sourceFile ) const;
        void updatePybindSource(const KeywordLoader& loader , const std::string& sourceFile ) const;
        void updateHeader(const KeywordLoader& loader, const std::string& headerBuildPath, const std::string& headerPath) const;
        void updateTest(const KeywordLoader& loader , const std::string& testFile) const;
    private:
        bool m_verbose;
    };
}

#endif
