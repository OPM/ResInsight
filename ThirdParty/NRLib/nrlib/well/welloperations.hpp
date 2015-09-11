// $Id: welloperations.hpp 883 2011-09-26 09:17:05Z perroe $

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

#ifndef NRLIB_WELLOPERATIONS_HPP
#define NRLIB_WELLOPERATIONS_HPP

#include <string>
#include <iomanip>
#include <vector>
#include <sstream>

#include "../exception/exception.hpp"
#include "well.hpp"
#include "rmswell.hpp"
#include "../stormgrid/stormcontgrid.hpp"
#include "../stormgrid/stormfaciesgrid.hpp"

namespace NRLib {
  /// Block the well into the grid.
  /// Possible grids to use are StormContGrid and StormFaciesGrid
  template <typename A>
  void BlockWell(const NRLib::Well & well,
                 const A           & grid,
                 NRLib::Well       & blocked_well);
}


template <typename A>
void NRLib::BlockWell(const NRLib::Well & well,
                      const A           & grid,
                      NRLib::Well       & blocked_well)
{
  size_t i;
  size_t loglength = well.GetContLogLength("x");
  std::vector<size_t> index(loglength);
  double x, y, z;

  for (i = 0; i < loglength; i++) {
    x = well.GetContValue(i, "x");
    y = well.GetContValue(i, "y");
    z = well.GetContValue(i, "z");
    index[i] = grid.FindIndex(x, y, z);
  }

  std::vector<size_t> indexvalues;
  indexvalues.push_back(index[0]);
  bool found = false;
  size_t k;
  for (i = 0; i < loglength; i++) {
    for (k = 0; k < indexvalues.size(); k++)
      if (index[i] == indexvalues[k])
        found = true;
    if (found == false)
      indexvalues.push_back(index[i]);
    found = false;
  }

  size_t novalues = indexvalues.size();
  std::vector<int> no(novalues);

  for (i = 0; i < novalues; i++) {
    no[i] = 0;
    for (k = 0; k < loglength; k++) {
      if (index[k] == indexvalues[i])
        no[i]++;
    }
  }
  size_t ncont = well.GetNContLog();
  size_t ndisc = well.GetNlog() - ncont;
  std::vector<std::vector<int> > disclogs(ndisc, std::vector<int>(novalues));
  std::vector<std::vector<int> > nodisc(ndisc, std::vector<int>(novalues));
  std::vector<std::vector<int> > nocont(ncont, std::vector<int>(novalues));
  std::vector<std::vector<double> > contlogs(ncont, std::vector<double>(novalues));
  for (k = 0; k < novalues; k++) {
    for (i = 0; i < ncont; i++) {
      contlogs[i][k] = 0.0;
      nocont[i][k] = 0;
    }
    for (i = 0; i < ndisc; i++) {
      disclogs[i][k] = 0;
      nodisc[i][k] = 0;
    }
  }
  std::vector<std::string> discnames(ndisc);
  std::vector<std::string> contnames(ncont);
  typedef std::map<std::string, std::vector<int> >::const_iterator CI;
  const std::map<std::string,std::vector<int> >& disclog = well.GetDiscLog();
  i = 0;
  for (CI p = disclog.begin(); p != disclog.end(); ++p) {
    discnames[i] = p->first;
    i++;
  }
  i = 0;
  typedef std::map<std::string, std::vector<double> >::const_iterator CID;
  const std::map<std::string,std::vector<double> >& contlog = well.GetContLog();
  for (CID p = contlog.begin(); p != contlog.end(); ++p) {
    contnames[i] = p->first;
    i++;
  }
  size_t j;
  double valuec;
  int valuei;
  for (i = 0; i < loglength; i++) {
    for (k = 0; k < novalues; k++) {
      if (index[i] == indexvalues[k]) {
        for (j = 0; j < ncont; j++) {
          valuec = well.GetContValue(i, contnames[j]);
          if (!well.IsMissing(valuec)) {
            contlogs[j][k] += valuec;
            nocont[j][k]++;
          }
        }
        for (j = 0; j < ndisc; j++) {
          valuei = well.GetDiscValue(i, discnames[j]);
          if (!well.IsMissing(valuei)) {
            disclogs[j][k] += valuei;
            nodisc[j][k]++;
         }
        }
      }
    }
  }
  for (k = 0; k < novalues; k++) {
    for (j = 0; j < ncont; j++) {
      if (nocont[j][k] == 0)
        contlogs[j][k] = well.GetContMissing();
      else
        contlogs[j][k] /= nocont[j][k];
    }
    for (j = 0; j < ndisc; j++) {
      if (nodisc[j][k] == 0)
        disclogs[j][k] = well.GetIntMissing();
      else
        disclogs[j][k] /= nodisc[j][k];
    }
  }

  std::map<std::string,std::vector<double> > contlognew;
  for (i = 0; i < ncont; i++) {
    contlognew[contnames[i]] = contlogs[i];
  }
  std::map<std::string, std::vector<int> > disclognew;
  for (i = 0; i < ndisc; i++) {
    disclognew[discnames[i]] = disclogs[i];
  }
  std::string well_name = well.GetWellName() + "blocked";
  blocked_well = Well(contlognew, disclognew, well_name);
}

#endif
