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

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/RawDeck/StarToken.hpp>
namespace Opm {

    ParserStringItem::ParserStringItem(const std::string& itemName) : ParserItem(itemName)
    {
        m_default = "";
        m_defaultSet = false;
    }


    ParserStringItem::ParserStringItem(const std::string& itemName, ParserItemSizeEnum sizeType_) : ParserItem(itemName, sizeType_)
    {
        m_default = "";
        m_defaultSet = false;
    }

    ParserStringItem::ParserStringItem(const std::string& itemName, ParserItemSizeEnum sizeType_, const std::string& defaultValue) : ParserItem(itemName, sizeType_) {
        setDefault(defaultValue);
    }


    ParserStringItem::ParserStringItem(const std::string& itemName, const std::string& defaultValue) : ParserItem(itemName) {
        setDefault(defaultValue);
    }


    ParserStringItem::ParserStringItem(const Json::JsonObject& jsonConfig) : ParserItem(jsonConfig) {
        m_default = "";
        if (jsonConfig.has_item("default"))
            setDefault( jsonConfig.get_string("default") );
    }



    void ParserStringItem::setDefault(const std::string& defaultValue) {
        if (sizeType() == ALL)
            throw std::invalid_argument("The size type ALL can not be combined with an explicit default value");

        m_default = defaultValue;
        m_defaultSet = true;
    }

    std::string ParserStringItem::getDefault() const {
        if (hasDefault())
            return m_default;

        if (sizeType() == Opm::ALL)
            return "";

        throw std::invalid_argument("No default value available for item "+name());
    }

    bool ParserStringItem::hasDefault() const {
        return m_defaultSet;
    }

    DeckItem ParserStringItem::scan( RawRecord& rawRecord ) const {
        return ParserItemScan<ParserStringItem,std::string>(this , rawRecord);
    }



    bool ParserStringItem::equal(const ParserItem& other) const
    {
        return parserRawItemEqual<ParserStringItem>(other);
    }

    std::string ParserStringItem::createCode() const {
        std::stringstream ss;

        ss << "new ParserStringItem(" << "\"" << name() << "\"" << ",Opm::" << ParserItemSizeEnum2String( sizeType() );
        if (m_defaultSet)
            ss << ",\"" << getDefault() << "\"";
        ss << ")";

        return ss.str();
    }




    void ParserStringItem::inlineClass(std::ostream& os , const std::string& indent) const {
        ParserItemInlineClassDeclaration<ParserStringItem,std::string>(this , os , indent , "std::string");
    }


    std::string ParserStringItem::inlineClassInit(const std::string& parentClass) const {
        if (hasDefault()) {
            std::string quotedDefault = "\"" + getDefault() + "\"";
            return ParserItemInlineClassInit<ParserStringItem,std::string>(this ,  parentClass , "std::string" , &quotedDefault);
        } else
            return ParserItemInlineClassInit<ParserStringItem,std::string>(this ,  parentClass , "std::string");
    }



}
