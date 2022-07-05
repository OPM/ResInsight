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

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>

namespace Opm {

    namespace {
        const char* default_dir = ".";

        bool normalize_case(std::string& s) {
            int upper_count = 0;
            int lower_count = 0;

            for (const auto& c : s) {
                if (std::isupper(c))
                    upper_count += 1;

                if (std::islower(c))
                    lower_count += 1;
            }

            if (upper_count * lower_count == 0)
                return false;

            for (auto& c : s)
                c = std::toupper(c);
            return true;
        }


        inline std::string basename( const std::string& path ) {
            return std::filesystem::path( path ).stem().string();
        }


        inline std::string outputdir( const std::string& path ) {
            auto dir = std::filesystem::path( path ).parent_path().string();

            if( dir.empty() ) return default_dir;

            return dir;
        }
    }


    IOConfig::IOConfig( const Deck& deck ) :
        IOConfig( GRIDSection( deck ),
                  RUNSPECSection( deck ),
                  deck.hasKeyword("NOSIM"),
                  deck.getDataFile() )
    {}

    IOConfig::IOConfig( const std::string& input_path ) :
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) )
    {
        this->setBaseName(basename(input_path));
    }

    IOConfig IOConfig::serializeObject()
    {
        IOConfig result;
        result.m_write_INIT_file = true;
        result.m_write_EGRID_file = false;
        result.m_UNIFIN = true;
        result.m_UNIFOUT = true;
        result.m_FMTIN = true;
        result.m_FMTOUT = true;
        result.m_deck_filename = "test1";
        result.m_output_enabled = false;
        result.m_output_dir = "test2";
        result.m_nosim = true;
        result.m_base_name = "test3";
        result.ecl_compatible_rst = false;

        return result;
    }

    static inline bool write_egrid_file( const GRIDSection& grid ) {
        if( grid.hasKeyword( "NOGGF" ) ) return false;
        if( !grid.hasKeyword( "GRIDFILE" ) ) return true;

        const auto& keyword = grid.getKeyword( "GRIDFILE" );
        const auto& rec = keyword.getRecord( 0 );

        {
            const auto& grid_item = rec.getItem( 0 );
            if (grid_item.get<int>(0) != 0) {
                std::cerr << "IOConfig: Reading GRIDFILE keyword from GRID section: "
                          << "Output of GRID file is not supported. "
                          << "Supported format: EGRID"
                          << std::endl;

                // It was asked for GRID file - that output is not
                // supported, but we will output EGRID file;
                // irrespective of whether that was actually
                // requested.
                return true;
            }
        }

        {
            const auto& egrid_item = rec.getItem( 1 );
            return (egrid_item.get<int>(0) == 1);
        }
    }

    IOConfig::IOConfig( const GRIDSection& grid,
                        const RUNSPECSection& runspec,
                        bool nosim,
                        const std::string& input_path ) :
        m_write_INIT_file( grid.hasKeyword( "INIT" ) ),
        m_write_EGRID_file( write_egrid_file( grid ) ),
        m_FMTIN( runspec.hasKeyword( "FMTIN" ) ),
        m_FMTOUT( runspec.hasKeyword( "FMTOUT" ) ),
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) ),
        m_nosim( nosim  )
    {
        this->setBaseName(basename(input_path));
        for (const auto& kw : runspec) {
            if (kw.name() == "UNIFOUT")
                this->m_UNIFOUT = true;
            else if (kw.name() == "UNIFIN")
                this->m_UNIFIN = true;
            else if (kw.name() == "MULTOUT")
                this->m_UNIFOUT = false;
            else if (kw.name() == "MULTIN")
                this->m_UNIFIN = false;
        }
    }


    bool IOConfig::getWriteEGRIDFile() const {
        return m_write_EGRID_file;
    }

    bool IOConfig::getWriteINITFile() const {
        return m_write_INIT_file;
    }


    bool IOConfig::getEclCompatibleRST() const {
        return this->ecl_compatible_rst;
    }


    void IOConfig::setEclCompatibleRST(bool ecl_rst) {
        this->ecl_compatible_rst = ecl_rst;
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

    void IOConfig::consistentFileFlags(){
        m_UNIFIN = getUNIFOUT();
        m_FMTIN = getFMTOUT();
    }

    bool IOConfig::getFMTIN() const {
        return m_FMTIN;
    }

    bool IOConfig::getFMTOUT() const {
        return m_FMTOUT;
    }



    std::string IOConfig::getRestartFileName(const std::string& restart_base, int report_step, bool output) const {
        bool unified  = output ? getUNIFOUT() : getUNIFIN();
        bool fmt_file = output ? getFMTOUT()  : getFMTIN();

        auto ext = std::string{};
        if (unified) {
            ext = fmt_file ? "FUNRST" : "UNRST";
        }
        else {
            std::ostringstream os;

            const char* fmt_prefix   = "FGH";
            const char* unfmt_prefix = "XYZ";

            const int cycle = 10 * 1000;
            const int p_ix  = report_step / cycle;
            const int n     = report_step % cycle;

            os << (fmt_file ? fmt_prefix[p_ix] : unfmt_prefix[p_ix])
               << std::setw(4) << std::setfill('0') << n;

            ext = os.str();
        }

        return restart_base + '.' + ext;
    }


    bool IOConfig::getOutputEnabled() const {
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

    void IOConfig::setBaseName(const std::string& baseName) {
        m_base_name = baseName;
        if (normalize_case(m_base_name))
            OpmLog::warning("The ALL CAPS case: " + m_base_name + " will be used when writing output files from this simulation.");
    }

    std::string IOConfig::fullBasePath( ) const {
        namespace fs = std::filesystem;

        fs::path dir( m_output_dir );
        fs::path base( m_base_name );
        fs::path full_path = dir.make_preferred() / base.make_preferred();

        return full_path.string();
    }


    bool IOConfig::initOnly( ) const {
        return m_nosim;
    }


    bool IOConfig::operator==(const IOConfig& data) const {
        return this->getWriteINITFile() == data.getWriteINITFile() &&
               this->getWriteEGRIDFile() == data.getWriteEGRIDFile() &&
               this->getUNIFIN() == data.getUNIFIN() &&
               this->getUNIFOUT() == data.getUNIFOUT() &&
               this->getFMTIN() == data.getFMTIN() &&
               this->getFMTOUT() == data.getFMTOUT() &&
               this->m_deck_filename == data.m_deck_filename &&
               this->getOutputEnabled() == data.getOutputEnabled() &&
               this->getOutputDir() == data.getOutputDir() &&
               this->initOnly() == data.initOnly() &&
               this->getBaseName() == data.getBaseName() &&
               this->getEclCompatibleRST() == data.getEclCompatibleRST();
    }


    bool IOConfig::rst_cmp(const IOConfig& full_config, const IOConfig& rst_config) {
        return full_config.getWriteINITFile() == rst_config.getWriteINITFile() &&
               full_config.getWriteEGRIDFile() == rst_config.getWriteEGRIDFile() &&
               full_config.getUNIFIN() == rst_config.getUNIFIN() &&
               full_config.getUNIFOUT() == rst_config.getUNIFOUT() &&
               full_config.getFMTIN() == rst_config.getFMTIN() &&
               full_config.getFMTOUT() == rst_config.getFMTOUT();
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
