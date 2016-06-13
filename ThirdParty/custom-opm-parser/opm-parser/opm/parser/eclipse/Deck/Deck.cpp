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

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <vector>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
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

    DeckView::DeckView( const_iterator first_arg, const_iterator last_arg ) :
        first( first_arg ), last( last_arg )
    {
        size_t index = 0;
        for( const auto& kw : *this )
            this->keywordMap[ kw.name() ].push_back( index++ );
    }

    DeckView::DeckView( std::pair< const_iterator, const_iterator > limits ) :
        DeckView( limits.first, limits.second )
    {}

    Deck::Deck() :
        Deck( std::vector< DeckKeyword >() )
    {
        m_dataFile = "";
    }

    Deck::Deck( std::vector< DeckKeyword >&& x ) :
        DeckView( x.begin(), x.end() ),
        keywordList( std::move( x ) ),
        m_dataFile("")
    {}

    void Deck::addKeyword( DeckKeyword&& keyword ) {
        this->keywordList.push_back( std::move( keyword ) );

        auto first = this->keywordList.begin();
        auto last = this->keywordList.end();

        this->add( &this->keywordList.back(), first, last );
    }

    void Deck::addKeyword( const DeckKeyword& keyword ) {
        DeckKeyword kw = keyword;
        this->addKeyword( std::move( kw ) );
    }


    DeckKeyword& Deck::getKeyword( size_t index ) {
        return this->keywordList.at( index );
    }

    MessageContainer&  Deck::getMessageContainer() const {
        return this->m_messageContainer;
    }

    
    UnitSystem& Deck::getDefaultUnitSystem() {
        if( !this->defaultUnits ) this->initUnitSystem();
        return *this->defaultUnits;
    }

    UnitSystem& Deck::getActiveUnitSystem() {
        if( !this->activeUnits ) this->initUnitSystem();
        return *this->activeUnits;
    }

    const UnitSystem& Deck::getDefaultUnitSystem() const {
        if( !this->defaultUnits ) this->initUnitSystem();
        return *this->defaultUnits;
    }

    const UnitSystem& Deck::getActiveUnitSystem() const {
        if( !this->activeUnits ) this->initUnitSystem();
        return *this->activeUnits;
    }

    void Deck::initUnitSystem() const {
        /*
         * The unit systems are lazily created as their exact value depend on
         * input values of the deck, but in a constructed deck this can be
         * considered constant (and in fact, if the deck is obtained through
         * ParseFromString/File, this these values set before the Deck is
         * available). The unit systems are needed from const contexts though,
         * but might not have been generated at that time. Generation is done
         * in this method, but it has to be callable from const'd this, and are
         * marked mutable.
         */

        this->defaultUnits = std::unique_ptr< UnitSystem >( UnitSystem::newMETRIC() );
        if (hasKeyword("FIELD"))
            this->activeUnits = std::unique_ptr< UnitSystem >( UnitSystem::newFIELD() );
        else
            this->activeUnits = std::unique_ptr< UnitSystem >( UnitSystem::newMETRIC() );
    }

    const std::string Deck::getDataFile() const {
        return m_dataFile;
    }

    void Deck::setDataFile(const std::string& dataFile) {
        m_dataFile = dataFile;
    }

    Deck::iterator Deck::begin() {
        return this->keywordList.begin();
    }

    Deck::iterator Deck::end() {
        return this->keywordList.end();
    }

}
