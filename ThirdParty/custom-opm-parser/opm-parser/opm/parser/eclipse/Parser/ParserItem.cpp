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

#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>


namespace Opm {

    ParserItem::ParserItem(const std::string& itemName, ParserItemSizeEnum p_sizeType) {
        m_name.assign(itemName);
        m_sizeType = p_sizeType;
        m_defaultSet = false;
        m_description = "";
    }


    ParserItem::ParserItem(const std::string& itemName) {
        m_name.assign(itemName);
        m_sizeType = SINGLE;
        m_defaultSet = false;
        m_description = "";
    }

    bool ParserItem::hasDimension() const {
        return false;
    }

    size_t ParserItem::numDimensions() const {
        return 0;
    }

    const std::string& ParserItem::getDimension(size_t /* index */) const {
        throw std::invalid_argument("Should not call this ... \n");
    }

    void ParserItem::push_backDimension(const std::string& /* dimension */) {
        throw std::invalid_argument("Should not call this ... \n");
    }

    ParserItem::ParserItem(const Json::JsonObject& jsonConfig) {
        if (jsonConfig.has_item("name"))
            m_name = jsonConfig.get_string("name");
        else
            throw std::invalid_argument("Json config object missing \"name\": ... item");

        if (jsonConfig.has_item("size_type")) {
            const std::string sizeTypeString = jsonConfig.get_string("size_type");
            m_sizeType = ParserItemSizeEnumFromString( sizeTypeString );
        } else
          m_sizeType = SINGLE;

        if (jsonConfig.has_item("description")) {
            m_description = jsonConfig.get_string("description");
        }

        m_defaultSet = false;
    }

    const std::string& ParserItem::name() const {
        return m_name;
    }

    const std::string ParserItem::className() const {
        return m_name;
    }



    ParserItemSizeEnum ParserItem::sizeType() const {
        return m_sizeType;
    }

    bool ParserItem::scalar() const {
        if (m_sizeType == SINGLE)
            return true;
        else
            return false;
    }

    std::string ParserItem::getDescription() const {
        return m_description;
    }

    void ParserItem::setDescription(std::string description) {
        m_description = description;
    }
}
