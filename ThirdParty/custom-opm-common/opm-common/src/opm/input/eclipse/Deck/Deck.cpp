/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <filesystem>
#include <vector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace fs = std::filesystem;

namespace Opm {

    std::vector< const DeckKeyword* > Deck::getKeywordList( const std::string& keyword ) const {
        std::vector<const DeckKeyword *> pointers;
        auto view = this->global_view().operator[](keyword);
        for (const auto& kw : view)
            pointers.push_back(&kw);
        return pointers;
    }

    Deck::const_iterator Deck::begin() const {
        return this->keywordList.begin();
    }

    Deck::const_iterator Deck::end() const {
        return this->keywordList.end();
    }


std::size_t Deck::count(const std::string& keyword) const {
    return this->global_view().count(keyword);
}

const DeckView& Deck::global_view() const {
    if (!this->m_global_view) {
        this->m_global_view = std::make_unique<DeckView>();
        for (const auto& kw : this->keywordList)
            this->m_global_view->add_keyword(kw);
    }
    return *this->m_global_view;
}

    Opm::DeckView Deck::operator[](const std::string& keyword) const {
        return this->global_view()[keyword];
    } 

    const DeckKeyword& Deck::operator[](std::size_t index) const {
        return this->keywordList.at(index);
    }


    Deck::Deck( const Deck& d )
        : keywordList( d.keywordList )
        , defaultUnits( d.defaultUnits )
        , activeUnits( d.activeUnits )
        , m_dataFile( d.m_dataFile )
        , input_path( d.input_path )
        , file_tree( d.file_tree )
        , unit_system_access_count(d.unit_system_access_count)
    {
    }

    Deck::Deck( Deck&& d )
        : keywordList( std::move(d.keywordList) )
        , defaultUnits( d.defaultUnits )
        , activeUnits( d.activeUnits )
        , m_dataFile( d.m_dataFile )
        , input_path( d.input_path )
        , file_tree( std::move(d.file_tree) )
        , unit_system_access_count(d.unit_system_access_count)
    {
    }

    Deck Deck::serializeObject()
    {
        Deck result;
        result.keywordList = {DeckKeyword::serializeObject()};
        result.defaultUnits = UnitSystem::serializeObject();
        result.activeUnits = UnitSystem::serializeObject();
        result.m_dataFile = "test1";
        result.input_path = "test2";
        result.unit_system_access_count = 1;

        return result;
    }

    void Deck::addKeyword( DeckKeyword&& keyword ) {
        if (keyword.name() == "FIELD")
            this->selectActiveUnitSystem( UnitSystem::UnitType::UNIT_TYPE_FIELD );
        else if (keyword.name() == "METRIC")
            this->selectActiveUnitSystem( UnitSystem::UnitType::UNIT_TYPE_METRIC );
        else if (keyword.name() == "LAB")
            this->selectActiveUnitSystem( UnitSystem::UnitType::UNIT_TYPE_LAB );
        else if (keyword.name() == "PVT-M")
            this->selectActiveUnitSystem( UnitSystem::UnitType::UNIT_TYPE_PVT_M );

        this->keywordList.push_back( std::move( keyword ) );
        this->m_global_view = nullptr;
    }

    void Deck::addKeyword( const DeckKeyword& keyword ) {
        DeckKeyword kw = keyword;
        this->addKeyword( std::move( kw ) );
    }

    UnitSystem& Deck::getDefaultUnitSystem() {
        return this->defaultUnits;
    }

    const UnitSystem& Deck::getDefaultUnitSystem() const {
        return this->defaultUnits;
    }

    UnitSystem& Deck::getActiveUnitSystem() {
        this->unit_system_access_count++;
        if (this->activeUnits.has_value())
            return this->activeUnits.value();
        else
            return this->defaultUnits;
    }

    void Deck::selectActiveUnitSystem(UnitSystem::UnitType unit_type) {
        const auto& current = this->getActiveUnitSystem();
        if (current.use_count() > 0 && (current.getType() != unit_type))
            throw std::invalid_argument("Sorry - can not change unit system after dimensionfull features have been entered");

        if (current.getType() != unit_type)
            this->activeUnits = UnitSystem(unit_type);
    }


    const UnitSystem& Deck::getActiveUnitSystem() const {
        this->unit_system_access_count++;
        if (this->activeUnits.has_value())
            return this->activeUnits.value();
        else
            return this->defaultUnits;
    }

    std::string Deck::getDataFile() const {
        return m_dataFile.value_or("");
    }

    const std::string& Deck::getInputPath() const {
        return this->input_path;
    }

    std::string Deck::makeDeckPath(const std::string& path) const {
        if (path.size() > 0 && path[0] == '/')
            return path;

        if (this->input_path.size() == 0)
            return path;
        else
            return this->input_path + "/" + path;
    }


    void Deck::setDataFile(const std::string& dataFile) {
        if (this->m_dataFile.has_value())
            throw std::logic_error("Can not reassign deck datafile");

        this->m_dataFile = dataFile;

        auto slash_pos = dataFile.find_last_of("/\\");
        if (slash_pos == std::string::npos)
            this->input_path = "";
        else
            this->input_path = dataFile.substr(0, slash_pos);

        this->file_tree.add_root(dataFile);
    }

    DeckTree& Deck::tree() {
        return this->file_tree;
    }

    DeckTree Deck::tree() const {
        return this->file_tree;
    }



    Deck::iterator Deck::begin() {
        return this->keywordList.begin();
    }

    Deck::iterator Deck::end() {
        return this->keywordList.end();
    }


    void Deck::write( DeckOutput& output ) const {
        size_t kw_index = 1;
        for (const auto& keyword: *this) {
            keyword.write( output );
            kw_index++;
            if (kw_index < size())
                output.write_string( output.fmt.keyword_sep );
        }
    }

    Deck& Deck::operator=(const Deck& data) {
        keywordList = data.keywordList;
        defaultUnits = data.defaultUnits;
        m_dataFile = data.m_dataFile;
        input_path = data.input_path;
        unit_system_access_count = data.unit_system_access_count;
        activeUnits = data.activeUnits;

        return *this;
    }

    bool Deck::operator==(const Deck& data) const {
        if ((activeUnits && !data.activeUnits) ||
            (!activeUnits && data.activeUnits))
            return false;

        if (activeUnits && *activeUnits != *data.activeUnits)
            return false;
        return this->keywordList == data.keywordList &&
               this->defaultUnits == data.defaultUnits &&
               this->m_dataFile == data.m_dataFile &&
               this->input_path == data.input_path &&
               this->unit_system_access_count == data.unit_system_access_count;
    }

    std::ostream& operator<<(std::ostream& os, const Deck& deck) {
        DeckOutput out( os, 10 );
        deck.write( out );
        return os;
    }

    std::size_t Deck::size() const {
        return this->keywordList.size();
    }

    bool Deck::empty() const {
        return this->keywordList.size() == 0;
    }

    bool Deck::hasKeyword(const std::string& keyword) const {
        return this->global_view().has_keyword(keyword);
    }

}
