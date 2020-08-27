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
#include <vector>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckOutput.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm {

    bool DeckView::hasKeyword( const DeckKeyword& keyword ) const {
        auto key = this->keywordMap.find( keyword.name() );

        if( key == this->keywordMap.end() ) return false;

        for( auto index : key->second )
            if( &this->getKeyword( index ) == &keyword ) return true;

        return false;
    }

    bool DeckView::hasKeyword( const std::string& keyword ) const {
        return this->keywordMap.find( keyword ) != this->keywordMap.end();
    }

    const DeckKeyword& DeckView::getKeyword( const std::string& keyword, size_t index ) const {
        if( !this->hasKeyword( keyword ) )
            throw std::invalid_argument("Keyword " + keyword + " not in deck.");

        return this->getKeyword( this->offsets( keyword ).at( index ) );
    }

    const DeckKeyword& DeckView::getKeyword( const std::string& keyword ) const {
        if( !this->hasKeyword( keyword ) )
            throw std::invalid_argument("Keyword " + keyword + " not in deck.");

        return this->getKeyword( this->offsets( keyword ).back() );
    }

    const DeckKeyword& DeckView::getKeyword( size_t index ) const {
        if( index >= this->size() )
            throw std::out_of_range("Keyword index " + std::to_string( index ) + " is out of range.");

        return *( this->begin() + index );
    }

    size_t DeckView::count( const std::string& keyword ) const {
        if( !this->hasKeyword( keyword ) ) return 0;

        return this->offsets( keyword ).size();
   }

    const std::vector< const DeckKeyword* > DeckView::getKeywordList( const std::string& keyword ) const {
        if( !hasKeyword( keyword ) ) return {};

        const auto& indices = this->offsets( keyword );

        std::vector< const DeckKeyword* > ret;
        ret.reserve( indices.size() );

        for( size_t i : indices )
            ret.push_back( &this->getKeyword( i ) );

        return ret;
    }

    size_t DeckView::size() const {
        return std::distance( this->begin(), this->end() );
    }

    DeckView::const_iterator DeckView::begin() const {
        return this->first;
    }

    DeckView::const_iterator DeckView::end() const {
        return this->last;
    }

    void DeckView::add( const DeckKeyword* kw, const_iterator f, const_iterator l ) {
        this->keywordMap[ kw->name() ].push_back( std::distance( f, l ) - 1 );
        this->first = f;
        this->last = l;
    }

    static const std::vector< size_t > empty_indices = {};
    const std::vector< size_t >& DeckView::offsets( const std::string& keyword ) const {
        if( !hasKeyword( keyword ) ) return empty_indices;

        return this->keywordMap.find( keyword )->second;
    }

    DeckView::DeckView( const_iterator first_arg, const_iterator last_arg)
    {
        this->init(first_arg, last_arg);
    }

    void DeckView::init( const_iterator first_arg, const_iterator last_arg ) {
        this->first = first_arg;
        this->last = last_arg;

        this->keywordMap.clear();

        size_t index = 0;
        for( const auto& kw : *this )
            this->keywordMap[ kw.name() ].push_back( index++ );
    }

    DeckView::DeckView( std::pair< const_iterator, const_iterator > limits ) :
        DeckView( limits.first, limits.second )
    {}

    Deck::Deck() :
        Deck( std::vector<DeckKeyword>() )
    {}

    /*
      This constructor should be ssen as a technical implemtation detail of the
      default constructor, and not something which should be invoked directly.
      The point is that the derived class DeckView contains iterators to the
      keywordList member in the base class - this represents some ordering
      challenges in the construction phase.
    */
    Deck::Deck( std::vector<DeckKeyword>&& x) :
        DeckView(x.begin(), x.end()),
        keywordList(std::move(x)),
        defaultUnits( UnitSystem::newMETRIC() )
    {
    }

    Deck::Deck( const Deck& d ) :
        DeckView(d.begin(), d.end()),
        keywordList( d.keywordList ),
        defaultUnits( d.defaultUnits ),
        m_dataFile( d.m_dataFile ),
        input_path( d.input_path )
    {
        this->init(this->keywordList.begin(), this->keywordList.end());
        if (d.activeUnits)
            this->activeUnits.reset( new UnitSystem(*d.activeUnits.get()));
        unit_system_access_count = d.unit_system_access_count;
    }

    Deck Deck::serializeObject()
    {
        Deck result;
        result.keywordList = {DeckKeyword::serializeObject()};
        result.defaultUnits = UnitSystem::serializeObject();
        result.activeUnits = std::make_unique<UnitSystem>(UnitSystem::serializeObject());
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
        auto fst = this->keywordList.begin();
        auto lst = this->keywordList.end();
        this->add( &this->keywordList.back(), fst, lst );
    }

    void Deck::addKeyword( const DeckKeyword& keyword ) {
        DeckKeyword kw = keyword;
        this->addKeyword( std::move( kw ) );
    }


    DeckKeyword& Deck::getKeyword( size_t index ) {
        return this->keywordList.at( index );
    }

    UnitSystem& Deck::getDefaultUnitSystem() {
        return this->defaultUnits;
    }

    const UnitSystem& Deck::getDefaultUnitSystem() const {
        return this->defaultUnits;
    }

    UnitSystem& Deck::getActiveUnitSystem() {
        this->unit_system_access_count++;
        if (this->activeUnits)
            return *this->activeUnits;
        else
            return this->defaultUnits;
    }

    void Deck::selectActiveUnitSystem(UnitSystem::UnitType unit_type) {
        const auto& current = this->getActiveUnitSystem();
        if (current.use_count() > 0 && (current.getType() != unit_type))
            throw std::invalid_argument("Sorry - can not change unit system after dimensionfull features have been entered");

        if (current.getType() != unit_type)
            this->activeUnits.reset( new UnitSystem(unit_type) );
    }


    const UnitSystem& Deck::getActiveUnitSystem() const {
        this->unit_system_access_count++;
        if (this->activeUnits)
            return *this->activeUnits;
        else
            return this->defaultUnits;
    }

    const std::string& Deck::getDataFile() const {
        return m_dataFile;
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
        this->m_dataFile = dataFile;

        auto slash_pos = dataFile.find_last_of("/\\");
        if (slash_pos == std::string::npos)
            this->input_path = "";
        else
            this->input_path = dataFile.substr(0, slash_pos);
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
                output.write_string( output.keyword_sep );
        }
    }

    Deck& Deck::operator=(const Deck& data) {
        keywordList = data.keywordList;
        defaultUnits = data.defaultUnits;
        m_dataFile = data.m_dataFile;
        input_path = data.input_path;
        unit_system_access_count = data.unit_system_access_count;
        this->init(this->keywordList.begin(), this->keywordList.end());
        activeUnits.reset();
        if (data.activeUnits)
            activeUnits.reset(new UnitSystem(*data.activeUnits));

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
}
