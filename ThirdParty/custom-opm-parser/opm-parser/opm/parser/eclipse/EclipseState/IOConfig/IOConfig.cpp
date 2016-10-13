/*
  Copyright 2015 Statoil ASA.

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

#include <stdio.h>
#include <iostream>
#include <iterator>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <ert/ecl/ecl_util.h>




namespace Opm {

    namespace {
        const char* default_dir = ".";

        inline std::string basename( const std::string& path ) {
            return boost::filesystem::path( path ).stem().string();
        }

        inline std::string outputdir( const std::string& path ) {
            auto dir = boost::filesystem::path( path ).parent_path().string();

            if( dir.empty() ) return default_dir;

            return dir;
        }
    }


    IOConfig::IOConfig( const Deck& deck ) :
        IOConfig( GRIDSection( deck ),
                  RUNSPECSection( deck ),
                  std::make_shared< const TimeMap >( deck ),
                  deck.hasKeyword("NOSIM"),
                  deck.getDataFile() )
    {}

    IOConfig::IOConfig( const std::string& input_path ) :
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) ),
        m_base_name( basename( input_path ) )
    {}

    static inline bool write_egrid_file( const GRIDSection& grid ) {
        if( grid.hasKeyword( "NOGGF" ) ) return false;
        if( !grid.hasKeyword( "GRIDFILE" ) ) return true;

        const auto& keyword = grid.getKeyword( "GRIDFILE" );

        if( keyword.size() == 0 ) return false;

        const auto& rec = keyword.getRecord( 0 );
        const auto& item1 = rec.getItem( 0 );

        if( item1.hasValue( 0 ) && item1.get< int >( 0 ) != 0 ) {
            std::cerr << "IOConfig: Reading GRIDFILE keyword from GRID section: "
                      << "Output of GRID file is not supported. "
                      << "Supported format: EGRID"
                      << std::endl;
            return true;
        }

        if( rec.size() < 1 ) return true;

        const auto& item2 = rec.getItem( 1 );
        return !item2.hasValue( 0 ) || item2.get< int >( 0 ) != 0;
    }

    IOConfig::IOConfig( const GRIDSection& grid,
                        const RUNSPECSection& runspec,
                        std::shared_ptr< const TimeMap > timemap,
                        bool nosim,
                        const std::string& input_path ) :
        m_timemap( timemap ),
        m_write_INIT_file( grid.hasKeyword( "INIT" ) ),
        m_write_EGRID_file( write_egrid_file( grid ) ),
        m_UNIFIN( runspec.hasKeyword( "UNIFIN" ) ),
        m_UNIFOUT( runspec.hasKeyword( "UNIFOUT" ) ),
        m_FMTIN( runspec.hasKeyword( "FMTIN" ) ),
        m_FMTOUT( runspec.hasKeyword( "FMTOUT" ) ),
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) ),
        m_base_name( basename( input_path ) ),
        m_nosim( nosim  )
    {}


    bool IOConfig::getWriteEGRIDFile() const {
        return m_write_EGRID_file;
    }

    bool IOConfig::getWriteINITFile() const {
        return m_write_INIT_file;
    }


    /*
      Will initialize an internal variable holding the first report
      step when rft output is queried. The reason we are interested in
      this report step is that when we reach this step the output
      files should be opened with mode 'w' - whereas for subsequent
      steps it should be opened with mode 'a'.
    */

    void IOConfig::initFirstRFTOutput(const Schedule& schedule) {
        m_first_rft_step = -1;

        for (const auto& well : schedule.getWells( )) {
            int well_output = well->firstRFTOutput();
            if (well_output >= 0) {
                if ((m_first_rft_step < 0) || (well_output < m_first_rft_step))
                    m_first_rft_step = well_output;
            }
        }
    }




    void IOConfig::overrideNOSIM(bool nosim) {
        m_nosim = nosim;
    }


    bool IOConfig::getUNIFIN() const {
        return m_UNIFIN;
    }

    bool IOConfig::getUNIFOUT() const {
        return m_UNIFOUT;
    }

    bool IOConfig::getFMTIN() const {
        return m_FMTIN;
    }

    bool IOConfig::getFMTOUT() const {
        return m_FMTOUT;
    }



    boost::gregorian::date IOConfig::getTimestepDate(size_t reportStep) const {
        auto time = (*m_timemap)[reportStep];
        return time.date();
    }



    std::string IOConfig::getRestartFileName(const std::string& restart_base, int report_step, bool output) const {
        bool unified  = output ? getUNIFOUT() : getUNIFIN();
        bool fmt_file = output ? getFMTOUT()  : getFMTIN();

        ecl_file_enum file_type = (unified) ? ECL_UNIFIED_RESTART_FILE : ECL_RESTART_FILE;
        char * c_str = ecl_util_alloc_filename( NULL , restart_base.c_str() , file_type, fmt_file , report_step);
        std::string restart_filename = c_str;
        free( c_str );

        return restart_filename;
    }


    int IOConfig::getFirstRFTStep() const {
        return m_first_rft_step;
    }

    bool IOConfig::getOutputEnabled(){
        return m_output_enabled;
    }

    void IOConfig::setOutputEnabled(bool enabled){
        m_output_enabled = enabled;
    }

    std::string IOConfig::getOutputDir() const {
        return m_output_dir;
    }

    void IOConfig::setOutputDir(const std::string& outputDir) {
        m_output_dir = outputDir;
    }

    const std::string& IOConfig::getBaseName() const {
        return m_base_name;
    }

    void IOConfig::setBaseName(std::string baseName) {
        m_base_name = baseName;
    }

    std::string IOConfig::fullBasePath( ) const {
        namespace fs = boost::filesystem;

        fs::path dir( m_output_dir );
        fs::path base( m_base_name );
        fs::path full_path = dir.make_preferred() / base.make_preferred();

        return full_path.string();
    }


    bool IOConfig::initOnly( ) const {
        return m_nosim;
    }


    /*****************************************************************/
    /* Here at the bottom are some forwarding proxy methods which just
       forward to the appropriate RestartConfig method. They are
       retained here as a temporary convenience method to prevent
       downstream breakage.

       Currently the EclipseState object can return a mutable IOConfig
       object, which application code can alter to override settings
       from the deck - this is quite ugly. When the API is reworked to
       remove the ability modify IOConfig objects we should also
       remove these forwarding methods.
    */

} //namespace Opm
