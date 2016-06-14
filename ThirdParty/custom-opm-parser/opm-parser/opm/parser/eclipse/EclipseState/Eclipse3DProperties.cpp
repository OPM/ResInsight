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
                       const GridProperties<double>* doubleGridProperties)
        {
            const auto iter = std::find_if( values.begin(), values.end(), []( double value ) { return std::isnan(value); });
            if (iter != values.end()) {
                const auto& poro = doubleGridProperties->getKeyword("PORO");
                const auto& ntg =  doubleGridProperties->getKeyword("NTG");

                if (poro.containsNaN())
                    throw std::logic_error("Do not have information for the PORV keyword - some defaulted values in PORO");
                else {
                    const auto& poroData = poro.getData();
                    for (size_t globalIndex = 0; globalIndex < poro.getCartesianSize(); globalIndex++) {
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
                const GridProperty<double>& multpvKeyword = doubleGridProperties->getKeyword("MULTPV");
                const auto& multpvData = multpvKeyword.getData();
                for (size_t globalIndex = 0; globalIndex < multpvData.size(); globalIndex++)
                    values[globalIndex] *= multpvData[globalIndex];
            }
        }


        /*
          In this codeblock we explicitly set the ACTNUM value to zero
          for cells with pore volume equal to zero. Observe the
          following:

             1. The current processing only considers cells with PORV
                == 0. Processing of the MINPV keyword for cells with
                small nonzero poervolumes is currently performed along
                with the Grid processing code.

             2. The PORO or PORV keyword must be explicitly entered
                in the deck, using getDoubleGridProperty( ) and
                relying on autocreation will fail - we therefor
                explicitly check if the PORO or PORV keywords are
                present in the deck.

             3. The code below will fail hard if it is called with a
                grid which does not have full cell information.
        */

        void ACTNUMPostProcessor( std::vector<int>&       values,
                                  const EclipseGrid*      eclipseGrid,
                                  const GridProperties<double>* doubleGridProperties)
        {
            const bool hasPORV = doubleGridProperties->hasKeyword( "PORV" ) || doubleGridProperties->hasKeyword( "PORO");
            if (!hasPORV)
                return;

            {
                const auto& porv = doubleGridProperties->getKeyword("PORV");
                {
                    const auto& porvData = porv.getData();
                    for (size_t i = 0; i < porvData.size(); i++)
                        if (porvData[i] == 0)
                            values[i] = 0;
                }
            }
        }
    }



    static std::vector< GridProperties< int >::SupportedKeywordInfo >
    makeSupportedIntKeywords() {
        return { GridProperties< int >::SupportedKeywordInfo( "SATNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "IMBNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "PVTNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "EQLNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "ENDNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "FLUXNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "MULTNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "FIPNUM" , 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "MISCNUM", 1, "1" ),
                GridProperties< int >::SupportedKeywordInfo( "OPERNUM", 1, "1" ) };
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
          m_doubleGridProperties(eclipseGrid, &m_deckUnitSystem,
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


        {
            auto initPORV = std::bind(&GridPropertyPostProcessor::initPORV,
                                      std::placeholders::_1,
                                      &eclipseGrid,
                                      &m_doubleGridProperties);

            m_doubleGridProperties.postAddKeyword( "PORV",
                                                   std::numeric_limits<double>::quiet_NaN(),
                                                   initPORV,
                                                   "Volume" );
        }

        {
            auto actnumPP = std::bind(&GridPropertyPostProcessor::ACTNUMPostProcessor,
                                      std::placeholders::_1,
                                      &eclipseGrid,
                                      &m_doubleGridProperties);

            m_intGridProperties.postAddKeyword( "ACTNUM",
                                                1,
                                                actnumPP ,
                                                "1");
        }

        processGridProperties(deck, eclipseGrid);
    }

    bool Eclipse3DProperties::supportsGridProperty(const std::string& keyword) const {
        return m_doubleGridProperties.supportsKeyword( keyword ) || m_intGridProperties.supportsKeyword( keyword );
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


    const GridProperty<int>& Eclipse3DProperties::getIntGridProperty( const std::string& keyword ) const {
        auto& gridProperty = const_cast< Eclipse3DProperties* >( this )->m_intGridProperties.getKeyword( keyword );
        gridProperty.runPostProcessor();
        return gridProperty;
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

    std::vector< int > Eclipse3DProperties::getRegions( const std::string& kw ) const {
        if( !this->hasDeckIntGridProperty( kw ) ) return {};

        const auto& property = this->getIntGridProperty( kw );

        std::set< int > regions( property.getData().begin(),
                                 property.getData().end() );

        return { regions.begin(), regions.end() };
    }

    ///  Due to the post processor which might be applied to the GridProperty
    ///  objects it is essential that this method use the m_intGridProperties /
    ///  m_doubleGridProperties fields directly and *NOT* use the public methods
    ///  getIntGridProperty / getDoubleGridProperty.
    void Eclipse3DProperties::loadGridPropertyFromDeckKeyword(const Box& inputBox,
                                                              const DeckKeyword& deckKeyword) {
        const std::string& keyword = deckKeyword.name();
        if (m_intGridProperties.supportsKeyword( keyword )) {
            auto& gridProperty = m_intGridProperties.getOrCreateProperty( keyword );
            gridProperty.loadFromDeckKeyword( inputBox, deckKeyword );
        } else if (m_doubleGridProperties.supportsKeyword( keyword )) {
            auto& gridProperty = m_doubleGridProperties.getOrCreateProperty( keyword );
            gridProperty.loadFromDeckKeyword( inputBox, deckKeyword );
        } else {
            throw std::logic_error( "Tried to load unsupported grid property from keyword: " + deckKeyword.name() );
        }
    }

    /*
      Issues regarding initialization order and default values.
      =========================================================

      The 3D properties objects, and in particular their
      initialization touches upon very fine details of the Eclipse
      behavior; which are not entirely clear from the documentation:


      Default values and EQUALS / COPY / ADD / ...
      --------------------------------------------

      The keywords which operate on an existing keyword, i.e. COPY,
      ADD and MULTIPLY require that the mentioned keyword has already
      been introduced explicitly in the deck, i.e. if this is the
      first mention of the PORO keyword in the deck this will fail
      hard:


         ADD
           PORO 0.10 /
         /

      Because the PORO keyword is undefined.

      All keywords have a default value or initializer; for some
      keywords this default value is sensical which can be used in a
      simulation, e.g. ACTNUM; defaults to 1 - whereas PORO defaults
      to nan.

      Using BOX functionality it is possible to limit the assignment
      and manipulations to only parts of a 3D property, we then have a
      partly initialized keyword which can be used in subsequent
      operations, and undetected give nonsensical results. Consider
      the following sequyence of keywords:

         -- Set the porosity equal to 0.25 in the top layer.
         EQUALS
            PORO 0.25 1 100 1 100 1 1 /
         /

        -- Shift the porosity with 0.10 in all cells
        ADD
            PORO 0.10
        /

      Now - the second ADD operation will 'silently succeed' because
      the PORO keyword has been autocreated with the preceeding EQUALS
      keyword, but since the EQUALS keyword only set a subset of the
      cells the PORO keyword will not be fully defined. This is fully
      undedected, and will hopefully blow up hard when you start
      simualating.


      Order of initialization
      -----------------------

      The kewyords are come into life and are manipulated as you go
      through the deck; i.e. in this part of the code some of the
      underlying imperative nature of the Eclipse datamodel comes
      through - hopefully we have got it right:

        1. The REGIONS section is scanned before the other sections;
           this is out-of-order compared to the ordering of ECLIPSE
           sections, but it is required to ensure that the relevant
           region keywords are defined before we read the properties
           keywords like SGU and SGOF.

        2. When using the region keywords in e.g. MULTIREG the current
           state of the region keyword is used, i.e. in the example
           below the two MULTIREG keywords will use different versions
           of the MULTNUM keyword:


              MULTIREG
                  PORO  2.0  1  M /
              /

              ADD
                MULTNUM 1 /
              /

              MULTIREG
                  PERMX  2.0  1  M /
              /
    */


    void Eclipse3DProperties::processGridProperties( const Deck& deck,
                                                     const EclipseGrid& eclipseGrid) {


        /*
          Observe that the REGIONS section is scanned out of order -
          before the other section, to ensure that the integer region
          keywords are correctly defined before parsing the remaining
          keywords.
        */
        if (Section::hasREGIONS(deck))
            scanSection(REGIONSSection(deck), eclipseGrid);

        if (Section::hasGRID(deck))
            scanSection(GRIDSection(deck), eclipseGrid);

        if (Section::hasEDIT(deck))
            scanSection(EDITSection(deck), eclipseGrid);

        if (Section::hasPROPS(deck))
            scanSection(PROPSSection(deck), eclipseGrid);

        if (Section::hasSOLUTION(deck))
            scanSection(SOLUTIONSection(deck), eclipseGrid);
    }



    void Eclipse3DProperties::scanSection(const Section& section,
                                          const EclipseGrid& eclipseGrid) {
        BoxManager boxManager(eclipseGrid.getNX(),
                              eclipseGrid.getNY(),
                              eclipseGrid.getNZ());

        for( const auto& deckKeyword : section ) {

            if (supportsGridProperty(deckKeyword.name()) )
                loadGridPropertyFromDeckKeyword( *boxManager.getActiveBox(),
                                                 deckKeyword);
            else {
                if (deckKeyword.name() == "BOX")
                    handleBOXKeyword(deckKeyword, boxManager);

                if (deckKeyword.name() == "ENDBOX")
                    handleENDBOXKeyword(boxManager);


                if (deckKeyword.name() == "COPY")
                    handleCOPYKeyword( deckKeyword , boxManager);

                if (deckKeyword.name() == "EQUALS")
                    handleEQUALSKeyword(deckKeyword, boxManager);


                if (deckKeyword.name() == "ADD")
                    handleADDKeyword( deckKeyword , boxManager);

                if (deckKeyword.name() == "MULTIPLY")
                    handleMULTIPLYKeyword(deckKeyword, boxManager);


                if (deckKeyword.name() == "EQUALREG")
                    handleEQUALREGKeyword(deckKeyword);

                if (deckKeyword.name() == "ADDREG")
                    handleADDREGKeyword(deckKeyword);

                if (deckKeyword.name() == "MULTIREG")
                    handleMULTIREGKeyword(deckKeyword);

                if (deckKeyword.name() == "COPYREG")
                    handleCOPYREGKeyword(deckKeyword);

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


    void Eclipse3DProperties::handleEQUALREGKeyword( const DeckKeyword& deckKeyword) {
       for( const auto& record : deckKeyword ) {
           const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
           auto& regionProperty = getRegion( record.getItem("REGION_NAME") );

           if (m_intGridProperties.supportsKeyword( targetArray ))
               m_intGridProperties.handleEQUALREGRecord( record , regionProperty );
           else if (m_doubleGridProperties.supportsKeyword( targetArray ))
               m_doubleGridProperties.handleEQUALREGRecord( record , regionProperty );
           else
               throw std::invalid_argument("Fatal error processing EQUALREG keyword - invalid/undefined keyword: " + targetArray);
       }
   }


    void Eclipse3DProperties::handleADDREGKeyword( const DeckKeyword& deckKeyword) {
       for( const auto& record : deckKeyword ) {
           const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
           const auto& regionProperty = getRegion( record.getItem("REGION_NAME") );

           if (m_intGridProperties.hasKeyword( targetArray ))
               m_intGridProperties.handleADDREGRecord( record , regionProperty );
           else if (m_doubleGridProperties.hasKeyword( targetArray ))
               m_doubleGridProperties.handleADDREGRecord( record , regionProperty );
           else
               throw std::invalid_argument("Fatal error processing ADDREG keyword - invalid/undefined keyword: " + targetArray);
       }
    }



    void Eclipse3DProperties::handleMULTIREGKeyword( const DeckKeyword& deckKeyword) {
        for( const auto& record : deckKeyword ) {
            const std::string& targetArray = record.getItem("ARRAY").get< std::string >(0);
            const auto& regionProperty = getRegion( record.getItem("REGION_NAME") );

           if (m_intGridProperties.supportsKeyword( targetArray ))
               m_intGridProperties.handleMULTIREGRecord( record , regionProperty );
           else if (m_doubleGridProperties.supportsKeyword( targetArray ))
               m_doubleGridProperties.handleMULTIREGRecord( record , regionProperty );
           else
               throw std::invalid_argument("Fatal error processing MULTIREG keyword - invalid/undefined keyword: " + targetArray);
        }
    }


    void Eclipse3DProperties::handleCOPYREGKeyword( const DeckKeyword& deckKeyword) {
        for( const auto& record : deckKeyword ) {
            const std::string& srcArray    = record.getItem("ARRAY").get< std::string >(0);
            const auto& regionProperty = getRegion( record.getItem("REGION_NAME") );

            if (m_intGridProperties.hasKeyword( srcArray ))
                m_intGridProperties.handleCOPYREGRecord( record, regionProperty );
            else if (m_doubleGridProperties.hasKeyword( srcArray ))
                m_doubleGridProperties.handleCOPYREGRecord( record, regionProperty );
            else
                throw std::invalid_argument("Fatal error processing COPYREG keyword - invalid/undefined keyword: " + srcArray);
        }
    }




    void Eclipse3DProperties::handleMULTIPLYKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);

            if (m_doubleGridProperties.hasKeyword( field ))
                m_doubleGridProperties.handleMULTIPLYRecord( record , boxManager );
            else if (m_intGridProperties.hasKeyword( field ))
                m_intGridProperties.handleMULTIPLYRecord( record , boxManager );
            else
                throw std::invalid_argument("Fatal error processing MULTIPLY keyword. Tried to shift not defined keyword " + field);

        }
    }


    /**
      The fine print of the manual says the ADD keyword should support
      some state dependent semantics regarding endpoint scaling arrays
      in the PROPS section. That is not supported.
    */
    void Eclipse3DProperties::handleADDKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);

            if (m_doubleGridProperties.hasKeyword( field ))
                m_doubleGridProperties.handleADDRecord( record , boxManager );
            else if (m_intGridProperties.hasKeyword( field ))
                m_intGridProperties.handleADDRecord( record , boxManager );
            else
                throw std::invalid_argument("Fatal error processing ADD keyword. Tried to shift not defined keyword " + field);

        }
    }


    void Eclipse3DProperties::handleCOPYKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("src").get< std::string >(0);

            if (m_doubleGridProperties.hasKeyword( field ))
                m_doubleGridProperties.handleCOPYRecord( record , boxManager );
            else if (m_intGridProperties.hasKeyword( field ))
                m_intGridProperties.handleCOPYRecord( record , boxManager );
            else
                throw std::invalid_argument("Fatal error processing COPY keyword. Tried to copy not defined keyword " + field);

        }
    }


    void Eclipse3DProperties::handleEQUALSKeyword( const DeckKeyword& deckKeyword, BoxManager& boxManager) {
        for( const auto& record : deckKeyword ) {
            const std::string& field = record.getItem("field").get< std::string >(0);

            if (m_doubleGridProperties.supportsKeyword( field ))
                m_doubleGridProperties.handleEQUALSRecord( record , boxManager );
            else if (m_intGridProperties.supportsKeyword( field ))
                m_intGridProperties.handleEQUALSRecord( record , boxManager );
            else
                throw std::invalid_argument("Fatal error processing EQUALS keyword. Tried to assign not defined keyword " + field);

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
