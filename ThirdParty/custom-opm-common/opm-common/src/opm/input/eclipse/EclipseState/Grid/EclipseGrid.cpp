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

#define _USE_MATH_DEFINES

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldProps.hpp>

#include <opm/common/ErrorMacros.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/utility/numeric/calculateCellVol.hpp>
#include <opm/common/utility/String.hpp>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Z.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <tuple>

#include <fmt/format.h>

namespace Opm {

namespace {


std::optional<UnitSystem> make_grid_units(const std::string& grid_unit) {
    if (grid_unit == "METRES")
        return UnitSystem(UnitSystem::UnitType::UNIT_TYPE_METRIC);

    if (grid_unit == "FEET")
        return UnitSystem(UnitSystem::UnitType::UNIT_TYPE_FIELD);

    if (grid_unit == "CM")
        return UnitSystem(UnitSystem::UnitType::UNIT_TYPE_LAB);

    return std::nullopt;
}

void apply_GRIDUNIT(const UnitSystem& deck_units, const UnitSystem& grid_units, std::vector<double>& data)
{
    double scale_factor = grid_units.getDimension(UnitSystem::measure::length).getSIScaling() / deck_units.getDimension(UnitSystem::measure::length).getSIScaling();
    for (auto& v : data)
        v *= scale_factor;
}

}

EclipseGrid::EclipseGrid(const std::array<int, 3>& dims ,
                         const std::vector<double>& coord ,
                         const std::vector<double>& zcorn ,
                         const int * actnum)
    : GridDims(dims),
      m_minpvMode(MinpvMode::ModeEnum::Inactive),
      m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
      m_multzMode(PinchMode::ModeEnum::TOP),
      m_pinchGapMode(PinchMode::ModeEnum::GAP)
{
    initCornerPointGrid( coord , zcorn , actnum );
}

/**
   Will create an EclipseGrid instance based on an existing
   GRID/EGRID file.
*/


EclipseGrid::EclipseGrid(const std::string& fileName )
    : GridDims(),
      m_minpvMode(MinpvMode::ModeEnum::Inactive),
      m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
      m_multzMode(PinchMode::ModeEnum::TOP),
      m_pinchGapMode(PinchMode::ModeEnum::GAP)
{

    Opm::EclIO::EclFile egridfile(fileName);
    m_useActnumFromGdfile=true;
    initGridFromEGridFile(egridfile, fileName);
}


EclipseGrid::EclipseGrid(const GridDims& gd)
    : GridDims(gd),
      m_minpvMode(MinpvMode::ModeEnum::Inactive),
      m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
      m_multzMode(PinchMode::ModeEnum::TOP),
      m_pinchGapMode(PinchMode::ModeEnum::GAP)
{
    this->m_nactive = this->getCartesianSize();
    this->active_volume = std::nullopt;
    // Nothing else initialized. Leaving in particular as empty:
    // m_actnum,
    // m_global_to_active,
    // m_active_to_global.
    //
    // EclipseGrid will only be usable for constructing Box objects with
    // all cells active.
}


EclipseGrid::EclipseGrid(size_t nx, size_t ny , size_t nz,
                         double dx, double dy, double dz)
    : GridDims(nx, ny, nz),
      m_minpvMode(MinpvMode::ModeEnum::Inactive),
      m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
      m_multzMode(PinchMode::ModeEnum::TOP),
      m_pinchGapMode(PinchMode::ModeEnum::GAP)
{

    m_coord.reserve((nx+1)*(ny+1)*6);

    for (size_t j = 0; j < (ny+1); j++) {
        for (size_t i = 0; i < (nx+1); i++) {
            m_coord.push_back(i*dx);
            m_coord.push_back(j*dy);
            m_coord.push_back(0.0);
            m_coord.push_back(i*dx);
            m_coord.push_back(j*dy);
            m_coord.push_back(nz*dz);
        }
    }

    m_zcorn.assign(nx*ny*nz*8, 0);

    for (size_t k = 0; k < nz ; k++) {
        for (size_t j = 0; j < ny ; j++) {
            for (size_t i = 0; i < nx ; i++) {

                // top face of cell
                int zind = i*2 + j*nx*4 + k*nx*ny*8;

                double zt = k*dz;
                double zb = (k+1)*dz;

                m_zcorn[zind] = zt;
                m_zcorn[zind + 1] = zt;

                zind = zind + nx*2;

                m_zcorn[zind] = zt;
                m_zcorn[zind + 1] = zt;

                // bottom face of cell
                zind = i*2 + j*nx*4 + k*nx*ny*8 + nx*ny*4;

                m_zcorn[zind] = zb;
                m_zcorn[zind + 1] = zb;

                zind = zind + nx*2;

                m_zcorn[zind] = zb;
                m_zcorn[zind + 1] = zb;
            }
        }
    }

    resetACTNUM();
}

EclipseGrid::EclipseGrid(const EclipseGrid& src, const double* zcorn, const std::vector<int>& actnum)
    : EclipseGrid(src)
{

    if (zcorn != nullptr) {
        size_t sizeZcorn = this->getCartesianSize()*8;

        for (size_t n=0; n < sizeZcorn; n++) {
            m_zcorn[n] = zcorn[n];
        }

        ZcornMapper mapper( getNX(), getNY(), getNZ());
        zcorn_fixed = mapper.fixupZCORN( m_zcorn );
    }

    resetACTNUM(actnum);
}

EclipseGrid::EclipseGrid(const EclipseGrid& src, const std::vector<int>& actnum)
        : EclipseGrid( src , nullptr , actnum )
{ }

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
    : GridDims(deck),
      m_minpvMode(MinpvMode::ModeEnum::Inactive),
      m_pinchoutMode(PinchMode::ModeEnum::TOPBOT),
      m_multzMode(PinchMode::ModeEnum::TOP),
      m_pinchGapMode(PinchMode::ModeEnum::GAP)
{
    if (deck.hasKeyword("GDFILE")){

        if (deck.hasKeyword("ACTNUM")){
            if (keywInputBeforeGdfile(deck, "ACTNUM"))  {
                m_useActnumFromGdfile = true;
            }
        } else {
            m_useActnumFromGdfile = true;
        }

    }

    updateNumericalAquiferCells(deck);

    initGrid(deck, actnum);

    if (deck.hasKeyword<ParserKeywords::MAPAXES>())
        this->m_mapaxes = std::make_optional<MapAxes>( deck );

    /*
      The GRIDUNIT handling is simplified compared to the full specification:

        1. The optional second item 'MAP' is ignored.

        2. In Eclipse the action of the GRIDUNIT keyword only applies to
           keywords in the same file as the GRIDUNIT keyword itself is in; in
           our implementation we apply the GRIDUNIT transformation if the deck
           contains the GRIDUNUT keyword - irrespective of exactly where it is
           located in the deck.
    */

    if (deck.hasKeyword<ParserKeywords::GRIDUNIT>()) {
        const auto& kw = deck.get<ParserKeywords::GRIDUNIT>().front();
        const auto& length_unit = trim_copy(kw.getRecord(0).getItem(0).get<std::string>(0));
        auto grid_units = make_grid_units(length_unit);
        if (!grid_units.has_value())
            throw OpmInputError(fmt::format("Invalid length specifier: [{}]", length_unit), kw.location());

        if (grid_units.value() != deck.getActiveUnitSystem()) {
            apply_GRIDUNIT(deck.getActiveUnitSystem(), grid_units.value(), this->m_zcorn);
            apply_GRIDUNIT(deck.getActiveUnitSystem(), grid_units.value(), this->m_coord);
            if (this->m_rv.has_value())
                apply_GRIDUNIT(deck.getActiveUnitSystem(), grid_units.value(), this->m_rv.value());
        }
    }
}


    bool EclipseGrid::circle( ) const{
        return this->m_circle;
    }

    void EclipseGrid::initGrid(const Deck& deck, const int* actnum)
    {
        if (deck.hasKeyword<ParserKeywords::RADIAL>()) {
            initCylindricalGrid(deck );
        } else if (deck.hasKeyword<ParserKeywords::SPIDER>()) {
            initSpiderwebGrid(deck );
        } else {
            if (hasCornerPointKeywords(deck)) {
                initCornerPointGrid(deck);
            } else if (hasCartesianKeywords(deck)) {
                initCartesianGrid(deck);
            } else if (hasGDFILE(deck)) {
                initBinaryGrid(deck);
            } else {
                throw std::invalid_argument("EclipseGrid needs cornerpoint or cartesian keywords.");
            }
        }

        if (deck.hasKeyword<ParserKeywords::PINCH>()) {
            const auto& record = deck.get<ParserKeywords::PINCH>( ).back().getRecord(0);
            const auto& item = record.getItem<ParserKeywords::PINCH::THRESHOLD_THICKNESS>( );
            m_pinch = item.getSIDouble(0);

            auto pinchoutString = record.getItem<ParserKeywords::PINCH::PINCHOUT_OPTION>().get< std::string >(0);
            m_pinchoutMode = PinchMode::PinchModeFromString(pinchoutString);

            auto multzString = record.getItem<ParserKeywords::PINCH::MULTZ_OPTION>().get< std::string >(0);
            m_multzMode = PinchMode::PinchModeFromString(multzString);
            auto pinchGapString = record.getItem<ParserKeywords::PINCH::CONTROL_OPTION>().get< std::string >(0);
            m_pinchGapMode = PinchMode::PinchModeFromString(pinchGapString);
        }

        if (deck.hasKeyword<ParserKeywords::MINPV>() && deck.hasKeyword<ParserKeywords::MINPVFIL>()) {
            throw std::invalid_argument("Can not have both MINPV and MINPVFIL in deck.");
        }

        m_minpvVector.resize(getCartesianSize(), 0.0);
        if (deck.hasKeyword<ParserKeywords::MINPV>()) {
            const auto& record = deck.get<ParserKeywords::MINPV>( ).back().getRecord(0);
            const auto& item = record.getItem<ParserKeywords::MINPV::VALUE>( );
            std::fill(m_minpvVector.begin(), m_minpvVector.end(), item.getSIDouble(0));
            m_minpvMode = MinpvMode::ModeEnum::EclSTD;
        } else if(deck.hasKeyword<ParserKeywords::MINPVV>()) {
            // We should use the grid properties to support BOX, but then we need the eclipseState
            const auto& record = deck.get<ParserKeywords::MINPVV>( ).back().getRecord(0);
            m_minpvVector =record.getItem(0).getSIDoubleData();
            m_minpvMode = MinpvMode::ModeEnum::EclSTD;
        }

        if (actnum != nullptr) {
            this->resetACTNUM(actnum);
        }
        else if (! this->m_useActnumFromGdfile) {
            const auto fp = FieldProps {
                deck, EclipseGrid { static_cast<GridDims&>(*this) }
            };

            this->resetACTNUM(fp.actnumRaw());
        }
    }

    void EclipseGrid::initGridFromEGridFile(Opm::EclIO::EclFile& egridfile, std::string fileName){
        OpmLog::info(fmt::format("\nCreating grid from: {} ", fileName));

        if (!egridfile.hasKey("GRIDHEAD")) {
            throw std::invalid_argument("file: " + fileName + " is not a valid egrid file, GRIDHEAD not found");
        }

        if (!egridfile.hasKey("COORD")) {
            throw std::invalid_argument("file: " + fileName + " is not a valid egrid file, COORD not found");
        }

        if (!egridfile.hasKey("ZCORN")) {
            throw std::invalid_argument("file: " + fileName + " is not a valid egrid file, ZCORN not found");
        }

        if (!egridfile.hasKey("GRIDUNIT")) {
            throw std::invalid_argument("file: " + fileName + " is not a valid egrid file, GRIDUNIT not found");
        }

        const std::vector<std::string>& gridunit = egridfile.get<std::string>("GRIDUNIT");

        const std::vector<int>& gridhead = egridfile.get<int>("GRIDHEAD");
        const std::array<int, 3> dims = {gridhead[1], gridhead[2], gridhead[3]};

        m_nx=dims[0];
        m_ny=dims[1];
        m_nz=dims[2];

        const std::vector<float>& coord_f = egridfile.get<float>("COORD");
        const std::vector<float>& zcorn_f = egridfile.get<float>("ZCORN");

        m_coord.assign(coord_f.begin(), coord_f.end());
        m_zcorn.assign(zcorn_f.begin(), zcorn_f.end());

        if (gridunit[0] != "METRES") {

            const auto length = ::Opm::UnitSystem::measure::length;

            if (gridunit[0] == "FEET"){
                Opm::UnitSystem units(Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD );
                units.to_si(length, m_coord);
                units.to_si(length, m_zcorn);
            } else if (gridunit[0] == "CM"){
                Opm::UnitSystem units(Opm::UnitSystem::UnitType::UNIT_TYPE_LAB );
                units.to_si(length, m_coord);
                units.to_si(length, m_zcorn);
            } else {
                std::string message = "gridunit '" + gridunit[0] + "' doesn't correspong to a valid unit system";
                throw std::invalid_argument(message);
            }
        }

        if ((egridfile.hasKey("ACTNUM")) && (m_useActnumFromGdfile)) {
            const std::vector<int>& actnum  = egridfile.get<int>("ACTNUM");
            resetACTNUM( actnum );
        } else
            resetACTNUM( );

        if (egridfile.hasKey("MAPAXES"))
            this->m_mapaxes = std::make_optional<MapAxes>(egridfile);


        ZcornMapper mapper( getNX(), getNY(), getNZ());
        zcorn_fixed = mapper.fixupZCORN( m_zcorn );
    }

    bool EclipseGrid::keywInputBeforeGdfile(const Deck& deck, const std::string& keyword) const {

        std::vector<std::string> keywordList;
        keywordList.reserve(deck.size());

        for (size_t n=0;n<deck.size();n++){
            DeckKeyword kw = deck[n];
            keywordList.push_back(kw.name());
        }

        int indKeyw = -1;
        int indGdfile = -1;

        for (size_t i = 0; i < keywordList.size(); i++){
            if (keywordList[i]=="GDFILE"){
                indGdfile=i;
            }

            if (keywordList[i]==keyword){
                indKeyw=i;
            }
        }

        if (indGdfile == -1) {
            throw std::runtime_error("keyword GDFILE not found in deck ");
        }

        if (indKeyw == -1) {
            throw std::runtime_error("keyword " + keyword + " not found in deck ");
        }

        return indKeyw < indGdfile;
    }


    size_t EclipseGrid::activeIndex(size_t i, size_t j, size_t k) const {

        return activeIndex( getGlobalIndex( i,j,k ));
    }

    size_t EclipseGrid::activeIndex(size_t globalIndex) const {
        if (m_global_to_active.empty()) {
            return globalIndex;
        }

        if (m_global_to_active[ globalIndex] == -1) {
            throw std::invalid_argument("Input argument does not correspond to an active cell");
        }

        return m_global_to_active[ globalIndex];
    }

    /**
       Observe: the input argument is assumed to be in the space
       [0,num_active).
    */

    size_t EclipseGrid::getGlobalIndex(size_t active_index) const {
        return m_active_to_global.at(active_index);
    }

    size_t EclipseGrid::getGlobalIndex(size_t i, size_t j, size_t k) const {

        return GridDims::getGlobalIndex(i,j,k);
    }


    bool EclipseGrid::isPinchActive( ) const {
        return m_pinch.has_value();
    }

    double EclipseGrid::getPinchThresholdThickness( ) const {
        return m_pinch.value();
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

    PinchMode::ModeEnum EclipseGrid::getPinchGapMode() const {
        return m_pinchGapMode;
    }

    const std::vector<double>& EclipseGrid::getMinpvVector( ) const {
        return m_minpvVector;
    }

    void EclipseGrid::initBinaryGrid(const Deck& deck) {

        const DeckKeyword& gdfile_kw = deck["GDFILE"].back();
        const std::string& gdfile_arg = gdfile_kw.getRecord(0).getItem("filename").get<std::string>(0);
        std::string filename = deck.makeDeckPath(gdfile_arg);

        std::unique_ptr<Opm::EclIO::EclFile> egridfile;
        // Turns out some windows applications export .DATA files with relative
        // GDFILE keywords with a windows formatted path, if open fails we give
        // it one more try with the replacement '\'' -> '/'.
        try {
            egridfile = std::make_unique<Opm::EclIO::EclFile>(filename);
        } catch (const std::runtime_error& ) {
            std::replace(filename.begin(), filename.end(), '\\', '/');
            egridfile = std::make_unique<Opm::EclIO::EclFile>(filename);
        }

        initGridFromEGridFile(*egridfile, filename);
    }

    void EclipseGrid::initCartesianGrid(const Deck& deck) {

        if (hasDVDEPTHZKeywords( deck )) {
            initDVDEPTHZGrid(deck );
        } else if (hasDTOPSKeywords(deck)) {
            initDTOPSGrid( deck );
        } else {
            throw std::invalid_argument("Tried to initialize cartesian grid without all required keywords");
        }
    }

    void EclipseGrid::initDVDEPTHZGrid(const Deck& deck) {
      OpmLog::info(fmt::format("\nCreating grid from keywords DXV, DYV, DZV and DEPTHZ"));
        const std::vector<double>& DXV = deck.get<ParserKeywords::DXV>().back().getSIDoubleData();
        const std::vector<double>& DYV = deck.get<ParserKeywords::DYV>().back().getSIDoubleData();
        const std::vector<double>& DZV = deck.get<ParserKeywords::DZV>().back().getSIDoubleData();
        const std::vector<double>& DEPTHZ = deck.get<ParserKeywords::DEPTHZ>().back().getSIDoubleData();
        auto nx = this->getNX();
        auto ny = this->getNY();
        auto nz = this->getNZ();

        assertVectorSize( DEPTHZ , static_cast<size_t>( (nx + 1)*(ny +1 )) , "DEPTHZ");
        assertVectorSize( DXV    , static_cast<size_t>( nx ) , "DXV");
        assertVectorSize( DYV    , static_cast<size_t>( ny ) , "DYV");
        assertVectorSize( DZV    , static_cast<size_t>( nz ) , "DZV");

        m_coord = makeCoordDxvDyvDzvDepthz(DXV, DYV, DZV, DEPTHZ);
        m_zcorn = makeZcornDzvDepthz(DZV, DEPTHZ);

        ZcornMapper mapper( getNX(), getNY(), getNZ());
        zcorn_fixed = mapper.fixupZCORN( m_zcorn );
    }

    void EclipseGrid::initDTOPSGrid(const Deck& deck) {
      OpmLog::info(fmt::format("\nCreating grid from keywords DX, DY, DZ and TOPS"));

        std::vector<double> DX = EclipseGrid::createDVector(  this->getNXYZ(), 0 , "DX" , "DXV" , deck);
        std::vector<double> DY = EclipseGrid::createDVector(  this->getNXYZ(), 1 , "DY" , "DYV" , deck);
        std::vector<double> DZ = EclipseGrid::createDVector(  this->getNXYZ(), 2 , "DZ" , "DZV" , deck);
        std::vector<double> TOPS = EclipseGrid::createTOPSVector( this->getNXYZ(), DZ , deck );

        m_coord = makeCoordDxDyDzTops(DX, DY, DZ, TOPS);
        m_zcorn = makeZcornDzTops(DZ, TOPS);

        ZcornMapper mapper( getNX(), getNY(), getNZ());
        zcorn_fixed = mapper.fixupZCORN( m_zcorn );
    }


    void EclipseGrid::getCellCorners(const std::array<int, 3>& ijk, const std::array<int, 3>& dims,
                                     std::array<double,8>& X,
                                     std::array<double,8>& Y,
                                     std::array<double,8>& Z) const
    {

        std::array<int, 8> zind;
        std::array<int, 4> pind;

        // calculate indices for grid pillars in COORD arrray
        const size_t p_offset = ijk[1]*(dims[0]+1)*6 + ijk[0]*6;

        pind[0] = p_offset;
        pind[1] = p_offset + 6;
        pind[2] = p_offset + (dims[0]+1)*6;
        pind[3] = pind[2] + 6;

        // get depths from zcorn array in ZCORN array
        const size_t z_offset = ijk[2]*dims[0]*dims[1]*8 + ijk[1]*dims[0]*4 + ijk[0]*2;

        zind[0] = z_offset;
        zind[1] = z_offset + 1;
        zind[2] = z_offset + dims[0]*2;
        zind[3] = zind[2] + 1;

        for (int n = 0; n < 4; n++)
            zind[n+4] = zind[n] + dims[0]*dims[1]*4;


        for (int n = 0; n< 8; n++)
           Z[n] = m_zcorn[zind[n]];


        for (int  n=0; n<4; n++) {
            double xt = m_coord[pind[n]];
            double yt = m_coord[pind[n] + 1];
            double zt = m_coord[pind[n] + 2];

            double xb = m_coord[pind[n] + 3];
            double yb = m_coord[pind[n] + 4];
            double zb = m_coord[pind[n]+5];

            if (zt == zb) {
                X[n] = xt;
                X[n + 4] = xt;

                Y[n] = yt;
                Y[n + 4] = yt;
            } else {
                X[n] = xt + (xb-xt) / (zt-zb) * (zt - Z[n]);
                X[n+4] = xt + (xb-xt) / (zt-zb) * (zt-Z[n+4]);

                Y[n] = yt+(yb-yt)/(zt-zb)*(zt-Z[n]);
                Y[n+4] = yt+(yb-yt)/(zt-zb)*(zt-Z[n+4]);
            }
        }
    }


    void EclipseGrid::getCellCorners(const std::size_t globalIndex,
                                     std::array<double,8>& X,
                                     std::array<double,8>& Y,
                                     std::array<double,8>& Z) const
    {
        this->assertGlobalIndex(globalIndex);
        auto ijk = this->getIJK(globalIndex);
        this->getCellCorners(ijk, this->getNXYZ(), X, Y, Z);
    }


    std::vector<double> EclipseGrid::makeCoordDxvDyvDzvDepthz(const std::vector<double>& dxv, const std::vector<double>& dyv, const std::vector<double>& dzv, const std::vector<double>& depthz) const {
        auto nx = this->getNX();
        auto ny = this->getNY();
        auto nz = this->getNZ();

        std::vector<double> coord;
        coord.reserve((nx+1)*(ny+1)*6);

        std::vector<double> x(nx + 1, 0.0);
        std::partial_sum(dxv.begin(), dxv.end(), x.begin() + 1);

        std::vector<double> y(ny + 1, 0.0);
        std::partial_sum(dyv.begin(), dyv.end(), y.begin() + 1);

        std::vector<double> z(nz + 1, 0.0);
        std::partial_sum(dzv.begin(), dzv.end(), z.begin() + 1);


        for (std::size_t j = 0; j < ny + 1; j++) {
            for (std::size_t i = 0; i<nx + 1; i++) {

                const double x0 = x[i];
                const double y0 = y[j];

                size_t ind = i+j*(nx+1);

                double zb = depthz[ind] + z[nz];

                coord.push_back(x0);
                coord.push_back(y0);
                coord.push_back(depthz[ind]);
                coord.push_back(x0);
                coord.push_back(y0);
                coord.push_back(zb);
            }
        }

        return coord;
    }

    std::vector<double> EclipseGrid::makeZcornDzvDepthz(const std::vector<double>& dzv, const std::vector<double>& depthz) const {
        auto nx = this->getNX();
        auto ny = this->getNY();
        auto nz = this->getNZ();

        std::vector<double> zcorn;
        size_t sizeZcorn = nx*ny*nz*8;

        zcorn.assign (sizeZcorn, 0.0);

        std::vector<double> z(nz + 1, 0.0);
        std::partial_sum(dzv.begin(), dzv.end(), z.begin() + 1);


        for (std::size_t k = 0; k < nz; k++) {
            for (std::size_t j = 0; j < ny; j++) {
                for (std::size_t i = 0; i < nx; i++) {

                    const double z0 = z[k];

                    // top face of cell
                    auto zind = i*2 + j*nx*4 + k*nx*ny*8;

                    zcorn[zind] = depthz[i+j*(nx+1)] + z0;
                    zcorn[zind + 1] = depthz[i+j*(nx+1) +1 ] + z0;

                    zind = zind + nx*2;

                    zcorn[zind] = depthz[i+(j+1)*(nx+1)] + z0;
                    zcorn[zind + 1] = depthz[i+(j+1)*(nx+1) + 1] + z0;

                    // bottom face of cell
                    zind = i*2 + j*nx*4 + k*nx*ny*8 + nx*ny*4;

                    zcorn[zind] = depthz[i+j*(nx+1)] + z0 + dzv[k];
                    zcorn[zind + 1] = depthz[i+j*(nx+1) +1 ] + z0 + dzv[k];

                    zind = zind + nx*2;

                    zcorn[zind] = depthz[i+(j+1)*(nx+1)] + z0 + dzv[k];
                    zcorn[zind + 1] = depthz[i+(j+1)*(nx+1) + 1] + z0 + dzv[k];
                }
            }
        }

        return zcorn;
    }

    namespace
    {
        std::vector<double> makeSumIdirAtK(const int nx, const int ny, const int k, const std::vector<double>& dx)
        {
            std::vector<double> s(nx * ny, 0.0);
            for (int j = 0; j < ny; ++j) {
                double sum = 0.0;
                for (int i = 0; i < nx; ++i) {
                    sum += dx[i + j*nx + k*nx*ny];
                    s[i + j*nx] = sum;
                }
            }
            return s;
        }

        std::vector<double> makeSumJdirAtK(const int nx, const int ny, const int k, const std::vector<double>& dy)
        {
            std::vector<double> s(nx * ny, 0.0);
            for (int i = 0; i < nx; ++i) {
                double sum = 0.0;
                for (int j = 0; j < ny; ++j) {
                    sum += dy[i + j*nx + k*nx*ny];
                    s[i + j*nx] = sum;
                }
            }
            return s;
        }

        std::vector<double> makeSumKdir(const int nx, const int ny, const int nz, const std::vector<double>& dz)
        {
            std::vector<double> s(nx * ny, 0.0);
            for (int i = 0; i < nx; ++i) {
                for (int j = 0; j < ny; ++j) {
                    double sum = 0.0;
                    for (int k = 0; k < nz; ++k) {
                        sum += dz[i + j*nx + k*nx*ny];
                    }
                    s[i + j*nx] = sum;
                }
            }
            return s;
        }

    } // anonymous namespace

    std::vector<double> EclipseGrid::makeCoordDxDyDzTops(const std::vector<double>& dx, const std::vector<double>& dy, const std::vector<double>& dz, const std::vector<double>& tops) const {
        auto nx = this->getNX();
        auto ny = this->getNY();
        auto nz = this->getNZ();

        std::vector<double> coord;
        coord.reserve((nx+1)*(ny+1)*6);

        std::vector<double> sum_idir_top = makeSumIdirAtK(nx, ny, 0, dx);
        std::vector<double> sum_idir_bot = makeSumIdirAtK(nx, ny, nz - 1, dx);
        std::vector<double> sum_jdir_top = makeSumJdirAtK(nx, ny, 0, dy);
        std::vector<double> sum_jdir_bot = makeSumJdirAtK(nx, ny, nz - 1, dy);
        std::vector<double> sum_kdir = makeSumKdir(nx, ny, nz, dz);

        for (std::size_t j = 0; j < ny; j++) {

            double y0 = 0;
            double zt = tops[0];
            double zb = zt + sum_kdir[0 + 0*nx];

            if (j == 0) {
                double x0 = 0.0;

                coord.push_back(x0);
                coord.push_back(y0);
                coord.push_back(zt);
                coord.push_back(x0);
                coord.push_back(y0);
                coord.push_back(zb);

                for (std::size_t i = 0; i < nx; i++) {

                    size_t ind = i+j*nx+1;

                    if (i == (nx-1)) {
                        ind=ind-1;
                    }

                    zt = tops[ind];
                    zb = zt + sum_kdir[i + j*nx];

                    double xt = x0 + dx[i + j*nx];
                    double xb = sum_idir_bot[i + j*nx];

                    coord.push_back(xt);
                    coord.push_back(y0);
                    coord.push_back(zt);
                    coord.push_back(xb);
                    coord.push_back(y0);
                    coord.push_back(zb);

                    x0=xt;
                }
            }

            std::size_t ind = (j+1)*nx;

            if (j == (ny-1) ) {
                ind = j*nx;
            }

            double x0 = 0.0;

            double yt = sum_jdir_top[0 + j*nx];
            double yb = sum_jdir_bot[0 + j*nx];

            zt = tops[ind];
            zb = zt + sum_kdir[0 + j*nx];

            coord.push_back(x0);
            coord.push_back(yt);
            coord.push_back(zt);
            coord.push_back(x0);
            coord.push_back(yb);
            coord.push_back(zb);

            for (std::size_t i=0; i < nx; i++) {

                ind = i+(j+1)*nx+1;

                if (j == (ny-1) ) {
                    ind = i+j*nx+1;
                }

                if (i == (nx - 1) ) {
                    ind=ind-1;
                }

                zt = tops[ind];
                zb = zt + sum_kdir[i + j*nx];

                double xt=-999;
                double xb;

                if (j == (ny-1) ) {
                    xt = sum_idir_top[i + j*nx];
                    xb = sum_idir_bot[i + j*nx];
                } else {
                    xt = sum_idir_top[i + (j+1)*nx];
                    xb = sum_idir_bot[i + (j+1)*nx];
                }

                if (i == (nx - 1) ) {
                    yt = sum_jdir_top[i + j*nx];
                    yb = sum_jdir_bot[i + j*nx];
                } else {
                    yt = sum_jdir_top[(i + 1) + j*nx];
                    yb = sum_jdir_bot[(i + 1) + j*nx];
                }

                coord.push_back(xt);
                coord.push_back(yt);
                coord.push_back(zt);
                coord.push_back(xb);
                coord.push_back(yb);
                coord.push_back(zb);
            }
        }

        return coord;
    }

    std::vector<double> EclipseGrid::makeZcornDzTops(const std::vector<double>& dz, const std::vector<double>& tops) const {

        std::vector<double> zcorn;
        size_t sizeZcorn = this->getCartesianSize() * 8;
        auto nx = this->getNX();
        auto ny = this->getNY();
        auto nz = this->getNZ();
        zcorn.assign (sizeZcorn, 0.0);

        for (std::size_t j = 0; j < ny; j++) {
            for (std::size_t i = 0; i < nx; i++) {
                std::size_t ind = i + j*nx;
                double z = tops[ind];

                for (std::size_t k = 0; k < nz; k++) {

                    // top face of cell
                    std::size_t zind = i*2 + j*nx*4 + k*nx*ny*8;

                    zcorn[zind] = z;
                    zcorn[zind + 1] = z;

                    zind = zind + nx*2;

                    zcorn[zind] = z;
                    zcorn[zind + 1] = z;

                    z = z + dz[i + j*nx + k*nx*ny];

                    // bottom face of cell
                    zind = i*2 + j*nx*4 + k*nx*ny*8 + nx*ny*4;

                    zcorn[zind] = z;
                    zcorn[zind + 1] = z;

                    zind = zind + nx*2;

                    zcorn[zind] = z;
                    zcorn[zind + 1] = z;
                }
            }
        }

        return zcorn;
    }

    void EclipseGrid::initCylindricalGrid(const Deck& deck)
    {
        initSpiderwebOrCylindricalGrid(deck, true);
    }

    void EclipseGrid::initSpiderwebGrid(const Deck& deck)
    {
        initSpiderwebOrCylindricalGrid(deck, false);
    }

    /*
      Limited implementaton - requires keywords: DRV, DTHETAV, DZV or DZ, and TOPS.
    */

    void EclipseGrid::initSpiderwebOrCylindricalGrid(const Deck& deck, const bool is_cylindrical)
    {
        const std::string kind = is_cylindrical ? "cylindrical" : "spiderweb";

        // The hasCylindricalKeywords( ) checks according to the
        // eclipse specification for RADIAL grid. We currently do not support all
        // aspects of cylindrical grids, we therefor have an
        // additional test here, which checks if we have the keywords
        // required by the current implementation.
        if (!hasCylindricalKeywords(deck))
            throw std::invalid_argument("Not all keywords required for " + kind + " grids present");

        if (!deck.hasKeyword<ParserKeywords::DTHETAV>())
            throw std::logic_error("The current implementation *must* have theta values specified using the DTHETAV keyword");

        if (!deck.hasKeyword<ParserKeywords::DRV>())
            throw std::logic_error("The current implementation *must* have radial values specified using the DRV keyword");

        if (!(deck.hasKeyword<ParserKeywords::DZ>() || deck.hasKeyword<ParserKeywords::DZV>()) || !deck.hasKeyword<ParserKeywords::TOPS>())
            throw std::logic_error("The vertical cell size must be specified using the DZ or DZV, and the TOPS keywords");

        const std::vector<double>& drv     = deck.get<ParserKeywords::DRV>().back().getSIDoubleData();
        const std::vector<double>& dthetav = deck.get<ParserKeywords::DTHETAV>().back().getSIDoubleData();
        const std::vector<double>& tops    = deck.get<ParserKeywords::TOPS>().back().getSIDoubleData();
        OpmLog::info(fmt::format("\nCreating {} grid from keywords DRV, DTHETAV, DZV and TOPS", kind));

        if (drv.size() != this->getNX())
            throw std::invalid_argument("DRV keyword should have exactly " + std::to_string( this->getNX() ) + " elements");

        if (dthetav.size() != this->getNY())
            throw std::invalid_argument("DTHETAV keyword should have exactly " + std::to_string( this->getNY() ) + " elements");

        auto area = this->getNX() * this->getNY();
        size_t volume = this->getNX() * this->getNY() * this->getNZ();

        std::vector<double> dz(volume);
        if (deck.hasKeyword<ParserKeywords::DZ>()) {
            const std::vector<double>& dz_deck = deck.get<ParserKeywords::DZ>().back().getSIDoubleData();
            if (dz_deck.size() != volume)
                throw std::invalid_argument("DZ keyword should have exactly " + std::to_string( volume ) + " elements");
            dz = dz_deck;
        } else {
            const std::vector<double>& dzv = deck.get<ParserKeywords::DZV>().back().getSIDoubleData();
            if (dzv.size() != this->getNZ())
                throw std::invalid_argument("DZV keyword should have exactly " + std::to_string( this->getNZ() ) + " elements");
            for (std::size_t k= 0; k < this->getNZ(); k++)
                std::fill(dz.begin() + k*area, dz.begin() + (k+1)*area, dzv[k]);
        }

        if (tops.size() != (this->getNX() * this->getNY()))
            throw std::invalid_argument("TOPS keyword should have exactly " + std::to_string( this->getNX() * this->getNY() ) + " elements");

        {
            double total_angle = 0;
            for (auto theta : dthetav)
                total_angle += theta;

            if (std::abs( total_angle - 360 ) < 0.01)
                m_circle = deck.hasKeyword<ParserKeywords::CIRCLE>();
            else {
                if (total_angle > 360)
                    throw std::invalid_argument("More than 360 degrees rotation - cells will be double covered");
            }
        }

        /*
          Now the data has been validated, now we continue to create
          ZCORN and COORD vectors, and we are almost done.
        */
        {
            ZcornMapper zm( this->getNX(), this->getNY(), this->getNZ());
            CoordMapper cm(this->getNX(), this->getNY());
            std::vector<double> zcorn( zm.size() );
            std::vector<double> coord( cm.size() );
            {                    
                std::vector<double> depth(tops);
                for (std::size_t k = 0; k < this->getNZ(); k++) {
                    for (std::size_t j = 0; j < this->getNY(); j++) {
                        for (std::size_t i = 0; i < this->getNX(); i++) {
                            auto current_depth = depth[j * this->getNX() + i];
                            auto next_depth = current_depth + dz[k * area + j * this->getNX() + i];
                            for (size_t c=0; c < 4; c++) {
                                zcorn[ zm.index(i,j,k,c) ]     = current_depth;
                                zcorn[ zm.index(i,j,k,c + 4) ] = next_depth;
                            }
                            depth[j * this->getNX() + i] = next_depth;
                        }
                    }
                }
            }
            {
                std::vector<double> ri(this->getNX() + 1);
                std::vector<double> tj(this->getNY() + 1);
                double z1 = *std::min_element( zcorn.begin() , zcorn.end());
                double z2 = *std::max_element( zcorn.begin() , zcorn.end());
                ri[0] = deck.get<ParserKeywords::INRAD>().back().getRecord(0).getItem(0).getSIDouble( 0 );
                for (std::size_t i = 1; i <= this->getNX(); i++)
                    ri[i] = ri[i - 1] + drv[i - 1];

                tj[0] = 0;
                for (std::size_t j = 1; j <= this->getNY(); j++)
                    tj[j] = tj[j - 1] + dthetav[j - 1];


                for (std::size_t j = 0; j <= this->getNY(); j++) {
                    /*
                      The theta value is supposed to go counterclockwise, starting at 'twelve o clock'.
                    */
                    double t = M_PI * (90 - tj[j]) / 180;
                    double c = cos( t );
                    double s = sin( t );
                    for (std::size_t i = 0; i <= this->getNX(); i++) {
                        double r = ri[i];
                        double x = r*c;
                        double y = r*s;

                        coord[ cm.index(i,j,0,0) ] = x;
                        coord[ cm.index(i,j,1,0) ] = y;
                        coord[ cm.index(i,j,2,0) ] = z1;

                        coord[ cm.index(i,j,0,1) ] = x;
                        coord[ cm.index(i,j,1,1) ] = y;
                        coord[ cm.index(i,j,2,1) ] = z2;
                    }
                }

                // Save angles, used by the cylindrical grid to calculate volumes.
                if (is_cylindrical) {
                    m_rv = ri;
                    m_thetav = dthetav;
                }

            }
            initCornerPointGrid( coord, zcorn, nullptr);
        }
    }



    void EclipseGrid::initCornerPointGrid(const std::vector<double>& coord ,
                                          const std::vector<double>& zcorn ,
                                          const int * actnum)


    {
        m_coord = coord;
        m_zcorn = zcorn;

        ZcornMapper mapper( getNX(), getNY(), getNZ());
        zcorn_fixed = mapper.fixupZCORN( m_zcorn );
        this->resetACTNUM(actnum);
    }

    void EclipseGrid::initCornerPointGrid(const Deck& deck)
    {
        this->assertCornerPointKeywords(deck);

        OpmLog::info(fmt::format("\nCreating corner-point grid from "
                                 "keywords COORD, ZCORN and others"));

        const auto& coord = deck.get<ParserKeywords::COORD>().back();
        const auto& zcorn = deck.get<ParserKeywords::ZCORN>().back();

        this->initCornerPointGrid(coord.getSIDoubleData(),
                                  zcorn.getSIDoubleData(),
                                  nullptr);
    }

    bool EclipseGrid::hasCornerPointKeywords(const Deck& deck) {
        if (deck.hasKeyword<ParserKeywords::ZCORN>() && deck.hasKeyword<ParserKeywords::COORD>())
            return true;
        else
            return false;
    }


    void EclipseGrid::assertCornerPointKeywords(const Deck& deck)
    {
        const int nx = this->getNX();
        const int ny = this->getNY();
        const int nz = this->getNZ();
        {
            const auto& ZCORNKeyWord = deck.get<ParserKeywords::ZCORN>().back();

            if (ZCORNKeyWord.getDataSize() != static_cast<size_t>(8*nx*ny*nz)) {
                const std::string msg =
                    "Wrong size of the ZCORN keyword: Expected 8*x*ny*nz = "
                    + std::to_string(static_cast<long long>(8*nx*ny*nz)) + " is "
                    + std::to_string(static_cast<long long>(ZCORNKeyWord.getDataSize()));
                OpmLog::error(msg);
                throw std::invalid_argument(msg);
            }
        }

        {
            const auto& COORDKeyWord = deck.get<ParserKeywords::COORD>().back();
            if (COORDKeyWord.getDataSize() != static_cast<size_t>(6*(nx + 1)*(ny + 1))) {
                const std::string msg =
                    "Wrong size of the COORD keyword: Expected 6*(nx + 1)*(ny + 1) = "
                    + std::to_string(static_cast<long long>(6*(nx + 1)*(ny + 1))) + " is "
                    + std::to_string(static_cast<long long>(COORDKeyWord.getDataSize()));
                OpmLog::error(msg);
                throw std::invalid_argument(msg);
            }
        }
    }


    bool EclipseGrid::hasGDFILE(const Deck& deck) {
        return deck.hasKeyword<ParserKeywords::GDFILE>();
    }

    bool EclipseGrid::hasCartesianKeywords(const Deck& deck) {
        if (hasDVDEPTHZKeywords( deck ))
            return true;
        else
            return hasDTOPSKeywords(deck);
    }

    bool EclipseGrid::hasCylindricalKeywords(const Deck& deck) {
        if (deck.hasKeyword<ParserKeywords::INRAD>() &&
            deck.hasKeyword<ParserKeywords::TOPS>() &&
            (deck.hasKeyword<ParserKeywords::DZ>() || deck.hasKeyword<ParserKeywords::DZV>()) &&
            (deck.hasKeyword<ParserKeywords::DRV>() || deck.hasKeyword<ParserKeywords::DR>()) &&
            (deck.hasKeyword<ParserKeywords::DTHETA>() || deck.hasKeyword<ParserKeywords::DTHETAV>()))
            return true;
        else
            return false;
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
    
    bool EclipseGrid::hasEqualDVDEPTHZ(const Deck& deck) {
        const std::vector<double>& DXV = deck.get<ParserKeywords::DXV>().back().getSIDoubleData();
        const std::vector<double>& DYV = deck.get<ParserKeywords::DYV>().back().getSIDoubleData();
        const std::vector<double>& DZV = deck.get<ParserKeywords::DZV>().back().getSIDoubleData();
        const std::vector<double>& DEPTHZ = deck.get<ParserKeywords::DEPTHZ>().back().getSIDoubleData();
        if (EclipseGrid::allEqual(DXV) &&
            EclipseGrid::allEqual(DYV)&&
            EclipseGrid::allEqual(DZV)&& 
            EclipseGrid::allEqual(DEPTHZ))
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
            throw std::invalid_argument("Wrong size for keyword: " + vectorName + ". Expected: " + std::to_string(expectedSize) + " got: " + std::to_string(vector.size()));
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

    std::vector<double> EclipseGrid::createTOPSVector(const std::array<int, 3>& dims,
            const std::vector<double>& DZ, const Deck& deck)
    {
        double z_tolerance = 1e-6;
        size_t volume = dims[0] * dims[1] * dims[2];
        size_t area = dims[0] * dims[1];
        const auto& TOPSKeyWord = deck.get<ParserKeywords::TOPS>().back();
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

std::vector<double> EclipseGrid::createDVector(const std::array<int,3>& dims, std::size_t dim, const std::string& DKey,
            const std::string& DVKey, const Deck& deck)
    {
        size_t volume = dims[0] * dims[1] * dims[2];
        size_t area = dims[0] * dims[1];
        std::vector<double> D;
        if (deck.hasKeyword(DKey)) {
            D = deck[DKey].back().getSIDoubleData();


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
            const auto& DVKeyWord = deck[DVKey].back();
            const std::vector<double>& DV = DVKeyWord.getSIDoubleData();
            if (DV.size() != static_cast<std::size_t>(dims[dim]))
                throw std::invalid_argument(DVKey + " size mismatch");
            D.resize( volume );
            scatterDim( dims , dim , DV , D );
        }
        return D;
    }


    void EclipseGrid::scatterDim(const std::array<int, 3>& dims , size_t dim , const std::vector<double>& DV , std::vector<double>& D) {
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
    
    bool EclipseGrid::allEqual(const std::vector<double> &v) {
        auto comp = [](double x, double y) { return std::fabs(x - y) < 1e-12; };
        return std::equal(v.begin() + 1, v.end(), v.begin(), comp);
    }
    
    bool EclipseGrid::equal(const EclipseGrid& other) const {

        //double reltol = 1.0e-6;

        if (m_coord.size() != other.m_coord.size())
            return false;

        if (m_zcorn.size() != other.m_zcorn.size())
            return false;

        if (!(m_mapaxes == other.m_mapaxes))
            return false;

        if (m_actnum != other.m_actnum)
            return false;

        if (m_coord != other.m_coord)
            return false;

        if (m_zcorn != other.m_zcorn)
            return false;

        bool status = ((m_pinch == other.m_pinch)  && (m_minpvMode == other.getMinpvMode()));

        if(m_minpvMode!=MinpvMode::ModeEnum::Inactive) {
            status = status && (m_minpvVector == other.getMinpvVector());
        }

        return status;
    }

    size_t EclipseGrid::getNumActive( ) const {
        return this->m_nactive;
    }

    bool EclipseGrid::allActive( ) const {
        return (getNumActive() == getCartesianSize());
    }

    bool EclipseGrid::cellActive( size_t globalIndex ) const {
        assertGlobalIndex( globalIndex );
        if (m_actnum.empty()) {
            return true;
        } else {
            return m_actnum[globalIndex]>0;
        }
    }

    bool EclipseGrid::cellActive( size_t i , size_t j , size_t k ) const {
        assertIJK(i,j,k);

        size_t globalIndex = getGlobalIndex(i,j,k);
        return this->cellActive(globalIndex);
    }

    const std::vector<double>& EclipseGrid::activeVolume() const {
        if (!this->active_volume.has_value()) {
            std::vector<double> volume(this->m_nactive);

            #pragma omp parallel for schedule(static)
            for (int active_index = 0; active_index < this->m_active_to_global.size(); active_index++) {
                std::array<double,8> X;
                std::array<double,8> Y;
                std::array<double,8> Z;
                auto global_index = this->m_active_to_global[active_index];
                this->getCellCorners(global_index, X, Y, Z );
                if (m_rv && m_thetav) {
                    const auto[i,j,k] = this->getIJK(global_index);
                    auto& r = *m_rv;
                    auto& t = *m_thetav;
                    volume[active_index] = calculateCylindricalCellVol(r[i], r[i+1], t[j], Z[4] - Z[0]);
                } else
                    volume[active_index] = calculateCellVol(X, Y, Z);
            }

            this->active_volume = std::move(volume);
        }

        return this->active_volume.value();
    }


    double EclipseGrid::getCellVolume(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        if (this->cellActive(globalIndex) && this->active_volume.has_value()) {
            auto active_index = this->activeIndex(globalIndex);
            return this->active_volume.value()[active_index];
        }

        std::array<double,8> X;
        std::array<double,8> Y;
        std::array<double,8> Z;
        this->getCellCorners(globalIndex, X, Y, Z );
        if (m_rv && m_thetav) {
            const auto[i,j,k] = this->getIJK(globalIndex);
            auto& r = *m_rv;
            auto& t = *m_thetav;
           return calculateCylindricalCellVol(r[i], r[i+1], t[j], Z[4] - Z[0]);
        } else {
            return calculateCellVol(X, Y, Z);
        }
    }

    double EclipseGrid::getCellVolume(size_t i , size_t j , size_t k) const {
        this->assertIJK(i,j,k);
        size_t globalIndex = getGlobalIndex(i,j,k);
        return this->getCellVolume(globalIndex);
    }

    double EclipseGrid::getCellThickness(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);

        const std::array<int, 3> dims = getNXYZ();
        size_t globalIndex = i + j*dims[0] + k*dims[0]*dims[1];

        return getCellThickness(globalIndex);
    }

    double EclipseGrid::getCellThickness(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        std::array<double,8> X;
        std::array<double,8> Y;
        std::array<double,8> Z;
        this->getCellCorners(globalIndex, X, Y, Z );

        double z2 = (Z[4]+Z[5]+Z[6]+Z[7])/4.0;
        double z1 = (Z[0]+Z[1]+Z[2]+Z[3])/4.0;
        double dz = z2-z1;
        return dz;
    }


    std::array<double, 3> EclipseGrid::getCellDims(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        std::array<double,8> X;
        std::array<double,8> Y;
        std::array<double,8> Z;
        this->getCellCorners(globalIndex, X, Y, Z );

        // calculate dx
        double x1 = (X[0]+X[2]+X[4]+X[6])/4.0;
        double y1 = (Y[0]+Y[2]+Y[4]+Y[6])/4.0;
        double x2 = (X[1]+X[3]+X[5]+X[7])/4.0;
        double y2 = (Y[1]+Y[3]+Y[5]+Y[7])/4.0;
        double dx = sqrt(pow((x2-x1), 2.0) + pow((y2-y1), 2.0) );

        // calculate dy
        x1 = (X[0]+X[1]+X[4]+X[5])/4.0;
        y1 = (Y[0]+Y[1]+Y[4]+Y[5])/4.0;
        x2 = (X[2]+X[3]+X[6]+X[7])/4.0;
        y2 = (Y[2]+Y[3]+Y[6]+Y[7])/4.0;
        double dy = sqrt(pow((x2-x1), 2.0) + pow((y2-y1), 2.0));

        // calculate dz

        double z2 = (Z[4]+Z[5]+Z[6]+Z[7])/4.0;
        double z1 = (Z[0]+Z[1]+Z[2]+Z[3])/4.0;
        double dz = z2-z1;


        return std::array<double,3> {{dx, dy, dz}};
    }

    std::array<double, 3> EclipseGrid::getCellDims(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);
        {
            size_t globalIndex = getGlobalIndex( i,j,k );

            return getCellDims(globalIndex);
        }
    }

    std::array<double, 3> EclipseGrid::getCellCenter(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        std::array<double,8> X;
        std::array<double,8> Y;
        std::array<double,8> Z;
        this->getCellCorners(globalIndex, X, Y, Z );
        return std::array<double,3> { { std::accumulate(X.begin(), X.end(), 0.0) / 8.0,
                                        std::accumulate(Y.begin(), Y.end(), 0.0) / 8.0,
                                        std::accumulate(Z.begin(), Z.end(), 0.0) / 8.0 } };
    }


    std::array<double, 3> EclipseGrid::getCellCenter(size_t i,size_t j, size_t k) const {
        assertIJK(i,j,k);

        const std::array<int, 3> dims = getNXYZ();

        size_t globalIndex = i + j*dims[0] + k*dims[0]*dims[1];

        return getCellCenter(globalIndex);
    }

    /*
      This is the numbering of the corners in the cell.

       bottom                           j
         6---7                        /|\
         |   |                         |
         4---5                         |
                                       |
       top                             o---------->  i
         2---3
         |   |
         0---1

     */

    std::array<double, 3> EclipseGrid::getCornerPos(size_t i,size_t j, size_t k, size_t corner_index) const {
        assertIJK(i,j,k);
        if (corner_index >= 8)
            throw std::invalid_argument("Invalid corner position");
        {
            const std::array<int, 3> dims = getNXYZ();

            std::array<double,8> X = {0.0};
            std::array<double,8> Y = {0.0};
            std::array<double,8> Z = {0.0};

            std::array<int, 3> ijk;

            ijk[0] = i;
            ijk[1] = j;
            ijk[2] = k;

            getCellCorners(ijk, dims, X, Y, Z );

            return std::array<double, 3> {{X[corner_index], Y[corner_index], Z[corner_index]}};
        }
    }

    bool EclipseGrid::isValidCellGeomtry(const std::size_t globalIndex,
                                         const UnitSystem& usys) const
    {
        const auto threshold = usys.to_si(UnitSystem::measure::length, 1.0e+20f);

        auto is_finite = [threshold](const double coord_component)
        {
            return std::abs(coord_component) < threshold;
        };

        std::array<double,8> X, Y, Z;
        this->getCellCorners(globalIndex, X, Y, Z);

        const auto finite_coord = std::all_of(X.begin(), X.end(), is_finite)
            && std::all_of(Y.begin(), Y.end(), is_finite)
            && std::all_of(Z.begin(), Z.end(), is_finite);

        if (! finite_coord) {
            return false;
        }

        const auto max_pillar_point_distance = std::max({
            Z[0 + 4] - Z[0 + 0],
            Z[1 + 4] - Z[1 + 0],
            Z[2 + 4] - Z[2 + 0],
            Z[3 + 4] - Z[3 + 0],
        });

        // Define points as "well separated" if maximum distance exceeds
        // 1e-4 length units (e.g., 0.1 mm in METRIC units).  May consider
        // using a coarser tolerance/threshold here.
        return max_pillar_point_distance
            >  usys.to_si(UnitSystem::measure::length, 1.0e-4);
    }

    double EclipseGrid::getCellDepth(size_t globalIndex) const {
        assertGlobalIndex( globalIndex );
        std::array<double,8> X;
        std::array<double,8> Y;
        std::array<double,8> Z;
        this->getCellCorners(globalIndex, X, Y, Z );

        double z2 = (Z[4]+Z[5]+Z[6]+Z[7])/4.0;
        double z1 = (Z[0]+Z[1]+Z[2]+Z[3])/4.0;
        return (z1 + z2)/2.0;
    }

    double EclipseGrid::getCellDepth(size_t i, size_t j, size_t k) const {
        this->assertIJK(i,j,k);
        size_t globalIndex = getGlobalIndex(i,j,k);
        return this->getCellDepth(globalIndex);
    }

    const std::vector<int>& EclipseGrid::getACTNUM( ) const {

        return m_actnum;
    }

    const std::vector<double>& EclipseGrid::getCOORD() const {

        return m_coord;
    }

    size_t EclipseGrid::fixupZCORN() {

        ZcornMapper mapper( getNX(), getNY(), getNZ());

        return mapper.fixupZCORN( m_zcorn );
    }

    const std::vector<double>& EclipseGrid::getZCORN( ) const {

        return m_zcorn;
    }

    void EclipseGrid::save(const std::string& filename, bool formatted, const std::vector<Opm::NNCdata>& nnc, const Opm::UnitSystem& units) const {

        Opm::UnitSystem::UnitType unitSystemType = units.getType();
        const auto length = ::Opm::UnitSystem::measure::length;

        const std::array<int, 3> dims = getNXYZ();

        // Preparing vectors to be saved

        // create coord vector of floats with input units, converted from SI
        std::vector<float> coord_f;
        coord_f.resize(m_coord.size());

        for (size_t n=0; n< m_coord.size(); n++){
            coord_f[n] = static_cast<double>(units.from_si(length, m_coord[n]));
        }

        // create zcorn vector of floats with input units, converted from SI
        std::vector<float> zcorn_f;
        zcorn_f.resize(m_zcorn.size());

        for (size_t n=0; n< m_zcorn.size(); n++){
            zcorn_f[n] = static_cast<double>(units.from_si(length, m_zcorn[n]));
        }

        std::vector<int> filehead(100,0);
        filehead[0] = 3;                     // version number
        filehead[1] = 2007;                  // release year
        filehead[6] = 1;                     // corner point grid

        std::vector<int> gridhead(100,0);
        gridhead[0] = 1;                    // corner point grid
        gridhead[1] = dims[0];              // nI
        gridhead[2] = dims[1];              // nJ
        gridhead[3] = dims[2];              // nK
        gridhead[24] = 1;                   // corner point grid

        std::vector<int> nnchead(10, 0);
        std::vector<int> nnc1;
        std::vector<int> nnc2;

        for (const NNCdata& n : nnc ) {
            nnc1.push_back(n.cell1 + 1);
            nnc2.push_back(n.cell2 + 1);
        }

        nnchead[0] = nnc1.size();

        std::vector<std::string> gridunits;

        switch (unitSystemType) {
        case Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC:
            gridunits.push_back("METRES");
            break;
        case Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD:
            gridunits.push_back("FEET");
            break;
        case Opm::UnitSystem::UnitType::UNIT_TYPE_LAB:
            gridunits.push_back("CM");
            break;
        default:
            OPM_THROW(std::runtime_error, "Unit system not supported when writing to EGRID file");
            break;
        }
        gridunits.push_back("");

        std::vector<int> endgrid = {};

        // Writing vectors to egrid file

        Opm::EclIO::EclOutput egridfile(filename, formatted);
        egridfile.write("FILEHEAD", filehead);

        if (this->m_mapaxes.has_value()) {
            const auto& mapunits = this->m_mapaxes.value().mapunits();
            if (mapunits.has_value())
                egridfile.write("MAPUNITS", std::vector<std::string>{ mapunits.value() });

            egridfile.write("MAPAXES", this->m_mapaxes.value().input());
        }

        egridfile.write("GRIDUNIT", gridunits);
        egridfile.write("GRIDHEAD", gridhead);

        egridfile.write("COORD", coord_f);
        egridfile.write("ZCORN", zcorn_f);

        egridfile.write("ACTNUM", m_actnum);
        egridfile.write("ENDGRID", endgrid);

        if (nnc1.size() > 0){
            egridfile.write("NNCHEAD", nnchead);
            egridfile.write("NNC1", nnc1);
            egridfile.write("NNC2", nnc2);
        }
    }

    const std::vector<int>& EclipseGrid::getActiveMap() const {

        return m_active_to_global;
    }

    void EclipseGrid::resetACTNUM() {
        std::size_t global_size = this->getCartesianSize();
        this->m_actnum.assign(global_size, 1);
        this->m_nactive = global_size;

        this->m_global_to_active.resize(global_size);
        std::iota(this->m_global_to_active.begin(), this->m_global_to_active.end(), 0);
        this->m_active_to_global = this->m_global_to_active;
        this->active_volume = std::nullopt;
    }

    void EclipseGrid::resetACTNUM(const int* actnum) {
        if (actnum == nullptr)
            this->resetACTNUM();
        else {
            auto global_size = this->getCartesianSize();
            this->m_global_to_active.clear();
            this->m_active_to_global.clear();
            this->m_actnum.resize(global_size);
            this->m_nactive = 0;

            for (size_t n = 0; n < global_size; n++) {
                this->m_actnum[n] = actnum[n];
                // numerical aquifer cells need to be active
                if (this->m_aquifer_cells.count(n) > 0) {
                    this->m_actnum[n] = 1;
                }
                if (this->m_actnum[n] > 0) {
                    this->m_global_to_active.push_back(this->m_nactive);
                    this->m_active_to_global.push_back(n);
                    this->m_nactive++;
                } else {
                    this->m_global_to_active.push_back(-1);

                }
            }
            this->active_volume = std::nullopt;
        }
    }

    void EclipseGrid::resetACTNUM(const std::vector<int>& actnum) {
        if (actnum.size() != getCartesianSize())
            throw std::runtime_error("resetACTNUM(): actnum vector size differs from logical cartesian size of grid.");

        this->resetACTNUM(actnum.data());
    }

    ZcornMapper EclipseGrid::zcornMapper() const {
        return ZcornMapper( getNX() , getNY(), getNZ() );
    }

    void EclipseGrid::updateNumericalAquiferCells(const Deck& deck) {
        using AQUNUM =ParserKeywords::AQUNUM;
        if ( !deck.hasKeyword<AQUNUM>() ) {
            return;
        }
        const auto &aqunum_keywords = deck.getKeywordList<AQUNUM>();
        for (const auto &keyword : aqunum_keywords) {
            for (const auto &record : *keyword) {
                const size_t i = record.getItem<AQUNUM::I>().get<int>(0) - 1;
                const size_t j = record.getItem<AQUNUM::J>().get<int>(0) - 1;
                const size_t k = record.getItem<AQUNUM::K>().get<int>(0) - 1;
                const size_t global_index = this->getGlobalIndex(i, j, k);
                this->m_aquifer_cells.insert(global_index);
            }
        }
    }

    ZcornMapper::ZcornMapper(size_t nx , size_t ny, size_t nz)
        : dims( {{nx,ny,nz}} ),
          stride( {{2 , 4*nx, 8*nx*ny}} ),
          cell_shift( {{0 , 1 , 2*nx , 2*nx + 1 , 4*nx*ny , 4*nx*ny + 1, 4*nx*ny + 2*nx , 4*nx*ny + 2*nx + 1 }})
    {
    }


    /* lower layer:   upper layer  (higher value of z - i.e. lower down in resrvoir).

         2---3           6---7
         |   |           |   |
         0---1           4---5
    */

    size_t ZcornMapper::index(size_t i, size_t j, size_t k, int c) const {
        if ((i >= dims[0]) || (j >= dims[1]) || (k >= dims[2]) || (c < 0) || (c >= 8))
            throw std::invalid_argument("Invalid cell argument");

        return i*stride[0] + j*stride[1] + k*stride[2] + cell_shift[c];
    }

    size_t ZcornMapper::size() const {
        return dims[0] * dims[1] * dims[2] * 8;
    }

    size_t ZcornMapper::index(size_t g, int c) const {
        int k = g / (dims[0] * dims[1]);
        g -= k * dims[0] * dims[1];

        int j = g / dims[0];
        g -= j * dims[0];

        int i = g;

        return index(i,j,k,c);
    }

    bool ZcornMapper::validZCORN( const std::vector<double>& zcorn) const {
        int sign = zcorn[ this->index(0,0,0,0) ] <= zcorn[this->index(0,0, this->dims[2] - 1,4)] ? 1 : -1;
        for (size_t j=0; j < this->dims[1]; j++)
            for (size_t i=0; i < this->dims[0]; i++)
                for (size_t c=0; c < 4; c++)
                    for (size_t k=0; k < this->dims[2]; k++) {
                        /* Between cells */
                        if (k > 0) {
                            size_t index1 = this->index(i,j,k-1,c+4);
                            size_t index2 = this->index(i,j,k,c);
                            if ((zcorn[index2] - zcorn[index1]) * sign < 0)
                                return false;
                        }

                        /* In cell */
                        {
                            size_t index1 = this->index(i,j,k,c);
                            size_t index2 = this->index(i,j,k,c+4);
                            if ((zcorn[index2] - zcorn[index1]) * sign < 0)
                                return false;
                        }
                    }

        return true;
    }


    size_t ZcornMapper::fixupZCORN( std::vector<double>& zcorn) {
        int sign = zcorn[ this->index(0,0,0,0) ] <= zcorn[this->index(0,0, this->dims[2] - 1,4)] ? 1 : -1;
        size_t cells_adjusted = 0;

        for (size_t k=0; k < this->dims[2]; k++)
            for (size_t j=0; j < this->dims[1]; j++)
                for (size_t i=0; i < this->dims[0]; i++)
                    for (size_t c=0; c < 4; c++) {
                        /* Cell to cell */
                        if (k > 0) {
                            size_t index1 = this->index(i,j,k-1,c+4);
                            size_t index2 = this->index(i,j,k,c);

                            if ((zcorn[index2] - zcorn[index1]) * sign < 0 ) {
                                zcorn[index2] = zcorn[index1];
                                cells_adjusted++;
                            }
                        }

                        /* Cell internal */
                        {
                            size_t index1 = this->index(i,j,k,c);
                            size_t index2 = this->index(i,j,k,c+4);

                            if ((zcorn[index2] - zcorn[index1]) * sign < 0 ) {
                                zcorn[index2] = zcorn[index1];
                                cells_adjusted++;
                            }
                        }
                    }
        return cells_adjusted;
    }

    CoordMapper::CoordMapper(size_t nx_, size_t ny_) :
        nx(nx_),
        ny(ny_)
    {
    }

    size_t CoordMapper::size() const {
        return (this->nx + 1) * (this->ny + 1) * 6;
    }


    size_t CoordMapper::index(size_t i, size_t j, size_t dim, size_t layer) const {
        if (i > this->nx)
            throw std::invalid_argument("Out of range");

        if (j > this->ny)
            throw std::invalid_argument("Out of range");

        if (dim > 2)
            throw std::invalid_argument("Out of range");

        if (layer > 1)
            throw std::invalid_argument("Out of range");

        return 6*( i + j*(this->nx + 1) ) +  layer * 3 + dim;
    }
}



