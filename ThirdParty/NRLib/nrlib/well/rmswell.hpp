// $Id: rmswell.hpp 1068 2012-09-18 11:21:53Z perroe $

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

#ifndef NRLIB_RMSWELL_HPP
#define NRLIB_RMSWELL_HPP

#include <vector>
#include <sstream>
#include <map>

#include "../exception/exception.hpp"
#include "well.hpp"


namespace NRLib {
  class RMSWell : public Well
  {
  public:
    /// Constructor
    /// \param[in] filename  Name of well file
    RMSWell(const std::string& filename);
    /// Construct RMSWell from general well.
    RMSWell(const Well& wellobj);
    /// Get the discnames for discrete log with index
    const std::map<int, std::string> GetDiscNames(const std::string& log_name) const;
    /// Write well to file
    void WriteToFile(const std::string& filename);

  //protected:
    //void SetNumberOfData(int n_data)  { Well::SetNumberOfData(n_data)  ;}

  private:
    /// Names for discrete logs
    std::map<std::string, std::map<int, std::string> > discnames_;
    double xpos0_, ypos0_;
    /// First line of RMSwell file
    std::string line1_;
    /// Second line of RMSwell file
    std::string line2_;
    /// Name of logs
    std::vector<std::string> lognames_;
    /// Vector telling which logs are discrete
    std::vector<bool> isDiscrete_;
    /// Units for continuous logs
    std::vector<std::string> unit_;
    /// Scale for continuous logs
    std::vector<std::string> scale_;

  };


}
#endif
