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

#include <boost/lexical_cast.hpp>

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

#include <opm/parser/eclipse/RawDeck/RawRecord.hpp>
#include <opm/parser/eclipse/RawDeck/StarToken.hpp>

namespace Opm
{

    ParserDoubleItem::ParserDoubleItem(const std::string& itemName,
            ParserItemSizeEnum p_sizeType) :
            ParserItem(itemName, p_sizeType)
    {
        // use NaN as 'default default'. (Keep in mind that in the deck it can be queried
        // using deckItem->defaultApplied(idx) if an item was defaulted or not...
        m_default = std::numeric_limits<double>::quiet_NaN();
    }

    ParserDoubleItem::ParserDoubleItem(const std::string& itemName)
        : ParserItem(itemName)
    {
        m_default = std::numeric_limits<double>::quiet_NaN();
        m_defaultSet = false;
    }


    ParserDoubleItem::ParserDoubleItem(const std::string& itemName, double defaultValue)
        : ParserItem(itemName)
    {
        setDefault( defaultValue );
    }


    ParserDoubleItem::ParserDoubleItem(const std::string& itemName, ParserItemSizeEnum p_sizeType, double defaultValue)
        : ParserItem(itemName, p_sizeType)
    {
        setDefault( defaultValue );
    }


    double ParserDoubleItem::getDefault() const {
        if (hasDefault())
            return m_default;

        if (sizeType() == Opm::ALL)
            return std::numeric_limits<double>::quiet_NaN();

        throw std::invalid_argument("No default value available for item "+name());
    }


    void ParserDoubleItem::setDefault(double defaultValue) {
        m_default = defaultValue;
        m_defaultSet = true;
    }

    bool ParserDoubleItem::hasDefault() const {
        return m_defaultSet;
    }

    ParserDoubleItem::ParserDoubleItem(const Json::JsonObject& jsonConfig) :
            ParserItem(jsonConfig)
    {
        m_default = std::numeric_limits<double>::quiet_NaN();
        if (jsonConfig.has_item("default"))
            setDefault( jsonConfig.get_double("default") );
    }

    bool ParserDoubleItem::equal(const ParserItem& other) const {
        return parserRawItemEqual<ParserDoubleItem>(other) && equalDimensions(other);
    }


    bool ParserDoubleItem::equalDimensions(const ParserItem& other) const {
        bool equal_=false;
        if (other.numDimensions() == numDimensions()) {
            equal_ = true;
            for (size_t idim=0; idim < numDimensions(); idim++) {
                if (other.getDimension(idim) != getDimension(idim))
                    equal_ = false;
            }
        }
        return equal_;
    }

    void ParserDoubleItem::push_backDimension(const std::string& dimension) {
        if ((sizeType() == SINGLE) && (m_dimensions.size() > 0))
            throw std::invalid_argument("Internal error: cannot add more than one dimension to an item of size 1");

        m_dimensions.push_back( dimension );
    }

    bool ParserDoubleItem::hasDimension() const {
        return (m_dimensions.size() > 0);
    }

    size_t ParserDoubleItem::numDimensions() const {
        return m_dimensions.size();
    }

    const std::string& ParserDoubleItem::getDimension(size_t index) const {
        if (index < m_dimensions.size())
            return m_dimensions[index];
        else
            throw std::invalid_argument("Invalid index ");
    }


    DeckItem ParserDoubleItem::scan( RawRecord& rawRecord ) const {
        return ParserItemScan<ParserDoubleItem,double>(this , rawRecord);
    }

    std::string ParserDoubleItem::createCode() const {
        std::stringstream ss;

        ss << "new ParserDoubleItem(" << "\"" << name() << "\"" << ",Opm::" << ParserItemSizeEnum2String( sizeType() );
        if (m_defaultSet)
            ss << "," << boost::lexical_cast<std::string>(getDefault());
        ss << ")";

        return ss.str();
    }



    void ParserDoubleItem::inlineClass(std::ostream& os, const std::string& indent) const {
        ParserItemInlineClassDeclaration<ParserDoubleItem,double>(this , os , indent , "double");
    }

    std::string ParserDoubleItem::inlineClassInit(const std::string& parentClass) const {
        return ParserItemInlineClassInit<ParserDoubleItem,int>(this ,  parentClass , "double");
    }


}

