/* -*-C++-*- */

#ifndef _MONOTCUBICINTERPOLATOR_H
#define _MONOTCUBICINTERPOLATOR_H

#include <vector>
#include <map>
#include <string>

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

namespace Opm
{

/**
   Class to represent a one-dimensional function f with single-valued
   argument x. The function is represented by a table of function
   values. Interpolation between table values is cubic and monotonicity
   preserving if input values are monotonous.

   Outside x_min and x_max, the class will extrapolate using the
   constant f(x_min) or f(x_max).

   Extra functionality:
    - Can return (x_1+x_2)/2 where x_1 and x_2 are such that
      abs(f(x_1) - f(x_2)) is maximized. This is used to determine where
      one should calculate a new value for increased accuracy in the
      current function

   Monotonicity preserving cubic interpolation algorithm is taken
   from Fritsch and Carlson, "Monotone piecewise cubic interpolation",
   SIAM J. Numer. Anal. 17, 238--246, no. 2,

   $Id$

   Algorithm also described here:
   http://en.wikipedia.org/wiki/Monotone_cubic_interpolation


   @author Håvard Berland <havb (at) statoil.com>, December 2006
   @brief Represents one dimensional function f with single valued argument x that can be interpolated using monotone cubic interpolation

*/

class MonotCubicInterpolator {
 public:

   /**
      @param datafilename A datafile with the x values and the corresponding f(x) values

      Accepts a filename as input and parses this file for
      two-column floating point data, interpreting the data as
      representing function values x and f(x).

      Ignores all lines not conforming to \<whitespace\>\<float\>\<whitespace\>\<float\>\<whatever\>\<newline\>
   */
  explicit MonotCubicInterpolator(const std::string & datafilename)
  {
    if (!read(datafilename)) {
      throw("Unable to constuct MonotCubicInterpolator from file.") ;
    } ;
  }


   /**
      @param datafilename A datafile with the x values and the corresponding f(x) values

      Accepts a filename as input and parses this file for
      two-column floating point data, interpreting the data as
      representing function values x and f(x).

      Ignores all lines not conforming to \<whitespace\>\<float\>\<whitespace\>\<float\>\<whatever\>\<newline\>

      All commas in the file will be treated as spaces when parsing.

   */

 explicit MonotCubicInterpolator(const char* datafilename)
  {
    if (!read(std::string(datafilename))) {
      throw("Unable to constuct MonotCubicInterpolator from file.") ;
    } ;
  }


   /**
       @param datafilename data file
       @param XColumn x values
       @param fColumn f values

       Accepts a filename as input, and parses the chosen columns in
       that file.
   */
   MonotCubicInterpolator(const char* datafilename, int xColumn, int fColumn)
  {
    if (!read(std::string(datafilename),xColumn,fColumn)) {
      throw("Unable to constuct MonotCubicInterpolator from file.") ;
    } ;
  }

   /**
       @param datafilename data file
       @param XColumn x values
       @param fColumn f values

       Accepts a filename as input, and parses the chosen columns in
       that file.
   */
   MonotCubicInterpolator(const std::string & datafilename, int xColumn, int fColumn)
  {
    if (!read(datafilename,xColumn,fColumn)) {
      throw("Unable to constuct MonotCubicInterpolator from file.") ;
    } ;
  }

   /**
      @param x vector of x values
      @param f vector of corresponding f values

      Accepts two equal-length vectors as input for constructing
      the interpolation object.  First vector is the x-values, the
      second vector is the function values
   */
     MonotCubicInterpolator(const std::vector<double> & x ,
                            const std::vector<double> & f);

   /**
      No input, an empty function object is created.

      This object must be treated with care until
      populated.
   */
   MonotCubicInterpolator() { }



   /**
      @param datafilename A datafile with the x values and the corresponding f(x) values

      Accepts a filename as input and parses this file for
      two-column floating point data, interpreting the data as
      representing function values x and f(x).

      returns true on success

      All commas in file will be treated as spaces when parsing

      Ignores all lines not conforming to \<whitespace\>\<float\>\<whitespace\>\<float\>\<whatever\>\<newline\>
   */
  bool read(const std::string & datafilename) {
    return read(datafilename,1,2) ;
  }

   /**
       @param datafilename data file
       @param XColumn x values
       @param fColumn f values

       Accepts a filename as input, and parses the chosen columns in
       that file.
   */
  bool read(const std::string &  datafilename, int xColumn, int fColumn) ;



   /**
      @param x x value

      Returns f(x) for given x (input). Interpolates (monotone cubic
      or linearly) if necessary.

      Extrapolates using the constants f(x_min) or f(x_max) if
      input x is outside (x_min, x_max)

      @return f(x) for a given x
   */
  double operator () (double x) const { return evaluate(x) ; }

   /**
      @param x x value

      Returns f(x) for given x (input). Interpolates (monotone cubic
      or linearly) if necessary.

      Extrapolates using the constants f(x_min) or f(x_max) if
      input x is outside (x_min, x_max)

      @return f(x) for a given x
   */
   double evaluate(double x) const;

   /**
      @param x x value
      @param errorestimate_output

      Returns f(x) and an error estimate for given x (input).

      Interpolates (linearly) if necessary.

      Throws an exception if extrapolation would be necessary for
      evaluation. We do not want to do extrapolation (yet).

      The error estimate for x1 < x < x2 is
      (x2 - x1)^2/8 * f''(x) where f''(x) is evaluated using
      the stencil (1 -2  1) using either (x0, x1, x2) or (x1, x2, x3);

      Throws an exception if the table contains only two x-values.

      NOT IMPLEMENTED YET!
   */
   double evaluate(double x, double & errorestimate_output ) const ;

   /**
      Minimum x-value, returns both x and f in a pair.

      @return minimum x value
      @return f(minimum x value)
   */
   std::pair<double,double> getMinimumX() const {
       // Easy since the data is sorted on x:
       return *data.begin();
   }

   /**
      Maximum x-value, returns both x and f in a pair.

      @return maximum x value
      @return f(maximum x value)
   */
   std::pair<double,double> getMaximumX() const {
       // Easy since the data is sorted on x:
       return *data.rbegin();
   }

   /**
      Maximum f-value, returns both x and f in a pair.

      @return x value corresponding to maximum f value
      @return maximum f value
   */
   std::pair<double,double> getMaximumF() const ;

   /**
      Minimum f-value, returns both x and f in a pair

      @return x value corresponding to minimal f value
      @return minimum f value
   */
   std::pair<double,double> getMinimumF() const  ;


   /**
      Provide a copy of the x-data as a vector

      Unspecified order, but corresponds to get_fVector.

      @return x values as a vector
   */
   std::vector<double> get_xVector() const ;

   /**
      Provide a copy of tghe function data as a vector

      Unspecified order, but corresponds to get_xVector

      @return f values as a vector

   */
   std::vector<double> get_fVector() const ;

   /**
      @param factor Scaling constant

      Scale all the function value data by a constant
   */
   void scaleData(double factor);

   /**
      Determines if the current function-value-data is strictly
      monotone. This is a utility function for outsiders if they want
      to invert the data for example.

      @return True if f(x) is strictly monotone, else False
   */
   bool isStrictlyMonotone() {

       /* Use cached value if it can be trusted */
       if (strictlyMonotoneCached) {
           return strictlyMonotone;
       }
       else {
           computeInternalFunctionData();
           return strictlyMonotone;
       }
   }

   /**
      Determines if the current function-value-data is monotone.

      @return True if f(x) is monotone, else False
   */
   bool isMonotone() const {
       if (monotoneCached) {
           return monotone;
       }
       else {
           computeInternalFunctionData();
           return monotone;
       }
   }
   /**
      Determines if the current function-value-data is strictly
      increasing. This is a utility function for outsiders if they want
      to invert the data for example.

      @return True if f(x) is strictly increasing, else False
   */
   bool isStrictlyIncreasing() {

       /* Use cached value if it can be trusted */
       if (strictlyMonotoneCached) {
           return (strictlyMonotone && strictlyIncreasing);
       }
       else {
           computeInternalFunctionData();
           return (strictlyMonotone && strictlyIncreasing);
       }
   }

   /**
      Determines if the current function-value-data is monotone and increasing.

      @return True if f(x) is monotone and increasing, else False
   */
   bool isMonotoneIncreasing() const {
       if (monotoneCached) {
           return (monotone && increasing);
       }
       else {
           computeInternalFunctionData();
           return (monotone && increasing);
       }
   }
   /**
      Determines if the current function-value-data is strictly
      decreasing. This is a utility function for outsiders if they want
      to invert the data for example.

      @return True if f(x) is strictly decreasing, else False
   */
   bool isStrictlyDecreasing() {

       /* Use cached value if it can be trusted */
       if (strictlyMonotoneCached) {
           return (strictlyMonotone && strictlyDecreasing);
       }
       else {
           computeInternalFunctionData();
           return (strictlyMonotone && strictlyDecreasing);
       }
   }

   /**
      Determines if the current function-value-data is monotone and decreasing

      @return True if f(x) is monotone and decreasing, else False
   */
   bool isMonotoneDecreasing() const {
       if (monotoneCached) {
           return (monotone && decreasing);
       }
       else {
           computeInternalFunctionData();
           return (monotone && decreasing);
       }
   }



   /**
      @param newx New x point
      @param newf New f(x) point

      Adds a new datapoint to the function.

      This causes all the derivatives at all points of the functions
      to be recomputed and then adjusted for monotone cubic
      interpolation. If this function ever enters a critical part of
      any code, the locality of the algorithm for monotone adjustment
      must be exploited.

   */
   void addPair(double newx, double newf);

   /**
      Returns an x-value that is believed to yield the best
      improvement in global accuracy for the interpolation if
      computed.

      Searches for the largest jump in f-values, and returns a x
      value being the average of the two x-values representing the
      f-value-jump.

      @return New x value beleived to yield the best improvement in global accuracy
      @return Maximal difference
   */
   std::pair<double,double> getMissingX() const;

   /**
      Constructs a string containing the data in a table

      @return a string containing the data in a table
   */
   std::string toString() const;

   /**
     @return Number of datapoint pairs in this object
   */
   int getSize() const {
       return data.size();
   }

    /**
       Checks if the function curve is flat at the endpoints, chop off
       endpoint data points if that is the case.

       The notion of "flat" is determined by the input parameter "epsilon"
       Values whose difference are less than epsilon are regarded as equal.

       This is implemented to be able to obtain a strictly monotone
       curve from a data set that is strictly monotone except at the
       endpoints.

       Example:
         The data points
            (1,3), (2,3), (3,4), (4,5), (5,5), (6,5)
         will become
            (2,3), (3,4), (4,5)

       Assumes at least 3 datapoints. If less than three, this function is a noop.
    */
    void chopFlatEndpoints(const double);

    /**
       Wrapper function for chopFlatEndpoints(const double)
       providing a default epsilon parameter
    */
    void chopFlatEndpoints() {
        chopFlatEndpoints(1e-14);
    }

    /**
       If function is monotone, but not strictly monotone,
       this function will remove datapoints from intervals
       with zero derivative so that the curve become
       strictly monotone.

       Example
         The data points
           (1,2), (2,3), (3,4), (4,4), (5,5), (6,6)
         will become
           (1,2), (2,3), (3,4), (5,5), (6,6)

       Assumes at least two datapoints, if one or zero datapoint, this is a noop.
    */
    void shrinkFlatAreas(const double);

    /**
       Wrapper function for shrinkFlatAreas(const double)
       providing a default epsilon parameter
    */
    void shrinkFlatAreas() {
        shrinkFlatAreas(1e-14);
    }



private:

   // Data structure to store x- and f-values
   std::map<double, double> data;

   // Data structure to store x- and d-values
   mutable std::map<double, double> ddata;


   // Storage containers for precomputed interpolation data
   //   std::vector<double> dvalues; // derivatives in Hermite interpolation.

   // Flag to determine whether the boolean strictlyMonotone can be
   // trusted.
   mutable bool strictlyMonotoneCached;
   mutable bool monotoneCached; /* only monotone, not stricly montone */

   mutable bool strictlyMonotone;
   mutable bool monotone;

   // if strictlyMonotone is true (and can be trusted), the two next are meaningful
   mutable bool strictlyDecreasing;
   mutable bool strictlyIncreasing;
   mutable bool decreasing;
   mutable bool increasing;


   /* Hermite basis functions, t \in [0,1] ,
      notation from:
      http://en.wikipedia.org/w/index.php?title=Cubic_Hermite_spline&oldid=84495502
   */

   double H00(double t) const {
       return 2*t*t*t - 3*t*t + 1;
   }
   double H10(double t) const {
       return t*t*t - 2*t*t + t;
   }
   double H01(double t) const {
       return -2*t*t*t + 3*t*t;
   }
   double H11(double t) const {
       return t*t*t - t*t;
   }


   void computeInternalFunctionData() const ;

   /**
       Computes initial derivative values using centered (second order) difference
       for internal datapoints, and one-sided derivative for endpoints

       The internal datastructure map<double,double> ddata is populated by this method.
   */

   void computeSimpleDerivatives() const ;


   /**
      Adjusts the derivative values (ddata) so that we can guarantee that
      the resulting piecewise Hermite polymial is monotone. This is
      done according to the algorithm of Fritsch and Carlsson 1980,
      see Section 4, especially the two last lines.
   */
  void adjustDerivativesForMonotoneness() const ;

   /**
       Checks if the coefficient alpha and beta is in
       the region that guarantees monotoneness of the
       derivative values they represent

       See Fritsch and Carlson 1980, Lemma 2,
       alternatively Step 5 in Wikipedia's article
       on Monotone cubic interpolation.
   */
   bool isMonotoneCoeff(double alpha, double beta) const {
       if ((alpha*alpha + beta*beta) <= 9) {
         return true;
       } else {
         return false;
       }
   }

};


} // namespace Opm

#endif
