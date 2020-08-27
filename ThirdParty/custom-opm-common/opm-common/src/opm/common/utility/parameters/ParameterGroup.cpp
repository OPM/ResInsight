//===========================================================================
//
// File: ParameterGroup.cpp
//
// Created: Tue Jun  2 19:13:17 2009
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

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <opm/common/utility/parameters/ParameterGroup.hpp>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include <opm/common/utility/parameters/Parameter.hpp>
#include <opm/common/utility/parameters/ParameterStrings.hpp>
#include <opm/common/utility/parameters/ParameterTools.hpp>

namespace Opm {
	ParameterGroup::ParameterGroup()
	: path_(ID_path_root), parent_(0), output_is_enabled_(true)
	{
	}

	ParameterGroup::~ParameterGroup() {
	    if (output_is_enabled_) {
		//TermColors::Normal();
	    }
	}

	std::string ParameterGroup::getTag() const {
	    return ID_xmltag__param_grp;
	}


	bool ParameterGroup::has(const std::string& name) const
        {
	    std::pair<std::string, std::string> name_path = splitParam(name);
	    map_type::const_iterator it = map_.find(name_path.first);
	    if (it == map_.end()) {
		return false;
	    } else if (name_path.second == "") {
		return true;
	    } else {
		ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(*it).second);
		return pg.has(name_path.second);
	    }
	}

	std::string ParameterGroup::path() const {
	    return path_;
	}

	ParameterGroup::ParameterGroup(const std::string& patharg,
	                               const ParameterGroup* parent,
                                       const bool enable_output)
	: path_(patharg), parent_(parent), output_is_enabled_(enable_output)
	{
	}

	ParameterGroup
	ParameterGroup::getGroup(const std::string& name) const
        {
	    return get<ParameterGroup>(name);
	}

	namespace {
	    inline std::istream&
	    samcode_readline(std::istream& is, std::string& parameter) {
		return std::getline(is, parameter);
	    }
	} // anonymous namespace

	void ParameterGroup::readParam(const std::string& param_filename) {
	    std::ifstream is(param_filename.c_str());
	    if (!is) {
		std::cerr << "Error: Failed to open parameter file '" << param_filename << "'.\n";
		throw std::exception();
	    }
	    std::string parameter;
	    int lineno = 0;
	    while (samcode_readline(is, parameter)) {
		++lineno;
                int commentpos = parameter.find(ID_comment);
                if (commentpos != 0) {
                    if (commentpos != int(std::string::npos)) {
                        parameter = parameter.substr(0, commentpos);
                    }
                    int fpos = parameter.find(ID_delimiter_assignment);
                    if (fpos == int(std::string::npos)) {
                        std::cerr << "WARNING: No '" << ID_delimiter_assignment << "' found on line " << lineno << ".\n";
                    }
                    int pos = fpos + ID_delimiter_assignment.size();
                    int spos = parameter.find(ID_delimiter_assignment, pos);
                    if (spos == int(std::string::npos)) {
                        std::string name = parameter.substr(0, fpos);
                        std::string value = parameter.substr(pos, spos);
                        this->insertParameter(name, value);
                    } else {
                        std::cerr << "WARNING: To many '" << ID_delimiter_assignment << "' found on line " << lineno << ".\n";
                    }
                }
	    }
	}

	void ParameterGroup::writeParam(const std::string& param_filename) const {
	    std::ofstream os(param_filename.c_str());
	    if (!os) {
		std::cerr << "Error: Failed to open parameter file '"
                          << param_filename << "'.\n";
		throw std::exception();
	    }
	    this->writeParamToStream(os);
	}

	void ParameterGroup::writeParamToStream(std::ostream& os) const
        {
	    if (map_.empty()) {
		os << this->path() << "/" << "dummy"
		   << ID_delimiter_assignment << "0\n";
	    }
	    for (map_type::const_iterator it = map_.begin(); it != map_.end(); ++it) {
		if ( (*it).second->getTag() == ID_xmltag__param_grp ) {
		    ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(*it).second);
		    pg.writeParamToStream(os);
		} else if ( (*it).second->getTag() == ID_xmltag__param) {
		    os << this->path() << "/" << (*it).first << ID_delimiter_assignment
		       << dynamic_cast<Parameter&>(*(*it).second).getValue() << "\n";
		} else {
		    os << this->path() << "/" << (*it).first << ID_delimiter_assignment
		       << "<" << (*it).second->getTag() << ">" << "\n";
		}
	    }
	}

	void ParameterGroup::insert(const std::string& name,
				    const std::shared_ptr<ParameterMapItem>& data)
        {
	    std::pair<std::string, std::string> name_path = splitParam(name);
	    map_type::const_iterator it = map_.find(name_path.first);
	    assert(name_path.second == "");
	    if (it == map_.end()) {
		map_[name] = data;
	    } else {
		if ( (map_[name]->getTag() == data->getTag())  &&
                     (data->getTag() == ID_xmltag__param_grp) ) {
		    ParameterGroup& alpha = dynamic_cast<ParameterGroup&>(*(*it).second);
		    ParameterGroup& beta  = dynamic_cast<ParameterGroup&>(*data);
		    for (map_type::const_iterator
                             item = beta.map_.begin(); item != beta.map_.end(); ++item) {
			alpha.insert((*item).first, (*item).second);
		    }
		} else {
		    std::cout << "WARNING : The '"
			      << map_[name]->getTag()
			      << "' element '"
			      << name
			      << "' already exist in group '"
			      << this->path()
			      << "'. The element will be replaced by a '"
			      << data->getTag()
			      << "' element.\n";
		    map_[name] = data;
		}
	    }
	}

	void ParameterGroup::insertParameter(const std::string& name,
                                             const std::string& value)
        {
	    std::pair<std::string, std::string> name_path = splitParam(name);
	    while (name_path.first == "") {
		name_path = splitParam(name_path.second);
	    }
	    map_type::const_iterator it = map_.find(name_path.first);
	    if (it == map_.end()) {
		if (name_path.second == "") {
		    std::shared_ptr<ParameterMapItem> data(new Parameter(value, ID_param_type__cmdline));
		    map_[name_path.first] = data;
		} else {
		    std::shared_ptr<ParameterMapItem> data(new ParameterGroup(this->path() + ID_delimiter_path + name_path.first,
                                                                              this,
                                                                              output_is_enabled_));
		    ParameterGroup& child = dynamic_cast<ParameterGroup&>(*data);
		    child.insertParameter(name_path.second, value);
		    map_[name_path.first] = data;
		}
	    } else if (name_path.second == "") {
		if ((*it).second->getTag() != ID_xmltag__param) {
		    std::cout << "WARNING : The "
			      << map_[name_path.first]->getTag()
			      << " element '"
			      << name
			      << "' already exist in group '"
			      << this->path()
			      << "'. It will be replaced by a "
			      << ID_xmltag__param
			      << " element.\n";
		}
		std::shared_ptr<ParameterMapItem> data(new Parameter(value, ID_param_type__cmdline));
		map_[name_path.first] = data;
	    } else {
		ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(*it).second);
		pg.insertParameter(name_path.second, value);
	    }
	}

#if 0
	void ParameterGroup::display() const {
	    std::cout << "Begin: " << this->path() << "\n";
	    for (map_type::const_iterator it = map_.begin(); it != map_.end(); ++it) {
		if ( (*it).second->getTag() == ID_xmltag__param_grp ) {
		    ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(*it).second);
		    pg.display();
		} else if ( (*it).second->getTag() == ID_xmltag__param) {
		    std::cout << (*it).first
			      << " ("
			      << (*it).second->getTag()
			      << ") has value "
			      << dynamic_cast<Parameter&>(*(*it).second).getValue()
			      << "\n";
		} else {
		    std::cout << (*it).first
			      << " ("
			      << (*it).second->getTag()
			      << ")\n";
		}
	    }
	    std::cout << "End: " << this->path() << "\n";
	}
#endif

        bool ParameterGroup::anyUnused() const
        {
            if (!this->used()) {
                return true;
            }
	    for (map_type::const_iterator it = map_.begin(); it != map_.end(); ++it) {
		if (it->second->getTag() == ID_xmltag__param_grp) {
		    ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(it->second));
		    if (pg.anyUnused()) {
                        return true;
                    }
		} else if (it->second->getTag() == ID_xmltag__param) {
		    if (!it->second->used()) {
			return true;
		    }
		}
	    }
            return false;
        }

	void ParameterGroup::displayUsage(bool used_params) const
	{
	    if (this->used() == used_params) {
		std::cout << this->path() << '\n';
	    }
	    for (map_type::const_iterator it = map_.begin(); it != map_.end(); ++it) {
		if (it->second->getTag() == ID_xmltag__param_grp) {
		    ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(it->second));
		    pg.displayUsage(used_params);
		} else if (it->second->getTag() == ID_xmltag__param) {
		    if (it->second->used() == used_params) {
			std::cout << path() << '/' << it->first << '\n';
		    }
		}
	    }
	    std::cout << std::flush;
	}

	void ParameterGroup::disableOutput() {
	    this->recursiveSetIsOutputEnabled(false);
	}

	void ParameterGroup::enableOutput() {
	    this->recursiveSetIsOutputEnabled(true);
	}

	bool ParameterGroup::isOutputEnabled() const {
	    return output_is_enabled_;
	}

	void ParameterGroup::recursiveSetIsOutputEnabled(bool output_is_enabled)
        {
	    output_is_enabled_ = output_is_enabled;
	    for (map_type::const_iterator it = map_.begin(); it != map_.end(); ++it) {
		if (it->second->getTag() == ID_xmltag__param_grp) {
		    ParameterGroup& pg = dynamic_cast<ParameterGroup&>(*(it->second));
		    pg.recursiveSetIsOutputEnabled(output_is_enabled);
		}
	    }
	}

        const std::vector<std::string>& ParameterGroup::unhandledArguments() const
        {
            return unhandled_arguments_;
        }

} // namespace Opm
