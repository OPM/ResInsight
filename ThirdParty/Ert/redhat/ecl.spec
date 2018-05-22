#
# spec file for package ecl
#

%define tag rc1

Name:           ecl
Version:        2017.10
Release:        0
Summary:        ECL library
License:        GPL-3+
Group:          Development/Libraries/C and C++
Url:            http://ert.nr.no
Source0:        https://github.com/Statoil/libecl/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  lapack-devel zlib-devel iputils
BuildRequires:  devtoolset-6-toolchain
%{?!el6:BuildRequires: python-devel numpy}
%{?el6:BuildRequires:  cmake3}
%{?!el6:BuildRequires:  cmake}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       libecl1 = %{version}

%description
libecl is a package for reading and writing the result files from the Eclipse reservoir simulator.

%package -n libecl1
Summary:        ECL library
Group:          System/Libraries

%{?!el6:
%package -n python-ecl
Summary:        ECL - Python bindings
Group:          Python/Libraries
Requires:       libecl1

%description -n python-ecl
libecl is a package for reading and writing the result files from the Eclipse reservoir simulator. This package contains the Python bindings.

%package -n python-cwrap
Summary:        Simplify ctypes based wrapping of C code.
Group:          Python/Libraries

%description -n python-cwrap
Package to simplify ctypes based wrapping of C code.
}

%description -n libecl1
libecl is a package for reading and writing the result files from the Eclipse reservoir simulator.

%package devel
Summary:        Development and header files for libecl
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       lapack-devel
Requires:       libecl1 = %{version}

%description devel
This package contains the development and header files for ecl

%prep
%setup -q -n libecl-release-%{version}-%{tag}

%build
scl enable devtoolset-6 bash
DESTDIR=${RPM_BUILD_ROOT} %{?el6:cmake3} %{?!el6:cmake} -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DBUILD_ECL_SUMMARY=1 %{?el6:-DENABLE_PYTHON=0} %{?!el6:-DENABLE_PYTHON=1}-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-6/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-6/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-6/root/usr/bin/gfortran
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

%post -n libecl1 -p /sbin/ldconfig

%postun -n libecl1 -p /sbin/ldconfig

%files
%doc README.md

%files -n libecl1
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_bindir}/*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/*
%{_datadir}/cmake/*
%{_datadir}/man/*

%{?!el6:
%files -n python-ecl
%defattr(-,root,root,-)
/usr/lib/python2.7/site-packages/ecl/*

%files -n python-cwrap
%defattr(-,root,root,-)
/usr/lib/python2.7/site-packages/cwrap/*
}
