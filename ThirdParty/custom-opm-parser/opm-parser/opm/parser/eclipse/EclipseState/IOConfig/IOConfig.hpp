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


        int  getFirstRFTStep() const;
        bool getWriteEGRIDFile() const;
        bool getWriteINITFile() const;
        bool getUNIFOUT() const;
        bool getUNIFIN() const;
        bool getFMTIN() const;
        bool getFMTOUT() const;
        const std::string& getEclipseInputPath() const;

        void overrideNOSIM(bool nosim);



        boost::gregorian::date getTimestepDate(size_t timestep) const;

        std::string getRestartFileName(const std::string& restart_base, int report_step, bool output) const;

        bool getOutputEnabled();
        void setOutputEnabled(bool enabled);

        std::string getOutputDir() const;
        void setOutputDir(const std::string& outputDir);

        const std::string& getBaseName() const;
        void setBaseName(std::string baseName);

        /// Return a string consisting of outputpath and basename;
        /// i.e. /path/to/sim/CASE
        std::string fullBasePath( ) const;

        bool initOnly() const;
        void initFirstRFTOutput(const Schedule& schedule);

        // Proxy methods forwarding directly to corresponding RestartConfig
        bool getWriteRestartFile(size_t timestep) const;
        int  getFirstRestartStep() const;
        void overrideRestartWriteInterval(size_t interval);
        void setWriteInitialRestartFile(bool writeInitialRestartFile);

    private:
        std::shared_ptr< const TimeMap > m_timemap;
        bool            m_write_INIT_file = false;
        bool            m_write_EGRID_file = true;
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
        bool            m_nosim;

        IOConfig( const GRIDSection&,
                  const RUNSPECSection&,
                  std::shared_ptr< const TimeMap >,
                  bool nosim,
                  const std::string& input_path );

    };


    typedef std::shared_ptr<IOConfig> IOConfigPtr;
    typedef std::shared_ptr<const IOConfig> IOConfigConstPtr;

} //namespace Opm



#endif
