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

#include <stdexcept>
#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Generator/KeywordLoader.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>

namespace Opm {

    KeywordLoader::KeywordLoader(bool verbose)
        : m_verbose(verbose)
    {
    }


    size_t KeywordLoader::size() const {
        return m_keywords.size();
    }


    bool KeywordLoader::hasKeyword(const std::string& keyword) const {
        if (m_keywords.find( keyword ) == m_keywords.end())
            return false;
        else
            return true;
    }


    std::shared_ptr<const ParserKeyword> KeywordLoader::getKeyword(const std::string& keyword) const {
        auto iter = m_keywords.find( keyword );
        if (iter == m_keywords.end())
            throw std::invalid_argument("Keyword " + keyword + " not loaded");
        else
            return (*iter).second;
    }


    std::string KeywordLoader::getJsonFile(const std::string& keyword) const {
        auto iter = m_jsonFile.find( keyword );
        if (iter == m_jsonFile.end())
            throw std::invalid_argument("Keyword " + keyword + " not loaded");
        else
            return (*iter).second;
    }


    size_t KeywordLoader::loadKeywordDirectory(boost::filesystem::path& path) {
        size_t loadCount = 0;
        if (boost::filesystem::is_directory( path )) {
            boost::filesystem::directory_iterator end_iterator;

            for (boost::filesystem::directory_iterator iter(path); iter != end_iterator; ++iter) {
                boost::filesystem::path iter_path = iter->path();

                if (boost::filesystem::is_directory( iter_path )) {
                    loadCount += loadKeywordDirectory( iter_path );
                } else {
                    std::string internalName = iter_path.filename().string();
                    if (ParserKeyword::validInternalName(internalName)) {
                        if (m_verbose)
                            std::cout << "Loading keyword " << internalName << " from file: " << iter_path << "....";
                        loadKeyword( iter_path );
                        if (m_verbose)
                            std::cout << std::endl;
                        loadCount += 1;
                    } else {
                        if (m_verbose)
                            std::cout << "Ignoring file " << iter_path << "  - incorrectly formatted name." << std::endl;
                    }
                }
            }

        } else
            throw std::invalid_argument("Input does not correspond to existing directory\n");

        return loadCount;
    }

    size_t KeywordLoader::loadKeywordDirectory(const std::string& directory) {
        boost::filesystem::path path( directory );
        return loadKeywordDirectory( path );
    }

    void KeywordLoader::loadKeyword(boost::filesystem::path& path) {
        std::shared_ptr<Json::JsonObject> jsonConfig = std::make_shared<Json::JsonObject>( path );
        std::shared_ptr<ParserKeyword> parserKeyword = std::make_shared<ParserKeyword>(*jsonConfig);
        {
            boost::filesystem::path abs_path = boost::filesystem::absolute( path );
            addKeyword( parserKeyword , abs_path.generic_string() );
        }
    }


    size_t KeywordLoader::loadMultipleKeywordDirectories(const std::string& directory) {
        std::vector<std::string> directories = sortSubdirectories( directory );

        size_t load_count = 0;
        for (auto iter = directories.begin(); iter != directories.end(); ++iter)
            load_count += loadKeywordDirectory(*iter);

        return load_count;
    }


    void KeywordLoader::loadKeyword(const std::string& filename) {
        boost::filesystem::path path( filename );
        return loadKeyword( path );
    }


    void KeywordLoader::addKeyword(std::shared_ptr<ParserKeyword> keyword , const std::string& jsonFile) {
        const std::string& name = keyword->getName();

        if (hasKeyword(name)) {
            m_keywords[name] = keyword;
            m_jsonFile[name] = jsonFile;
        } else {
            m_keywords.insert( std::pair<std::string , std::shared_ptr<ParserKeyword> > (name , keyword) );
            m_jsonFile.insert( std::pair<std::string , std::string> ( name , jsonFile));
        }
    }


    std::vector<std::string> KeywordLoader::sortSubdirectories( const std::string& root_path) {
        boost::filesystem::path root(root_path);
        if (boost::filesystem::is_directory( root )) {
            std::vector<std::string> paths_in_root;
            boost::filesystem::directory_iterator end_iterator;

            for (boost::filesystem::directory_iterator iter(root); iter != end_iterator; ++iter) {
                if (boost::filesystem::is_directory( iter->path() ))
                    paths_in_root.push_back(iter->path().string());
            }

            std::sort(paths_in_root.begin(), paths_in_root.end());
            return paths_in_root;
        } else
            throw std::invalid_argument("Input argument is not a directory");
    }

    std::map<std::string , std::shared_ptr<ParserKeyword> >::const_iterator KeywordLoader::keyword_begin( ) const {
        return m_keywords.begin( );
    }

    std::map<std::string , std::shared_ptr<ParserKeyword> >::const_iterator KeywordLoader::keyword_end( ) const {
        return m_keywords.end( );
    }


}


