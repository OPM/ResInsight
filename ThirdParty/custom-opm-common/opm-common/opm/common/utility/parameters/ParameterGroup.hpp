//===========================================================================
//
// File: ParameterGroup.hpp
//
// Created: Tue Jun  2 19:11:11 2009
//
// Author(s): Bård Skaflestad     <bard.skaflestad@sintef.no>
//            Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

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

#ifndef OPM_PARAMETERGROUP_HEADER
#define OPM_PARAMETERGROUP_HEADER

#include <exception>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <opm/common/utility/parameters/ParameterMapItem.hpp>
#include <opm/common/utility/parameters/ParameterRequirement.hpp>

namespace Opm {
	/// ParameterGroup is a class that is used to provide run-time parameters.
	/// The standard use of the class is to call create it with the
	/// (int argc, char** argv) constructor (where the arguments are those
	/// given by main). This parses the command line, where each token
	/// either
	/// A) specifies a parameter (by a "param=value" token).
	/// B) specifies a param file to be read (by a "filename.param" token).
	/// After the tokens are parsed they are stored in a tree structure
	/// in the ParameterGroup object; it is worth mentioning that parameters
	/// are inherited in this tree structure. Thus, if ``grid\_prefix'' is
	/// given a value in the root node, this value will be visible in all
	/// parts of the tree (unless the parameter is overwritten in a subtree).
	/// Applications using this ParameterGroup objects will usually write out
	/// a message for each node in the three that is used by the application;
	/// this is one way to determine valid parameters.
	///
	/// Parameters specified as "param=value" on the command line
	///
	/// To specify a parameter on the command line, you must know where in the tree the
	/// parameter resides. The syntax for specifying parameters on the command line given
	/// an application called ``simulator'' is
	///         simulator param1=value grp/param2=value
	//  for parameter ``param1'' lying at the root and ``param2'' in the group
	/// ``grp''. If the same parameters are specified multiple times on the command
	/// line, only the last will be used. Thus an application named ``simulator'' run with
	/// the following command
	///         simulator param=value1 param=value2
	/// will use value2 as the value of ``param''.
	///
	/// param files
	///
	/// A param file consists of multiple lienes, where each line consists of "param=value".
	/// This syntax is identical to the one for parameters specified on the command line.
	class ParameterGroup : public ParameterMapItem {
	public:
	    struct NotFoundException          : public std::exception {};
	    struct WrongTypeException         : public std::exception {};
	    struct ConversionFailedException  : public std::exception {};

	    template<typename T>
	    struct RequirementFailedException : public std::exception {};

	    ParameterGroup();
	    ParameterGroup(const std::string& path, const ParameterGroup* parent,
                           const bool enable_output);

	    // From ParameterMapItem
	    virtual ~ParameterGroup();
	    virtual std::string getTag() const;

	    /// \brief A constructor typically used to initialize a
	    ///        ParameterGroup from command-line arguments.
	    ///
	    /// It is required that argv[0] is the program name, while if
	    /// 0 < i < argc, then argv[i] is either
	    /// the name of a parameter file or parametername=value.
	    ///
	    /// \param argc is the number of command-line arguments,
	    ///        including the name of the executable.
	    /// \param argv is an array of char*, each of which is a
	    ///        command line argument.
	    /// \param verify_syntax  If true (default), then it is an error to
	    ///        pass arguments that cannot be handled by the ParameterGroup,
	    ///        or an empty argument list. If false, such arguments are stored
	    ///        and can be retrieved later with unhandledArguments().
            /// \param enable_output Whether to enable output or not.
            template <typename StringArray>
	    ParameterGroup(int argc, StringArray argv, const bool verify_syntax = true,
                           const bool enabled_output=true);

	    /// \brief This method checks if there is something with name
	    ///        \p name in the parameter gropup.
	    ///
	    /// \param name is the name of the parameter.
	    /// \return true if \p name is the name of something in the parameter
	    ///         group, false otherwise.
	    bool has(const std::string& name) const;

	    /// \brief This method is used to read a parameter from the
	    ///        parameter group.
	    ///
	    /// NOTE:  If the reading of the parameter fails, then this method
	    ///        throws an appropriate exception.
	    ///
	    /// \param name is the name of the parameter in question.
	    /// \return The value associated with then name in this parameter
	    ///         group.
	    template<typename T>
	    T get(const std::string& name) const;

	    template<typename T, class Requirement>
	    T get(const std::string& name, const Requirement&) const;

	    /// \brief This method is used to read a parameter from the
	    ///        parameter group.
	    ///
	    /// NOTE:  If the reading of the parameter fails, then either
	    ///        a) this method returns \p default_value if there
	    ///           was no parameter with name \p name in
	    ///           the parameter group
	    ///        or
	    ///        b) this method throws an appropriate exception.
	    ///
	    /// \param name is the name of the parameter in question.
	    /// \param default_value the default value of the parameter in
	    ///        question.
	    /// \return The value associated with this name in this parameter
	    ///         group.
	    template<typename T>
	    T getDefault(const std::string& name,
			 const T& default_value) const;

	    template<typename T, class Requirement>
	    T getDefault(const std::string& name,
			 const T& default_value,
			 const Requirement& r) const;

	    /// \brief This method returns the parameter group given by name,
	    ///        i.e. it is an alias of get<ParameterGroup>().
	    ///
	    /// \param name is the name of the parameter group sought.
	    /// \return the parameter group sought.
	    ParameterGroup getGroup(const std::string& name) const;

	    /// \brief Disables the output from get, getDefault and getGroup.
	    ///        By default, such output is enabled.
	    void disableOutput();

	    /// \brief Enables the output from get, getDefault and getGroup.
	    ///        By default, such output is enabled.
	    void enableOutput();

	    /// \brief Returs true if and only if output from get, getDefault
	    ///        and getGroup is enabled.
	    ///
	    /// \return true if and only if output from get, getDefault and
	    ///         getGroup is enabled.
	    bool isOutputEnabled() const;

	    /// \brief Reads the contents of the param file specified by
	    ///        param_filename into this ParameterGroup.
	    ///
	    /// NOTE: A param file contains lines on the form 'a/b/c=d'.
	    //        The '/' separates ParameterGroups and Parameters
	    ///       (e.g. c is a Parameter in the ParameterGroup b,
	    ///       and b is a ParameterGroup in the ParameterGroup a)
	    ///       while '=' separates the name from the value (e.g. the
	    ///       value of the parameter c above will be d).
	    /// NOTE: A param file does not store any type information about
	    ///       its values.
	    ///
	    /// \param param_filename is the name of a param file.
	    void readParam(const std::string& param_filename);

	    /// \brief Writes this ParameterGroup into a param file
	    ///        specified by param_filename.
	    ///
	    /// \param param_filename is the name of a param file.
	    void writeParam(const std::string& param_filename) const;

	    /// \brief Writes this ParameterGroup to a stream.
	    ///
	    /// \param stream is the stream to write to.
	    void writeParamToStream(std::ostream& stream) const;


	    /// vki param interface - deprecated
	    template<typename T>
	    void get(const char* name, T& value, const T& default_value) const {
		value = this->getDefault<T>(name, default_value);
	    }

	    /// vki param interface - deprecated
	    template<typename T>
	    void get(const char* name, T& value) const {
		value = this->get<T>(name);
	    }

            /// \brief Return true if any parameters are unused.
            bool anyUnused() const;

	    /// \brief Shows which parameters which are used or unused.
	    void displayUsage(bool used_params = false) const;

            /// \brief Returns the path of the parameter group.
	    std::string path() const;

            /// Insert a new item into the group.
	    void insert(const std::string& name,
                        const std::shared_ptr<ParameterMapItem>& data);

            /// Insert a new parameter item into the group.
	    void insertParameter(const std::string& name, const std::string& value);

            /// Unhandled arguments from command line parsing.
            const std::vector<std::string>& unhandledArguments() const;

	private:
	    typedef std::shared_ptr<ParameterMapItem> data_type;
	    typedef std::pair<std::string, data_type> pair_type;
	    typedef std::map<std::string, data_type> map_type;

	    std::string path_;
	    map_type map_;
	    const ParameterGroup* parent_;
	    bool output_is_enabled_;
	    std::vector<std::string> unhandled_arguments_;

	    template<typename T, class Requirement>
	    T translate(const pair_type& data, const Requirement& chk) const;
            template <typename StringArray>
	    void parseCommandLineArguments(int argc, StringArray argv, bool verify_syntax);
	    void recursiveSetIsOutputEnabled(bool output_is_enabled);

	    // helper routines to do textual I/O
	    template <typename T>
	    static std::string to_string(const T& val);
	    static std::pair<std::string, std::string>
	    filename_split(const std::string& filename);
	};
} // namespace Opm

#include <opm/common/utility/parameters/ParameterGroup_impl.hpp>

#endif // OPM_PARAMETERGROUP_HEADER
