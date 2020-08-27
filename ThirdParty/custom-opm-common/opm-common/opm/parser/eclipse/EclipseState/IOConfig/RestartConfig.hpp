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

#ifndef OPM_RESTART_CONFIG_HPP
#define OPM_RESTART_CONFIG_HPP

#include <vector>
#include <set>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>

/*
  The RestartConfig class internalizes information of when (at which
  report steps) we should save restart files, and which properties
  should be included in the restart files. The configuration of this
  immensely complex, and this code is unfortunately also way too
  complex.

  The most basic question to disentangle is the "When to write restart
  files" versus "What data to store write in the restart file". As
  expressed in the deck keywords this completely entangled, in this
  implementation we have tried to disentangle it:

  Keywords involved
  -----------------

  RPTRST: This is the main keyword for configuring restart output; it
     can be used to configure bothe when to write the files and which
     properties should be included in the restart files.


  RPTSCHED: The main purpose of the RPTSCHED keyword is to configure
     output from the SCHEDULE section to the PRINT file. However the
     mneomnic RESTART=n can be used to turn writing of restart files
     on, and also for values > 2 to some configuration of what is
     written to the restart file:

     RESTART=1 : As RPTRST,BASIC=1
     RESTART>1 : As RPTRST,BASIC=2
     RESTART>2 : Flow is added to restart file
     RESTART>3 : Fluid in place is added to restart file
     RESTART=6 : Restart file for every timestep.



  RPTSOL: The RPTSOL keyword is very similar to the RPTCHED keyword,
     it configures output from the SOLUTION section to the PRINT file,
     but just as the RPTSCHED keyword it accepts a RESTART=n mnenonic
     which can be used similarly to the BASIC=n mnenonic of the RPTRST
     keyword. In particular the writing of an initial restart files
     with initial equilibrium solution is controlled by the RPTSOL
     keyword. If the restart mneonic is greater than 2 that can be
     used to configure FLOWS and FIP keywords in the restart file.

     RESTART=1 : As RPTRST,BASIC=1
     RESTART>1 : As RPTRST,BASIC=2
     RESTART>2 : Flow is added to restart file
     RESTART>3 : Fluid in place is added to restart file


  The basic rule in ECLIPSE is generally that the 'last keyword wins',
  but for the RPTRST RPTSHCED combination a BASIC setting with n >= 3
  will override consecutive RESTART=n settings from RPTSCHED.


  When to write restart files:
  ----------------------------

  When to write the restart file is governed by the BASIC=n setting in
  the RPTRST keyword and the RESTART=n settings in the RPTSOL and
  RPTSCHED keywords. The most common setting is 'ON' - i.e. BASIC=2
  which means write a restart file for every report step, that can be
  turned off again with BASIC=0. For BASIC>2 there are varietes of
  every n'th report step, and the first report step in every month and
  every year.


  Old style / new style
  ---------------------

  All of the relevant keywords can be specified using a new style
  based on string mneomnics and alternatively an old style represented
  with a *strictly ordered* list of integers. For instance both of
  these keywords request restart files written for every report step;
  in addition to the fields required to actually restart the files
  should contain the relative permeabilities KRO, KRW, KRG:

    RPTRST
       BASIC=2  KRG KRW  KRO /


    RPTRST
       2 9*0 3*1 17*0

  Integer controls and string mneomnics can not be mixed in the same
  keyword, but they can be mixed in the same deck - and that is
  actually quite common.


  What is written to the restart file
  -----------------------------------

  The BASIC=n mneonics request the writing of a restart file which
  should contain 'all properties required to restart', in addition you
  can configure extra keywords to be added to the restart file. This
  is configured by just adding a list as:

     RPTRST
       BASIC=2  KRG KRW  KRO /

  It is really *not clear* what is the correct persistence semantics
  for these keywords, consider for insance the following series of keywords:

     -- Request restart file at every report step, the restart files
     -- should contain additional properties KRO, KRG and KRW.
     RPTRST
        BASIC=2 KRG KRW KRO /

     -- Advance the simulator forward with TSTEP / DATES
     TSTEP / DATES / WCONxxx

     -- Turn writing of restart files OFF using integer controls.
     RPTRST
        0 /

     -- Advance the simulator forward with TSTEP / DATES
     TSTEP / DATES / WCONxxx

    -- Turn writing of restart files ON using integer controls.
    RPTRST
        2 /

  When writing of restart files is turned on again with the last
  RPTRST keyword, should still the relative permeabilites KRO, KRW and
  KRG be added to the restart files? The model we have implemented is:

   - The list of keywords written to the restart file is persisted
     independtly of the BASIC=n setting.

   - Using string based mnonics you can *only add* kewyords to be
     written to the files. To stop writing a keyword you must use an
     integer control with value 0.

  Based on this best guess heuristic the final restart files will
  still contain KRO, KRW and KRG.



  What is required to restart?
  ----------------------------

  A restart capable files is requested with the 'BASIC' mneomnic, but
  exactly which properties the 'BASIC' keyword is expanded to is the
  responsability of the simulator; i.e. for a black oil simulation you
  will at the very least need the expansion:

     BASIC -> PRESSURE, SWAT, SGAS, RS, RV

  But this class just carries the boolean information: Yes - restart
  is requested - expanding as illustrated is the responsability of the
  simulator.




  What is not supported?
  ----------------------

  The SAVE keyword is not supported in OPM at all, this implies that
  the SAVE and SFREQ mneomics are not supported. 
*/



namespace Opm {

    template< typename > class DynamicState;

    class Deck;
    class DeckKeyword;
    class GRIDSection;
    class RUNSPECSection;
    class SCHEDULESection;
    class SOLUTIONSection;
    class Schedule;
    class ParseContext;
    class ErrorGuard;

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

    //namespace {

    class RestartSchedule {
        /*
           The content of this struct is logically divided in two; either the
           restart behaviour is governed by { timestep , basic , frequency }, or
           alternatively by { rptshec_restart_set , rptsched_restart }.

           The former triplet is mainly governed by the RPTRST keyword and the
           latter pair by the RPTSCHED keyword.
           */
        public:

            RestartSchedule() = default;
            explicit RestartSchedule( size_t sched_restart);
            RestartSchedule( size_t step, size_t b, size_t freq);

            static RestartSchedule serializeObject();

            bool writeRestartFile( size_t timestep , const TimeMap& timemap) const;
            bool operator!=(const RestartSchedule& rhs) const;
            bool operator==( const RestartSchedule& rhs ) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(timestep);
                serializer(basic);
                serializer(frequency);
                serializer(rptsched_restart_set);
                serializer(rptsched_restart);
            }

        //private:
            size_t timestep = 0;
            size_t basic = 0;
            size_t frequency = 0;
            bool   rptsched_restart_set = false;
            size_t rptsched_restart = 0;
    };
    //    }
     class RestartConfig {

    public:

        RestartConfig() = default;

        template<typename T>
        RestartConfig( const TimeMap& time_map, const Deck&, const ParseContext& parseContext, T&& errors );
        RestartConfig( const TimeMap& time_map, const Deck&, const ParseContext& parseContext, ErrorGuard& errors );
        RestartConfig( const TimeMap& time_map, const Deck& );

        static RestartConfig serializeObject();

        int  getFirstRestartStep() const;
        bool getWriteRestartFile(size_t timestep, bool log=true) const;
        const std::map< std::string, int >& getRestartKeywords( size_t timestep ) const;
        int getKeyword( const std::string& keyword, size_t timeStep) const;

        void overrideRestartWriteInterval(size_t interval);
        void handleSolutionSection(const SOLUTIONSection& solutionSection, const ParseContext& parseContext, ErrorGuard& errors);
        void setWriteInitialRestartFile(bool writeInitialRestartFile);

        RestartSchedule getNode( size_t timestep ) const;
        static std::string getRestartFileName(const std::string& restart_base, int report_step, bool unified, bool fmt_file);

        bool operator==(const RestartConfig& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_timemap.serializeOp(serializer);
            serializer(m_first_restart_step);
            serializer(m_write_initial_RST_file);
            restart_schedule.serializeOp(serializer);
            restart_keywords.serializeOp<Serializer, false>(serializer);
            serializer(save_keywords);
        }

    private:
        /// This method will internalize variables with information of
        /// the first report step with restart and rft output
        /// respectively. This information is important because right
        /// at the first output step we must reset the files to size
        /// zero, for subsequent output steps we should append.
        void initFirstOutput( );

        bool getWriteRestartFileFrequency(size_t timestep,
                                          size_t start_timestep,
                                          size_t frequency,
                                          bool years  = false,
                                          bool months = false) const;
        void handleRPTSOL( const DeckKeyword& keyword);
        void handleScheduleSection( const SCHEDULESection& schedule, const ParseContext& parseContext, ErrorGuard& errors);
        void update( size_t step, const RestartSchedule& rs);
        static RestartSchedule rptsched( const DeckKeyword& );

        TimeMap m_timemap;
        int     m_first_restart_step = 1;
        bool    m_write_initial_RST_file = false;

        DynamicState< RestartSchedule > restart_schedule;
        DynamicState< std::map< std::string, int > > restart_keywords;
        std::vector< bool > save_keywords;
    };
} //namespace Opm



#endif
