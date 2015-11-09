// $Id: exception.hpp 1107 2012-11-06 08:40:41Z perroe $

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

#ifndef NRLIB_EXCEPTION_HPP
#define NRLIB_EXCEPTION_HPP

#include <exception>
#include <string>

//Test: Information flows from external back to original?
//Test: And of course it flows downwards?

namespace NRLib {

class Exception : public std::exception
{
public:
  explicit Exception(const std::string& msg = "") : msg_(msg) { }
  virtual ~Exception() throw() {}
  virtual const char * what() const throw() {return msg_.c_str();}
private:
  std::string msg_;
};

class IndexOutOfRange : public Exception
{
public:
  explicit IndexOutOfRange(const std::string& msg = "")
    : Exception(msg) {}

  virtual ~IndexOutOfRange() throw() {}
};

class FFTError : public Exception
{
public:
  explicit FFTError(const std::string& msg = "")
    : Exception(msg) {}

  virtual ~FFTError() throw() {}
};

class IOError : public Exception
{
public:
  explicit IOError(const std::string& msg = "")
    : Exception(msg) {}

  virtual ~IOError() throw() {}
};

class FileFormatError : public IOError
{
public:
  explicit FileFormatError(const std::string& msg = "")
    : IOError(msg) {}

  virtual ~FileFormatError() throw() {}
};

class EndOfFile : public IOError
{
public:
  explicit EndOfFile(const std::string& msg = "")
    : IOError(msg) {}

  virtual ~EndOfFile() throw() {}
};

class JobCanceled : public Exception
{
public:
  explicit JobCanceled(const std::string& msg = "")
    : Exception(msg) {}

  virtual ~JobCanceled() throw() {}
};

}

#endif
