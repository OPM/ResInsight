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

#ifndef PARSERDOUBLEITEM_HPP
#define PARSERDOUBLEITEM_HPP

#include <memory>

#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

namespace Json {
    class JsonObject;
}

namespace Opm {

    class RawRecord;

    class ParserDoubleItem : public ParserItem {
    public:
        ParserDoubleItem(const std::string& itemName);
        ParserDoubleItem(const std::string& itemName, ParserItemSizeEnum sizeType);
        ParserDoubleItem(const std::string& itemName, double defaultValue);
        ParserDoubleItem(const std::string& itemName, ParserItemSizeEnum sizeType, double defaultValue);
        explicit ParserDoubleItem( const Json::JsonObject& jsonConfig);

        size_t numDimensions() const override;
        bool hasDimension() const override;
        void push_backDimension(const std::string& dimension) override;
        const std::string& getDimension(size_t index) const override;
        bool equalDimensions(const ParserItem& other) const;

        DeckItem scan( RawRecord& rawRecord ) const override;
        bool equal(const ParserItem& other) const override;

        std::string createCode() const override;
        void inlineClass(std::ostream& os, const std::string& indent) const override;
        std::string inlineClassInit(const std::string& parentClass) const override;
        void setDefault(double defaultValue);
        double getDefault() const;
        bool hasDefault() const;

        size_t dimensionSize() const;

    private:
        double m_default;
        std::vector<std::string> m_dimensions;
    };

    typedef std::shared_ptr<const ParserDoubleItem> ParserDoubleItemConstPtr;
    typedef std::shared_ptr<ParserDoubleItem> ParserDoubleItemPtr;
}

#endif  /* PARSERINTITEM_HPP */

