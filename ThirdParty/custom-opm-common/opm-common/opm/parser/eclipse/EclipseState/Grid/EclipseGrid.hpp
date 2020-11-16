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
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>

#include <opm/io/eclipse/EclFile.hpp>

#include <array>
#include <memory>
#include <vector>

namespace Opm {

    class Deck;
    class ZcornMapper;
    class NNC;

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
        EclipseGrid() = default;
        explicit EclipseGrid(const std::string& filename);

        /*
          These constructors will make a copy of the src grid, with
          zcorn and or actnum have been adjustments.
        */
        EclipseGrid(const EclipseGrid& src) = default;
        EclipseGrid(const EclipseGrid& src, const std::vector<int>& actnum);
        EclipseGrid(const EclipseGrid& src, const double* zcorn, const std::vector<int>& actnum);

        EclipseGrid(size_t nx, size_t ny, size_t nz,
                    double dx = 1.0, double dy = 1.0, double dz = 1.0);

        EclipseGrid(std::array<int, 3>& dims ,
                    const std::vector<double>& coord ,
                    const std::vector<double>& zcorn ,
                    const int * actnum = nullptr,
                    const double * mapaxes = nullptr);


        /// EclipseGrid ignores ACTNUM in Deck, and therefore needs ACTNUM
        /// explicitly.  If a null pointer is passed, every cell is active.
        EclipseGrid(const Deck& deck, const int * actnum = nullptr);

        static bool hasGDFILE(const Deck& deck);
        static bool hasCylindricalKeywords(const Deck& deck);
        static bool hasCornerPointKeywords(const Deck&);
        static bool hasCartesianKeywords(const Deck&);
        size_t  getNumActive( ) const;
        bool allActive() const;

        size_t activeIndex(size_t i, size_t j, size_t k) const;
        size_t activeIndex(size_t globalIndex) const;

        void save(const std::string& filename, bool formatted, const Opm::NNC& nnc, const Opm::UnitSystem& units) const;
        /*
          Observe that the there is a getGlobalIndex(i,j,k)
          implementation in the base class. This method - translating
          from an active index to a global index must be implemented
          in the current class.
        */
        size_t getGlobalIndex(size_t active_index) const;
        size_t getGlobalIndex(size_t i, size_t j, size_t k) const;

        /*
          For RADIAL grids you can *optionally* use the keyword
          'CIRCLE' to denote that period boundary conditions should be
          applied in the 'THETA' direction; this will only apply if
          the theta keywords entered sum up to exactly 360 degrees!
        */

        bool circle( ) const;
        bool isPinchActive( ) const;
        double getPinchThresholdThickness( ) const;
        PinchMode::ModeEnum getPinchOption( ) const;
        PinchMode::ModeEnum getMultzOption( ) const;

        MinpvMode::ModeEnum getMinpvMode() const;
        const std::vector<double>& getMinpvVector( ) const;

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
        std::array<double, 3> getCornerPos(size_t i,size_t j, size_t k, size_t corner_index) const;
        std::vector<double> activeVolume() const;
        double getCellVolume(size_t globalIndex) const;
        double getCellVolume(size_t i , size_t j , size_t k) const;
        double getCellThickness(size_t globalIndex) const;
        double getCellThickness(size_t i , size_t j , size_t k) const;
        std::array<double, 3> getCellDims(size_t i,size_t j, size_t k) const;
        std::array<double, 3> getCellDims(size_t globalIndex) const;
        bool cellActive( size_t globalIndex ) const;
        bool cellActive( size_t i , size_t j, size_t k ) const;
        double getCellDepth(size_t i,size_t j, size_t k) const;
        double getCellDepth(size_t globalIndex) const;
        ZcornMapper zcornMapper() const;

        const std::vector<double>& getCOORD() const;
        const std::vector<double>& getZCORN() const;
        const std::vector<int>& getACTNUM( ) const;
        const std::vector<double>& getMAPAXES() const;
        const std::string& getMAPUNITS() const { return m_mapunits; } ;

        /*
          The fixupZCORN method is run as part of constructiong the grid. This will adjust the
          z-coordinates to ensure that cells do not overlap. The return value is the number of
          points which have been adjusted. The number of zcorn nodes that has ben fixed is
          stored in private member zcorn_fixed.
        */

        size_t fixupZCORN();
        size_t getZcornFixed() { return zcorn_fixed; };

        // resetACTNUM with no arguments will make all cells in the grid active.

        void resetACTNUM();
        void resetACTNUM( const std::vector<int>& actnum);

        bool equal(const EclipseGrid& other) const;

    private:
        std::vector<double> m_minpvVector;
        MinpvMode::ModeEnum m_minpvMode;
        Value<double> m_pinch;
        PinchMode::ModeEnum m_pinchoutMode;
        PinchMode::ModeEnum m_multzMode;

        bool m_circle = false;

        size_t zcorn_fixed = 0;
        bool m_useActnumFromGdfile = false;

        // Input grid data.
        std::vector<double> m_zcorn;
        std::vector<double> m_coord;
        std::vector<double> m_mapaxes;
        std::vector<int> m_actnum;
        std::string m_mapunits;

        // Mapping to/from active cells.
        int m_nactive;
        std::vector<int> m_active_to_global;
        std::vector<int> m_global_to_active;

        void initGridFromEGridFile(Opm::EclIO::EclFile& egridfile, std::string fileName);
        void resetACTNUM( const int* actnum);

        void initBinaryGrid(const Deck& deck);

        void initCornerPointGrid(const std::vector<double>& coord ,
                                 const std::vector<double>& zcorn ,
                                 const int * actnum,
                                 const double * mapaxes);

        bool keywInputBeforeGdfile(const Deck& deck, const std::string keyword) const;

        void initCylindricalGrid(const Deck&);
        void initCartesianGrid(const Deck&);
        void initDTOPSGrid(const Deck&);
        void initDVDEPTHZGrid(const Deck&);
        void initGrid(const Deck&);
        void initCornerPointGrid(const Deck&);
        void assertCornerPointKeywords(const Deck&);

        static bool hasDVDEPTHZKeywords(const Deck&);
        static bool hasDTOPSKeywords(const Deck&);
        static void assertVectorSize(const std::vector<double>& vector, size_t expectedSize, const std::string& msg);

        static std::vector<double> createTOPSVector(const std::array<int, 3>& dims, const std::vector<double>& DZ, const Deck&);
        static std::vector<double> createDVector(const std::array<int, 3>& dims, std::size_t dim, const std::string& DKey, const std::string& DVKey, const Deck&);
        static void scatterDim(const std::array<int, 3>& dims , size_t dim , const std::vector<double>& DV , std::vector<double>& D);


        std::vector<double> makeCoordDxDyDzTops(const std::vector<double>& dx, const std::vector<double>& dy, const std::vector<double>& dz, const std::vector<double>& tops) const;
        std::vector<double> makeZcornDzTops(const std::vector<double>& dz, const std::vector<double>& tops) const ;
        std::vector<double> makeZcornDzvDepthz(const std::vector<double>& dzv, const std::vector<double>& depthz) const;
        std::vector<double> makeCoordDxvDyvDzvDepthz(const std::vector<double>& dxv, const std::vector<double>& dyv, const std::vector<double>& dzv, const std::vector<double>& depthz) const;

        void getCellCorners(const std::array<int, 3>& ijk, const std::array<int, 3>& dims, std::array<double,8>& X, std::array<double,8>& Y, std::array<double,8>& Z) const;
        void getCellCorners(const std::size_t globalIndex,
                            std::array<double,8>& X,
                            std::array<double,8>& Y,
                            std::array<double,8>& Z) const;

   };

    class CoordMapper {
    public:
        CoordMapper(size_t nx, size_t ny);
        size_t size() const;


        /*
          dim = 0,1,2 for x, y and z coordinate respectively.
          layer = 0,1 for k=0 and k=nz layers respectively.
        */

        size_t index(size_t i, size_t j, size_t dim, size_t layer) const;
    private:
        size_t nx;
        size_t ny;
    };



    class ZcornMapper {
    public:
        ZcornMapper(size_t nx, size_t ny, size_t nz);
        size_t index(size_t i, size_t j, size_t k, int c) const;
        size_t index(size_t g, int c) const;
        size_t size() const;

        /*
          The fixupZCORN method will take a zcorn vector as input and
          run through it. If the following situation is detected:

                      /|                     /|
                     / |                    / |
                    /  |                  /   |
                   /   |                 /    |
                  /    |     ==>       /      |
                 /     |             /        |
             ---/------x            /---------x
             | /
             |/

        */
        size_t fixupZCORN( std::vector<double>& zcorn);
        bool validZCORN( const std::vector<double>& zcorn) const;
    private:
        std::array<size_t,3> dims;
        std::array<size_t,3> stride;
        std::array<size_t,8> cell_shift;
    };
}




#endif // OPM_PARSER_ECLIPSE_GRID_HPP
