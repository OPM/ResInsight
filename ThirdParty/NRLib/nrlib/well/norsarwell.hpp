// $Id: norsarwell.hpp 883 2011-09-26 09:17:05Z perroe $

// Copyright (c)  2011, Norwegian Computing Center
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// •  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// •  Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef NRLIB_NORSARWELL_HPP
#define NRLIB_NORSARWELL_HPP

#include <vector>
#include <sstream>
#include <map>

#include "../exception/exception.hpp"
#include "well.hpp"


namespace NRLib {
  class NorsarWell : public Well
  {
  public:
    /// Constructor
    /// \param[in] filename  Name of well file
    NorsarWell(const std::string & filename);

    ///NBNB add when convenient
    /*
    /// Copy constructor
    NorsarWell(Well *wellobj);
    /// Write well to file
    void WriteWell(const std::string& filename);
    */

    double GetXPosOrigin() {return(xpos0_);}
    double GetYPosOrigin() {return(ypos0_);}

    std::string GetLogUnit(const std::string & name);

  private:
    /// Version (currently only reading 1000)
    std::string version_;
    /// Format (currently only reading ascii)
    std::string format_;
    /// Minimum measured depth
    double md_min_;
    /// Maximum measured depth
    double md_max_;
    /// Minimum measured depth step
    double md_min_step_;
    /// Maximum measured depth step
    double md_max_step_;
    /// Original position
    double xpos0_, ypos0_;
    /// Kelly bushing elevation
    double ekb_;
    /// Units for the above, in order
    std::vector<std::string> header_units_;

    /// Names and units of logs
    std::map<std::string,std::string> units_;

    std::vector<std::vector<double> > ReadLogs(const std::string & filename, int n_col, int n_row, int skip_lines);
    std::vector<std::vector<double> > MergeLogs(const std::vector<std::vector<double> > & track_logs, int track_MD,
                                                const std::vector<std::vector<double> > & log_logs, int log_MD);

  };


}
#endif
