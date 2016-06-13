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

#ifndef OPM_IO_CONFIG_HPP
#define OPM_IO_CONFIG_HPP

#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace Opm {

    template< typename > class DynamicState;

    class Deck;
    class DeckKeyword;
    class GRIDSection;
    class RUNSPECSection;
    class SCHEDULESection;
    class SOLUTIONSection;
    class TimeMap;
    class Schedule;

    /*The IOConfig class holds data about input / ouput configurations

      Amongst these configuration settings, a IOConfig object knows if
      a restart file should be written for a specific report step

      The write of restart files is governed by several eclipse keywords.
      These keywords are all described in the eclipse manual, but some
      of them are rather porly described there.
      To have equal sets of restart files written from Eclipse and Flow for various
      configurations, we have made a qualified guess on the behaviour
      for some of the keywords (by running eclipse for different configurations,
      and looked at which restart files that have been written).


      ------ RPTSOL RESTART (solution section) ------
      If RPTSOL RESTART > 1 initial restart file is written.


      ------ RPTRST (solution section) ------
      Eclipse manual states that the initial restart file is to be written
      if RPTSOL RESTART > 1. But - due to that the initial restart file
      is written from Eclipse for data where RPTSOL RESTART is not set, - we
      have made a guess that when RPTRST is set in SOLUTION (no basic though...),
      it means that the initial restart file should be written.
      Running of eclipse with different settings have proven this to be a qualified guess.


      ------ RPTRST BASIC=0 (solution or schedule section) ------
      No restart files are written


      ------ RPTRST BASIC=1 or RPTRST BASIC=2 (solution or schedule section) ------
      Restart files are written for every timestep, from timestep 1 to number of timesteps.
      (Write of inital timestep is governed by a separate setting)

      Notice! Eclipse simulator RPTRST BASIC=1 writes restart files for every
      report step, but only keeps the last one written. This functionality is
      not supported in Flow; so to compare Eclipse results with Flow results
      for every report step, set RPTRST BASIC=2 for the eclipse run


      ------ RPTRST BASIC=3 FREQ=n (solution or schedule section) ------
      Restart files are created every nth report time.  Default frequency is 1 (every report step)

      If a frequency higher than 1 is given:
      start_rs = report step the setting was given.
      write report step rstep if (rstep >= start_rs) && ((rstep % frequency) == 0).


      ------ RPTRST BASIC=4 FREQ=n or RPTRST BASIC=5 FREQ=n (solution or schedule section) ------
      For the settings BASIC 4 or BASIC 5, - first report step of every new year(4) or new month(5),
      the first report step is compared with report step 0 (start), and then every report step is
      compared with the previous one to see if year/month has changed.

      This leaves us with a set of timesteps.
      All timesteps in the set that are higher or equal to the timestep the RPTRST keyword was set on is written.

      If in addition FREQUENCY is given (higher than 1), every n'the value of this set are to be written.

      If the setting BASIC=4 or BASIC=5 is set on a timestep that is a member of the set "first timestep of
      each year" / "First timestep of each month", then the timestep that is freq-1 timesteps (within the set) from
      this start timestep will be written, and then every n'the timestep (within the set) from this one will be written.

      If the setting BASIC=4 or BASIC=5 is set on a timestep that is not a member of the list "first timestep of
      each year" / "First timestep of each month", then the list is searched for the closest timestep that are
      larger than the timestep that introduced the setting, and then; same as above - the timestep that is freq-1
      timesteps from this one (within the set) will be written, and then every n'the timestep (within the set) from
      this one will be written.


      ------ RPTRST BASIC=6 (solution or schedule section) ------
      Not supported in Flow


      ------ Default ------
      If no keywords for config of writing restart files have been handled; no restart files are written.

    */



    class IOConfig {

    public:

        IOConfig() = default;
        explicit IOConfig( const Deck& );
        explicit IOConfig( const std::string& input_path );

        int  getFirstRestartStep() const;
        int  getFirstRFTStep() const;
        bool getWriteRestartFile(size_t timestep) const;
        bool getWriteEGRIDFile() const;
        bool getWriteINITFile() const;
        bool getUNIFOUT() const;
        bool getUNIFIN() const;
        bool getFMTIN() const;
        bool getFMTOUT() const;
        const std::string& getEclipseInputPath() const;

        void overrideRestartWriteInterval(size_t interval);

        void handleSolutionSection(std::shared_ptr< const TimeMap > timemap, std::shared_ptr<const SOLUTIONSection> solutionSection);
        void setWriteInitialRestartFile(bool writeInitialRestartFile);

        /// This method will internalize variables with information of
        /// the first report step with restart and rft output
        /// respectively. This information is important because right
        /// at the first output step we must reset the files to size
        /// zero, for subsequent output steps we should append.
        void initFirstOutput(const Schedule& schedule);

        boost::gregorian::date getTimestepDate(size_t timestep) const;
        void dumpRestartConfig() const;

        std::string getRestartFileName(const std::string& restart_base, int report_step, bool output) const;

        bool getOutputEnabled();
        void setOutputEnabled(bool enabled);

        std::string getOutputDir();
        void setOutputDir(const std::string& outputDir);

        const std::string& getBaseName() const;
        void setBaseName(std::string baseName);

    private:

        IOConfig( const GRIDSection&,
                  const RUNSPECSection&,
                  const SCHEDULESection&,
                  std::shared_ptr< const TimeMap >,
                  const std::string& input_path );

        void assertTimeMap(std::shared_ptr< const TimeMap > timemap);
        bool getWriteRestartFileFrequency(size_t timestep,
                                          size_t start_timestep,
                                          size_t frequency,
                                          bool years  = false,
                                          bool months = false) const;
        void handleRPTSOL( const DeckKeyword& keyword);

        std::shared_ptr< const TimeMap > m_timemap;
        bool            m_write_INIT_file = false;
        bool            m_write_EGRID_file = true;
        bool            m_write_initial_RST_file = false;
        bool            m_UNIFIN = false;
        bool            m_UNIFOUT = false;
        bool            m_FMTIN = false;
        bool            m_FMTOUT = false;
        int             m_first_restart_step;
        int             m_first_rft_step;
        std::string     m_deck_filename;
        bool            m_output_enabled = true;
        std::string     m_output_dir;
        std::string     m_base_name;

        struct restartConfig {
            /*
              The content of this struct is logically divided in two; either the
              restart behaviour is governed by { timestep , basic , frequency }, or
              alternatively by { rptshec_restart_set , rptsched_restart }.

              The former triplet is mainly governed by the RPTRST keyword and the
              latter pair by the RPTSCHED keyword.
            */
            size_t timestep = 0;
            size_t basic = 0;
            size_t frequency = 0;
            bool   rptsched_restart_set = false;
            size_t rptsched_restart = 0;

            restartConfig() = default;
            restartConfig( size_t sched_restart ) :
                rptsched_restart_set( true ),
                rptsched_restart( sched_restart )
            {}

            restartConfig( size_t step, size_t b, size_t freq ) :
                timestep( step ),
                basic( b ),
                frequency( freq )
            {}

            bool operator!=(const restartConfig& rhs) const {
                return !( *this == rhs );
            }

            bool operator==( const restartConfig& rhs ) const {
                if( this->rptsched_restart_set ) {
                    return rhs.rptsched_restart_set
                        && this->rptsched_restart == rhs.rptsched_restart;
                }

                return this->timestep == rhs.timestep &&
                       this->basic == rhs.basic &&
                       this->frequency == rhs.frequency;
            }
        };

        static DynamicState< restartConfig > rstconf( const SCHEDULESection&,
                                                      std::shared_ptr< const TimeMap > );
        static restartConfig rptrst( const DeckKeyword&, size_t );
        static restartConfig rptsched( const DeckKeyword& );

        std::shared_ptr<DynamicState<restartConfig>> m_restart_output_config;


    };


    typedef std::shared_ptr<IOConfig> IOConfigPtr;
    typedef std::shared_ptr<const IOConfig> IOConfigConstPtr;

} //namespace Opm



#endif
