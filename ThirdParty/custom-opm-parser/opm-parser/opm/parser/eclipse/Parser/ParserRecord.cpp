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

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm {

    ParserRecord::ParserRecord()
        : m_dataRecord( false )
    {
    }

    size_t ParserRecord::size() const {
        return m_items.size();
    }

    void ParserRecord::addItem(ParserItemConstPtr item) {
        if (m_dataRecord)
            throw std::invalid_argument("Record is already marked as DataRecord - can not add items");

        if (m_itemMap.find(item->name()) == m_itemMap.end()) {
            m_items.push_back(item);
            m_itemMap[item->name()] = item;
        } else
            throw std::invalid_argument("Itemname: " + item->name() + " already exists.");
    }

    void ParserRecord::addDataItem(ParserItemConstPtr item) {
        if (m_items.size() > 0)
            throw std::invalid_argument("Record already contains items - can not add Data Item");

        addItem(item);
        m_dataRecord = true;
    }



    std::vector<ParserItemConstPtr>::const_iterator ParserRecord::begin() const {
        return m_items.begin();
    }


    std::vector<ParserItemConstPtr>::const_iterator ParserRecord::end() const {
        return m_items.end();
    }


    bool ParserRecord::hasDimension() const {
        bool hasDim = false;
        for (auto iter=begin(); iter != end(); ++iter) {
            if ((*iter)->hasDimension())
                hasDim = true;
        }
        return hasDim;
    }



    void ParserRecord::applyUnitsToDeck( Deck& deck, DeckRecord& deckRecord ) const {
        for (auto iter=begin(); iter != end(); ++iter) {
            if ((*iter)->hasDimension()) {
                auto& deckItem = deckRecord.getItem( (*iter)->name() );
                std::shared_ptr<const ParserItem> parserItem = get( (*iter)->name() );

                for (size_t idim=0; idim < (*iter)->numDimensions(); idim++) {
                    std::shared_ptr<const Dimension> activeDimension  = deck.getActiveUnitSystem().getNewDimension( parserItem->getDimension(idim) );
                    std::shared_ptr<const Dimension> defaultDimension = deck.getDefaultUnitSystem().getNewDimension( parserItem->getDimension(idim) );
                    deckItem.push_backDimension( activeDimension , defaultDimension );
                }
            }
        }
    }


    ParserItemConstPtr ParserRecord::get(size_t index) const {
        if (index < m_items.size())
            return m_items[ index ];
        else
            throw std::out_of_range("Out of range");
    }

    bool ParserRecord::hasItem(const std::string& itemName) const {
        if (m_itemMap.find(itemName) == m_itemMap.end())
            return false;
        else
            return true;
    }

    ParserItemConstPtr ParserRecord::get(const std::string& itemName) const {
        if (m_itemMap.find(itemName) == m_itemMap.end())
            throw std::invalid_argument("Itemname: " + itemName + " does not exist.");
        else {
            std::map<std::string, ParserItemConstPtr>::const_iterator theItem = m_itemMap.find(itemName);
            return (*theItem).second;
        }
    }

    DeckRecord ParserRecord::parse(const ParseContext& parseContext , MessageContainer& msgContainer, RawRecord& rawRecord ) const {
        std::vector< DeckItem > items;
        items.reserve( this->size() + 20 );
        for( const auto& parserItem : *this )
            items.emplace_back( parserItem->scan( rawRecord ) );

        if (rawRecord.size() > 0) {
            std::string msg = "The RawRecord for keyword \""  + rawRecord.getKeywordName() + "\" in file\"" + rawRecord.getFileName() + "\" contained " +
                std::to_string(rawRecord.size()) +
                " too many items according to the spec. RawRecord was: " + rawRecord.getRecordString();
            parseContext.handleError(ParseContext::PARSE_EXTRA_DATA , msgContainer, msg);
        }

        return { std::move( items ) };
    }

    bool ParserRecord::equal(const ParserRecord& other) const {
        bool equal_ = true;
        if (size() == other.size()) {
           size_t itemIndex = 0;
           while (true) {
               if (itemIndex == size())
                   break;
               {
                   ParserItemConstPtr item = get(itemIndex);
                   ParserItemConstPtr otherItem = other.get(itemIndex);

                   if (!item->equal(*otherItem)) {
                       equal_ = false;
                       break;
                   }
               }
               itemIndex++;
            }
        } else
            equal_ = false;
        return equal_;
    }

    bool ParserRecord::isDataRecord() const {
        return m_dataRecord;
    }


}
