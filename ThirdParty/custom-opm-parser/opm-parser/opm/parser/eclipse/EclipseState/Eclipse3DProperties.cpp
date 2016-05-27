/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <functional>

#include <boost/algorithm/string/join.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/BoxManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm {

    namespace GridPropertyPostProcessor {

        void distTopLayer( std::vector<double>&    values,
                           const EclipseGrid*      eclipseGrid )
        {
            size_t layerSize = eclipseGrid->getNX() * eclipseGrid->getNY();
            size_t gridSize  = eclipseGrid->getCartesianSize();

            for( size_t globalIndex = layerSize; globalIndex < gridSize; globalIndex++ ) {
                if( std::isnan( values[ globalIndex ] ) )
                    values[globalIndex] = values[globalIndex - layerSize];
            }
        }

        /// initPORV uses doubleGridProperties: PORV, PORO, NTG, MULTPV
        void initPORV( std::vector<double>&    values,
                       const EclipseGrid*      eclipseGrid,
                       GridProperties<double>* doubleGridProperties)
        {
            /*
               Observe that this apply method does not alter the values input
               vector, instead it fetches the PORV property one more time, and
               then manipulates that.
               */

            const auto& porv = doubleGridProperties->getOrCreateProperty("PORV");

            if (porv.containsNaN()) {
                const auto& poro = doubleGridProperties->getOrCreateProperty("PORO");

                const auto& ntg =  doubleGridProperties->getOrCreateProperty("NTG");
                if (poro.containsNaN())
                    throw std::logic_error("Do not have information for the PORV keyword - some defaulted values in PORO");
                else {
                    const auto& poroData = poro.getData();
                    for (size_t globalIndex = 0; globalIndex < porv.getCartesianSize(); globalIndex++) {
                        if (std::isnan(values[globalIndex])) {
                            double cell_poro = poroData[globalIndex];
                            double cell_ntg = ntg.iget(globalIndex);
                            double cell_volume = eclipseGrid->getCellVolume(globalIndex);
                            values[globalIndex] = cell_poro * cell_volume * cell_ntg;
                        }
                    }
                }
            }

            if (doubleGridProperties->hasKeyword("MULTPV")) {
                const auto& multpvData = doubleGridProperties->getKeyword("MULTPV").getData();
                for (size_t globalIndex = 0; globalIndex < porv.getCartesianSize(); globalIndex++) {
                    values[globalIndex] *= multpvData[globalIndex];
                }
            }
        }
    }




    static std::vector< GridProperties< int >::SupportedKeywordInfo >
    makeSupportedIntKeywords() {
        return {GridProperties< int >::SupportedKeywordInfo( "ACTNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "SATNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "IMBNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "PVTNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "EQLNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "ENDNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "FLUXNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "MULTNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "FIPNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "MISCNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "OPERNUM", 1, "1" ),
                };
    }

    static std::vector< GridProperties< double >::SupportedKeywordInfo >
    makeSupportedDoubleKeywords(const TableManager*        tableManager,
                                const EclipseGrid*         eclipseGrid,
                                GridProperties<int>* intGridProperties)
    {
        using std::placeholders::_1;

        const auto SGLLookup    = std::bind( SGLEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISGLLookup   = std::bind( ISGLEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto SWLLookup    = std::bind( SWLEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISWLLookup   = std::bind( ISWLEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto SGULookup    = std::bind( SGUEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISGULookup   = std::bind( ISGUEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto SWULookup    = std::bind( SWUEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISWULookup   = std::bind( ISWUEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto SGCRLookup   = std::bind( SGCREndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISGCRLookup  = std::bind( ISGCREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto SOWCRLookup  = std::bind( SOWCREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISOWCRLookup = std::bind( ISOWCREndpoint, _1, tableManager, eclipseGrid, intGridProperties );
        const auto SOGCRLookup  = std::bind( SOGCREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISOGCRLookup = std::bind( ISOGCREndpoint, _1, tableManager, eclipseGrid, intGridProperties );
        const auto SWCRLookup   = std::bind( SWCREndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto ISWCRLookup  = std::bind( ISWCREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );

        const auto PCWLookup    = std::bind( PCWEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto IPCWLookup   = std::bind( IPCWEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto PCGLookup    = std::bind( PCGEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto IPCGLookup   = std::bind( IPCGEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRWLookup    = std::bind( KRWEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRWLookup   = std::bind( IKRWEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRWRLookup   = std::bind( KRWREndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRWRLookup  = std::bind( IKRWREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto KROLookup    = std::bind( KROEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKROLookup   = std::bind( IKROEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRORWLookup  = std::bind( KRORWEndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRORWLookup = std::bind( IKRORWEndpoint, _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRORGLookup  = std::bind( KRORGEndpoint,  _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRORGLookup = std::bind( IKRORGEndpoint, _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRGLookup    = std::bind( KRGEndpoint,    _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRGLookup   = std::bind( IKRGEndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto KRGRLookup   = std::bind( KRGREndpoint,   _1, tableManager, eclipseGrid, intGridProperties );
        const auto IKRGRLookup  = std::bind( IKRGREndpoint,  _1, tableManager, eclipseGrid, intGridProperties );

        const auto tempLookup = std::bind( temperature_lookup, _1, tableManager, eclipseGrid, intGridProperties );

        const auto distributeTopLayer = std::bind( &GridPropertyPostProcessor::distTopLayer, _1, eclipseGrid );

        std::vector< GridProperties< double >::SupportedKeywordInfo > supportedDoubleKeywords;

        // keywords to specify the scaled connate gas saturations.
        for( const auto& kw : { "SGL", "SGLX", "SGLX-", "SGLY", "SGLY-", "SGLZ", "SGLZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SGLLookup, "1" );
        for( const auto& kw : { "ISGL", "ISGLX", "ISGLX-", "ISGLY", "ISGLY-", "ISGLZ", "ISGLZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISGLLookup, "1" );

        // keywords to specify the connate water saturation.
        for( const auto& kw : { "SWL", "SWLX", "SWLX-", "SWLY", "SWLY-", "SWLZ", "SWLZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SWLLookup, "1" );
        for( const auto& kw : { "ISWL", "ISWLX", "ISWLX-", "ISWLY", "ISWLY-", "ISWLZ", "ISWLZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISWLLookup, "1" );

        // keywords to specify the maximum gas saturation.
        for( const auto& kw : { "SGU", "SGUX", "SGUX-", "SGUY", "SGUY-", "SGUZ", "SGUZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SGULookup, "1" );
        for( const auto& kw : { "ISGU", "ISGUX", "ISGUX-", "ISGUY", "ISGUY-", "ISGUZ", "ISGUZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISGULookup, "1" );

        // keywords to specify the maximum water saturation.
        for( const auto& kw : { "SWU", "SWUX", "SWUX-", "SWUY", "SWUY-", "SWUZ", "SWUZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SWULookup, "1" );
        for( const auto& kw : { "ISWU", "ISWUX", "ISWUX-", "ISWUY", "ISWUY-", "ISWUZ", "ISWUZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISWULookup, "1" );

        // keywords to specify the scaled critical gas saturation.
        for( const auto& kw : { "SGCR", "SGCRX", "SGCRX-", "SGCRY", "SGCRY-", "SGCRZ", "SGCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SGCRLookup, "1" );
        for( const auto& kw : { "ISGCR", "ISGCRX", "ISGCRX-", "ISGCRY", "ISGCRY-", "ISGCRZ", "ISGCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISGCRLookup, "1" );

        // keywords to specify the scaled critical oil-in-water saturation.
        for( const auto& kw : { "SOWCR", "SOWCRX", "SOWCRX-", "SOWCRY", "SOWCRY-", "SOWCRZ", "SOWCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SOWCRLookup, "1" );
        for( const auto& kw : { "ISOWCR", "ISOWCRX", "ISOWCRX-", "ISOWCRY", "ISOWCRY-", "ISOWCRZ", "ISOWCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISOWCRLookup, "1" );

        // keywords to specify the scaled critical oil-in-gas saturation.
        for( const auto& kw : { "SOGCR", "SOGCRX", "SOGCRX-", "SOGCRY", "SOGCRY-", "SOGCRZ", "SOGCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SOGCRLookup, "1" );
        for( const auto& kw : { "ISOGCR", "ISOGCRX", "ISOGCRX-", "ISOGCRY", "ISOGCRY-", "ISOGCRZ", "ISOGCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISOGCRLookup, "1" );

        // keywords to specify the scaled critical water saturation.
        for( const auto& kw : { "SWCR", "SWCRX", "SWCRX-", "SWCRY", "SWCRY-", "SWCRZ", "SWCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, SWCRLookup, "1" );
        for( const auto& kw : { "ISWCR", "ISWCRX", "ISWCRX-", "ISWCRY", "ISWCRY-", "ISWCRZ", "ISWCRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, ISWCRLookup, "1" );

        // keywords to specify the scaled oil-water capillary pressure
        for( const auto& kw : { "PCW", "PCWX", "PCWX-", "PCWY", "PCWY-", "PCWZ", "PCWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, PCWLookup, "1" );
        for( const auto& kw : { "IPCW", "IPCWX", "IPCWX-", "IPCWY", "IPCWY-", "IPCWZ", "IPCWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IPCWLookup, "1" );

        // keywords to specify the scaled gas-oil capillary pressure
        for( const auto& kw : { "PCG", "PCGX", "PCGX-", "PCGY", "PCGY-", "PCGZ", "PCGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, PCGLookup, "1" );
        for( const auto& kw : { "IPCG", "IPCGX", "IPCGX-", "IPCGY", "IPCGY-", "IPCGZ", "IPCGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IPCGLookup, "1" );

        // keywords to specify the scaled water relative permeability
        for( const auto& kw : { "KRW", "KRWX", "KRWX-", "KRWY", "KRWY-", "KRWZ", "KRWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, KRWLookup, "1" );
        for( const auto& kw : { "IKRW", "IKRWX", "IKRWX-", "IKRWY", "IKRWY-", "IKRWZ", "IKRWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRWLookup, "1" );

        // keywords to specify the scaled water relative permeability at the critical
        // saturation
        for( const auto& kw : { "KRWR" , "KRWRX" , "KRWRX-" , "KRWRY" , "KRWRY-" , "KRWRZ" , "KRWRZ-"  } )
            supportedDoubleKeywords.emplace_back( kw, KRWRLookup, "1" );
        for( const auto& kw : { "IKRWR" , "IKRWRX" , "IKRWRX-" , "IKRWRY" , "IKRWRY-" , "IKRWRZ" , "IKRWRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRWRLookup, "1" );

        // keywords to specify the scaled oil relative permeability
        for( const auto& kw : { "KRO", "KROX", "KROX-", "KROY", "KROY-", "KROZ", "KROZ-" } )
            supportedDoubleKeywords.emplace_back( kw, KROLookup, "1" );
        for( const auto& kw : { "IKRO", "IKROX", "IKROX-", "IKROY", "IKROY-", "IKROZ", "IKROZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKROLookup, "1" );

        // keywords to specify the scaled water relative permeability at the critical
        // water saturation
        for( const auto& kw : { "KRORW", "KRORWX", "KRORWX-", "KRORWY", "KRORWY-", "KRORWZ", "KRORWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, KRORWLookup, "1" );
        for( const auto& kw : { "IKRORW", "IKRORWX", "IKRORWX-", "IKRORWY", "IKRORWY-", "IKRORWZ", "IKRORWZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRORWLookup, "1" );

        // keywords to specify the scaled water relative permeability at the critical
        // water saturation
        for( const auto& kw : { "KRORG", "KRORGX", "KRORGX-", "KRORGY", "KRORGY-", "KRORGZ", "KRORGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, KRORGLookup, "1" );
        for( const auto& kw : { "IKRORG", "IKRORGX", "IKRORGX-", "IKRORGY", "IKRORGY-", "IKRORGZ", "IKRORGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRORGLookup, "1" );

        // keywords to specify the scaled gas relative permeability
        for( const auto& kw : { "KRG", "KRGX", "KRGX-", "KRGY", "KRGY-", "KRGZ", "KRGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, KRGLookup, "1" );
        for( const auto& kw : { "IKRG", "IKRGX", "IKRGX-", "IKRGY", "IKRGY-", "IKRGZ", "IKRGZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRGLookup, "1" );

        // keywords to specify the scaled gas relative permeability
        for( const auto& kw : { "KRGR", "KRGRX", "KRGRX-", "KRGRY", "KRGRY-", "KRGRZ", "KRGRZ-" } )
             supportedDoubleKeywords.emplace_back( kw, KRGRLookup, "1" );
        for( const auto& kw : { "IKRGR", "IKRGRX", "IKRGRX-", "IKRGRY", "IKRGRY-", "IKRGRZ", "IKRGRZ-" } )
            supportedDoubleKeywords.emplace_back( kw, IKRGRLookup, "1" );

        // Solution keywords - required fror enumerated restart.
        supportedDoubleKeywords.emplace_back( "PRESSURE", 0.0 , "Pressure" );
        supportedDoubleKeywords.emplace_back( "SWAT", 0.0 , "1" );
        supportedDoubleKeywords.emplace_back( "SGAS", 0.0 , "1" );


        // cell temperature (E300 only, but makes a lot of sense for E100, too)
        supportedDoubleKeywords.emplace_back( "TEMPI", tempLookup, "Temperature" );

        const double nan = std::numeric_limits<double>::quiet_NaN();
        // porosity
        supportedDoubleKeywords.emplace_back( "PORO", nan, distributeTopLayer, "1" );

        // pore volume multipliers
        supportedDoubleKeywords.emplace_back( "MULTPV", 1.0, "1" );

        // the permeability keywords
        for( const auto& kw : { "PERMX", "PERMY", "PERMZ" } )
            supportedDoubleKeywords.emplace_back( kw, nan, distributeTopLayer, "Permeability" );

        /* E300 only */
        for( const auto& kw : { "PERMXY", "PERMYZ", "PERMZX" } )
            supportedDoubleKeywords.emplace_back( kw, nan, distributeTopLayer, "Permeability" );

        /* the transmissibility keywords for neighboring connections. note that
         * these keywords don't seem to require a post-processor
         */
        for( const auto& kw : { "TRANX", "TRANY", "TRANZ" } )
            supportedDoubleKeywords.emplace_back( kw, nan, "Transmissibility" );

        /* gross-to-net thickness (acts as a multiplier for PORO and the
         * permeabilities in the X-Y plane as well as for the well rates.)
         */
        supportedDoubleKeywords.emplace_back( "NTG", 1.0, "1" );

        // transmissibility multipliers
        for( const auto& kw : { "MULTX", "MULTY", "MULTZ", "MULTX-", "MULTY-", "MULTZ-" } )
            supportedDoubleKeywords.emplace_back( kw, 1.0, "1" );

        // initialisation
        supportedDoubleKeywords.emplace_back( "SWATINIT", 0.0, "1");
        supportedDoubleKeywords.emplace_back( "THCONR", 0.0, "1");

        return supportedDoubleKeywords;
    }

    Eclipse3DProperties::Eclipse3DProperties( const Deck&         deck,
                                              const TableManager& tableManager,
                                              const EclipseGrid&  eclipseGrid)
        :

          m_defaultRegion("FLUXNUM"),
          m_deckUnitSystem(deck.getActiveUnitSystem()),
		  // Note that the variants of grid keywords for radial grids are not
		  // supported. (and hopefully never will be)
		  // register the grid properties
		  m_intGridProperties(eclipseGrid, makeSupportedIntKeywords()),
		  m_doubleGridProperties(eclipseGrid,
                        makeSupportedDoubleKeywords(&tableManager, &eclipseGrid, &m_intGridProperties))
    {
        /*
         * The EQUALREG, MULTREG, COPYREG, ... keywords are used to manipulate
         * vectors based on region values; for instance the statement
         *
         *   EQUALREG
         *      PORO  0.25  3    /   -- Region array not specified
         *      PERMX 100   3  F /
         *   /
         *
         * will set the PORO field to 0.25 for all cells in region 3 and the PERMX
         * value to 100 mD for the same cells. The fourth optional argument to the
         * EQUALREG keyword is used to indicate which REGION array should be used
         * for the selection.
         *
         * If the REGION array is not indicated (as in the PORO case) above, the
         * default region to use in the xxxREG keywords depends on the GRIDOPTS
         * keyword:
         *
         *   1. If GRIDOPTS is present, and the NRMULT item is greater than zero,
         *      the xxxREG keywords will default to use the MULTNUM region.
         *
         *   2. If the GRIDOPTS keyword is not present - or the NRMULT item equals
         *      zero, the xxxREG keywords will default to use the FLUXNUM keyword.
         *
         * This quite weird behaviour comes from reading the GRIDOPTS and MULTNUM
         * documentation, and practical experience with ECLIPSE
         * simulations. Ufortunately the documentation of the xxxREG keywords does
         * not confirm this.
         */
        if (deck.hasKeyword( "GRIDOPTS" )) {
            const auto& gridOpts = deck.getKeyword( "GRIDOPTS" );
            const auto& record = gridOpts.getRecord( 0 );
            const auto& nrmult_item = record.getItem( "NRMULT" );

            if (nrmult_item.get<int>( 0 ) > 0)
                m_defaultRegion = "MULTNUM"; // GRIDOPTS and positive NRMULT
        }


        auto initPORV = std::bind(&GridPropertyPostProcessor::initPORV,
                                                    std::placeholders::_1,
                                                    &eclipseGrid,
                                                    &m_doubleGridProperties);
        // pore volume
        m_doubleGridProperties.postAddKeyword( "PORV",
                                                std::numeric_limits<double>::quiet_NaN(),
                                                initPORV,
                                                "Volume" );

        // actually create the grid property objects. we need to first process
        // all integer grid properties before the double ones as these may be
        // needed in order to initialize the double properties
        processGridProperties(deck, eclipseGrid, /*enabledTypes=*/IntProperties);
        processGridProperties(deck, eclipseGrid, /*enabledTypes=*/DoubleProperties);

    }

    bool Eclipse3DProperties::supportsGridProperty(const std::string& keyword,
                                                 int enabledTypes) const
    {
        bool result = false;
        if (enabledTypes & IntProperties)
            result = result ||
                m_intGridProperties.supportsKeyword( keyword );

        if (enabledTypes & DoubleProperties)
            result = result ||
                m_doubleGridProperties.supportsKeyword( keyword );
        return result;
    }



    bool Eclipse3DProperties::hasDeckIntGridProperty(const std::string& keyword) const {
        if (!m_intGridProperties.supportsKeyword( keyword ))
            throw std::logic_error("Integer grid property " + keyword + " is unsupported!");

        return m_intGridProperties.hasKeyword( keyword );
    }

    bool Eclipse3DProperties::hasDeckDoubleGridProperty(const std::string& keyword) const {
        if (!m_doubleGridProperties.supportsKeyword( keyword ))
            throw std::logic_error("Double grid property " + keyword + " is unsupported!");

        return m_doubleGridProperties.hasKeyword( keyword );
    }

    /*
      1. The public methods getIntGridProperty & getDoubleGridProperty will
         invoke and run the property post processor (if any is registered); the
         post processor will only run one time.

         It is important that post processor is not run prematurely, internal
         functions in EclipseState should therefore ask for properties by
         invoking the getKeyword() method of the m_intGridProperties /
         m_doubleGridProperties() directly and not through these methods.

      2. Observe that this will autocreate a property if it has not been
         explicitly added.
    */
    const GridProperty<int>& Eclipse3DProperties::getIntGridProperty( const std::string& keyword ) const {
        return m_intGridProperties.getKeyword( keyword );
    }

    /// gets property from doubleGridProperty --- and calls the runPostProcessor
    const GridProperty<double>& Eclipse3DProperties::getDoubleGridProperty( const std::string& keyword ) const {
        auto& gridProperty = const_cast< Eclipse3DProperties* >( this )->m_doubleGridProperties.getKeyword( keyword );
        gridProperty.runPostProcessor();
        return gridProperty;
    }

    std::string Eclipse3DProperties::getDefaultRegionKeyword() const {
        return m_defaultRegion;
    }

    const GridProperty<int>& Eclipse3DProperties::getRegion( const DeckItem& regionItem ) const {
        if (regionItem.defaultApplied(0))
            return m_intGridProperties.getKeyword( m_defaultRegion );
        else {
            const std::string regionArray = MULTREGT::RegionNameFromDeckValue( regionItem.get< std::string >(0) );
            return m_intGridProperties.getInitializedKeyword( regionArray );
        }
    }

    ///  Due to the post processor which might be applied to the GridProperty
    ///  objects it is essential that this method use the m_intGridProperties /
    ///  m_doubleGridProperties fields directly and *NOT* use the public methods
    ///  getIntGridProperty / getDoubleGridProperty.
    void Eclipse3DProperties::loadGridPropertyFromDeckKeyword(const Box& inputBox,
                                                            const DeckKeyword& deckKeyword,
                                                            int enabledTypes)
    {
        const std::string& keyword = deckKeyword.name();
        if (m_intGridProperties.supportsKeyword( keyword )) {
            if (enabledTypes & IntProperties) {
                auto& gridProperty = m_intGridProperties.getOrCreateProperty( keyword );
                gridProperty.loadFromDeckKeyword( inputBox, deckKeyword );
            }
        } else if (m_doubleGridProperties.supportsKeyword( keyword )) {
            if (enabledTypes & DoubleProperties) {
                auto& gridProperty = m_doubleGridProperties.getOrCreateProperty( keyword );
                gridProperty.loadFromDeckKeyword( inputBox, deckKeyword );
            }
        } else {
            throw std::logic_error( "Tried to load unsupported grid property from keyword: " + deckKeyword.name() );
        }
    }

    static bool isInt(double value) {
        return fabs( nearbyint( value ) - value ) < 1e-6;
    }

    void Eclipse3DProperties::processGridProperties( const Deck& deck,
                                                   const EclipseGrid& eclipseGrid,
                                                   int enabledTypes) {

        if (Section::hasGRID(deck)) {
            scanSection(GRIDSection(deck), eclipseGrid, enabledTypes);
        }

        if (Section::hasEDIT(deck)) {
            scanSection(EDITSection(deck), eclipseGrid, enabledTypes);
        }

        if (Section::hasPROPS(deck)) {
            scanSection(PROPSSection(deck), eclipseGrid, enabledTypes);
        }

        if (Section::hasREGIONS(deck)) {
            scanSection(REGIONSSection(deck), eclipseGrid, enabledTypes);
        }

        if (Section::hasSOLUTION(deck)) {
            scanSection(SOLUTIONSection(deck), eclipseGrid, enabledTypes);
        }
    }



    // private method
    double Eclipse3DProperties::getSIScaling(const std::string &dimensionString) const
    {
        return m_deckUnitSystem.getDimension(dimensionString)->getSIScaling();
    }

    void Eclipse3DProperties::scanSection(const Section& section,
                                        const EclipseGrid& eclipseGrid,
                                        int enabledTypes) {
        BoxManager boxManager(eclipseGrid.getNX(),
                              eclipseGrid.getNY(),
                              eclipseGrid.getNZ());

        for( const auto& deckKeyword : section ) {

            if (supportsGridProperty(deckKeyword.name(), enabledTypes) )
                loadGridPropertyFromDeckKeyword( *boxManager.getActiveBox(),
                                                 deckKeyword,
                                                 enabledTypes);
            else {
                if (deckKeyword.name() == "ADD")
                    handleADDKeyword(deckKeyword, boxManager, enabledTypes);

                if (deckKeyword.name() == "BOX")
                    handleBOXKeyword(deckKeyword, boxManager);

                if (deckKeyword.name() == "COPY")
                    handleCOPYKeyword(deckKeyword, boxManager, enabledTypes);

                if (deckKeyword.name() == "EQUALS")
                    handleEQUALSKeyword(deckKeyword, boxManager, enabledTypes);

                if (deckKeyword.name() == "MULTIPLY")
                    handleMULTIPLYKeyword(deckKeyword, boxManager, enabledTypes);

                if (deckKeyword.name() == "ENDBOX")
                    handleENDBOXKeyword(boxManager);

                if (deckKeyword.name() == "EQUALREG")
                    handleEQUALREGKeyword(deckKeyword, enabledTypes);

                if (deckKeyword.name() == "ADDREG")
                    handleADDREGKeyword(deckKeyword,   enabledTypes);

                if (deckKeyword.name() == "MULTIREG")
                    handleMULTIREGKeyword(deckKeyword, enabledTypes);

                if (deckKeyword.name() == "COPYREG")
                    handleCOPYREGKeyword(deckKeyword,  enabledTypes);

                boxManager.endKeyword();
            }
        }
        boxManager.endSection();
    }


    void Eclipse3DProperties::handleBOXKeyword( const DeckKeyword& deckKeyword,  BoxManager& boxManager) {
        const auto& record = deckKeyword.getRecord(0);
        int I1 = record.getItem("I1").get< int >(0) - 1;
        int I2 = record.getItem("I2").get< int >(0) - 1;
        int J1 = record.getItem("J1").get< int >(0) - 1;
        int J2 = record.getItem("J2").get< int >(0) - 1;
        int K1 = record.getItem("K1").get< int >(0) - 1;
        int K2 = record.getItem("K2").get< int >(0) - 1;

        boxManager.setInputBox( I1 , I2 , J1 , J2 , K1 , K2 );
    }


    void Eclipse3DProperties::handleENDBOXKeyword(BoxManager& boxManager) {
        boxManager.endInputBox();
    }


   void Eclipse3DProperties::handleEQUALREGKeyword( const DeckKeyword& deckKeyword,
                                                  int enabledTypes)
   {
       for( const auto& record : deckKeyword ) {
           const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);

            if (!supportsGridProperty( targetArray , IntProperties + DoubleProperties))
                throw std::invalid_argument("Fatal error processing EQUALREG keyword - invalid/undefined keyword: " + targetArray);

            double doubleValue = record.getItem("VALUE").template get<double>(0);
            int regionValue = record.getItem("REGION_NUMBER").template get<int>(0);
            auto& regionProperty = getRegion( record.getItem("REGION_NAME") );
            std::vector<bool> mask;

            regionProperty.initMask( regionValue , mask);

            if (m_intGridProperties.supportsKeyword( targetArray )) {
                if (enabledTypes & IntProperties) {
                    if (isInt( doubleValue )) {
                        auto& targetProperty = m_intGridProperties.getOrCreateProperty( targetArray );
                        int intValue = static_cast<int>( doubleValue + 0.5 );
                        targetProperty.maskedSet( intValue , mask);
                    } else
                        throw std::invalid_argument("Fatal error processing EQUALREG keyword - expected integer value for: " + targetArray);
                }
            }
            else if (m_doubleGridProperties.supportsKeyword( targetArray )) {
                if (enabledTypes & DoubleProperties) {
                    auto& targetProperty = m_doubleGridProperties.getOrCreateProperty( targetArray );

                    const std::string& dimensionString = targetProperty.getDimensionString();
                    double SIValue = doubleValue * getSIScaling( dimensionString );

                    targetProperty.maskedSet( SIValue , mask);
                }
            }
            else {
                throw std::invalid_argument("Fatal error processing EQUALREG keyword - invalid/undefined keyword: " + targetArray);
            }
        }
    }


    void Eclipse3DProperties::handleADDREGKeyword( const DeckKeyword& deckKeyword, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);

            if (!supportsGridProperty( targetArray , IntProperties + DoubleProperties))
                throw std::invalid_argument("Fatal error processing ADDREG keyword - invalid/undefined keyword: " + targetArray);

            if (supportsGridProperty( targetArray , enabledTypes)) {
                double doubleValue = record.getItem("SHIFT").get< double >(0);
                int regionValue = record.getItem("REGION_NUMBER").get< int >(0);
                auto& regionProperty = getRegion( record.getItem("REGION_NAME") );
                std::vector<bool> mask;

                regionProperty.initMask( regionValue , mask);

                if (m_intGridProperties.hasKeyword( targetArray )) {
                    if (enabledTypes & IntProperties) {
                        if (isInt( doubleValue )) {
                            GridProperty<int>& targetProperty = m_intGridProperties.getKeyword(targetArray);
                            int intValue = static_cast<int>( doubleValue + 0.5 );
                            targetProperty.maskedAdd( intValue , mask);
                        } else
                            throw std::invalid_argument("Fatal error processing ADDREG keyword - expected integer value for: " + targetArray);
                    }
                }
                else if (m_doubleGridProperties.hasKeyword( targetArray )) {
                    if (enabledTypes & DoubleProperties) {
                        GridProperty<double>& targetProperty = m_doubleGridProperties.getKeyword(targetArray);

                        const std::string& dimensionString = targetProperty.getDimensionString();
                        double SIValue = doubleValue * getSIScaling( dimensionString );

                        targetProperty.maskedAdd( SIValue , mask);
                    }
                }
                else {
                    throw std::invalid_argument("Fatal error processing ADDREG keyword - invalid/undefined keyword: " + targetArray);
                }
            }
        }
    }



    void Eclipse3DProperties::handleMULTIREGKeyword( const DeckKeyword& deckKeyword, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);

            if (!supportsGridProperty( targetArray , IntProperties + DoubleProperties))
                throw std::invalid_argument("Fatal error processing MULTIREG keyword - invalid/undefined keyword: " + targetArray);

            if (supportsGridProperty( targetArray , enabledTypes)) {
                double doubleValue = record.getItem("FACTOR").get< double >(0);
                int regionValue = record.getItem("REGION_NUMBER").get< int >(0);
                auto& regionProperty = getRegion( record.getItem("REGION_NAME") );
                std::vector<bool> mask;

                regionProperty.initMask( regionValue , mask);

                if (enabledTypes & IntProperties) {
                    if (isInt( doubleValue )) {
                        auto& targetProperty = m_intGridProperties.getOrCreateProperty( targetArray );
                        int intValue = static_cast<int>( doubleValue + 0.5 );

                        targetProperty.maskedMultiply( intValue , mask);
                    } else
                        throw std::invalid_argument(
                            "Fatal error processing MULTIREG keyword - expected"
                            " integer value for: " + targetArray);
                }
                if (enabledTypes & DoubleProperties) {
                    auto& targetProperty = m_doubleGridProperties.getOrCreateProperty(targetArray);
                    targetProperty.maskedMultiply( doubleValue , mask);
                }
            }
        }
    }


    void Eclipse3DProperties::handleCOPYREGKeyword( const DeckKeyword& deckKeyword, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& srcArray    = record.getItem("ARRAY").get< std::string >(0);
            const std::string& targetArray = record.getItem("TARGET_ARRAY").get< std::string >(0);

            if (!supportsGridProperty( targetArray , IntProperties + DoubleProperties))
                throw std::invalid_argument("Fatal error processing MULTIREG keyword - invalid/undefined keyword: " + targetArray);

            if (!supportsGridProperty( srcArray , IntProperties + DoubleProperties))
                throw std::invalid_argument("Fatal error processing MULTIREG keyword - invalid/undefined keyword: " + srcArray);

            if (supportsGridProperty( srcArray , enabledTypes))
            {
                int regionValue = record.getItem("REGION_NUMBER").get< int >(0);
                auto& regionProperty = getRegion( record.getItem("REGION_NAME") );
                std::vector<bool> mask;

                regionProperty.initMask( regionValue , mask );

                if (m_intGridProperties.hasKeyword( srcArray )) {
                    auto& srcProperty = m_intGridProperties.getInitializedKeyword( srcArray );
                    if (supportsGridProperty( targetArray , IntProperties)) {
                        GridProperty<int>& targetProperty = m_intGridProperties.getOrCreateProperty( targetArray );
                        targetProperty.maskedCopy( srcProperty , mask );
                    } else
                        throw std::invalid_argument("Fatal error processing COPYREG keyword.");
                } else if (m_doubleGridProperties.hasKeyword( srcArray )) {
                    const GridProperty<double>& srcProperty = m_doubleGridProperties.getInitializedKeyword( srcArray );
                    if (supportsGridProperty( targetArray , DoubleProperties)) {
                        auto& targetProperty = m_doubleGridProperties.getOrCreateProperty( targetArray );
                        targetProperty.maskedCopy( srcProperty , mask );
                    }
                }
                else {
                    throw std::invalid_argument("Fatal error processing COPYREG keyword - invalid/undefined keyword: " + targetArray);
                }
            }
        }
    }




    void Eclipse3DProperties::handleMULTIPLYKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);
            double      scaleFactor  = record.getItem("factor").get< double >(0);

            setKeywordBox(deckKeyword, record, boxManager);

            if (m_intGridProperties.hasKeyword( field )) {
                if (enabledTypes & IntProperties) {
                    int intFactor = static_cast<int>(scaleFactor);
                    GridProperty<int>& property = m_intGridProperties.getKeyword( field );
                    property.scale( intFactor , *boxManager.getActiveBox() );
                }
            } else if (m_doubleGridProperties.hasKeyword( field )) {
                if (enabledTypes & DoubleProperties) {
                    GridProperty<double>& property = m_doubleGridProperties.getKeyword( field );
                    property.scale( scaleFactor , *boxManager.getActiveBox() );
                }
            } else if (!m_intGridProperties.supportsKeyword(field) &&
                       !m_doubleGridProperties.supportsKeyword(field))
                throw std::invalid_argument("Fatal error processing MULTIPLY keyword. Tried to multiply not defined keyword " + field);
        }
    }


    /**
      The fine print of the manual says the ADD keyword should support
      some state dependent semantics regarding endpoint scaling arrays
      in the PROPS section. That is not supported.
    */
    void Eclipse3DProperties::handleADDKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);
            double      shiftValue  = record.getItem("shift").get< double >(0);

            setKeywordBox(deckKeyword, record, boxManager);

            if (m_intGridProperties.hasKeyword( field )) {
                if (enabledTypes & IntProperties) {
                    int intShift = static_cast<int>(shiftValue);
                    GridProperty<int>& property = m_intGridProperties.getKeyword( field );

                    property.add( intShift , *boxManager.getActiveBox() );
                }
            } else if (m_doubleGridProperties.hasKeyword( field )) {
                if (enabledTypes & DoubleProperties) {
                    GridProperty<double>& property = m_doubleGridProperties.getKeyword( field );

                    double siShiftValue = shiftValue * getSIScaling(property.getDimensionString());
                    property.add(siShiftValue , *boxManager.getActiveBox() );
                }
            } else if (!m_intGridProperties.supportsKeyword(field) &&
                       !m_doubleGridProperties.supportsKeyword(field))
                throw std::invalid_argument("Fatal error processing ADD keyword. Tried to shift not defined keyword " + field);
        }
    }


    void Eclipse3DProperties::handleEQUALSKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);
            double      value  = record.getItem("value").get< double >(0);

            setKeywordBox(deckKeyword, record, boxManager);

            if (m_intGridProperties.supportsKeyword( field )) {
                if (enabledTypes & IntProperties) {
                    int intValue = static_cast<int>(value);
                    GridProperty<int>& property = m_intGridProperties.getOrCreateProperty( field );

                    property.setScalar( intValue , *boxManager.getActiveBox() );
                }
            } else if (m_doubleGridProperties.supportsKeyword( field )) {

                if (enabledTypes & DoubleProperties) {
                    GridProperty<double>& property = m_doubleGridProperties.getOrCreateProperty( field );

                    double siValue = value * getSIScaling(property.getKeywordInfo().getDimensionString());
                    property.setScalar( siValue , *boxManager.getActiveBox() );
                }
            } else
                throw std::invalid_argument("Fatal error processing EQUALS keyword. Tried to set not defined keyword " + field);
        }
    }



    void Eclipse3DProperties::handleCOPYKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager, int enabledTypes) {
        for( const auto& record : deckKeyword ) {
            const std::string& srcField = record.getItem("src").get< std::string >(0);
            const std::string& targetField = record.getItem("target").get< std::string >(0);

            setKeywordBox(deckKeyword, record, boxManager);

            if (m_intGridProperties.hasKeyword( srcField )) {
                if (enabledTypes & IntProperties)
                    m_intGridProperties.copyKeyword(    srcField , targetField , *boxManager.getActiveBox() );
            }
            else if (m_doubleGridProperties.hasKeyword( srcField )) {
                if (enabledTypes & DoubleProperties)
                    m_doubleGridProperties.copyKeyword( srcField , targetField , *boxManager.getActiveBox() );
            }
            else
                if (!m_intGridProperties.supportsKeyword(srcField) &&
                    !m_doubleGridProperties.supportsKeyword(srcField))
                    throw std::invalid_argument("Fatal error processing COPY keyword."
                                                " Tried to copy from not defined keyword " + srcField);
        }
    }

    

    MessageContainer Eclipse3DProperties::getMessageContainer() {
        MessageContainer messages;
        messages.appendMessages(m_intGridProperties.getMessageContainer());
        messages.appendMessages(m_doubleGridProperties.getMessageContainer());
        return messages;
    }


    void Eclipse3DProperties::setKeywordBox( const DeckKeyword& deckKeyword,
                                           const DeckRecord& deckRecord,
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
            m_intGridProperties.getMessageContainer().error(deckKeyword.getFileName() + std::to_string(deckKeyword.getLineNumber()) + msg);
        }
    }
}
