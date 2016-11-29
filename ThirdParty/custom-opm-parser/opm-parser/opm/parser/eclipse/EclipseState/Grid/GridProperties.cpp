/*
  Copyright 2016 Statoil ASA.

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

#include <cmath>

#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/Utility/String.hpp>

namespace Opm {

    /*
      Before lookup the keyword strings are uppercased and trailing
      space is trimmed.
    */

    static std::string normalize(const std::string& keyword) {
        std::string kw(keyword.begin() , std::find( keyword.begin() , keyword.end() , ' '));
        uppercase( kw , kw );
        return kw;
    }


    template <>
    GridProperties<double>::GridProperties(const EclipseGrid& eclipseGrid,
                                           const UnitSystem*  deckUnitSystem,
                                           std::vector< GridProperty<double>::SupportedKeywordInfo >&& supportedKeywords) :
        m_eclipseGrid( eclipseGrid ),
        m_deckUnitSystem( deckUnitSystem )
    {
        for (auto iter = supportedKeywords.begin(); iter != supportedKeywords.end(); ++iter)
            m_supportedKeywords.emplace( iter->getKeywordName(), std::move( *iter ) );
    }


    template <>
    GridProperties<int>::GridProperties(const EclipseGrid& eclipseGrid,
                                        std::vector< GridProperty<int>::SupportedKeywordInfo >&& supportedKeywords) :
        m_eclipseGrid( eclipseGrid )
    {
        for (auto iter = supportedKeywords.begin(); iter != supportedKeywords.end(); ++iter)
            m_supportedKeywords.emplace( iter->getKeywordName(), std::move( *iter ) );
    }




    /*
      In the case of integer properties we never really do any
      transformation, but we have implemented this dummy int
      specializations to enable a uniform implementation.
    */


    template<>
    double GridProperties<double>::convertInputValue(const GridProperty<double>& property, double doubleValue) const {
        const std::string& dimensionString = property.getDimensionString( );
        return m_deckUnitSystem->getDimension(dimensionString)->getSIScaling() * doubleValue;
    }



    template<>
    double GridProperties<double>::convertInputValue( double doubleValue) const {
        return doubleValue;
    }


    template<>
    int GridProperties<int>::convertInputValue(double doubleValue) const {
        if (std::fabs( std::nearbyint( doubleValue ) - doubleValue ) < 1e-6)
            return static_cast<int>( doubleValue );
        else
            throw std::invalid_argument("Expected integer argument - got: " + std::to_string( doubleValue ));
    }

    template<>
    int GridProperties<int>::convertInputValue(const GridProperty<int>& /* property */, double doubleValue) const {
        return convertInputValue(doubleValue);
    }



    void setKeywordBox( const DeckRecord& deckRecord,
                        BoxManager& boxManager) {
        const auto& I1Item = deckRecord.getItem("I1");
        const auto& I2Item = deckRecord.getItem("I2");
        const auto& J1Item = deckRecord.getItem("J1");
        const auto& J2Item = deckRecord.getItem("J2");
        const auto& K1Item = deckRecord.getItem("K1");
        const auto& K2Item = deckRecord.getItem("K2");

        size_t setCount = 0;

        if (!I1Item.defaultApplied(0))
            setCount++;

        if (!I2Item.defaultApplied(0))
            setCount++;

        if (!J1Item.defaultApplied(0))
            setCount++;

        if (!J2Item.defaultApplied(0))
            setCount++;

        if (!K1Item.defaultApplied(0))
            setCount++;

        if (!K2Item.defaultApplied(0))
            setCount++;

        if (setCount == 6) {
            boxManager.setKeywordBox( I1Item.get< int >(0) - 1,
                                      I2Item.get< int >(0) - 1,
                                      J1Item.get< int >(0) - 1,
                                      J2Item.get< int >(0) - 1,
                                      K1Item.get< int >(0) - 1,
                                      K2Item.get< int >(0) - 1);
        } else if (setCount != 0) {
            std::string msg = "BOX modifiers on keywords must be either "
                "specified completely or not at all. Ignoring.";
            throw std::invalid_argument( msg );
        }
    }

    template< typename T >
    const MessageContainer& GridProperties<T>::getMessageContainer() const {
        return m_messages;
    }

    template< typename T >
    MessageContainer& GridProperties<T>::getMessageContainer() {
        return m_messages;
    }


    template< typename T >
    bool GridProperties<T>::supportsKeyword(const std::string& keyword) const {
        return m_supportedKeywords.count( normalize( keyword ) ) > 0;
    }

    template< typename T >
    bool GridProperties<T>::hasKeyword(const std::string& keyword) const {
        const std::string kw = normalize( keyword );

        const auto cnt = m_properties.count( kw );
        const bool positive = cnt > 0;

        return positive && !isAutoGenerated_(kw);
    }



    template< typename T >
    size_t GridProperties<T>::size() const {
        return m_properties.size();
    }


    template< typename T >
    const GridProperty<T>& GridProperties<T>::getKeyword(const std::string& keyword) const {
        const std::string kw = normalize(keyword);

        if (!hasKeyword(kw))
            addAutoGeneratedKeyword_(kw);

        GridProperty<T>& property = m_properties.at( kw );
        property.runPostProcessor( );
        return property;
    }



    template< typename T >
    const GridProperty<T>& GridProperties<T>::getInitializedKeyword(const std::string& keyword) const {
        const std::string kw = normalize(keyword);

        if (hasKeyword(kw))
            return m_properties.at( kw );
        else {
            if (supportsKeyword(kw))
                throw std::invalid_argument("Keyword: " + kw + " is supported - but not initialized.");
            else
                throw std::invalid_argument("Keyword: " + kw + " is not supported.");
        }
    }


    template< typename T >
    void GridProperties<T>::insertKeyword(const SupportedKeywordInfo& supportedKeyword) const {
        int nx = m_eclipseGrid.getNX();
        int ny = m_eclipseGrid.getNY();
        int nz = m_eclipseGrid.getNZ();

        m_properties.emplace( supportedKeyword.getKeywordName() , GridProperty<T>( nx, ny , nz , supportedKeyword ));
    }


    template< typename T >
    bool GridProperties<T>::addKeyword(const std::string& keywordName) {

        if (!supportsKeyword( keywordName ))
            throw std::invalid_argument("The keyword: " + keywordName + " is not supported in this container");

        if (hasKeyword(keywordName))
            return false;
        else {
            const std::string kw = normalize(keywordName);

            // if the property was already added auto generated, we just need to make it
            // non-auto generated
            if (m_autoGeneratedProperties.count(kw)) {
                m_messages.warning("The keyword "+kw+" has been used to calculate the "
                                   "defaults of another keyword before the first time it was "
                                   "explicitly mentioned in the deck. Maybe you need to change "
                                   "the ordering of your keywords (move "+kw+" to the front?).");
                m_autoGeneratedProperties.erase(m_autoGeneratedProperties.find(kw));
                return true;
            }

            insertKeyword( m_supportedKeywords.at( kw ) );
            return true;
        }
    }


    template< typename T >
    void GridProperties<T>::copyKeyword(const std::string& srcField ,
                     const std::string& targetField ,
                     const Box& inputBox) {
        const auto& src = this->getKeyword( srcField );
        auto& target    = this->getOrCreateProperty( targetField );

        target.copyFrom( src , inputBox );
    }




    template< typename T >
    GridProperty<T>& GridProperties<T>::getOrCreateProperty(const std::string& name) {
        if (!hasKeyword(name))
            addKeyword(name);

        return getKeyword(name);
    }

    /**
       The fine print of the manual says the ADD keyword should support
       some state dependent semantics regarding endpoint scaling arrays
       in the PROPS section. That is not supported.
    */

    template< typename T >
    void GridProperties<T>::handleADDRecord( const DeckRecord& record, BoxManager& boxManager) {
        const std::string& field = record.getItem("field").get< std::string >(0);

        if (hasKeyword( field )) {
            GridProperty<T>& property = getKeyword( field );
            T shiftValue  = convertInputValue( property , record.getItem("shift").get< double >(0) );
            setKeywordBox(record, boxManager);
            property.add( shiftValue , *boxManager.getActiveBox() );
        } else
            throw std::invalid_argument("Fatal error processing ADD keyword. Tried to shift not defined keyword " + field);
    }

    template< typename T >
    void GridProperties<T>::handleMULTIPLYRecord( const DeckRecord& record, BoxManager& boxManager) {
        const std::string& field = record.getItem("field").get< std::string >(0);

        if (hasKeyword( field )) {
            GridProperty<T>& property = getKeyword( field );
            T factor  = convertInputValue( record.getItem("factor").get< double >(0) );
            setKeywordBox(record, boxManager);
            property.scale( factor , *boxManager.getActiveBox() );
        } else
            throw std::invalid_argument("Fatal error processing ADD keyword. Tried to shift not defined keyword " + field);
    }


    template< typename T >
    void GridProperties<T>::handleCOPYRecord( const DeckRecord& record, BoxManager& boxManager) {
        const std::string& srcField = record.getItem("src").get< std::string >(0);
        const std::string& targetField = record.getItem("target").get< std::string >(0);

        if (hasKeyword( srcField )) {
            setKeywordBox(record, boxManager);
            copyKeyword( srcField , targetField , *boxManager.getActiveBox() );
        } else {
            if (!supportsKeyword( srcField))
                throw std::invalid_argument("Fatal error processing COPY keyword."
                                            " Tried to copy from not defined keyword " + srcField);
        }
    }

    template< typename T >
    void GridProperties<T>::handleEQUALSRecord( const DeckRecord& record, BoxManager& boxManager) {
        const std::string& field = record.getItem("field").get< std::string >(0);
        double      value  = record.getItem("value").get< double >(0);

        if (supportsKeyword( field )) {
            GridProperty<T>& property = getOrCreateProperty( field );
            T targetValue = convertInputValue( property , value );

            setKeywordBox(record, boxManager);
            property.setScalar( targetValue , *boxManager.getActiveBox() );
        } else
            throw std::invalid_argument("Fatal error processing EQUALS keyword. Tried to set not defined keyword " + field);
    }


    template< typename T >
    void GridProperties<T>::handleEQUALREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty ) {
        const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
        if (supportsKeyword( targetArray )) {
            GridProperty<T>& targetProperty = getOrCreateProperty( targetArray  );
            double inputValue = record.getItem("VALUE").get<double>(0);
            int regionValue = record.getItem("REGION_NUMBER").get<int>(0);
            T targetValue = convertInputValue( targetProperty , inputValue );
            std::vector<bool> mask;

            regionProperty.initMask( regionValue , mask);
            targetProperty.maskedSet( targetValue , mask);
        } else
            throw std::invalid_argument("Fatal error processing EQUALREG record - invalid/undefined keyword: " + targetArray);
    }

    template< typename T >
    void GridProperties<T>::handleADDREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty ) {
        const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
        if (hasKeyword( targetArray )) {
            GridProperty<T>& targetProperty = getKeyword( targetArray  );
            double inputValue = record.getItem("SHIFT").get<double>(0);
            int regionValue = record.getItem("REGION_NUMBER").get<int>(0);
            T shiftValue = convertInputValue( targetProperty , inputValue );
            std::vector<bool> mask;

            regionProperty.initMask( regionValue , mask);
            targetProperty.maskedAdd( shiftValue , mask);
        } else
            throw std::invalid_argument("Fatal error processing ADDREG record - invalid/undefined keyword: " + targetArray);
    }

    template< typename T >
    void GridProperties<T>::handleMULTIREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty ) {
        const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
        if (supportsKeyword( targetArray )) {
            GridProperty<T>& targetProperty = getOrCreateProperty( targetArray  );
            double inputValue = record.getItem("FACTOR").get<double>(0);
            int regionValue = record.getItem("REGION_NUMBER").get<int>(0);
            T factor = convertInputValue( inputValue );
            std::vector<bool> mask;

            regionProperty.initMask( regionValue , mask);
            targetProperty.maskedMultiply( factor , mask);
        } else
            throw std::invalid_argument("Fatal error processing MULTIREG record - invalid/undefined keyword: " + targetArray);
    }

    template< typename T >
    void GridProperties<T>::handleCOPYREGRecord( const DeckRecord& record, const GridProperty<int>& regionProperty ) {
        const std::string& srcArray    = record.getItem("ARRAY").get< std::string >(0);
        const std::string& targetArray = record.getItem("TARGET_ARRAY").get< std::string >(0);

        if (!supportsKeyword( targetArray))
            throw std::invalid_argument("Fatal error processing COPYREG record - invalid/undefined keyword: " + targetArray);

        if (!hasKeyword( srcArray ))
            throw std::invalid_argument("Fatal error processing COPYREG record - invalid/undefined keyword: " + srcArray);

        {
            int regionValue = record.getItem("REGION_NUMBER").get< int >(0);
            std::vector<bool> mask;
            GridProperty<T>& targetProperty = getOrCreateProperty( targetArray );
            GridProperty<T>& srcProperty = getKeyword( srcArray );

            regionProperty.initMask( regionValue , mask);
            targetProperty.maskedCopy( srcProperty , mask );
        }
    }

    template< typename T >
    void GridProperties<T>::postAddKeyword(const std::string& name,
                                           const T defaultValue,
                                           std::function< void( std::vector< T >& ) > postProcessor,
                                           const std::string& dimString )
    {
        m_supportedKeywords.emplace(name,
                                    SupportedKeywordInfo( name,
                                                          defaultValue,
                                                          postProcessor,
                                                          dimString ));
    }

    template< typename T >
    GridProperty<T>& GridProperties<T>::getKeyword(const std::string& keyword) {
        const std::string kw = normalize(keyword);

        if (!hasKeyword(kw))
            addAutoGeneratedKeyword_(kw);

        return m_properties.at( kw );
    }


    /*
      This is const because of the auto-generation of keywords on get().
    */
    template< typename T >
    bool GridProperties<T>::addAutoGeneratedKeyword_(const std::string& keywordName) const {
        if (!supportsKeyword( keywordName ))
            throw std::invalid_argument("The keyword: " + keywordName + " is not supported in this container");

        if (m_properties.count( keywordName ) > 0)
            return false; // property already exists (if it is auto generated or not doesn't matter)
        else {
            m_autoGeneratedProperties.insert(keywordName);
            insertKeyword( m_supportedKeywords.at( keywordName ) );
            return true;
        }
    }

    template< typename T >
    bool GridProperties<T>::isAutoGenerated_(const std::string& keyword) const {
        return m_autoGeneratedProperties.count(keyword) > 0;
    }

}


template class Opm::GridProperties< int >;
template class Opm::GridProperties< double >;
