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


#include <iostream>
#include <tuple>
#include <cmath>

#include <boost/lexical_cast.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/Z.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <ert/ecl/ecl_grid.h>

namespace Opm {

    /**
       Will create an EclipseGrid instance based on an existing
       GRID/EGRID file.
    */
    EclipseGrid::EclipseGrid(const std::string& filename )
        : m_minpvValue(0),
          m_minpvMode(MinpvMode::ModeEnum::Inactive),
          m_pinch("PINCH"),
          m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
          m_multzMode(PinchMode::ModeEnum::TOP)
    {
        ecl_grid_type * new_ptr = ecl_grid_load_case( filename.c_str() );
        if (new_ptr)
            m_grid.reset( new_ptr );
        else
            throw std::invalid_argument("Could not load grid from binary file: " + filename);

        m_nx = static_cast<size_t>( ecl_grid_get_nx( c_ptr() ));
        m_ny = static_cast<size_t>( ecl_grid_get_ny( c_ptr() ));
        m_nz = static_cast<size_t>( ecl_grid_get_nz( c_ptr() ));
    }

    /* Copy constructor */
    EclipseGrid::EclipseGrid(const EclipseGrid& src)
        : m_minpvValue( src.m_minpvValue ),
          m_minpvMode( src.m_minpvMode ),
          m_pinch( src.m_pinch ),
          m_pinchoutMode( src.m_pinchoutMode ),
          m_multzMode( src.m_multzMode ),
          m_nx( src.m_nx ),
          m_ny( src.m_ny ),
          m_nz( src.m_nz ),
          m_messages( src.m_messages )
    {
        if (src.hasCellInfo())
            m_grid.reset( ecl_grid_alloc_copy( src.c_ptr() ) );

    }

    /*
      This creates a grid which only has dimension, and no pointer to
      a true grid structure. This grid will answer false to
      hasCellInfo() - but can be used in all situations where the grid
      dependency is really only on the dimensions.
    */

    EclipseGrid::EclipseGrid(size_t nx, size_t ny , size_t nz,
                             double dx, double dy, double dz)
        : m_minpvValue(0),
          m_minpvMode(MinpvMode::ModeEnum::Inactive),
          m_pinch("PINCH"),
          m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
          m_multzMode(PinchMode::ModeEnum::TOP)
    {
        m_nx = nx;
        m_ny = ny;
        m_nz = nz;
        m_grid.reset(ecl_grid_alloc_rectangular(nx, ny, nz, dx, dy, dz, NULL));
    }


    // keyword must be DIMENS or SPECGRID
    static std::vector<int> getDims( const DeckKeyword& keyword ) {
        const auto& record = keyword.getRecord(0);
        std::vector<int> dims = {record.getItem("NX").get< int >(0) ,
            record.getItem("NY").get< int >(0) ,
            record.getItem("NZ").get< int >(0) };
        return dims;
    }

    EclipseGrid::EclipseGrid(const std::shared_ptr<const Deck>& deckptr, const int * actnum)
       :
               EclipseGrid(*deckptr, actnum)
    {}


    /*
      This is the main EclipseGrid constructor, it will inspect the
      input Deck for grid keywords, either the corner point keywords
      COORD and ZCORN, or the various rectangular keywords like DX,DY
      and DZ.

      Actnum is treated specially:

        1. If an actnum pointer is passed in that should be a pointer
           to 0 and 1 values which will be used as actnum mask.

        2. If the actnum pointer is not present the constructor will
           look in the deck for an actnum keyword, and use that if it
           is found. This is a best effort which will work in many
           cases, but if the ACTNUM keyword is manipulated in the deck
           those manipulations will be silently lost; if the ACTNUM
           keyword has size different from nx*ny*nz it will also be
           silently ignored.

      With a mutable EclipseGrid instance you can later call the
      EclipseGrid::resetACTNUM() method when you have complete actnum
      information. The EclipseState based construction of EclipseGrid
      is a two-pass operation, which guarantees that actnum is handled
      correctly.
    */




    EclipseGrid::EclipseGrid(const Deck& deck, const int * actnum)
        : m_minpvValue(0),
          m_minpvMode(MinpvMode::ModeEnum::Inactive),
          m_pinch("PINCH"),
          m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
          m_multzMode(PinchMode::ModeEnum::TOP)
    {
        const bool hasRUNSPEC = Section::hasRUNSPEC(deck);
        const bool hasGRID = Section::hasGRID(deck);
        if (hasRUNSPEC && hasGRID) {
            // Equivalent to first constructor.
            RUNSPECSection runspecSection( deck );
            if( runspecSection.hasKeyword<ParserKeywords::DIMENS>() ) {
                const auto& dimens = runspecSection.getKeyword<ParserKeywords::DIMENS>();
                std::vector<int> dims = getDims(dimens);
                initGrid(dims, deck);
            } else {
                const std::string msg = "The RUNSPEC section must have the DIMENS keyword with logically Cartesian grid dimensions.";
                m_messages.error(msg);
                throw std::invalid_argument(msg);
            }
        } else if (hasGRID) {
            // Look for SPECGRID instead of DIMENS.
            if (deck.hasKeyword<ParserKeywords::SPECGRID>()) {
                const auto& specgrid = deck.getKeyword<ParserKeywords::SPECGRID>();
                std::vector<int> dims = getDims(specgrid);
                initGrid(dims, deck);
            } else {
                const std::string msg = "With no RUNSPEC section, the GRID section must specify the grid dimensions using the SPECGRID keyword.";
                m_messages.error(msg);
                throw std::invalid_argument(msg);
            }
        } else {
            // The deck contains no relevant section, so it is probably a sectionless GRDECL file.
            // Either SPECGRID or DIMENS is OK.
            if (deck.hasKeyword("SPECGRID")) {
                const auto& specgrid = deck.getKeyword<ParserKeywords::SPECGRID>();
                std::vector<int> dims = getDims(specgrid);
                initGrid(dims, deck);
            } else if (deck.hasKeyword<ParserKeywords::DIMENS>()) {
                const auto& dimens = deck.getKeyword<ParserKeywords::DIMENS>();
                std::vector<int> dims = getDims(dimens);
                initGrid(dims, deck);
            } else {
                const std::string msg = "The deck must specify grid dimensions using either DIMENS or SPECGRID.";
                m_messages.error(msg);
                throw std::invalid_argument(msg);
            }
        }
        if (actnum != nullptr)
            resetACTNUM(actnum);
        else {
            if (deck.hasKeyword<ParserKeywords::ACTNUM>()) {
                const auto& actnumData = deck.getKeyword<ParserKeywords::ACTNUM>().getIntData();
                if (actnumData.size() == getCartesianSize())
                    resetACTNUM( actnumData.data());
                else {
                    const std::string msg = "The ACTNUM keyword has " + std::to_string( actnumData.size() ) + " elements - expected : " + std::to_string( getCartesianSize()) + " - ignored.";
                    m_messages.warning(msg);
                }
            }
        }
    }


    void EclipseGrid::initGrid( const std::vector<int>& dims, const Deck& deck) {
        m_nx = static_cast<size_t>(dims[0]);
        m_ny = static_cast<size_t>(dims[1]);
        m_nz = static_cast<size_t>(dims[2]);

        if (hasCornerPointKeywords(deck)) {
            initCornerPointGrid(dims , deck);
        } else if (hasCartesianKeywords(deck)) {
            initCartesianGrid(dims , deck);
        }

        if (deck.hasKeyword<ParserKeywords::PINCH>()) {
            const auto& record = deck.getKeyword<ParserKeywords::PINCH>( ).getRecord(0);
            const auto& item = record.getItem<ParserKeywords::PINCH::THRESHOLD_THICKNESS>( );
            m_pinch.setValue( item.getSIDouble(0) );

            auto pinchoutString = record.getItem<ParserKeywords::PINCH::PINCHOUT_OPTION>().get< std::string >(0);
            m_pinchoutMode = PinchMode::PinchModeFromString(pinchoutString);

            auto multzString = record.getItem<ParserKeywords::PINCH::MULTZ_OPTION>().get< std::string >(0);
            m_multzMode = PinchMode::PinchModeFromString(multzString);
        }

        if (deck.hasKeyword<ParserKeywords::MINPV>() && deck.hasKeyword<ParserKeywords::MINPVFIL>()) {
            throw std::invalid_argument("Can not have both MINPV and MINPVFIL in deck.");
        }

        if (deck.hasKeyword<ParserKeywords::MINPV>()) {
            const auto& record = deck.getKeyword<ParserKeywords::MINPV>( ).getRecord(0);
            const auto& item = record.getItem<ParserKeywords::MINPV::VALUE>( );
            m_minpvValue = item.getSIDouble(0);
            m_minpvMode = MinpvMode::ModeEnum::EclSTD;
        }


        if (deck.hasKeyword<ParserKeywords::MINPVFIL>()) {
            const auto& record = deck.getKeyword<ParserKeywords::MINPVFIL>( ).getRecord(0);
            const auto& item = record.getItem<ParserKeywords::MINPVFIL::VALUE>( );
            m_minpvValue = item.getSIDouble(0);
            m_minpvMode = MinpvMode::ModeEnum::OpmFIL;
        }
    }


    size_t EclipseGrid::activeIndex(size_t i, size_t j, size_t k) const {
        return activeIndex( getGlobalIndex( i,j,k ));
    }



    size_t EclipseGrid::activeIndex(size_t globalIndex) const {
        assertCellInfo();
        {
            int active_index = ecl_grid_get_active_index1( m_grid.get() , globalIndex );
            if (active_index < 0)
                throw std::invalid_argument("Input argument does not correspond to an active cell");
            return static_cast<size_t>( active_index );
        }
    }



    size_t EclipseGrid::getNX( ) const {
        return m_nx;
    }

    size_t EclipseGrid::getNY( ) const {
        return m_ny;
    }

    size_t EclipseGrid::getNZ( ) const {
        return m_nz;
    }

    std::array< int, 3 > EclipseGrid::getNXYZ() const {
        return {{ int( m_nx ), int( m_ny ), int( m_nz ) }};
    }

    size_t EclipseGrid::getCartesianSize( ) const {
        return m_nx * m_ny * m_nz;
    }

    bool EclipseGrid::isPinchActive( ) const {
        return m_pinch.hasValue();
    }

    double EclipseGrid::getPinchThresholdThickness( ) const {
        return m_pinch.getValue();
    }

    PinchMode::ModeEnum EclipseGrid::getPinchOption( ) const {
        return m_pinchoutMode;
    }

    PinchMode::ModeEnum EclipseGrid::getMultzOption( ) const {
        return m_multzMode;
    }

    MinpvMode::ModeEnum EclipseGrid::getMinpvMode() const {
        return m_minpvMode;
    }

    double EclipseGrid::getMinpvValue( ) const {
        return m_minpvValue;
    }


    size_t EclipseGrid::getGlobalIndex(size_t i, size_t j, size_t k) const {
        return (i + j * getNX() + k * getNX() * getNY());
    }

    std::array<int, 3> EclipseGrid::getIJK(size_t globalIndex) const {
        std::array<int, 3> r = {{ 0, 0, 0 }};
        int k = globalIndex / (getNX() * getNY()); globalIndex -= k * (getNX() * getNY());
        int j = globalIndex / getNX();             globalIndex -= j *  getNX();
        int i = globalIndex;
        r[0] = i;
        r[1] = j;
        r[2] = k;
        return r;
    }

    void EclipseGrid::assertGlobalIndex(size_t globalIndex) const {
        if (globalIndex >= getCartesianSize())
            throw std::invalid_argument("input index above valid range");
    }

    void EclipseGrid::assertIJK(size_t i , size_t j , size_t k) const {
        if (i >= getNX() || j >= getNY() || k >= getNZ())
            throw std::invalid_argument("input index above valid range");
    }



    void EclipseGrid::initCartesianGrid(const std::vector<int>& dims , const Deck& deck) {
        if (hasDVDEPTHZKeywords( deck ))
            initDVDEPTHZGrid( dims , deck );
        else if (hasDTOPSKeywords(deck))
            initDTOPSGrid( dims ,deck );
        else
            throw std::invalid_argument("Tried to initialize cartesian grid without all required keywords");
    }


    void EclipseGrid::initDVDEPTHZGrid(const std::vector<int>& dims, const Deck& deck) {
        const std::vector<double>& DXV = deck.getKeyword<ParserKeywords::DXV>().getSIDoubleData();
        const std::vector<double>& DYV = deck.getKeyword<ParserKeywords::DYV>().getSIDoubleData();
        const std::vector<double>& DZV = deck.getKeyword<ParserKeywords::DZV>().getSIDoubleData();
        const std::vector<double>& DEPTHZ = deck.getKeyword<ParserKeywords::DEPTHZ>().getSIDoubleData();

        assertVectorSize( DEPTHZ , static_cast<size_t>( (dims[0] + 1)*(dims[1] +1 )) , "DEPTHZ");
        assertVectorSize( DXV    , static_cast<size_t>( dims[0] ) , "DXV");
        assertVectorSize( DYV    , static_cast<size_t>( dims[1] ) , "DYV");
        assertVectorSize( DZV    , static_cast<size_t>( dims[2] ) , "DZV");

        m_grid.reset( ecl_grid_alloc_dxv_dyv_dzv_depthz( dims[0] , dims[1] , dims[2] , DXV.data() , DYV.data() , DZV.data() , DEPTHZ.data() , nullptr ) );
    }


    void EclipseGrid::initDTOPSGrid(const std::vector<int>& dims , const Deck& deck) {
        std::vector<double> DX = createDVector( dims , 0 , "DX" , "DXV" , deck);
        std::vector<double> DY = createDVector( dims , 1 , "DY" , "DYV" , deck);
        std::vector<double> DZ = createDVector( dims , 2 , "DZ" , "DZV" , deck);
        std::vector<double> TOPS = createTOPSVector( dims , DZ , deck );
        m_grid.reset( ecl_grid_alloc_dx_dy_dz_tops( dims[0] , dims[1] , dims[2] , DX.data() , DY.data() , DZ.data() , TOPS.data() , nullptr ) );
    }

    void EclipseGrid::initCornerPointGrid(const std::vector<int>& dims, const Deck& deck) {
        assertCornerPointKeywords( dims , deck);
        {
            const auto& ZCORNKeyWord = deck.getKeyword<ParserKeywords::ZCORN>();
            const auto& COORDKeyWord = deck.getKeyword<ParserKeywords::COORD>();
            const std::vector<double>& zcorn = ZCORNKeyWord.getSIDoubleData();
            const std::vector<double>& coord = COORDKeyWord.getSIDoubleData();
            double    * mapaxes = NULL;

            if (deck.hasKeyword<ParserKeywords::MAPAXES>()) {
                const auto& mapaxesKeyword = deck.getKeyword<ParserKeywords::MAPAXES>();
                const auto& record = mapaxesKeyword.getRecord(0);
                mapaxes = new double[6];
                for (size_t i = 0; i < 6; i++) {
                    mapaxes[i] = record.getItem( i ).getSIDouble( 0 );
                }
            }


            {
                const std::vector<float> zcorn_float( zcorn.begin() , zcorn.end() );
                const std::vector<float> coord_float( coord.begin() , coord.end() );
                float * mapaxes_float = NULL;
                if (mapaxes) {
                    mapaxes_float = new float[6];
                    for (size_t i=0; i < 6; i++)
                        mapaxes_float[i] = mapaxes[i];
                }
                m_grid.reset( ecl_grid_alloc_GRDECL_data(dims[0] , dims[1] , dims[2] , zcorn_float.data() , coord_float.data() , nullptr , mapaxes_float) );

                if (mapaxes) {
                    delete[] mapaxes_float;
                    delete[] mapaxes;
                }
            }
        }
    }



    bool EclipseGrid::hasCornerPointKeywords(const Deck& deck) {
        if (deck.hasKeyword<ParserKeywords::ZCORN>() && deck.hasKeyword<ParserKeywords::COORD>())
            return true;
        else
            return false;
    }


    void EclipseGrid::assertCornerPointKeywords( const std::vector<int>& dims , const Deck& deck)
    {
        const int nx = dims[0];
        const int ny = dims[1];
        const int nz = dims[2];
        {
            const auto& ZCORNKeyWord = deck.getKeyword<ParserKeywords::ZCORN>();

            if (ZCORNKeyWord.getDataSize() != static_cast<size_t>(8*nx*ny*nz)) {
                const std::string msg =
                    "Wrong size of the ZCORN keyword: Expected 8*x*ny*nz = "
                    + std::to_string(static_cast<long long>(8*nx*ny*nz)) + " is "
                    + std::to_string(static_cast<long long>(ZCORNKeyWord.getDataSize()));
                m_messages.error(msg);
                throw std::invalid_argument(msg);
            }
        }

        {
            const auto& COORDKeyWord = deck.getKeyword<ParserKeywords::COORD>();
            if (COORDKeyWord.getDataSize() != static_cast<size_t>(6*(nx + 1)*(ny + 1))) {
                const std::string msg =
                    "Wrong size of the COORD keyword: Expected 6*(nx + 1)*(ny + 1) = "
                    + std::to_string(static_cast<long long>(6*(nx + 1)*(ny + 1))) + " is "
                    + std::to_string(static_cast<long long>(COORDKeyWord.getDataSize()));
                m_messages.error(msg);
                throw std::invalid_argument(msg);
            }
        }
    }



    bool EclipseGrid::hasCartesianKeywords(const Deck& deck) {
        if (hasDVDEPTHZKeywords( deck ))
            return true;
        else
            return hasDTOPSKeywords(deck);
    }


    bool EclipseGrid::hasDVDEPTHZKeywords(const Deck& deck) {
        if (deck.hasKeyword<ParserKeywords::DXV>() &&
            deck.hasKeyword<ParserKeywords::DYV>() &&
            deck.hasKeyword<ParserKeywords::DZV>() &&
            deck.hasKeyword<ParserKeywords::DEPTHZ>())
            return true;
        else
            return false;
    }

    bool EclipseGrid::hasDTOPSKeywords(const Deck& deck) {
        if ((deck.hasKeyword<ParserKeywords::DX>() || deck.hasKeyword<ParserKeywords::DXV>()) &&
            (deck.hasKeyword<ParserKeywords::DY>() || deck.hasKeyword<ParserKeywords::DYV>()) &&
            (deck.hasKeyword<ParserKeywords::DZ>() || deck.hasKeyword<ParserKeywords::DZV>()) &&
            deck.hasKeyword<ParserKeywords::TOPS>())
            return true;
        else
            return false;
    }



    void EclipseGrid::assertVectorSize(const std::vector<double>& vector , size_t expectedSize , const std::string& vectorName) {
        if (vector.size() != expectedSize)
            throw std::invalid_argument("Wrong size for keyword: " + vectorName + ". Expected: " + boost::lexical_cast<std::string>(expectedSize) + " got: " + boost::lexical_cast<std::string>(vector.size()));
    }


    /*
      The body of the for loop in this method looks slightly
      peculiar. The situation is as follows:

        1. The EclipseGrid class will assemble the necessary keywords
           and create an ert ecl_grid instance.

        2. The ecl_grid instance will export ZCORN, COORD and ACTNUM
           data which will be used by the UnstructureGrid constructor
           in opm-core. If the ecl_grid is created with ZCORN as an
           input keyword that data is retained in the ecl_grid
           structure, otherwise the ZCORN data is created based on the
           internal cell geometries.

        3. When constructing the UnstructuredGrid structure strict
           numerical comparisons of ZCORN values are used to detect
           cells in contact, if all the elements the elements in the
           TOPS vector are specified[1] we will typically not get
           bitwise equality between the bottom of one cell and the top
           of the next.

       To remedy this we enforce bitwise equality with the
       construction:

          if (std::abs(nextValue - TOPS[targetIndex]) < z_tolerance)
             TOPS[targetIndex] = nextValue;

       [1]: This is of course assuming the intention is to construct a
            fully connected space covering grid - if that is indeed
            not the case the barriers must be thicker than 1e-6m to be
            retained.
    */

    std::vector<double> EclipseGrid::createTOPSVector(const std::vector<int>& dims,
            const std::vector<double>& DZ, const Deck& deck)
    {
        double z_tolerance = 1e-6;
        size_t volume = dims[0] * dims[1] * dims[2];
        size_t area = dims[0] * dims[1];
        const auto& TOPSKeyWord = deck.getKeyword<ParserKeywords::TOPS>();
        std::vector<double> TOPS = TOPSKeyWord.getSIDoubleData();

        if (TOPS.size() >= area) {
            size_t initialTOPSize = TOPS.size();
            TOPS.resize( volume );

            for (size_t targetIndex = area; targetIndex < volume; targetIndex++) {
                size_t sourceIndex = targetIndex - area;
                double nextValue = TOPS[sourceIndex] + DZ[sourceIndex];

                if (targetIndex >= initialTOPSize)
                    TOPS[targetIndex] = nextValue;
                else {
                    if (std::abs(nextValue - TOPS[targetIndex]) < z_tolerance)
                        TOPS[targetIndex] = nextValue;
                }

            }
        }

        if (TOPS.size() != volume)
            throw std::invalid_argument("TOPS size mismatch");

        return TOPS;
    }

    std::vector<double> EclipseGrid::createDVector(const std::vector<int>& dims, size_t dim, const std::string& DKey,
            const std::string& DVKey, const Deck& deck)
    {
        size_t volume = dims[0] * dims[1] * dims[2];
        size_t area = dims[0] * dims[1];
        std::vector<double> D;
        if (deck.hasKeyword(DKey)) {
            D = deck.getKeyword( DKey ).getSIDoubleData();


            if (D.size() >= area && D.size() < volume) {
                /*
                  Only the top layer is required; for layers below the
                  top layer the value from the layer above is used.
                */
                size_t initialDSize = D.size();
                D.resize( volume );
                for (size_t targetIndex = initialDSize; targetIndex < volume; targetIndex++) {
                    size_t sourceIndex = targetIndex - area;
                    D[targetIndex] = D[sourceIndex];
                }
            }

            if (D.size() != volume)
                throw std::invalid_argument(DKey + " size mismatch");
        } else {
            const auto& DVKeyWord = deck.getKeyword(DVKey);
            const std::vector<double>& DV = DVKeyWord.getSIDoubleData();
            if (DV.size() != (size_t) dims[dim])
                throw std::invalid_argument(DVKey + " size mismatch");
            D.resize( volume );
            scatterDim( dims , dim , DV , D );
        }
        return D;
    }


    void EclipseGrid::scatterDim(const std::vector<int>& dims , size_t dim , const std::vector<double>& DV , std::vector<double>& D) {
        int index[3];
        for (index[2] = 0;  index[2] < dims[2]; index[2]++) {
            for (index[1] = 0; index[1] < dims[1]; index[1]++) {
                for (index[0] = 0;  index[0] < dims[0]; index[0]++) {
                    size_t globalIndex = index[2] * dims[1] * dims[0] + index[1] * dims[0] + index[0];
                    D[globalIndex] = DV[ index[dim] ];
                }
            }
        }
    }


    /*
      This function checks if the grid has a pointer to an underlying
      ecl_grid_type; which must be used to read cell info as
      size/depth/active of individual cells.
    */

    bool EclipseGrid::hasCellInfo() const {
        return static_cast<bool>( m_grid );
    }


    void EclipseGrid::assertCellInfo() const {
        if (!hasCellInfo())
            throw std::invalid_argument("Tried to access cell information in a grid with only dimensions");
    }


    const ecl_grid_type * EclipseGrid::c_ptr() const {
        assertCellInfo();
        return m_grid.get();
    }


    const MessageContainer& EclipseGrid::getMessageContainer() const {
        return m_messages;
    }


    MessageContainer& EclipseGrid::getMessageContainer() {
        return m_messages;
    }


    bool EclipseGrid::equal(const EclipseGrid& other) const {
        bool status = (m_pinch.equal( other.m_pinch ) && (ecl_grid_compare( c_ptr() , other.c_ptr() , true , false , false )) && (m_minpvMode == other.getMinpvMode()));
        if(m_minpvMode!=MinpvMode::ModeEnum::Inactive){
            status = status && (m_minpvValue == other.getMinpvValue());
        }
        return  status;
    }


    size_t EclipseGrid::getNumActive( ) const {
        return static_cast<size_t>(ecl_grid_get_nactive( c_ptr() ));
    }

    bool EclipseGrid::cellActive( size_t globalIndex ) const {
        assertGlobalIndex( globalIndex );
        return ecl_grid_cell_active1( c_ptr() , static_cast<int>(globalIndex));
    }

    bool EclipseGrid::cellActive( size_t i , size_t j , size_t k ) const {
        assertIJK(i,j,k);
        return ecl_grid_cell_active3( c_ptr() , static_cast<int>(i),static_cast<int>(j),static_cast<int>(k));
    }


    double EclipseGrid::getCellVolume(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        return ecl_grid_get_cell_volume1( c_ptr() , static_cast<int>(globalIndex));
    }


    double EclipseGrid::getCellVolume(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);
        return ecl_grid_get_cell_volume3( c_ptr() , static_cast<int>(i),static_cast<int>(j),static_cast<int>(k));
    }

    double EclipseGrid::getCellThicknes(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);
        return ecl_grid_get_cell_thickness3( c_ptr() , static_cast<int>(i),static_cast<int>(j),static_cast<int>(k));
    }

    double EclipseGrid::getCellThicknes(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        return ecl_grid_get_cell_thickness1( c_ptr() , static_cast<int>(globalIndex));
    }


    std::array<double, 3> EclipseGrid::getCellDims(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        {
            double dx = ecl_grid_get_cell_dx1( c_ptr() , globalIndex);
            double dy = ecl_grid_get_cell_dy1( c_ptr() , globalIndex);
            double dz = ecl_grid_get_cell_thickness1( c_ptr() , globalIndex);

            return std::array<double,3>{ {dx , dy , dz }};
        }
    }

    std::array<double, 3> EclipseGrid::getCellDims(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);
        {
            size_t globalIndex = getGlobalIndex( i,j,k );
            double dx = ecl_grid_get_cell_dx1( c_ptr() , globalIndex);
            double dy = ecl_grid_get_cell_dy1( c_ptr() , globalIndex);
            double dz = ecl_grid_get_cell_thickness1( c_ptr() , globalIndex);

            return std::array<double,3>{ {dx , dy , dz }};
        }
    }

    std::array<double, 3> EclipseGrid::getCellCenter(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        {
            double x,y,z;
            ecl_grid_get_xyz1( c_ptr() , static_cast<int>(globalIndex) , &x , &y , &z);
            return std::array<double, 3>{{x,y,z}};
        }
    }


    std::array<double, 3> EclipseGrid::getCellCenter(size_t i,size_t j, size_t k) const {
        assertIJK(i,j,k);
        {
            double x,y,z;
            ecl_grid_get_xyz3( c_ptr() , static_cast<int>(i),static_cast<int>(j),static_cast<int>(k), &x , &y , &z);
            return std::array<double, 3>{{x,y,z}};
        }
    }

    double EclipseGrid::getCellDepth(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        return ecl_grid_get_cdepth1( c_ptr() , static_cast<int>(globalIndex));
    }


    double EclipseGrid::getCellDepth(size_t i,size_t j, size_t k) const {
        assertIJK(i,j,k);
        return ecl_grid_get_cdepth3( c_ptr() , static_cast<int>(i),static_cast<int>(j),static_cast<int>(k));
    }



    void EclipseGrid::exportACTNUM( std::vector<int>& actnum) const {
        size_t volume = getNX() * getNY() * getNZ();
        if (getNumActive() == volume)
            actnum.resize(0);
        else {
            actnum.resize( volume );
            ecl_grid_init_actnum_data( c_ptr() , actnum.data() );
        }
    }

    void EclipseGrid::exportMAPAXES( std::vector<double>& mapaxes) const {
        if (ecl_grid_use_mapaxes( c_ptr())) {
            mapaxes.resize(6);
            ecl_grid_init_mapaxes_data_double( c_ptr() , mapaxes.data() );
        } else {
            mapaxes.resize(0);
        }
    }

    void EclipseGrid::exportCOORD( std::vector<double>& coord) const {
        coord.resize( ecl_grid_get_coord_size( c_ptr() ));
        ecl_grid_init_coord_data_double( c_ptr() , coord.data() );
    }

    void EclipseGrid::exportZCORN( std::vector<double>& zcorn) const {
        zcorn.resize( ecl_grid_get_zcorn_size( c_ptr() ));
        ecl_grid_init_zcorn_data_double( c_ptr() , zcorn.data() );
    }



    void EclipseGrid::resetACTNUM( const int * actnum) {
        assertCellInfo();
        ecl_grid_reset_actnum( m_grid.get() , actnum );
    }


    void EclipseGrid::fwriteEGRID( const std::string& filename, bool output_metric ) const {
        assertCellInfo();
        ecl_grid_fwrite_EGRID( m_grid.get() , filename.c_str(), output_metric );
    }


}


