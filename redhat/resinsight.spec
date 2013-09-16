#
# spec file for package resinsight
#

Name:           resinsight
Version:        0.9.2
Release:        0
Summary:        ResInsight - the 3D reservoir viewer and post processor
License:        GPL-3.0+
Group:          Science
Url:            http://opm-project.org
Source0:        %{name}-%{version}.orig.tar.gz
Patch0: 	01_use_system_ert.patch
Patch1: 	02_install_docdir.patch
Patch2: 	03_remove_internal_header_include.patch
Patch3:		04_ert_api_changes.patch
BuildRequires:  lapack-devel octave-devel qt qt-devel ert.ecl-devel
BuildRequires:  gcc gcc-c++
BuildRequires:  cmake28 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
ResInsight is a 3D viewer and post processing tool for reservoir models. 
It has been co-developed by Statoil and Ceetron with the aim to provide
a versatile tool for professionals who need to visualize and process
reservoir models. 

%package octave
Summary:        ResInsight plugins for Octave
Group:          Scientific
Requires:       %{name} = %{version}

%description octave
This package contains the ResInsight octave plugins.

%prep
%setup -q
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1

%build
cmake28 -DRESINSIGHT_PRIVATE_INSTALL=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/resinsight-0.9.2
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

%post -n resinsight -p /sbin/ldconfig

%postun -n resinsight -p /sbin/ldconfig

%files
%{_bindir}/*
%{_datadir}/*

%files octave
%{_libdir}/*
