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


#ifndef OPM_PARSER_ECLIPSE_GRID_HPP
#define OPM_PARSER_ECLIPSE_GRID_HPP


#include <opm/parser/eclipse/EclipseState/Util/Value.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/MinpvMode.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/PinchMode.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridDims.hpp>

#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

#include <ert/ecl/ecl_grid.h>
#include <ert/util/ert_unique_ptr.hpp>

#include <array>
#include <memory>
#include <vector>

namespace Opm {

    class Deck;
    class ZcornMapper;

    /**
       About cell information and dimension: The actual grid
       information is held in a pointer to an ERT ecl_grid_type
       instance. This pointer must be used for access to all cell
       related properties, including:

         - Size of cells
         - Real world position of cells
         - Active/inactive status of cells
    */

    class EclipseGrid : public GridDims {
    public:
        explicit EclipseGrid(const std::string& filename);

        /*
          These constructors will make a copy of the src grid, with
          zcorn and or actnum have been adjustments.
        */
        EclipseGrid(const EclipseGrid& src, const double* zcorn , const std::vector<int>& actnum);
        EclipseGrid(const EclipseGrid& src, const std::vector<double>& zcorn , const std::vector<int>& actnum);
        EclipseGrid(const EclipseGrid& src, const std::vector<int>& actnum);

        EclipseGrid(size_t nx, size_t ny, size_t nz,
                    double dx = 1.0, double dy = 1.0, double dz = 1.0);

        EclipseGrid(std::array<int, 3>& dims ,
                    const std::vector<double>& coord ,
                    const std::vector<double>& zcorn ,
                    const int * actnum = nullptr,
                    const double * mapaxes = nullptr);


        /// EclipseGrid ignores ACTNUM in Deck, and therefore needs ACTNUM
        /// explicitly.  If a null pointer is passed, every cell is active.
        explicit EclipseGrid(const Deck& deck, const int * actnum = nullptr);
        /// [deprecated]
        explicit EclipseGrid(const std::shared_ptr<const Deck>& deck, const int * actnum = nullptr);

        static bool hasCornerPointKeywords(const Deck&);
        static bool hasCartesianKeywords(const Deck&);
        size_t  getNumActive( ) const;
        bool allActive() const;

        size_t activeIndex(size_t i, size_t j, size_t k) const;
        size_t activeIndex(size_t globalIndex) const;

        /*
          Observe that the there is a getGlobalIndex(i,j,k)
          implementation in the base class. This method - translating
          from an active index to a global index must be implemented
          in the current class.
        */
        size_t getGlobalIndex(size_t active_index) const;
        size_t getGlobalIndex(size_t i, size_t j, size_t k) const;

        bool isPinchActive( ) const;
        double getPinchThresholdThickness( ) const;
        PinchMode::ModeEnum getPinchOption( ) const;
        PinchMode::ModeEnum getMultzOption( ) const;

        MinpvMode::ModeEnum getMinpvMode() const;
        double getMinpvValue( ) const;


        /*
          Will return a vector of nactive elements. The method will
          behave differently depending on the lenght of the
          input_vector:

             nx*ny*nz: only the values corresponding to active cells
               are copied out.

             nactive: The input vector is copied straight out again.

             ??? : Exception.
        */
        template<typename T>
        std::vector<T> compressedVector(const std::vector<T>& input_vector) const {
            if( input_vector.size() == this->getNumActive() ) {
                return input_vector;
            }

            if (input_vector.size() != getCartesianSize())
                throw std::invalid_argument("Input vector must have full size");

            {
                std::vector<T> compressed_vector( this->getNumActive() );
                const auto& active_map = this->getActiveMap( );

                for (size_t i = 0; i < this->getNumActive(); ++i)
                    compressed_vector[i] = input_vector[ active_map[i] ];

                return compressed_vector;
            }
        }


        /// Will return a vector a length num_active; where the value
        /// of each element is the corresponding global index.
        const std::vector<int>& getActiveMap() const;
        std::array<double, 3> getCellCenter(size_t i,size_t j, size_t k) const;
        std::array<double, 3> getCellCenter(size_t globalIndex) const;
        double getCellVolume(size_t globalIndex) const;
        double getCellVolume(size_t i , size_t j , size_t k) const;
        double getCellThicknes(size_t globalIndex) const;
        double getCellThicknes(size_t i , size_t j , size_t k) const;
        std::array<double, 3> getCellDims(size_t i,size_t j, size_t k) const;
        std::array<double, 3> getCellDims(size_t globalIndex) const;
        bool cellActive( size_t globalIndex ) const;
        bool cellActive( size_t i , size_t j, size_t k ) const;
        double getCellDepth(size_t i,size_t j, size_t k) const;
        double getCellDepth(size_t globalIndex) const;
        ZcornMapper zcornMapper() const;


        void exportMAPAXES( std::vector<double>& mapaxes) const;
        void exportCOORD( std::vector<double>& coord) const;
        void exportZCORN( std::vector<double>& zcorn) const;
        void exportACTNUM( std::vector<int>& actnum) const;
        void resetACTNUM( const int * actnum);
        bool equal(const EclipseGrid& other) const;
        const ecl_grid_type * c_ptr() const;
        const MessageContainer& getMessageContainer() const;
        MessageContainer& getMessageContainer();
    private:
        MessageContainer m_messages;

        double m_minpvValue;
        MinpvMode::ModeEnum m_minpvMode;
        Value<double> m_pinch;
        PinchMode::ModeEnum m_pinchoutMode;
        PinchMode::ModeEnum m_multzMode;
        mutable std::vector< int > activeMap;

        /*
          The internal class grid_ptr is a a std::unique_ptr with
          special copy semantics. The purpose of implementing this is
          that the EclipseGrid class can now use the default
          implementation for the copy and move constructors.
        */
        using ert_ptr = ERT::ert_unique_ptr<ecl_grid_type , ecl_grid_free>;
        class grid_ptr : public ert_ptr {
        public:
            using ert_ptr::unique_ptr;
            grid_ptr() = default;
            grid_ptr(grid_ptr&&) = default;
            grid_ptr(const grid_ptr& src) :
                ert_ptr( ecl_grid_alloc_copy( src.get() ) ) {}
        };
        grid_ptr m_grid;

        void initCornerPointGrid(const std::array<int,3>& dims ,
                                 const std::vector<double>& coord ,
                                 const std::vector<double>& zcorn ,
                                 const int * actnum,
                                 const double * mapaxes);

        void initCartesianGrid(         const std::array<int, 3>&, const Deck&);
        void initCornerPointGrid(       const std::array<int, 3>&, const Deck&);
        void initDTOPSGrid(             const std::array<int, 3>&, const Deck&);
        void initDVDEPTHZGrid(          const std::array<int, 3>&, const Deck&);
        void initGrid(                  const std::array<int, 3>&, const Deck&);
        void assertCornerPointKeywords( const std::array<int, 3>&, const Deck&);

        static bool hasDVDEPTHZKeywords(const Deck&);
        static bool hasDTOPSKeywords(   const Deck&);
        static void assertVectorSize(   const std::vector<double>& vector, size_t expectedSize, const std::string& msg);
        static std::vector<double> createTOPSVector(const std::array<int, 3>& dims, const std::vector<double>& DZ,
                const Deck&);
        static std::vector<double> createDVector(const std::array<int, 3>& dims, size_t dim, const std::string& DKey,
                const std::string& DVKey, const Deck&);
        static void scatterDim(const std::array<int, 3>& dims , size_t dim , const std::vector<double>& DV , std::vector<double>& D);
   };

    typedef std::shared_ptr<EclipseGrid> EclipseGridPtr;
    typedef std::shared_ptr<const EclipseGrid> EclipseGridConstPtr;

    class ZcornMapper {
    public:
        ZcornMapper(size_t nx, size_t ny, size_t nz);
        size_t index(size_t i, size_t j, size_t k, int c) const;
        size_t index(size_t g, int c) const;

    private:
        std::array<size_t,3> dims;
        std::array<size_t,3> stride;
        std::array<size_t,8> cell_shift;
    };
}




#endif // OPM_PARSER_ECLIPSE_GRID_HPP
