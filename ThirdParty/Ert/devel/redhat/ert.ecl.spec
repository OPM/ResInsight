#
# spec file for package ert.ecl
#

Name:           ert.ecl
Version:        1.0
Release:        1
Summary:        ERT - Ensemble based Reservoir Tool - ECL library
License:        GPL-3+
Group:          Development/Libraries/C and C++
Url:            http://ert.nr.no
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  lapack-devel
BuildRequires:  gcc
BuildRequires:  cmake28 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       libert.ecl1 = %{version}

%description
ERT - Ensemble based Reservoir Tool is a tool for managing en ensemble
of reservoir models. The initial motivation for creating ERT was a as
tool to do assisted history matching with Ensemble Kalman Filter
(EnKF).

%package -n libert.ecl1
Summary:        ERT - Ensemble based Reservoir Tool - ECL library
Group:          System/Libraries

%description -n libert.ecl1
ERT - Ensemble based Reservoir Tool is a tool for managing en ensemble
of reservoir models. The initial motivation for creating ERT was a as
tool to do assisted history matching with Ensemble Kalman Filter
(EnKF).

%package devel
Summary:        Development and header files for libert.ecl
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       lapack-devel
Requires:       libert.ecl1 = %{version}
BuildArch:      noarch

%description devel
This package contains the development and header files for ert.ecl

%prep
%setup -q

%build
cd devel
cmake28 -DSHARED_LIB=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DBUILD_ECL_SUMMARY=1
make

%install
cd devel
make install DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

%post -n libert.ecl1 -p /sbin/ldconfig

%postun -n libert.ecl1 -p /sbin/ldconfig

%files
%doc README

%files -n libert.ecl1
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_bindir}/*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/*

%changelog
* Tue Feb 19 2013 Arne Morten Kvarving <arne.morten.kvarving@sintef.no> 1.0-1
- Mark -devel package as architecture independent
