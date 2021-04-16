/*
  MonotCubicInterpolator
  Copyright (C) 2006 Statoil ASA

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
   @file MonotCubicInterpolator.C
   @brief Represents one dimensional function f with single valued argument x

   Class to represent a one-dimensional function with single-valued
   argument.  Cubic interpolation, preserving the monotonicity of the
   discrete known function values

   @see MonotCubicInterpolator.h for further documentation.

*/


#include "config.h"
#include <opm/common/utility/numeric/MonotCubicInterpolator.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

/*

  SOME DISCUSSION ON DATA STORAGE:

  Internal data structure of points and values:

  vector(s):
   - Easier coding
   - Faster vector operations when setting up derivatives.
   - sorting slightly more complex.
   - insertion of further values bad.

  vector<double,double>
   - easy sorting
   - code complexity almost as for map.
   - insertion of additional values bad

  vector over struct datapoint { x, f, d}
   - nice code
   - not as sortable, insertion is cumbersome.

   ** This is used currently: **
  map<double, double> one for (x,f) and one for (x,d)
   - Naturally sorted on x-values (done by the map-construction)
   - Slower to set up, awkward loop coding (?)
   - easy to add more points.
   - easier to just add code to linear interpolation code
   - x-data is duplicated, but that memory waste is
     unlikely to represent a serious issue.

  map<double, <double, double> >
   - naturally couples x-value, f-value and d-value
   - even more unreadable(??) code?
   - higher skills needed by programmer.


  MONOTONE CUBIC INTERPOLATION:

  It is a local algorithm. Adding one point only incur recomputation
  of values in a neighbourhood of the point (in the interval getting
  divided).

  NOTE: We do not currently make use of this local fact. Upon
  insertion of a new data pair, everything is recomputed. Revisit
  this when needed.

*/


namespace Opm
{


MonotCubicInterpolator::
MonotCubicInterpolator(const vector<double> & x, const vector<double> & f) {
  if (x.size() != f.size()) {
    throw("Unable to constuct MonotCubicInterpolator from vectors.") ;
  }

  // Add the contents of the input vectors to our map of values.
  vector<double>::const_iterator posx, posf;
  for (posx = x.begin(), posf = f.begin(); posx != x.end(); ++posx, ++posf) {
    data[*posx] = *posf ;
  }

  computeInternalFunctionData();
}



bool
MonotCubicInterpolator::
read(const std::string & datafilename, int xColumn, int fColumn)
{
  data.clear() ;
  ddata.clear() ;

  ifstream datafile_fs(datafilename.c_str());
  if (!datafile_fs) {
    return false ;
  }

  string linestring;
  while (!datafile_fs.eof()) {
    getline(datafile_fs, linestring);

    // Replace commas with space:
    string::size_type pos = 0;
    while ( (pos = linestring.find(",", pos)) != string::npos ) {
        // cout << "Found comma at position " << pos << endl;
        linestring.replace(pos, 1, " ");
        pos++;
    }

    stringstream strs(linestring);
    int columnindex = 0;
    std::vector<double> value;
    if (linestring.size() > 0 && linestring.at(0) != '#') {
        while (!(strs.rdstate() & std::ios::failbit)) {
            double tmp;
            strs >> tmp;
            value.push_back(tmp);
            columnindex++;
        }
    }
    if (columnindex >= (max(xColumn, fColumn))) {
      data[value[xColumn-1]] =  value[fColumn-1] ;
    }
  }
  datafile_fs.close();

  if (data.size() == 0) {
    return false ;
  }

  computeInternalFunctionData();
  return true ;
}


void
MonotCubicInterpolator::
addPair(double newx, double newf) {
  if (std::isnan(newx) || std::isinf(newx) || std::isnan(newf) || std::isinf(newf)) {
    throw("MonotCubicInterpolator: addPair() received inf/nan input.");
  }
  data[newx] = newf ;

  // In a critical application, we should only update the
  // internal function data for the offended interval,
  // not for all function values over again.
  computeInternalFunctionData();
}


double
MonotCubicInterpolator::
evaluate(double x) const {

  if (std::isnan(x) || std::isinf(x)) {
    throw("MonotCubicInterpolator: evaluate() received inf/nan input.");
  }

  // xf becomes the first (xdata,fdata) pair where xdata >= x
  map<double,double>::const_iterator xf_iterator = data.lower_bound(x);

  // First check if we must extrapolate:
  if (xf_iterator == data.begin()) {
    if (data.begin()->first == x) { // Just on the interval limit
      return data.begin()->second;
    }
    else {
      // Constant extrapolation (!!)
      return data.begin()->second;
    }
  }
  if (xf_iterator == data.end()) {
      // Constant extrapolation (!!)
      return data.rbegin()->second;
  }


  // Ok, we have x_min < x < x_max

  pair<double,double> xf2 = *xf_iterator;
  pair<double,double> xf1 = *(--xf_iterator);
  // we now have: xf2.first > x

  // Linear interpolation if derivative data is not available:
  if (ddata.size() != data.size()) {
    double finterp =  xf1.second +
      (xf2.second - xf1.second) / (xf2.first - xf1.first)
      * (x - xf1.first);
    return finterp;
  }
  else { // Do Cubic Hermite spline
    double t = (x - xf1.first)/(xf2.first - xf1.first); // t \in [0,1]
    double h = xf2.first - xf1.first;
    double finterp
      = xf1.second       * H00(t)
      + ddata[xf1.first] * H10(t) * h
      + xf2.second       * H01(t)
      + ddata[xf2.first] * H11(t) * h ;
    return finterp;
  }

}


// double
// MonotCubicInterpolator::
// evaluate(double x, double& errorestimate_output) {
//    cout << "Error: errorestimate not implemented" << endl;
//    throw("error estimate not implemented");
//    return x;
// }

vector<double>
MonotCubicInterpolator::
get_xVector() const
{
  map<double,double>::const_iterator xf_iterator = data.begin();

  vector<double> outputvector;
  outputvector.reserve(data.size());
  for (xf_iterator = data.begin(); xf_iterator != data.end(); ++xf_iterator) {
    outputvector.push_back(xf_iterator->first);
  }
  return outputvector;
}


vector<double>
MonotCubicInterpolator::
get_fVector() const
{

  map<double,double>::const_iterator xf_iterator = data.begin();

  vector<double> outputvector;
  outputvector.reserve(data.size());
  for (xf_iterator = data.begin(); xf_iterator != data.end(); ++xf_iterator) {
    outputvector.push_back(xf_iterator->second);
  }
  return outputvector;
}



string
MonotCubicInterpolator::
toString() const
{
  const int precision = 20;
  std::stringstream dataStringStream;
  for (map<double,double>::const_iterator it = data.begin();
       it != data.end(); ++it) {
      dataStringStream << setprecision(precision) << it->first;
    dataStringStream << '\t';
    dataStringStream << setprecision(precision) << it->second;
    dataStringStream << '\n';
  }
  dataStringStream << "Derivative values:" << endl;
  for (map<double,double>::const_iterator it = ddata.begin();
       it != ddata.end(); ++it) {
    dataStringStream << setprecision(precision) << it->first;
    dataStringStream << '\t';
    dataStringStream << setprecision(precision) << it->second;
    dataStringStream << '\n';
  }

  return dataStringStream.str();

}


pair<double,double>
MonotCubicInterpolator::
getMissingX() const
{
  if( data.size() < 2) {
    throw("MonotCubicInterpolator::getMissingX() only one datapoint.");
  }

  // Search for biggest difference value in function-datavalues:

  map<double,double>::const_iterator  maxfDiffPair_iterator = data.begin();;
  double maxfDiffValue = 0;

  map<double,double>::const_iterator xf_iterator;
  map<double,double>::const_iterator xf_next_iterator;

  for (xf_iterator = data.begin(), xf_next_iterator = ++(data.begin());
       xf_next_iterator != data.end();
       ++xf_iterator, ++xf_next_iterator) {
    double absfDiff = fabs((double)(*xf_next_iterator).second
                           - (double)(*xf_iterator).second);
    if (absfDiff > maxfDiffValue) {
      maxfDiffPair_iterator = xf_iterator;
      maxfDiffValue = absfDiff;
    }
  }

  double newXvalue = ((*maxfDiffPair_iterator).first + ((*(++maxfDiffPair_iterator)).first))/2;
  return make_pair(newXvalue, maxfDiffValue);

}



pair<double,double>
MonotCubicInterpolator::
getMaximumF() const {
  if (data.size() <= 1) {
    throw ("MonotCubicInterpolator::getMaximumF() empty data.") ;
  }
  if (strictlyIncreasing)
    return *data.rbegin();
  else if (strictlyDecreasing)
    return *data.begin();
  else {
    pair<double,double> maxf = *data.rbegin() ;
    map<double,double>::const_iterator xf_iterator;
    for (xf_iterator = data.begin() ; xf_iterator != data.end(); ++xf_iterator) {
      if (xf_iterator->second > maxf.second) {
        maxf = *xf_iterator ;
      } ;
    }
    return maxf ;
  }
}


pair<double,double>
MonotCubicInterpolator::
getMinimumF() const {
  if (data.size() <= 1) {
    throw ("MonotCubicInterpolator::getMinimumF() empty data.") ;
  }
  if (strictlyIncreasing)
    return *data.begin();
  else if (strictlyDecreasing) {
    return *data.rbegin();
  }
  else {
    pair<double,double> minf = *data.rbegin() ;
    map<double,double>::const_iterator xf_iterator;
    for (xf_iterator = data.begin() ; xf_iterator != data.end(); ++xf_iterator) {
      if (xf_iterator->second < minf.second) {
        minf = *xf_iterator ;
      } ;
    }
    return minf ;
  }
}


void
MonotCubicInterpolator::
computeInternalFunctionData() const {

  /* The contents of this function is meaningless if there is only one datapoint */
  if (data.size() <= 1) {
    return;
  }

  /* We do not check the caching flag if we are instructed
     to do this computation */

  /* We compute monotoneness and directions by assuming
     monotoneness, and setting to false if the function is not for
     some value */

  map<double,double>::const_iterator xf_iterator;
  map<double,double>::const_iterator xf_next_iterator;


  strictlyMonotone = true; // We assume this is true, and will set to false if not
  monotone = true;
  strictlyDecreasing = true;
  decreasing = true;
  strictlyIncreasing = true;
  increasing = true;

  // Increasing or decreasing??
  xf_iterator = data.begin();
  xf_next_iterator = ++(data.begin());
  /* Cater for non-strictness, search for direction for monotoneness */
  while (xf_next_iterator != data.end() &&
         xf_iterator->second == xf_next_iterator->second) {
    /* Ok, equal values, this is not strict. */
    strictlyMonotone = false;
    strictlyIncreasing = false;
    strictlyDecreasing = false;

    ++xf_iterator;
    ++xf_next_iterator;
  }


  if (xf_next_iterator != data.end()) {

    if ( xf_iterator->second > xf_next_iterator->second) {
      // Ok, decreasing, check monotoneness:
      strictlyDecreasing = true;// if strictlyMonotone == false, this one should not be trusted anyway
      decreasing = true;
      strictlyIncreasing = false;
      increasing = false;
      while(++xf_iterator, ++xf_next_iterator != data.end()) {
        if ((*xf_iterator).second <  (*xf_next_iterator).second) {
          monotone = false;
          strictlyMonotone = false;
          strictlyDecreasing = false; // meaningless now
          break; // out of while loop
        }
        if ((*xf_iterator).second <= (*xf_next_iterator).second) {
          strictlyMonotone = false;
          strictlyDecreasing = false; // meaningless now
        }
      }
    }
    else if (xf_iterator->second < xf_next_iterator->second) {
      // Ok, assume increasing, check monotoneness:
      strictlyDecreasing = false;
      strictlyIncreasing = true;
      decreasing = false;
      increasing = true;
      while(++xf_iterator, ++xf_next_iterator != data.end()) {
        if ((*xf_iterator).second >  (*xf_next_iterator).second) {
          monotone = false;
          strictlyMonotone = false;
          strictlyIncreasing = false; // meaningless now
          break; // out of while loop
        }
        if ((*xf_iterator).second >= (*xf_next_iterator).second) {
          strictlyMonotone = false;
          strictlyIncreasing = false; // meaningless now
        }
      }
    }
    else {
      // first two values must be equal if we get
      // here, but that should have been taken care
      // of by the while loop above.
      throw("Programming logic error.") ;
    }

  }
  computeSimpleDerivatives();


  // If our input data is monotone, we can do monotone cubic
  // interpolation, so adjust the derivatives if so.
  //
  // If input data is not monotone, we should not touch
  // the derivatives, as this code should reduce to a
  // standard cubic interpolation algorithm.
  if (monotone) {
    adjustDerivativesForMonotoneness();
  }

  strictlyMonotoneCached = true;
  monotoneCached = true;
}

//       Checks if the function curve is flat (zero derivative) at the
//       endpoints, chop off endpoint data points if that is the case.
//
//       The notion of "flat" is determined by the input parameter "epsilon"
//       Values whose difference are less than epsilon are regarded as equal.
//
//       This is implemented to be able to obtain a strictly monotone
//       curve from a data set that is strictly monotone except at the
//       endpoints.
//
//       Example:
//         The data points
//            (1,3), (2,3), (3,4), (4,5), (5,5), (6,5)
//         will become
//            (2,3), (3,4), (4,5)
//
//       Assumes at least 3 datapoints. If less than three, this function is a noop.
void
MonotCubicInterpolator::
chopFlatEndpoints(const double epsilon) {

    if (getSize() < 3) {
        return;
    }

    map<double,double>::iterator xf_iterator;
    map<double,double>::iterator xf_next_iterator;

    // Clear flags:
    strictlyMonotoneCached = false;
    monotoneCached = false;

    // Chop left end:
    xf_iterator = data.begin();
    xf_next_iterator = ++(data.begin());
    // Erase data points that are similar to its right value from the left end.
    while ((xf_next_iterator != data.end()) &&
           (fabs(xf_iterator->second - xf_next_iterator->second) < epsilon )) {
        ++xf_next_iterator;
        data.erase(xf_iterator);
        ++xf_iterator;
    }

    xf_iterator = data.end();
    --xf_iterator;   // (data.end() points beyond the last element)
    xf_next_iterator = xf_iterator;
    --xf_next_iterator;
    // Erase data points that are similar to its left value from the right end.
    while ((xf_next_iterator != data.begin()) &&
           (fabs(xf_iterator->second - xf_next_iterator->second) < epsilon )) {
        --xf_next_iterator;
        data.erase(xf_iterator);
        --xf_iterator;
    }

    // Finished chopping, so recompute function data:
    computeInternalFunctionData();
}


//
//       If function is monotone, but not strictly monotone,
//       this function will remove datapoints from intervals
//       with zero derivative so that the curves become
//       strictly monotone.
//
//       Example
//         The data points
//           (1,2), (2,3), (3,4), (4,4), (5,5), (6,6)
//         will become
//           (1,2), (2,3), (3,4), (5,5), (6,6)
//
//       Assumes at least two datapoints, if one or zero datapoint, this is a noop.
//
//
void
MonotCubicInterpolator::
shrinkFlatAreas(const double epsilon) {

    if (getSize() < 2) {
        return;
    }

    map<double,double>::iterator xf_iterator;
    map<double,double>::iterator xf_next_iterator;


    // Nothing to do if we already are strictly monotone
    if (isStrictlyMonotone()) {
        return;
    }

    // Refuse to change a curve that is not monotone.
    if (!isMonotone()) {
        return;
    }

    // Clear flags, they are not to be trusted after we modify the
    // data
    strictlyMonotoneCached = false;
    monotoneCached = false;

    // Iterate through data values, if two data pairs
    // have equal values, delete one of the data pair.
    // Do not trust the source code on which data point is being
    // removed (x-values of equal y-points might be averaged in the future)
    xf_iterator = data.begin();
    xf_next_iterator = ++(data.begin());

    while (xf_next_iterator != data.end()) {
        //cout << xf_iterator->first << "," << xf_iterator->second << " " << xf_next_iterator->first << "," << xf_next_iterator->second << "\n";
        if (fabs(xf_iterator->second - xf_next_iterator->second) < epsilon ) {
            //cout << "erasing data pair" << xf_next_iterator->first << " " << xf_next_iterator->second << "\n";
            map <double,double>::iterator xf_tobedeleted_iterator = xf_next_iterator;
            ++xf_next_iterator;
            data.erase(xf_tobedeleted_iterator);
        }
        else {
            ++xf_iterator;
            ++xf_next_iterator;
        }
    }

}


void
MonotCubicInterpolator::
computeSimpleDerivatives() const {

  ddata.clear();

  // Do endpoints first:
  map<double,double>::const_iterator xf_iterator;
  map<double,double>::const_iterator xf_next_iterator;
  double diff;

  // Leftmost interval:
  xf_iterator = data.begin();
  xf_next_iterator = ++(data.begin());
  diff =
    (xf_next_iterator->second - xf_iterator->second) /
    (xf_next_iterator->first  - xf_iterator->first);
  ddata[xf_iterator->first] = diff ;

  // Rightmost interval:
  xf_iterator = --(--(data.end()));
  xf_next_iterator = --(data.end());
  diff =
    (xf_next_iterator->second - xf_iterator->second) /
    (xf_next_iterator->first  - xf_iterator->first);
  ddata[xf_next_iterator->first] = diff ;

  // If we have more than two intervals, loop over internal points:
  if (data.size() > 2) {

    map<double,double>::const_iterator intpoint;
    for (intpoint = ++data.begin(); intpoint != --data.end(); ++intpoint) {
      /*
        diff = (f2 - f1)/(x2-x1)/w + (f3-f1)/(x3-x2)/2

        average of the forward and backward difference.
        Weights are equal, should we weigh with h_i?
      */

      map<double,double>::const_iterator lastpoint = intpoint; --lastpoint;
      map<double,double>::const_iterator nextpoint = intpoint; ++nextpoint;

      diff = (nextpoint->second - intpoint->second)/
        (2*(nextpoint->first - intpoint->first))
        +
        (intpoint->second - lastpoint->second) /
        (2*(intpoint->first - lastpoint->first));

      ddata[intpoint->first] = diff ;
    }
  }
}



void
MonotCubicInterpolator::
adjustDerivativesForMonotoneness() const {
  map<double,double>::const_iterator point, dpoint;

  /* Loop over all intervals, ie. loop over all points and look
     at the interval to the right of the point */
  for (point = data.begin(), dpoint = ddata.begin();
       point != --data.end();
       ++point, ++dpoint) {
    map<double,double>::const_iterator nextpoint, nextdpoint;
    nextpoint = point; ++nextpoint;
    nextdpoint = dpoint; ++nextdpoint;

    double delta =
      (nextpoint->second - point->second) /
      (nextpoint->first  - point->first);
    if (fabs(delta) < 1e-14) {
      ddata[point->first] = 0.0;
      ddata[nextpoint->first] = 0.0;
    } else {
      double alpha = ddata[point->first] / delta;
      double beta = ddata[nextpoint->first] / delta;

      if (! isMonotoneCoeff(alpha, beta)) {
        double tau = 3/sqrt(alpha*alpha + beta*beta);

        ddata[point->first]     = tau*alpha*delta;
        ddata[nextpoint->first] = tau*beta*delta;
      }
    }


  }


}




void
MonotCubicInterpolator::
scaleData(double factor) {
  map<double,double>::iterator it , itd  ;
  if (data.size() == ddata.size()) {
    for (it = data.begin() , itd = ddata.begin() ; it != data.end() ; ++it , ++itd) {
      it->second  *= factor ;
      itd->second *= factor ;
    } ;
  } else {
    for (it = data.begin() ; it != data.end() ; ++it ) {
      it->second  *= factor ;
    }
  }
}


} // namespace Opm
