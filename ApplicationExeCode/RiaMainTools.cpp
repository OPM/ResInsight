/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaMainTools.h"
#include "RiaBaseDefs.h"
#include "RiaFileLogger.h"
#include "RiaLogging.h"
#include "RiaOpenTelemetryManager.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSocketCommand.h"
#include "RiaVersionInfo.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QDir>

#include <atomic>
#include <csignal>
#include <exception>
#include <map>
#include <sstream>
#include <typeinfo>
#include <version>

// std::stacktrace is C++23; libc++ in Homebrew llvm@19 (and older) does not
// ship it.  Without it we skip stack-trace capture in the crash path but
// still log the signal.  Use the feature-test macro from <version> rather
// than __has_include, since the header may be present without a usable
// implementation.  See #14045.
#if defined( __cpp_lib_stacktrace ) && __cpp_lib_stacktrace >= 202011L
#include <stacktrace>
#define RIA_HAS_STD_STACKTRACE 1
#else
#define RIA_HAS_STD_STACKTRACE 0
#endif

#ifndef WIN32
#include <signal.h>
// macOS marks <ucontext.h> as deprecated and the mcontext layout
// (gregs[REG_RIP]) is Linux-only — skip the program-counter capture
// path on Apple platforms.  See #14045.
#ifndef __APPLE__
#include <ucontext.h>
#endif
#endif

namespace internal
{
#if RIA_HAS_STD_STACKTRACE
// Custom formatter for stacktrace
std::string formatStacktrace( const std::stacktrace& st )
{
    std::stringstream ss;
    int               frame = 0;
    for ( const auto& entry : st )
    {
        ss << "  [" << frame++ << "] " << entry.description() << " at " << entry.source_file() << ":"
           << entry.source_line() << "\n";
    }
    return ss.str();
}
#endif

#ifndef WIN32
static std::string signalCodeDescription( int signo, int siCode )
{
    switch ( signo )
    {
        case SIGSEGV:
            switch ( siCode )
            {
                case SEGV_MAPERR:
                    return "address not mapped (SEGV_MAPERR)";
                case SEGV_ACCERR:
                    return "invalid access permissions (SEGV_ACCERR)";
                default:
                    return "unknown SIGSEGV code";
            }
        case SIGFPE:
            switch ( siCode )
            {
                case FPE_INTDIV:
                    return "integer divide by zero (FPE_INTDIV)";
                case FPE_INTOVF:
                    return "integer overflow (FPE_INTOVF)";
                case FPE_FLTDIV:
                    return "floating-point divide by zero (FPE_FLTDIV)";
                case FPE_FLTOVF:
                    return "floating-point overflow (FPE_FLTOVF)";
                case FPE_FLTUND:
                    return "floating-point underflow (FPE_FLTUND)";
                case FPE_FLTRES:
                    return "floating-point inexact result (FPE_FLTRES)";
                case FPE_FLTINV:
                    return "invalid floating-point operation (FPE_FLTINV)";
                case FPE_FLTSUB:
                    return "subscript out of range (FPE_FLTSUB)";
                default:
                    return "unknown SIGFPE code";
            }
        case SIGILL:
            switch ( siCode )
            {
                case ILL_ILLOPC:
                    return "illegal opcode (ILL_ILLOPC)";
                case ILL_ILLOPN:
                    return "illegal operand (ILL_ILLOPN)";
                case ILL_ILLADR:
                    return "illegal addressing mode (ILL_ILLADR)";
                case ILL_ILLTRP:
                    return "illegal trap (ILL_ILLTRP)";
                case ILL_PRVOPC:
                    return "privileged opcode (ILL_PRVOPC)";
                case ILL_PRVREG:
                    return "privileged register (ILL_PRVREG)";
                case ILL_COPROC:
                    return "coprocessor error (ILL_COPROC)";
                case ILL_BADSTK:
                    return "internal stack error (ILL_BADSTK)";
                default:
                    return "unknown SIGILL code";
            }
        case SIGBUS:
            switch ( siCode )
            {
                case BUS_ADRALN:
                    return "invalid address alignment (BUS_ADRALN)";
                case BUS_ADRERR:
                    return "nonexistent physical address (BUS_ADRERR)";
                case BUS_OBJERR:
                    return "object-specific hardware error (BUS_OBJERR)";
                default:
                    return "unknown SIGBUS code";
            }
        default:
            return "";
    }
}
#endif
} // namespace internal

static std::atomic<bool> s_crashHandlingInProgress( false );

//--------------------------------------------------------------------------------------------------
/// Shared crash logging: writes to file logger and OpenTelemetry.
/// extraAttrs may include platform-specific context like fault address and signal code description.
///
/// Note: Executing logging functions from a signal handler is not async-signal-safe, but works as
/// expected on Windows. Behavior on Linux is undefined, but will work in most cases.
/// https://github.com/gabime/spdlog/issues/1607
//--------------------------------------------------------------------------------------------------
static void performCrashLogging( int signalCode, const std::map<std::string, std::string>& extraAttrs )
{
    // Block timer-driven application logic (e.g. gRPC idle processing) for the rest of the process
    // lifetime: the telemetry flush below pumps the event loop, and a timer slot firing there could
    // close the project and destroy objects the crashed code still uses, causing a nested crash.
    s_crashHandlingInProgress = true;

#if RIA_HAS_STD_STACKTRACE
    auto st = std::stacktrace::current();
#endif
    auto loggers = RiaLogging::loggerInstances();

    for ( auto logger : loggers )
    {
        if ( auto fileLogger = dynamic_cast<RiaFileLogger*>( logger ) )
        {
            auto versionText = QString( STRPRODUCTVER );
            auto str = QString( "Crash (signal: %1) - ResInsight version %2" ).arg( signalCode ).arg( versionText );
            fileLogger->error( str.toStdString().data() );

            for ( const auto& [key, value] : extraAttrs )
            {
                std::string line = key + ": " + value;
                fileLogger->error( line.data() );
            }

#if RIA_HAS_STD_STACKTRACE
            std::string message = "Stack trace:\n" + internal::formatStacktrace( st );
            logger->error( message.data() );
#else
            logger->error( "Stack trace: not available (compiled without <stacktrace>)" );
#endif

            fileLogger->flush();
        }
    }

    auto& otelManager = RiaOpenTelemetryManager::instance();
    if ( otelManager.isEnabled() )
    {
#if RIA_HAS_STD_STACKTRACE
        otelManager.reportCrash( signalCode, st, extraAttrs );
#endif
    }
}

//--------------------------------------------------------------------------------------------------
/// Terminate handler for uncaught C++ exceptions. Unlike the signal handler, this runs BEFORE the
/// stack is unwound: when no handler is found, libstdc++ calls std::terminate from the throw site,
/// so the stack trace captured here points at where the exception was originally thrown rather than
/// at the crash-logging plumbing. Also recovers the exception type and message, which a bare SIGABRT
/// trace cannot provide. Runs outside any signal handler, so logging here is safe (no async-signal
/// constraints, unlike performCrashLogging when reached via the signal path).
//--------------------------------------------------------------------------------------------------
void manageTerminate()
{
    std::map<std::string, std::string> extraAttrs;

    if ( auto ex = std::current_exception() )
    {
        try
        {
            std::rethrow_exception( ex );
        }
        catch ( const std::exception& e )
        {
            extraAttrs["crash.exception_type"] = typeid( e ).name();
            extraAttrs["crash.exception_what"] = e.what();
        }
        catch ( ... )
        {
            extraAttrs["crash.exception_type"] = "non-std exception";
        }
    }

    // performCrashLogging captures std::stacktrace::current() itself; called here (no signal, no
    // prior unwind) that trace still includes the original throw site.
    performCrashLogging( SIGABRT, extraAttrs );

    std::abort(); // preserve default terminate behavior (core dump, exit status)
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void manageSegFailure( int signalCode )
{
    performCrashLogging( signalCode, {} );
    exit( 1 );
}

#ifndef WIN32
//--------------------------------------------------------------------------------------------------
/// SA_SIGINFO-compatible handler: captures fault address, signal code, and program counter
/// for enhanced crash diagnostics on Linux.
//--------------------------------------------------------------------------------------------------
void manageSegFailureSA( int signalCode, siginfo_t* info, void* ucontext )
{
    std::map<std::string, std::string> extraAttrs;

    if ( info )
    {
        std::ostringstream addrStr;
        addrStr << "0x" << std::hex << reinterpret_cast<uintptr_t>( info->si_addr );
        extraAttrs["crash.fault_address"] = addrStr.str();

        std::string codeDesc = internal::signalCodeDescription( signalCode, info->si_code );
        if ( !codeDesc.empty() )
        {
            extraAttrs["crash.signal_code"] = codeDesc;
        }
    }

    // Program-counter capture relies on the Linux ucontext layout
    // (gregs[REG_RIP] for x86_64, mcontext.pc for aarch64).  macOS exposes
    // a different layout and marks <ucontext.h> as deprecated, so skip it
    // there; remaining crash attributes (signal code, fault address) are
    // still recorded.  See #14045.
#if !defined( __APPLE__ )
#if defined( __x86_64__ )
    if ( ucontext )
    {
        auto*              ctx = static_cast<ucontext_t*>( ucontext );
        std::ostringstream pcStr;
        pcStr << "0x" << std::hex << static_cast<uintptr_t>( ctx->uc_mcontext.gregs[REG_RIP] );
        extraAttrs["crash.program_counter"] = pcStr.str();
    }
#elif defined( __aarch64__ )
    if ( ucontext )
    {
        auto*              ctx = static_cast<ucontext_t*>( ucontext );
        std::ostringstream pcStr;
        pcStr << "0x" << std::hex << ctx->uc_mcontext.pc;
        extraAttrs["crash.program_counter"] = pcStr.str();
    }
#endif
#else
    (void)ucontext;
#endif

    performCrashLogging( signalCode, extraAttrs );
    exit( 1 );
}
#endif

namespace RiaMainTools
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isCrashHandlingInProgress()
{
    return s_crashHandlingInProgress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void initializeSingletons()
{
    caf::CmdFeatureManager::createSingleton();
    RiaRegressionTestRunner::createSingleton();
    caf::PdmDefaultObjectFactory::createSingleton();
}

//--------------------------------------------------------------------------------------------------
/// This method is used to release memory allocated by static functions. This enables use of memory allocation tools
/// after the application has closed down.
//--------------------------------------------------------------------------------------------------
void releaseSingletonAndFactoryObjects()
{
    caf::CmdFeatureManager::deleteSingleton();
    RiaRegressionTestRunner::deleteSingleton();
    caf::PdmDefaultObjectFactory::deleteSingleton();

    {
        auto factory = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance();
        factory->deleteCreatorObjects();
    }

    {
        auto factory = caf::Factory<caf::CmdFeature, std::string>::instance();
        factory->deleteCreatorObjects();
    }
    {
        auto factory = caf::Factory<RiaSocketCommand, QString>::instance();
        factory->deleteCreatorObjects();
    }
}

//--------------------------------------------------------------------------------------------------
/// On Linux, application settings are stored in a text file in the users home folder. On Windows, these settings are
/// stored in Registry. Users have reported stale lock files the configuration directory. In some cases, these lock
/// files can prevent the application from starting. It appears that the application start, but no GUI is displayed.
///
/// This method deletes stale lock files.
///
/// https://github.com/OPM/ResInsight/issues/12205
//--------------------------------------------------------------------------------------------------
void deleteStaleSettingsLockFiles()
{
    auto organizationName = QString( RI_COMPANY_NAME );
    auto applicationName  = QString( RI_APPLICATION_NAME );

    auto lockFilePath = QDir::homePath() + "/.config/" + organizationName + "/" + applicationName + ".conf.lock";

    auto isLockStale = []( const QString& lockFilePath ) -> bool
    {
        QFileInfo lockFileInfo( lockFilePath );

        if ( !lockFileInfo.exists() ) return false;

        QDateTime currentTime      = QDateTime::currentDateTime();
        int       thresholdSeconds = 2 * 60; // 2 minutes

        return lockFileInfo.lastModified().secsTo( currentTime ) > thresholdSeconds;
    };

    if ( isLockStale( lockFilePath ) )
    {
        QFile lockFile( lockFilePath );

        QString logMessage;
        if ( lockFile.remove() )
        {
            logMessage = QString( "Deleted stale lock file: %1" ).arg( lockFilePath );
        }
        else
        {
            logMessage = QString( "Tried, but failed to delete stale lock file: %1" ).arg( lockFilePath );
        }

        qDebug() << logMessage;
    }
}

} // namespace RiaMainTools
