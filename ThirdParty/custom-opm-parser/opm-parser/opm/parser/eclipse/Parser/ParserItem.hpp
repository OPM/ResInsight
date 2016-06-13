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
#ifndef PARSER_ITEM_H
#define PARSER_ITEM_H

#include <string>
#include <sstream>
#include <iostream>
#include <deque>

#include <memory>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>
#include <opm/parser/eclipse/RawDeck/RawRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/RawDeck/StarToken.hpp>

namespace Json {
    class JsonObject;
}

namespace Opm {

    class ParserItem {
    public:
        ParserItem(const std::string& itemName);
        ParserItem(const std::string& itemName, ParserItemSizeEnum sizeType);
        explicit ParserItem(const Json::JsonObject& jsonConfig);

        virtual void push_backDimension(const std::string& dimension);
        virtual const std::string& getDimension(size_t index) const;
        virtual DeckItem scan( RawRecord& rawRecord) const = 0;
        virtual bool hasDimension() const;
        virtual size_t numDimensions() const;
        const std::string className() const;
        const std::string& name() const;
        ParserItemSizeEnum sizeType() const;
        std::string getDescription() const;
        bool scalar() const;
        void setDescription(std::string helpText);

        virtual std::string createCode() const = 0;
        virtual void inlineClass(std::ostream& /* os */ , const std::string& indent) const = 0;
        virtual std::string inlineClassInit(const std::string& parentClass) const = 0;

        virtual ~ParserItem() {
        }

        virtual bool equal(const ParserItem& other) const = 0;

    protected:
        template <class T>
        bool parserRawItemEqual(const ParserItem &other) const {
            const T * lhs = dynamic_cast<const T*>(this);
            const T * rhs = dynamic_cast<const T*>(&other);
            if (!lhs || !rhs)
                return false;

            if (lhs->name() != rhs->name())
                return false;

            if (lhs->getDescription() != rhs->getDescription())
                return false;

            if (lhs->sizeType() != rhs->sizeType())
                return false;

            if (lhs->m_defaultSet != rhs->m_defaultSet)
                return false;

            // we only care that the default value is equal if it was
            // specified...
            if (lhs->m_defaultSet && lhs->getDefault() != rhs->getDefault())
                return false;

            return true;
        }

        bool m_defaultSet;

    private:
        std::string m_name;
        ParserItemSizeEnum m_sizeType;
        std::string m_description;
    };

    typedef std::shared_ptr<ParserItem> ParserItemPtr;
    typedef std::shared_ptr<const ParserItem> ParserItemConstPtr;



    template<typename ParserItemType, typename ValueType>
    void ParserItemInlineClassDeclaration(const ParserItemType * self , std::ostream& os, const std::string& indent , const std::string& typeString) {
        os << indent << "class " << self->className( ) << " {" << std::endl;
        os << indent << "public:" << std::endl;
        {
            std::string local_indent = indent + "    ";
            os << local_indent << "static const std::string itemName;" << std::endl;
            if (self->hasDefault())
                os << local_indent << "static const " << typeString << " defaultValue;" << std::endl;
        }
        os << indent << "};" << std::endl;
    }


    template<typename ParserItemType, typename ValueType>
    std::string ParserItemInlineClassInit(const ParserItemType * self ,
                                          const std::string& parentClass ,
                                          const std::string& typeString ,
                                          const std::string * defaultValue = NULL) {

        std::stringstream ss;
        ss << "const std::string " << parentClass << "::" << self->className() << "::itemName = \"" << self->name() << "\";" << std::endl;

        if (self->hasDefault()) {
            if (defaultValue)
                ss << "const " << typeString << " " << parentClass << "::" << self->className() << "::defaultValue = " << *defaultValue << ";" << std::endl;
            else
                ss << "const " << typeString << " " << parentClass << "::" << self->className() << "::defaultValue = " << self->getDefault() << ";" << std::endl;
        }

        return ss.str();
    }





    /// Scans the rawRecords data according to the ParserItems definition.
    /// returns a DeckItem object.
    /// NOTE: data are popped from the rawRecords deque!
    template<typename ParserItemType, typename ValueType>
    DeckItem ParserItemScan(const ParserItemType * self, RawRecord& rawRecord ) {
        auto deckItem = DeckItem::make< ValueType >( self->name(), rawRecord.size() );

        if (self->sizeType() == ALL) {
            while (rawRecord.size() > 0) {
                auto token = rawRecord.pop_front();

                std::string countString;
                std::string valueString;
                if (isStarToken(token, countString, valueString)) {
                    StarToken st(token, countString, valueString);
                    ValueType value;

                    if (st.hasValue()) {
                        value = readValueToken<ValueType>(st.valueString());
                        deckItem.push_back( value , st.count());
                    } else {
                        value = self->getDefault();
                        for (size_t i=0; i < st.count(); i++)
                            deckItem.push_backDefault( value );
                    }
                } else {
                    deckItem.push_back( readValueToken<ValueType>( token ) );
                }
            }
        } else {
            if (rawRecord.size() == 0) {
                // if the record was ended prematurely,
                if (self->hasDefault()) {
                    // use the default value for the item, if there is one...
                    deckItem.push_backDefault( self->getDefault() );
                } else {
                    // ... otherwise indicate that the deck item should throw once the
                    // item's data is accessed.
                    deckItem.push_backDummyDefault();
                }
            } else {
                // The '*' should be interpreted as a repetition indicator, but it must
                // be preceeded by an integer...
                auto token = rawRecord.pop_front();
                std::string countString;
                std::string valueString;
                if (isStarToken(token, countString, valueString)) {
                    StarToken st(token, countString, valueString);

                    if (!st.hasValue()) {
                        if (self->hasDefault())
                            deckItem.push_backDefault( self->getDefault() );
                        else
                            deckItem.push_backDummyDefault();
                    } else
                        deckItem.push_back(readValueToken<ValueType>(st.valueString()));

                    // replace the first occurence of "N*FOO" by a sequence of N-1 times
                    // "1*FOO". this is slightly hacky, but it makes it work if the
                    // number of defaults pass item boundaries...
                    std::string singleRepetition;
                    if (st.hasValue())
                        singleRepetition = st.valueString();
                    else
                        singleRepetition = "1*";

                    for (size_t i=0; i < st.count() - 1; i++)
                        rawRecord.push_front(singleRepetition);
                } else {
                    deckItem.push_back( readValueToken<ValueType>( token ) );
                }
            }
        }

        return deckItem;
    }



}

#endif

