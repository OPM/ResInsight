/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RiaGraphicsEnvironmentTools.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QStringList>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Classify the X11 DISPLAY without reporting the host name
//--------------------------------------------------------------------------------------------------
std::string displayKind( const QString& display )
{
    if ( display.isEmpty() ) return "none";

    const QString host = display.section( ':', 0, 0 );
    if ( host.isEmpty() || host == "unix" ) return "local";

    // ssh -X/-Y forwarding uses localhost with a display number of 10 and above
    if ( host == "localhost" || host == "127.0.0.1" ) return "localhost";

    return "remote";
}

//--------------------------------------------------------------------------------------------------
/// Names of known remote display servers running on this host. Remote graphics solutions such as HP RGS
/// capture the local console display, so they are not visible in the environment of the application.
//--------------------------------------------------------------------------------------------------
std::string runningRemoteDisplayServers()
{
#ifdef Q_OS_LINUX
    // Process names as shown in /proc/<pid>/comm, truncated by the kernel to 15 characters
    const QStringList knownServers = { "rgsender", "pcoip-server", "Xvnc", "x0vncserver", "vncagent", "nxnode", "nxserver.bin" };

    QStringList found;
    for ( const auto& pid : QDir( "/proc" ).entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
        if ( !pid.front().isDigit() ) continue;

        QFile commFile( "/proc/" + pid + "/comm" );
        if ( !commFile.open( QIODevice::ReadOnly ) ) continue;

        const QString name = QString::fromLatin1( commFile.readAll() ).trimmed();
        if ( knownServers.contains( name ) && !found.contains( name ) ) found.push_back( name );
    }

    return found.join( "," ).toStdString();
#else
    return {};
#endif
}

//--------------------------------------------------------------------------------------------------
/// Driver strings are free text from the driver. Keep printable ASCII only and limit the length.
//--------------------------------------------------------------------------------------------------
std::string sanitizeGlString( const std::string& text )
{
    const size_t maxLength = 100;

    std::string result;
    for ( const char c : text )
    {
        if ( result.size() >= maxLength ) break;
        if ( c >= 0x20 && c < 0x7f ) result.push_back( c );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string glString( QOpenGLFunctions* functions, GLenum name )
{
    const auto* str = reinterpret_cast<const char*>( functions->glGetString( name ) );
    return str ? sanitizeGlString( str ) : std::string( "unknown" );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::string> RiaGraphicsEnvironmentTools::displayEnvironment()
{
    std::map<std::string, std::string> attributes;

    attributes["graphics.qt_runtime_version"] = qVersion();
    attributes["graphics.qpa_platform"]       = QGuiApplication::platformName().toStdString();
    attributes["graphics.session_type"]       = qEnvironmentVariable( "XDG_SESSION_TYPE" ).toStdString();
    attributes["graphics.desktop"]            = qEnvironmentVariable( "XDG_CURRENT_DESKTOP" ).toStdString();
    attributes["graphics.x11_display"]        = displayKind( qEnvironmentVariable( "DISPLAY" ) );

    // Set when running under a Wayland compositor, also when Qt uses xcb through XWayland
    attributes["graphics.wayland_display"] = qEnvironmentVariableIsSet( "WAYLAND_DISPLAY" ) ? "true" : "false";

    // Presence of variables set by remote access tools. Only the names are reported, never the values.
    {
        const QStringList remoteHintVariables = { "SSH_CONNECTION", "VGL_ISACTIVE", "TLSESSIONDATA", "VNCDESKTOP" };

        QStringList present;
        for ( const auto& name : remoteHintVariables )
        {
            if ( qEnvironmentVariableIsSet( name.toLatin1().constData() ) ) present.push_back( name );
        }

        // VirtualGL can also be injected without vglrun
        if ( qEnvironmentVariable( "LD_PRELOAD" ).contains( "vglfaker" ) ) present.push_back( "LD_PRELOAD=vglfaker" );

        attributes["graphics.remote_hints"] = present.join( "," ).toStdString();
    }

    attributes["graphics.remote_display_servers"] = runningRemoteDisplayServers();

    // Overrides that change the OpenGL code path in Qt or the driver
    {
        const QStringList overrideVariables = { "QT_QPA_PLATFORM",
                                                "QT_XCB_GL_INTEGRATION",
                                                "QT_WIDGETS_RHI",
                                                "QT_WIDGETS_RHI_BACKEND",
                                                "QT_OPENGL",
                                                "LIBGL_ALWAYS_SOFTWARE",
                                                "LIBGL_ALWAYS_INDIRECT",
                                                "MESA_LOADER_DRIVER_OVERRIDE",
                                                "__GLX_VENDOR_LIBRARY_NAME",
                                                "__GL_SYNC_TO_VBLANK",
                                                "RESINSIGHT_ENABLE_NATIVE_GL_WIDGET" };

        const QStringList knownValues = { "0",        "1",        "true",    "false",    "xcb",         "wayland",    "offscreen",
                                          "minimal",  "eglfs",    "linuxfb", "software", "desktop",     "angle",      "mesa",
                                          "nvidia",   "egl",      "glx",     "egl_x11",  "egl_wayland", "glx_nvidia", "swrast",
                                          "llvmpipe", "radeonsi", "iris",    "nouveau" };

        QStringList overrides;
        for ( const auto& name : overrideVariables )
        {
            const QByteArray latinName = name.toLatin1();
            if ( qEnvironmentVariableIsSet( latinName.constData() ) )
            {
                // Values are free text set by the user. Only report values from a fixed list, never arbitrary text.
                const QString value    = qEnvironmentVariable( latinName.constData() ).toLower();
                const QString reported = knownValues.contains( value ) ? value : QString( "other" );
                overrides.push_back( name + "=" + reported );
            }
        }

        attributes["graphics.env_overrides"] = overrides.join( "," ).toStdString();
    }

    return attributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::string> RiaGraphicsEnvironmentTools::currentOpenGLContextInfo()
{
    std::map<std::string, std::string> attributes;

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if ( !context ) return attributes;

    QOpenGLFunctions* functions = context->functions();

    attributes["graphics.gl_vendor"]       = glString( functions, GL_VENDOR );
    attributes["graphics.gl_renderer"]     = glString( functions, GL_RENDERER );
    attributes["graphics.gl_version"]      = glString( functions, GL_VERSION );
    attributes["graphics.gl_glsl_version"] = glString( functions, GL_SHADING_LANGUAGE_VERSION );

    const QSurfaceFormat format = context->format();

    std::string profile = "none";
    if ( format.profile() == QSurfaceFormat::CoreProfile ) profile = "core";
    if ( format.profile() == QSurfaceFormat::CompatibilityProfile ) profile = "compatibility";

    attributes["graphics.gl_context"] = std::to_string( format.majorVersion() ) + "." + std::to_string( format.minorVersion() ) + " " +
                                        profile + ( context->isOpenGLES() ? " es" : "" );

    return attributes;
}
