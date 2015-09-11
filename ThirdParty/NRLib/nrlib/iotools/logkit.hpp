// $Id: logkit.hpp 1072 2012-09-18 13:53:37Z perroe $

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

#ifndef NRLIB_LOGKIT_H
#define NRLIB_LOGKIT_H

#include<ostream>
#include<vector>
#include<string>

namespace NRLib {

class LogStream;
struct BufferMessage;

/// Kit for logging of messages from program.
class LogKit {
public:
  ///Philosophy:
  ///A message has a level, determining the type of message, and a phase,
  ///determining the stage in the program. Each stream has a level (which may
  ///be a combination of basic levels) for each phase. If phase and flag
  ///matches, the message is sent to the stream. A message without phase is
  ///default sent to all streams that would send it in at least one phase. If
  ///the ignore_general flag is set to true when generating a stream, that
  ///stream will ignore messages without phase.
  ///
  ///The system can be used without bothering with phases. All NRLib logging
  ///is phase 0 LOW.
  ///

  ///Symbols for use when sending message level and parsing exact levels.
  enum MessageLevels {Error = 1, Warning = 2, Low = 4, Medium = 8, High = 16, DebugLow = 32, DebugHigh = 64};

  ///Symbols for use when parsing given level and lower.
  enum LimitLevels {L_Error = 1, L_Warning = 3, L_Low = 7, L_Medium = 15,
                    L_High = 31, L_DebugLow = 63, L_DebugHigh = 127};

  ///Set a file that logs independent of phase.
  static void SetFileLog(const std::string & fileName, int levels,
                         bool includeNRLibLogging = true);

  ///Set a full phase dependent file log
  static void SetFileLog(const std::string & fileName,
                         const std::vector<int> & levels,
                         bool ignore_general = false);

  ///Set single-phase file log, useful for debugging given phase.
  static void SetFileLog(const std::string & fileName,
                         int levels,
                         int phase,
                         bool ignore_general = false);


  ///Set a screen log independent of phase.
  static void SetScreenLog(int levels, bool includeNRLibLogging = true);

  ///Set a full phase dependent screen log
  static void SetScreenLog(const std::vector<int> & levels, bool ignore_general = false);


  ///Send message independent of phase
  static void LogMessage(int level, const std::string & message);

  ///Send message in given phase
  static void LogMessage(int level, int phase, const std::string & message);

  ///Send message as c-style format string and arguments.
  // Sending format as reference fails.
  static void LogFormatted(int level, std::string format, ...);

  ///Send message as c-style format string and arguments.
  static void LogFormatted(int level, int phase, std::string format, ...);


  ///Close streams
  static void EndLog();

  ///Buffering allows temporary storage of messages for sending to files
  ///opened later. When a file log is opened, the buffer is dumped to it.
  ///EndBuffering should be called once all files are opened.
  static void StartBuffering();
  static void EndBuffering();

  static void SetPrefix(const std::string & prefix, int level);
  static int GetNMessages(int level) { return n_messages_[level];}
  static void WriteHeader(const std::string & text, MessageLevels logLevel = Low);


private:
  static std::vector<LogStream *> logstreams_;
  static int screenLog_; //Remembers which log is screen.
  static std::vector<BufferMessage *> * buffer_;
  static std::vector<int> n_messages_;
  static std::vector<std::string> prefix_;

  static void SendToBuffer(int level, int phase, const std::string & message);
  static void DumpBuffer(LogStream * logstream);
};

///Class LogStream is for internal use in LogKit only.
class LogStream {
public:
  ///Convention: logstream = NULL means cout.
  LogStream(std::ostream * logstream, int level);
  LogStream(std::ostream * logstream, const std::vector<int> & levels, bool ignore_general = false);
  ~LogStream();

  void LogMessage(int level, const std::string & message);
  void LogMessage(int level, int phase, const std::string & message);

private:
  std::ostream * logstream_;
  std::vector<int> levels_;
  int fullLevel_;
  bool deleteStream;
};

struct BufferMessage {
  std::string text_;
  int         phase_;
  int         level_;
};

}
#endif

