/*
  Copyright 2014 Statoil ASA.

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

#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

namespace Opm {

    template< typename T >
    static std::function< std::vector< T >( size_t ) > constant( T val ) {
        return [=]( size_t size ) { return std::vector< T >( size, val ); };
    };

    template< typename T >
    static std::function< void( std::vector< T >& ) > noop() {
        return []( std::vector< T >& ) { return; };
    };

    template< typename T >
    GridPropertySupportedKeywordInfo< T >::GridPropertySupportedKeywordInfo(
            const std::string& name,
            std::function< std::vector< T >( size_t ) > initializer,
            std::function< void( std::vector< T >& ) > postProcessor,
            const std::string& dimString ) :
        m_keywordName( name ),
        m_initializer( initializer ),
        m_postProcessor( postProcessor ),
        m_dimensionString( dimString )
    {}

    template< typename T >
    GridPropertySupportedKeywordInfo< T >::GridPropertySupportedKeywordInfo(
            const std::string& name,
            std::function< std::vector< T >( size_t ) > initializer,
            const std::string& dimString ) :
        m_keywordName( name ),
        m_initializer( initializer ),
        m_postProcessor( noop< T >() ),
        m_dimensionString( dimString )
    {}

    template< typename T >
    GridPropertySupportedKeywordInfo< T >::GridPropertySupportedKeywordInfo(
            const std::string& name,
            const T defaultValue,
            const std::string& dimString ) :
        m_keywordName( name ),
        m_initializer( constant( defaultValue ) ),
        m_postProcessor( noop< T >() ),
        m_dimensionString( dimString )
    {}

    template< typename T >
    GridPropertySupportedKeywordInfo< T >::GridPropertySupportedKeywordInfo(
            const std::string& name,
            const T defaultValue,
            std::function< void( std::vector< T >& ) > postProcessor,
            const std::string& dimString ) :
        m_keywordName( name ),
        m_initializer( constant( defaultValue ) ),
        m_postProcessor( postProcessor ),
        m_dimensionString( dimString )
    {}

    template< typename T >
    const std::string& GridPropertySupportedKeywordInfo< T >::getKeywordName() const {
        return this->m_keywordName;
    }

    template< typename T >
    const std::string& GridPropertySupportedKeywordInfo< T >::getDimensionString() const {
        return this->m_dimensionString;
    }

    template< typename T >
    const std::function< std::vector< T >( size_t ) >& GridPropertySupportedKeywordInfo< T >::initializer() const {
        return this->m_initializer;
    }

    template< typename T >
    const std::function< void( std::vector< T >& ) >& GridPropertySupportedKeywordInfo< T >::postProcessor() const {
        return this->m_postProcessor;
    }

    template< typename T >
    GridProperty< T >::GridProperty( size_t nx, size_t ny, size_t nz, const SupportedKeywordInfo& kwInfo ) :
        m_nx( nx ),
        m_ny( ny ),
        m_nz( nz ),
        m_kwInfo( kwInfo ),
        m_data( kwInfo.initializer()( nx * ny * nz ) ),
        m_hasRunPostProcessor( false )
    {}

    template< typename T >
    size_t GridProperty< T >::getCartesianSize() const {
        return m_data.size();
    }

    template< typename T >
    size_t GridProperty< T >::getNX() const {
        return m_nx;
    }

    template< typename T >
    size_t GridProperty< T >::getNY() const {
        return m_ny;
    }

    template< typename T >
    size_t GridProperty< T >::getNZ() const {
        return m_nz;
    }

    template< typename T >
    T GridProperty< T >::iget( size_t index ) const {
        if (index < m_data.size()) {
            return m_data[index];
        } else {
            throw std::invalid_argument("Index out of range \n");
        }
    }

    template< typename T >
    T GridProperty< T >::iget(size_t i , size_t j , size_t k) const {
        size_t g = i + j*m_nx + k*m_nx*m_ny;
        return iget(g);
    }

    template< typename T >
    void GridProperty< T >::iset(size_t index, T value) {
        if (index < m_data.size())
            m_data[index] = value;
        else
            throw std::invalid_argument("Index out of range \n");
    }

    template< typename T >
    void GridProperty< T >::iset(size_t i , size_t j , size_t k , T value) {
        size_t g = i + j*m_nx + k*m_nx*m_ny;
        iset(g,value);
    }

    template< typename T >
    const std::vector< T >& GridProperty< T >::getData() const {
        return m_data;
    }

    template< typename T >
    void GridProperty< T >::multiplyWith( const GridProperty< T >& other ) {
        if ((m_nx == other.m_nx) && (m_ny == other.m_ny) && (m_nz == other.m_nz)) {
            for (size_t g=0; g < m_data.size(); g++)
                m_data[g] *= other.m_data[g];
        } else
            throw std::invalid_argument("Size mismatch between properties in mulitplyWith.");
    }

    template< typename T >
    void GridProperty< T >::multiplyValueAtIndex(size_t index, T factor) {
        m_data[index] *= factor;
    }

    template< typename T >
    void GridProperty< T >::maskedSet( T value, const std::vector< bool >& mask ) {
        for (size_t g = 0; g < getCartesianSize(); g++) {
            if (mask[g])
                m_data[g] = value;
        }
    }

    template< typename T >
    void GridProperty< T >::maskedMultiply( T value, const std::vector<bool>& mask ) {
        for (size_t g = 0; g < getCartesianSize(); g++) {
            if (mask[g])
                m_data[g] *= value;
        }
    }


    template< typename T >
    void GridProperty< T >::maskedAdd( T value, const std::vector<bool>& mask ) {
        for (size_t g = 0; g < getCartesianSize(); g++) {
            if (mask[g])
                m_data[g] += value;
        }
    }

    template< typename T >
    void GridProperty< T >::maskedCopy( const GridProperty< T >& other, const std::vector< bool >& mask) {
        for (size_t g = 0; g < getCartesianSize(); g++) {
            if (mask[g])
                m_data[g] = other.m_data[g];
        }
    }

    template< typename T >
    void GridProperty< T >::initMask( T value, std::vector< bool >& mask ) const {
        mask.resize(getCartesianSize());
        for (size_t g = 0; g < getCartesianSize(); g++) {
            if (m_data[g] == value)
                mask[g] = true;
            else
                mask[g] = false;
        }
    }

    template< typename T >
    void GridProperty< T >::loadFromDeckKeyword( const DeckKeyword& deckKeyword ) {
        const auto& deckItem = getDeckItem(deckKeyword);
        const auto size = deckItem.size();
        for (size_t dataPointIdx = 0; dataPointIdx < size; ++dataPointIdx) {
            if (!deckItem.defaultApplied(dataPointIdx))
                setDataPoint(dataPointIdx, dataPointIdx, deckItem);
        }
    }

    template< typename T >
    void GridProperty< T >::loadFromDeckKeyword( const Box& inputBox, const DeckKeyword& deckKeyword) {
        if (inputBox.isGlobal())
            loadFromDeckKeyword( deckKeyword );
        else {
            const auto& deckItem = getDeckItem(deckKeyword);
            const std::vector<size_t>& indexList = inputBox.getIndexList();
            if (indexList.size() == deckItem.size()) {
                for (size_t sourceIdx = 0; sourceIdx < indexList.size(); sourceIdx++) {
                    size_t targetIdx = indexList[sourceIdx];
                    if (sourceIdx < deckItem.size()
                        && !deckItem.defaultApplied(sourceIdx))
                        {
                            setDataPoint(sourceIdx, targetIdx, deckItem);
                        }
                }
            } else {
                std::string boxSize = std::to_string(static_cast<long long>(indexList.size()));
                std::string keywordSize = std::to_string(static_cast<long long>(deckItem.size()));

                throw std::invalid_argument("Size mismatch: Box:" + boxSize + "  DeckKeyword:" + keywordSize);
            }
        }
    }

    template< typename T >
    void GridProperty< T >::copyFrom( const GridProperty< T >& src, const Box& inputBox ) {
        if (inputBox.isGlobal()) {
            for (size_t i = 0; i < src.getCartesianSize(); ++i)
                m_data[i] = src.m_data[i];
        } else {
            const std::vector<size_t>& indexList = inputBox.getIndexList();
            for (size_t i = 0; i < indexList.size(); i++) {
                size_t targetIndex = indexList[i];
                m_data[targetIndex] = src.m_data[targetIndex];
            }
        }
    }

    template< typename T >
    void GridProperty< T >::scale( T scaleFactor, const Box& inputBox ) {
        if (inputBox.isGlobal()) {
            for (size_t i = 0; i < m_data.size(); ++i)
                m_data[i] *= scaleFactor;
        } else {
            const std::vector<size_t>& indexList = inputBox.getIndexList();
            for (size_t i = 0; i < indexList.size(); i++) {
                size_t targetIndex = indexList[i];
                m_data[targetIndex] *= scaleFactor;
            }
        }
    }

    template< typename T >
    void GridProperty< T >::add( T shiftValue, const Box& inputBox ) {
        if (inputBox.isGlobal()) {
            for (size_t i = 0; i < m_data.size(); ++i)
                m_data[i] += shiftValue;
        } else {
            const std::vector<size_t>& indexList = inputBox.getIndexList();
            for (size_t i = 0; i < indexList.size(); i++) {
                size_t targetIndex = indexList[i];
                m_data[targetIndex] += shiftValue;
            }
        }
    }

    template< typename T >
    void GridProperty< T >::setScalar( T value, const Box& inputBox ) {
        if (inputBox.isGlobal()) {
            std::fill(m_data.begin(), m_data.end(), value);
        } else {
            const std::vector<size_t>& indexList = inputBox.getIndexList();
            for (size_t i = 0; i < indexList.size(); i++) {
                size_t targetIndex = indexList[i];
                m_data[targetIndex] = value;
            }
        }
    }

    template< typename T >
    const std::string& GridProperty< T >::getKeywordName() const {
        return m_kwInfo.getKeywordName();
    }

    template< typename T >
    const typename GridProperty< T >::SupportedKeywordInfo&
    GridProperty< T >::getKeywordInfo() const {
        return m_kwInfo;
    }

    template< typename T >
    void GridProperty< T >::runPostProcessor() {
        if( this->m_hasRunPostProcessor ) return;
        this->m_hasRunPostProcessor = true;
        this->m_kwInfo.postProcessor()( m_data );
    }

    template< typename T >
    void GridProperty< T >::checkLimits( T min, T max ) const {
        for (size_t g=0; g < m_data.size(); g++) {
            T value = m_data[g];
            if ((value < min) || (value > max))
                throw std::invalid_argument("Property element " + std::to_string( value) + " in " + getKeywordName() + " outside valid limits: [" + std::to_string(min) + ", " + std::to_string(max) + "]");
        }
    }

    template< typename T  >
    const DeckItem& GridProperty< T >::getDeckItem( const DeckKeyword& deckKeyword ) {
        if (deckKeyword.size() != 1)
            throw std::invalid_argument("Grid properties can only have a single record (keyword "
                                        + deckKeyword.name() + ")");
        if (deckKeyword.getRecord(0).size() != 1)
            // this is an error of the definition of the ParserKeyword (most likely in
            // the corresponding JSON file)
            throw std::invalid_argument("Grid properties may only exhibit a single item  (keyword "
                                        + deckKeyword.name() + ")");

        const auto& deckItem = deckKeyword.getRecord(0).getItem(0);

        if (deckItem.size() > m_data.size())
            throw std::invalid_argument("Size mismatch when setting data for:" + getKeywordName()
                                        + " keyword size: " + std::to_string( deckItem.size() )
                                        + " input size: " + std::to_string( m_data.size()) );

        return deckItem;
    }

template<>
void GridProperty<int>::setDataPoint(size_t sourceIdx, size_t targetIdx, const DeckItem& deckItem) {
    m_data[targetIdx] = deckItem.get< int >(sourceIdx);
}

template<>
void GridProperty<double>::setDataPoint(size_t sourceIdx, size_t targetIdx, const DeckItem& deckItem) {
    m_data[targetIdx] = deckItem.getSIDouble(sourceIdx);
}

template<>
bool GridProperty<int>::containsNaN( ) const {
    throw std::logic_error("Only <double> and can be meaningfully queried for nan");
}

template<>
bool GridProperty<double>::containsNaN( ) const {
    bool return_value = false;
    size_t size = m_data.size();
    size_t index = 0;
    while (true) {
        if (std::isnan(m_data[index])) {
            return_value = true;
            break;
        }

        index++;
        if (index == size)
            break;
    }
    return return_value;
}

template<>
const std::string& GridProperty<int>::getDimensionString() const {
    throw std::logic_error("Only <double> grid properties have dimension");
}

template<>
const std::string& GridProperty<double>::getDimensionString() const {
    return m_kwInfo.getDimensionString();
}


std::vector< double > temperature_lookup( size_t size,
                                            const TableManager* tables,
                                            const EclipseGrid* grid,
                                            const GridProperties<int>* ig_props ) {

    if( !tables->useEqlnum() ) {
        /* if values are defaulted in the TEMPI keyword, but no
            * EQLNUM is specified, you will get NaNs
            */
        return std::vector< double >( size, std::numeric_limits< double >::quiet_NaN() );
    }

    std::vector< double > values( size, 0 );

    const auto& rtempvdTables = tables->getRtempvdTables();
    const std::vector< int >& eqlNum = ig_props->getKeyword("EQLNUM").getData();

    for (size_t cellIdx = 0; cellIdx < eqlNum.size(); ++ cellIdx) {
        int cellEquilNum = eqlNum[cellIdx];
        const RtempvdTable& rtempvdTable = rtempvdTables.getTable<RtempvdTable>(cellEquilNum);
        double cellDepth = std::get<2>(grid->getCellCenter(cellIdx));
        values[cellIdx] = rtempvdTable.evaluate("Temperature", cellDepth);
    }

    return values;
}

}

template class Opm::GridPropertySupportedKeywordInfo< int >;
template class Opm::GridPropertySupportedKeywordInfo< double >;

template class Opm::GridProperty< int >;
template class Opm::GridProperty< double >;
