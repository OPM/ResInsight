#
# spec file for package ert.ecl
#

%define tag final2

Name:           ert.ecl
Version:        2015.10
Release:        0
Summary:        ERT - Ensemble based Reservoir Tool - ECL library
License:        GPL-3+
Group:          Development/Libraries/C and C++
Url:            http://ert.nr.no
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  lapack-devel zlib-devel iputils
BuildRequires:  gcc
%{?!el6:BuildRequires: python-devel numpy}
%{?el6:BuildRequires:  cmake28 devtoolset-3-toolchain}
%{?!el6:BuildRequires:  cmake}
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

%{?!el6:
%package -n python-ert.ecl
Summary:        ERT - Ensemble based Reservoir Tool - Python bindings
Group:          Python/Libraries
Requires:       libert.ecl1 python-cwrap

%description -n python-ert.ecl
ERT - Ensemble based Reservoir Tool is a tool for managing en ensemble
of reservoir models. The initial motivation for creating ERT was a as
tool to do assisted history matching with Ensemble Kalman Filter
(EnKF). This package contains the Python bindings.

%package -n python-cwrap
Summary:        Simplify ctypes based wrapping of C code.
Group:          Python/Libraries

%description -n python-cwrap
Package to simplify ctypes based wrapping of C code.
}

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

%description devel
This package contains the development and header files for ert.ecl

%prep
%setup -q -n ert-release-%{version}-%{tag}

%build
%{?el6:scl enable devtoolset-3 bash}
DESTDIR=${RPM_BUILD_ROOT} %{?el6:cmake28} %{?!el6:cmake} -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DBUILD_ECL_SUMMARY=1 %{?el6:-DBUILD_PYTHON=0} %{?el6:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-3/root/usr/bin/gfortran}
make

%install
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

%{?!el6:
%files -n python-ert.ecl
%defattr(-,root,root,-)
/usr/lib/python2.7/site-packages/ert/*

%files -n python-cwrap
%defattr(-,root,root,-)
/usr/lib/python2.7/site-packages/cwrap/*
}
