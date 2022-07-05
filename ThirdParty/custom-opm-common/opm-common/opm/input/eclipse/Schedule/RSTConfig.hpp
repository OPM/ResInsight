/*
  Copyright 2021 Equinor ASA.

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

#ifndef OPM_RST_CONFIG_HPP
#define OPM_RST_CONFIG_HPP


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

#include <unordered_map>
#include <map>
#include <optional>

namespace Opm {

class DeckKeyword;
class ErrorGuard;
class ParseContext;
class SOLUTIONSection;

class RSTConfig {
public:
    RSTConfig() = default;
    RSTConfig(const SOLUTIONSection& solution_section, const ParseContext& parseContext, ErrorGuard& errors);
    void update(const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors);
    void init_next();
    static RSTConfig first(const RSTConfig& src);
    static RSTConfig serializeObject();

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer(write_rst_file);
        serializer.template map<std::map<std::string, int>, false>(keywords);
        serializer(basic);
        serializer(freq);
        serializer(save);
    }

    bool operator==(const RSTConfig& other) const;

    std::optional<bool> write_rst_file;
    std::map<std::string, int> keywords;
    std::optional<int> basic;
    std::optional<int> freq;
    bool save = false;

private:
    void handleRPTSOL(const DeckKeyword& keyword);
    void handleRPTRST(const DeckKeyword& keyword, const ParseContext& parse_context, ErrorGuard& errors);
    void handleRPTSCHED(const DeckKeyword& keyword, const ParseContext& parse_context, ErrorGuard& errors);
    void update_schedule(const std::pair<std::optional<int>, std::optional<int>>& basic_freq);
};



} //namespace Opm



#endif
