#
# spec file for package opm-common
#

%define tag final

Name:           opm-common
Version:        2018.10
Release:        0
Summary:        Open Porous Media - common helpers and buildsystem
License:        GPL-3.0
Group:          Development/Libraries/C and C++
Url:            http://www.opm-project.org/
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  git doxygen bc openmpi-devel mpich-devel
%{?!el8:BuildRequires: devtoolset-8-toolchain}
%{?el8:BuildRequires: boost-devel}
%{?!el8:BuildRequires: boost148-devel}
BuildRequires: cmake3
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
The Open Porous Media (OPM) initiative provides a set of open-source tools centered around the simulation of flow and transport of fluids in porous media. The goal of the initiative is to establish a sustainable environment for the development of an efficient and well-maintained software suite.

%package -n libopm-common1
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common1
This package contains library for opm-common

%package -n libopm-common1-openmpi
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common1-openmpi
This package contains library for opm-common

%package -n libopm-common1-mpich
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common1-mpich
This package contains library for opm-common

%package devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description devel
This package contains the development and header files for opm-common

%package openmpi-devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       libopm-common1-openmpi = %{version}

%description openmpi-devel
This package contains the development and header files for opm-common

%package mpich-devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       libopm-common1-mpich = %{version}

%description mpich-devel
This package contains the development and header files for opm-common

%package bin
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       %{name} = %{version}

%description bin
This package the applications for opm-common

%package openmpi-bin
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       libopm-common1-openmpi = %{version}

%description openmpi-bin
This package the applications for opm-common

%package mpich-bin
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       libopm-common1-mpich = %{version}

%description mpich-bin
This package the applications for opm-common

%package doc
Summary:        Documentation files for opm-common
Group:          Documentation
BuildArch:	noarch

%description doc
This package contains the documentation files for opm-common

%global debug_package %{nil}

%prep
%setup -q -n %{name}-release-%{version}-%{tag}

# consider using -DUSE_VERSIONED_DIR=ON if backporting
%build
mkdir serial
cd serial
cmake3 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/%{name}-%{version} -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF %{!?el8:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gfortran} %{?!el8:-DBOOST_LIBRARYDIR=%{_libdir}/boost148 -DBOOST_INCLUDEDIR=%{_includedir}/boost148} ..
make %{?_smp_mflags}
make test
cd ..

mkdir openmpi
cd openmpi
%{?el6:module load openmpi-x86_64}
%{?!el6:module load mpi/openmpi-x86_64}
cmake3 -DUSE_MPI=1 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo  -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix}/lib64/openmpi -DCMAKE_INSTALL_LIBDIR=lib -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF %{?!el8:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gfortran} %{?!el8:-DBOOST_LIBRARYDIR=%{_libdir}/boost148 -DBOOST_INCLUDEDIR=%{_includedir}/boost148} ..
make %{?_smp_mflags}
make test
cd ..

mkdir mpich
cd mpich
%{?el6:module rm openmpi-x86_64}
%{?el6:module load mpich-x86_64}
%{?!el6:module rm mpi/openmpi-x86_64}
%{?!el6:module load mpi/mpich-x86_64}
cmake3 -DUSE_MPI=1 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo  -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix}/lib64/mpich -DCMAKE_INSTALL_LIBDIR=lib -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF %{?!el8:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gfortran} %{?!el8:-DBOOST_LIBRARYDIR=%{_libdir}/boost148 -DBOOST_INCLUDEDIR=%{_includedir}/boost148} ..
make %{?_smp_mflags}
make test

%install
cd serial
make install DESTDIR=${RPM_BUILD_ROOT}
make install-html DESTDIR=${RPM_BUILD_ROOT}
cd ..
cd openmpi
make install DESTDIR=${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}/usr/include/openmpi-x86_64/
mv ${RPM_BUILD_ROOT}/usr/lib64/openmpi/include/* ${RPM_BUILD_ROOT}/usr/include/openmpi-x86_64/
cd ..

cd mpich
make install DESTDIR=${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}/usr/include/mpich-x86_64/
mv ${RPM_BUILD_ROOT}/usr/lib64/mpich/include/* ${RPM_BUILD_ROOT}/usr/include/mpich-x86_64/

%clean
rm -rf %{buildroot}

%files
%doc README.md

%files doc
%{_docdir}/*

%files bin
%{_bindir}/*

%files openmpi-bin
%{_libdir}/openmpi/bin/*

%files mpich-bin
%{_libdir}/mpich/bin/*

%files -n libopm-common1
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files -n libopm-common1-openmpi
%defattr(-,root,root,-)
%{_libdir}/openmpi/lib/*.so.*

%files -n libopm-common1-mpich
%defattr(-,root,root,-)
%{_libdir}/mpich/lib/*.so.*

%files devel
%defattr(-,root,root,-)
/usr/lib/dunecontrol/*
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_datadir}/cmake/*
%{_datadir}/opm/*
%{_libdir}/*.so

%files openmpi-devel
%defattr(-,root,root,-)
%{_libdir}/openmpi/lib/dunecontrol/*
%{_libdir}/openmpi/lib/pkgconfig/*
%{_includedir}/openmpi-x86_64/*
%{_libdir}/openmpi/share/cmake/*
%{_libdir}/openmpi/share/opm/*
%{_libdir}/openmpi/lib/*.so

%files mpich-devel
%defattr(-,root,root,-)
%{_libdir}/mpich/lib/dunecontrol/*
%{_libdir}/mpich/lib/pkgconfig/*
%{_includedir}/mpich-x86_64/*
%{_libdir}/mpich/share/cmake/*
%{_libdir}/mpich/share/opm/*
%{_libdir}/mpich/lib/*.so
