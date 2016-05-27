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

#include <string>
#include <map>
#include <vector>

#include <opm/parser/eclipse/Parser/InputErrorAction.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

namespace Opm {


    /*
       The ParseContext class is meant to control the behavior of the
       parsing and EclipseState construction phase when
       errors/inconsistencies/... are encountered in the input.

       For each of the possible problems encountered the possible
       actions are goverened by the InputError::Action enum:

          InputError::THROW_EXCEPTION
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

    class ParseContext {
    public:
        ParseContext();
        ParseContext(const std::vector<std::pair<std::string , InputError::Action>> initial);
        Message::type handleError( const std::string& errorKey, const std::string& msg ) const;
        bool hasKey(const std::string& key) const;
        ParseContext  withKey(const std::string& key, InputError::Action action = InputError::WARN) const;
        ParseContext& withKey(const std::string& key, InputError::Action action = InputError::WARN);
        void updateKey(const std::string& key , InputError::Action action);
        void update(InputError::Action action);
        void update(const std::string& keyString , InputError::Action action);
        InputError::Action get(const std::string& key) const;
        std::map<std::string,InputError::Action>::const_iterator begin() const;
        std::map<std::string,InputError::Action>::const_iterator end() const;
        /*
          When the key is added it is inserted in 'strict mode',
          i.e. with the value 'InputError::THROW_EXCEPTION. If you
          want a different value you must subsequently call the update
          method.
        */
        void addKey(const std::string& key);
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
          Some property modfiers can be modified in the Schedule
          section; this effectively means that Eclipse supports time
          dependent geology. This is marked as an exocit special
          feature in Eclipse, and not supported at all in the
          EclipseState object of opm-parser. If these modifiers are
          encountered in the Schedule section the behavior is
          regulated by this setting.
        */
        const static std::string UNSUPPORTED_SCHEDULE_GEO_MODIFIER;

        /*
          In the COMPORD implementation only the 'TRACK' input mode is supported.
        */
        const static std::string UNSUPPORTED_COMPORD_TYPE;

        /*
          If the third item in the THPRES keyword is defaulted the
          threshold pressure is inferred from the initial pressure;
          this currently not supported.
        */
        const static std::string UNSUPPORTED_INITIAL_THPRES;


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

    private:
        void initDefault();
        void initEnv();
        void envUpdate( const std::string& envVariable , InputError::Action action );
        void patternUpdate( const std::string& pattern , InputError::Action action);
        std::map<std::string , InputError::Action> m_errorContexts;
}; }


#endif
