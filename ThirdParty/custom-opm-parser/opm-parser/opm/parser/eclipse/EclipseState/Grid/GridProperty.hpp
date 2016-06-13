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
#ifndef ECLIPSE_GRIDPROPERTY_HPP_
#define ECLIPSE_GRIDPROPERTY_HPP_

#include <functional>
#include <string>
#include <vector>

#include <ert/ecl/EclKW.hpp>

/*
  This class implemenents a class representing properties which are
  define over an ECLIPSE grid, i.e. with one value for each logical
  cartesian cell in the grid.
*/

namespace Opm {

    class Box;
    class DeckItem;
    class DeckKeyword;
    class EclipseGrid;
    class TableManager;
    template< typename > class GridProperties;

template< typename T >
class GridPropertySupportedKeywordInfo {

    public:
        GridPropertySupportedKeywordInfo() = default;

        using init = std::function< std::vector< T >( size_t ) >;
        using post = std::function< void( std::vector< T >& ) >;

        GridPropertySupportedKeywordInfo(
            const std::string& name,
            init initializer,
            post  postProcessor,
            const std::string& dimString );

        GridPropertySupportedKeywordInfo(
                const std::string& name,
                init initializer,
                const std::string& dimString);

        /* this is a convenience constructor which can be used if the default
         * value for the grid property is just a constant.
         */
        GridPropertySupportedKeywordInfo(
                const std::string& name,
                const T defaultValue,
                const std::string& dimString );

        GridPropertySupportedKeywordInfo(
                const std::string& name,
                const T defaultValue,
                post postProcessor,
                const std::string& dimString );

        const std::string& getKeywordName() const;
        const std::string& getDimensionString() const;
        const init& initializer() const;
        const post& postProcessor() const;

    private:

        std::string m_keywordName;
        init m_initializer;
        post m_postProcessor;
        std::string m_dimensionString;
};

template< typename T >
class GridProperty {
public:
    typedef GridPropertySupportedKeywordInfo<T> SupportedKeywordInfo;

    GridProperty( size_t nx, size_t ny, size_t nz, const SupportedKeywordInfo& kwInfo );

    size_t getCartesianSize() const;
    size_t getNX() const;
    size_t getNY() const;
    size_t getNZ() const;


    T iget(size_t index) const;
    T iget(size_t i , size_t j , size_t k) const;
    void iset(size_t index, T value);
    void iset(size_t i , size_t j , size_t k , T value);

    const std::vector<T>& getData() const;

    bool containsNaN() const;
    const std::string& getDimensionString() const;

    void multiplyWith( const GridProperty<T>& );
    void multiplyValueAtIndex( size_t index, T factor );
    void maskedSet( T value, const std::vector< bool >& mask );
    void maskedMultiply( T value, const std::vector< bool >& mask );
    void maskedAdd( T value, const std::vector< bool >& mask );
    void maskedCopy( const GridProperty< T >& other, const std::vector< bool >& mask );
    void initMask( T value, std::vector<bool>& mask ) const;

    /**
       Due to the convention where it is only necessary to supply the
       top layer of the petrophysical properties we can unfortunately
       not enforce that the number of elements elements in the
       DeckKeyword equals nx*ny*nz.
    */

    void loadFromDeckKeyword( const DeckKeyword& );
    void loadFromDeckKeyword( const Box&, const DeckKeyword& );

    void copyFrom( const GridProperty< T >&, const Box& );
    void scale( T scaleFactor, const Box& );
    void add( T shiftValue, const Box& );
    void setScalar( T value, const Box& );

    const std::string& getKeywordName() const;
    const SupportedKeywordInfo& getKeywordInfo() const;

    ERT::EclKW<T> getEclKW() const;
    ERT::EclKW<T> getEclKW( const EclipseGrid & ) const;

    /**
       Will check that all elements in the property are in the closed
       interval [min,max].
    */
    void checkLimits( T min, T max ) const;


private:
    const DeckItem& getDeckItem( const DeckKeyword& );
    void setDataPoint(size_t sourceIdx, size_t targetIdx, const DeckItem& deckItem);

    void runPostProcessor();

    friend class Eclipse3DProperties; // currently needed for runPostProcessor, see Eclipse3DProperties::getDoubleGridProperty

    size_t m_nx, m_ny, m_nz;
    SupportedKeywordInfo m_kwInfo;
    std::vector<T> m_data;
    bool m_hasRunPostProcessor = false;
};

// initialize the TEMPI grid property using the temperature vs depth
// table (stemming from the TEMPVD or the RTEMPVD keyword)
std::vector< double > temperature_lookup( size_t,
                                            const TableManager*,
                                            const EclipseGrid*,
                                            GridProperties<int>* );
}

#endif
