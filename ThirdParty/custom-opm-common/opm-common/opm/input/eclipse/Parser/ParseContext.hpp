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


#ifndef OPM_PARSE_CONTEXT_HPP
#define OPM_PARSE_CONTEXT_HPP

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/input/eclipse/Parser/InputErrorAction.hpp>

namespace Opm {


class KeywordLocation;


    /*
       The ParseContext class is meant to control the behavior of the
       parsing and EclipseState construction phase when
       errors/inconsistencies/... are encountered in the input.

       For each of the possible problems encountered the possible
       actions are goverened by the InputError::Action enum:

          InputError::THROW_EXCEPTION
          InputError::EXIT1
          InputError::WARN
          InputError::IGNORE

       The internal datastructure is a map between string keys and
       enum InputError::Action values. The string keys are meant to be
       descriptive like:

          "PARSE_RANDOMTEXT"


       The constructor will consult the env variable
       OPM_ERRORS_IGNORE, OPM_ERRORS_WARN and OPM_ERRORS_EXCEPTION
       when initializing. The variables should be set as strings of
       update syntax.

       update_syntax: The main function for updating the policy of a
       parseContext instance is the update() method. That takes a string
       as input, and updates the matching flags. The string can
       contain wildcards ('* and '?' mathced with fnmatch()) and is
       split on ':' or '|' to allow multiple settings to be applied in
       one go:

       Just set one variable:
          update("PARSE_RANDOM_SLASH" , InputError::IGNORE)

       Ignore all unsupported features:
         update("UNSUPPORTED_*" , InputError::IGNORE)

       Set two variables:
        update("UNSUPPORTED_INIITIAL_THPRES:PARSE_RANDOM_SLASH" , InputError::IGNORE)

       The update function itself is quite tolerant, and will silently
       ignore unknown keys. If you use the updateKey() function only
       recognizd keys will be allowed.
    */

    class ErrorGuard;

    class ParseContext {
    public:
        ParseContext();
        explicit ParseContext(InputError::Action default_action);
        explicit ParseContext(const std::vector<std::pair<std::string , InputError::Action>>& initial);

        void handleError( const std::string& errorKey, const std::string& msg, const std::optional<KeywordLocation>& location, ErrorGuard& errors)  const;
        void handleUnknownKeyword(const std::string& keyword, const std::optional<KeywordLocation>& location, ErrorGuard& errors) const;
        bool hasKey(const std::string& key) const;
        ParseContext  withKey(const std::string& key, InputError::Action action = InputError::WARN) const;
        ParseContext& withKey(const std::string& key, InputError::Action action = InputError::WARN);
        void updateKey(const std::string& key , InputError::Action action);
        void update(InputError::Action action);
        void update(const std::string& keyString , InputError::Action action);
        void ignoreKeyword(const std::string& keyword);
        InputError::Action get(const std::string& key) const;
        std::map<std::string,InputError::Action>::const_iterator begin() const;
        std::map<std::string,InputError::Action>::const_iterator end() const;
        /*
          When the key is added it is inserted in 'strict mode',
          i.e. with the value 'InputError::THROW_EXCEPTION. If you
          want a different value you must subsequently call the update
          method.
        */
      void addKey(const std::string& key, InputError::Action default_action);
        /*
          The PARSE_EXTRA_RECORDS field regulates how the parser
          responds to keywords whose size has been defined in the
          previous keyword.
          Example:
          EQLDIMS
            2  100  20  1  1  /
          EQUIL\n
           2469   382.4   1705.0  0.0    500    0.0     1     1      20 /
           2469   382.4   1705.0  0.0    500    0.0     1     1      20 /
           2470   382.4   1705.0  0.0    500    0.0     1     1      20 /
          EQLDIMS's first entry is 2 and defines the record size of the
          EQUIL keyword. Since there are 3 records in EQUIL, this results
          in an error that needs to be handled by the parser. By default,
          an exception is thrown, or it may be specified in the
          PARSE_EXTRA_RECORDS field that this error is to be ignored.
        */
        const static std::string PARSE_EXTRA_RECORDS;
        /*
          The unknownKeyword field regulates how the parser should
          react when it encounters an unknwon keyword. Observe that
          'keyword' in this context means:

             o A string of 8 characters or less - starting in column
               0.

             o A string consisiting of UPPERCASE characters and
               numerals, staring with an UPPERCASE character [Hmmm -
               actually lowercase is also accepted?!]

           Observe that unknownKeyword does *not* consult any global
           collection of keywords to see if a particular string
           corresponds to a known valid keyword which we just happen
           to ignore for this particualar parse operation.

           The 'unknownkeyword' and 'randomText' error situations are
           not fully orthogonal, and in particualar if a unknown
           keyword has been encountered - without halting the parser, a
           subsequent piece of 'random text' might not be identified
           correctly as such.
        */
        const static std::string PARSE_UNKNOWN_KEYWORD;

        /*
          With random text we mean a string in the input deck is not
          correctly formatted as a keyword heading.
        */
        const static std::string PARSE_RANDOM_TEXT;

        /*
          It turns out that random '/' - i.e. typically an extra slash
          which is not needed - is quite common. This is therefor a
          special case treatment of the 'randomText' behaviour.
        */
        const static std::string PARSE_RANDOM_SLASH;


        /*
          For some keywords the number of records (i.e. size) is given
          as an item in another keyword. A typical example is the
          EQUIL keyword where the number of records is given by the
          NTEQUL item of the EQLDIMS keyword. If the size defining
          XXXDIMS keyword is not in the deck, we can use the default
          values of the XXXDIMS keyword; this is regulated by the
          'missingDIMskeyword' field.

          Observe that a fully defaulted XXXDIMS keyword does not
          trigger this behavior.
        */
        const static std::string PARSE_MISSING_DIMS_KEYWORD;

        /*
          If the number of elements in the input record exceeds the
          number of items in the keyword configuration this error
          situation will be triggered. Many keywords end with several
          ECLIPSE300 only items - in some cases we have omitted those
          items in the Json configuration; that will typically trigger
          this error situation when encountering an ECLIPSE300 deck.
        */
        const static std::string PARSE_EXTRA_DATA;

        /*
          If an include file is not found we can configure the parser
          to contine reading; of course the resulting deck can
          obviously be quite broken.
        */
        const static std::string PARSE_MISSING_INCLUDE;

        /*
          For certain keywords, other, specific keywords are either
          required or prohibited. When such keywords are found in an
          invalid combination (missing required or present prohibited
          keyword), this error situation occurs.
         */
        const static std::string PARSE_INVALID_KEYWORD_COMBINATION;

        /// Dynamic number of wells exceeds maximum declared in
        /// RUNSPEC keyword WELLDIMS (item 1).
        const static std::string RUNSPEC_NUMWELLS_TOO_LARGE;

        /// Dynamic number of connections per well exceeds maximum
        /// declared in RUNSPEC keyword WELLDIMS (item 2).
        const static std::string RUNSPEC_CONNS_PER_WELL_TOO_LARGE;

        /// Dynamic number of groups exceeds maximum number declared in
        /// RUNSPEC keyword WELLDIMS (item 3).
        const static std::string RUNSPEC_NUMGROUPS_TOO_LARGE;

        /// Dynamic group size exceeds maximum number declared in
        /// RUNSPEC keyword WELLDIMS (item 4).
        const static std::string RUNSPEC_GROUPSIZE_TOO_LARGE;

        /*
          Should we allow keywords of length more than eight characters? If the
          keyword is too long it will be internalized using only the eight first
          characters.
        */
        const static std::string PARSE_LONG_KEYWORD;

        /*
          The unit system specified via the FILEUNIT keyword is different from the unit
          system used by the deck.
        */
        const static std::string UNIT_SYSTEM_MISMATCH;


        /*
          If the third item in the THPRES keyword is defaulted the
          threshold pressure is inferred from the initial pressure;
          this currently not supported.
        */
        const static std::string UNSUPPORTED_INITIAL_THPRES;

        /*
          If the second item in the WHISTCTL keyword is set to YES
          The simulator is supposed to terminate if the well is
          changed to BHP control. This feature is not yet supported.
        */
        const static std::string UNSUPPORTED_TERMINATE_IF_BHP;

        const static std::string UDQ_PARSE_ERROR;
        const static std::string UDQ_TYPE_ERROR;

        /*
          If the third item in the THPRES keyword is defaulted the
          threshold pressure is inferred from the initial pressure -
          if you still ask the ThresholdPressure instance for a
          pressure value this error will be signalled.  this currently
          not supported.
        */
        const static std::string INTERNAL_ERROR_UNINITIALIZED_THPRES;

        /*
         If the deck is partial deck, and thus a full EclipseState is
         meaningless, we can still construct a slim EclipseGrid.
         */
        const static std::string PARSE_MISSING_SECTIONS;

        /*
          When defining wells and groups with the WELSPECS and GRUPTREE keywords
          we do not allow leading or trailing spaces. The code in Schedule.cpp
          will *unconditionally* remove the spaces, but with PARSE_WGNAME_SPACE
          setting you can additionally configure the normal IGNORE|WARN|ERROR
          behavior.
        */
        const static std::string PARSE_WGNAME_SPACE;

        /*
          If you have configured a specific well in the summary section,
          which is not recognized - how to handle.
        */
        const static std::string SUMMARY_UNKNOWN_WELL;
        const static std::string SUMMARY_UNKNOWN_GROUP;
        const static std::string SUMMARY_UNKNOWN_NODE;
        const static std::string SUMMARY_UNKNOWN_AQUIFER;
        const static std::string SUMMARY_UNHANDLED_KEYWORD;
        const static std::string SUMMARY_UNDEFINED_UDQ;
        const static std::string SUMMARY_UDQ_MISSING_UNIT;
        const static std::string SUMMARY_INVALID_FIPNUM;
        const static std::string SUMMARY_EMPTY_REGION;
        const static std::string SUMMARY_REGION_TOO_LARGE;
        /*
          A well must be specified (e.g. WELSPECS) and have completions
          (e.g. COMPDAT) to be able to set control mode (e.g. WCONPROD).
          A well missing specification and/or completion(s) will throw.
        */
        const static std::string SCHEDULE_INVALID_NAME;


        /*
          Only keywords explicitly white-listed can be included in the ACTIONX
          block. This error flag controls what should happen when an illegal
          keyword is encountered in an ACTIONX block.
         */
        const static std::string ACTIONX_ILLEGAL_KEYWORD;


        /*
          The RPTSCH, RPTSOL and RPTSCHED keywords have two alternative forms,
          in the old style all the items are set as integers, i.e. the RPTRST
          keyword can be configured as:

            RPTRST
               0 0 0 1 0 1 0 2 0 0 0 0 0 1 0 0 2/

          The new way is based on string mneomnics which can optionally have an
          integer value, i.e something like:

            RPTRST
              BASIC=2  FLOWS  ALLPROS /

          It is strictly illegal to mix the two ways to configure keywords. A
          situation with mixed input style is identified if any of the items are
          integers. To avoid that the values in the assignments like BASIC=2 are
          interpreted as integers it is essential that there are no spaces
          around the '=', and that is also documented in the manual. However -
          it turns out that Eclipse actually handles e.g.

             RPTRST
                BASIC = 2 /

          So we have introduced a error mode RPT_MIXED_STYLE which tries to
          handle this situation. Observe that really mixed input style is
          impossible to handle, and will lead to a hard exception, but with the
          RPT_MIXED_STYLE error mode it is possible to configure lenient
          behavior towards interpreting the input as new style string mneomnics.
        */
        const static std::string RPT_MIXED_STYLE;

        const static std::string RPT_UNKNOWN_MNEMONIC;

        const static std::string SCHEDULE_GROUP_ERROR;
        const static std::string SCHEDULE_IGNORED_GUIDE_RATE;

        const static std::string SCHEDULE_COMPSEGS_INVALID;
        const static std::string SCHEDULE_COMPSEGS_NOT_SUPPORTED;

        /*
          The SIMULATOR_KEYWORD_ errormodes are for the situation where the
          parser recognizes, and correctly parses a keyword, but we know that
          the simulator does not support the intended use of the keyword. These
          errormodes are invoked from the simulator.
        */
        const static std::string SIMULATOR_KEYWORD_NOT_SUPPORTED;
        const static std::string SIMULATOR_KEYWORD_NOT_SUPPORTED_CRITICAL;
        const static std::string SIMULATOR_KEYWORD_ITEM_NOT_SUPPORTED;
        const static std::string SIMULATOR_KEYWORD_ITEM_NOT_SUPPORTED_CRITICAL;

    private:
        void initDefault();
        void initEnv();
        void envUpdate( const std::string& envVariable , InputError::Action action );
        void patternUpdate( const std::string& pattern , InputError::Action action);

        std::map<std::string , InputError::Action> m_errorContexts;
        std::set<std::string> ignore_keywords;
    };
}


#endif
