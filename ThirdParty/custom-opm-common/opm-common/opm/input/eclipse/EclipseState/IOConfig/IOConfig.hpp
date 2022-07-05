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

#include <string>

namespace Opm {

    class Deck;
    class GRIDSection;
    class RUNSPECSection;

    /*The IOConfig class holds data about input / output configurations

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


      ECL compatible restart
      ======================

      Unfortunately flow & eclipse are not compatible across restarts. The
      RestartIO implementation can write restart files for flow -> flow restart
      or alternatively for flow -> eclipse restart. This is regulated by the
      boolean flag ecl_compatible_restart in the IOConfig class. The difference
      between the two are as follows:

      ecl_compatible_restart = false:

        1. The 'extra' fields in the RestartValue structure are actually
           written to disk.

        2. You can optionally ask the RestartIO::save() function to save the
           solution in double precision.

        3. The RestartIO::save() function will save opm specific vector OPM_IWEL
           and OPM_XWEL.

      ecl_compatible_restart = true:

        1. The 'extra' fields in the RestartValue are silently ignored.

        2. If request double precision solution data that is silently ignored,
           it will be float.

        3. The OPM_IWEL and OPM_XWEL vectors will not be written.

      Observe that the solution data in the RestartValue is passed
      unconditionally to the solution section in the restart file, so if you
      pass a field in the solution section which Eclipse does not recognize you
      will end up with a restart file which Eclipse can not read, even if you
      have set ecl_compatible_restart to true.
    */



    class IOConfig {

    public:

        IOConfig() = default;
        explicit IOConfig( const Deck& );
        explicit IOConfig( const std::string& input_path );

        static IOConfig serializeObject();

        void setEclCompatibleRST(bool ecl_rst);
        bool getEclCompatibleRST() const;
        bool getWriteEGRIDFile() const;
        bool getWriteINITFile() const;
        bool getUNIFOUT() const;
        bool getUNIFIN() const;
        bool getFMTIN() const;
        bool getFMTOUT() const;
        const std::string& getEclipseInputPath() const;

        void overrideNOSIM(bool nosim);
        void consistentFileFlags();

        std::string getRestartFileName(const std::string& restart_base, int report_step, bool output) const;

        bool getOutputEnabled() const;
        void setOutputEnabled(bool enabled);

        std::string getOutputDir() const;
        void setOutputDir(const std::string& outputDir);

        const std::string& getBaseName() const;
        void setBaseName(const std::string& baseName);

        /// Return a string consisting of outputpath and basename;
        /// i.e. /path/to/sim/CASE
        std::string fullBasePath( ) const;

        bool initOnly() const;

        bool operator==(const IOConfig& data) const;
        static bool rst_cmp(const IOConfig& full_config, const IOConfig& rst_config);


        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_write_INIT_file);
            serializer(m_write_EGRID_file);
            serializer(m_UNIFIN);
            serializer(m_UNIFOUT);
            serializer(m_FMTIN);
            serializer(m_FMTOUT);
            serializer(m_deck_filename);
            serializer(m_output_enabled);
            serializer(m_output_dir);
            serializer(m_nosim);
            serializer(m_base_name);
            serializer(ecl_compatible_rst);
        }

    private:
        bool            m_write_INIT_file = false;
        bool            m_write_EGRID_file = true;
        bool            m_UNIFIN = false;
        bool            m_UNIFOUT = false;
        bool            m_FMTIN = false;
        bool            m_FMTOUT = false;
        std::string     m_deck_filename;
        bool            m_output_enabled = true;
        std::string     m_output_dir;
        bool            m_nosim;
        std::string     m_base_name;
        bool            ecl_compatible_rst = true;

        IOConfig( const GRIDSection&,
                  const RUNSPECSection&,
                  bool nosim,
                  const std::string& input_path );

    };

} //namespace Opm



#endif
