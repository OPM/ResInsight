#
# spec file for package opm-common
#

%define tag rc4

Name:           opm-common
Version:        2015.10
Release:        0
Summary:        Open Porous Media - common helpers and buildsystem
License:        GPL-3.0
Group:          Development/Libraries/C and C++
Url:            http://www.opm-project.org/
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  git doxygen bc
%{?el6:BuildRequires: devtoolset-3-toolchain cmake28 boost148-devel}
%{!?el6:BuildRequires: gcc gcc-c++ cmake boost-devel}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
The Open Porous Media (OPM) initiative provides a set of open-source tools centered around the simulation of flow and transport of fluids in porous media. The goal of the initiative is to establish a sustainable environment for the development of an efficient and well-maintained software suite.

%package -n libopm-common1
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common1
This package contains library for opm-common

%package devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description devel
This package contains the development and header files for opm-common

%package doc
Summary:        Documentation files for opm-common
Group:          Documentation
BuildArch:	noarch

%description doc
This package contains the documentation files for opm-common

%prep
%setup -q -n %{name}-release-%{version}-%{tag}

# consider using -DUSE_VERSIONED_DIR=ON if backporting
%build
%{?el6:scl enable devtoolset-3 bash}
%{?el6:cmake28} %{?!el6:cmake} -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo  -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/%{name}-%{version} -DUSE_RUNPATH=OFF %{?el6:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/gfortran -DBOOST_LIBRARYDIR=%{_libdir}/boost148 -DBOOST_INCLUDEDIR=%{_includedir}/boost148}
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

%files
%doc README.md

%files doc
%{_docdir}/*

%files -n libopm-common1
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/dunecontrol/*
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_datadir}/cmake/*
%{_datadir}/opm/*
%{_libdir}/*.so
