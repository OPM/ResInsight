#
# spec file for package opm-common
#

%define tag final
%define rtype release
%define toolset devtoolset-9
%define build_openmpi 1
%define build_openmpi3 1
%define build_mpich 1

Name:           opm-common
Version:        2018.10
Release:        0
Summary:        Open Porous Media - common helpers and buildsystem
License:        GPL-3.0
Group:          Development/Libraries/C and C++
Url:            http://www.opm-project.org/
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  git doxygen bc
BuildRequires: %{toolset}-toolchain
BuildRequires: boost-devel graphviz
BuildRequires: cmake3 python3-devel python36-numpy
BuildRequires: python36-setuptools_scm python36-pytest-runner
%if %{build_openmpi}
BuildRequires: openmpi-devel
%endif

%if %{build_openmpi3}
BuildRequires: openmpi3-devel
%endif

%if %{build_mpich}
BuildRequires: mpich-devel
%endif

BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
The Open Porous Media (OPM) initiative provides a set of open-source tools centered around the simulation of flow and transport of fluids in porous media. The goal of the initiative is to establish a sustainable environment for the development of an efficient and well-maintained software suite.

%package -n libopm-common%{opm_package_version}
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common%{opm_package_version}
This package contains library for opm-common

%package devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       libopm-common%{opm_package_version} = %{version}

%description devel
This package contains the development and header files for opm-common

%package -n python3-opm-common%{opm_package_version}
Summary: OPM-common - python library
Group:          Python/Libraries
Requires:       libopm-common = %{version}

%description -n python3-opm-common%{opm_package_version}
This package contains the python library for opm-common

%package bin%{opm_package_version}
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       %{name} = %{version}

%description bin%{opm_package_version}
This package the applications for opm-common

%package doc
Summary:        Documentation files for opm-common
Group:          Documentation
BuildArch:	noarch

%description doc
This package contains the documentation files for opm-common

%if %{build_openmpi}
%package -n libopm-common-openmpi%{opm_package_version}
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common-openmpi%{opm_package_version}
This package contains library for opm-common

%package openmpi-devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       libopm-common-openmpi%{opm_package_version} = %{version}

%description openmpi-devel
This package contains the development and header files for opm-common

%package openmpi-bin%{opm_package_version}
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       libopm-common-openmpi%{opm_package_version} = %{version}

%description openmpi-bin%{opm_package_version}
This package the applications for opm-common
%endif

%if %{build_openmpi3}
%package -n libopm-common-openmpi3%{opm_package_version}
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common-openmpi3%{opm_package_version}
This package contains library for opm-common

%package openmpi3-devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       libopm-common-openmpi3%{opm_package_version} = %{version}

%description openmpi3-devel
This package contains the development and header files for opm-common

%package openmpi3-bin%{opm_package_version}
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       libopm-common-openmpi3%{opm_package_version} = %{version}

%description openmpi3-bin%{opm_package_version}
This package the applications for opm-common
%endif

%if %{build_mpich}
%package -n libopm-common-mpich%{opm_package_version}
Summary: OPM-common - library
Group:          System/Libraries

%description -n libopm-common-mpich%{opm_package_version}
This package contains library for opm-common

%package mpich-devel
Summary:        Development and header files for opm-common
Group:          Development/Libraries/C and C++
Requires:       libopm-common-mpich%{opm_package_version} = %{version}

%description mpich-devel
This package contains the development and header files for opm-common

%package mpich-bin%{opm_package_version}
Summary:        Applications for opm-common
Group:          System/Binaries
Requires:       libopm-common-mpich%{opm_package_version} = %{version}

%description mpich-bin%{opm_package_version}
This package the applications for opm-common
%endif

%global debug_package %{nil}

%prep
%setup -q -n %{name}-testing-%{version}-%{tag}

# consider using -DUSE_VERSIONED_DIR=ON if backporting
%build
rm -f python/pybind11/tools/mkdoc.py
mkdir serial
pushd serial
scl enable %{toolset} 'cmake3 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/%{name}-%{version} -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF -DOPM_ENABLE_PYTHON=1 -DOPM_ENABLE_EMBEDDED_PYTHON=1 -DOPM_INSTALL_PYTHON=1 -DBUILD_TESTING=0 ..'
scl enable %{toolset} 'make %{?_smp_mflags}'
popd

%if %{build_openmpi}
mkdir openmpi
pushd openmpi
module load mpi/openmpi-x86_64
scl enable %{toolset} 'cmake3 -DUSE_MPI=1 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix}/lib64/openmpi -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=/usr/include/openmpi-x86_64 -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF -DOPM_ENABLE_PYTHON=1 -DOPM_ENABLE_EMBEDDED_PYTHON=1 -DBUILD_TESTING=0 ..'
scl enable %{toolset} 'make %{?_smp_mflags}'
module unload mpi/openmpi-x86_64
popd
%endif

%if %{build_openmpi3}
mkdir openmpi3
pushd openmpi3
module load mpi/openmpi3-x86_64
scl enable %{toolset} 'cmake3 -DUSE_MPI=1 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix}/lib64/openmpi3 -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=/usr/include/openmpi3-x86_64 -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF -DOPM_ENABLE_PYTHON=1 -DOPM_ENABLE_EMBEDDED_PYTHON=1 -DBUILD_TESTING=0 ..'
scl enable %{toolset} 'make %{?_smp_mflags}'
module unload mpi/openmpi3-x86_64
popd
%endif

%if %{build_mpich}
mkdir mpich
pushd mpich
module load mpi/mpich-x86_64
scl enable %{toolset} 'cmake3 -DUSE_MPI=1 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DSTRIP_DEBUGGING_SYMBOLS=ON -DCMAKE_INSTALL_PREFIX=%{_prefix}/lib64/mpich -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=/usr/include/mpich-x86_64 -DUSE_RUNPATH=OFF -DWITH_NATIVE=OFF -DOPM_ENABLE_PYTHON=1 -DOPM_ENABLE_EMBEDDED_PYTHON=1 -DBUILD_TESTING=0 ..'
scl enable %{toolset} 'make %{?_smp_mflags}'
module unload mpi/mpich-x86_64
popd
%endif

%install
scl enable %{toolset} 'make install DESTDIR=%{buildroot} -C serial'
scl enable %{toolset} 'make install-html DESTDIR=%{buildroot} -C serial'

%if %{build_openmpi}
scl enable %{toolset} 'make install DESTDIR=%{buildroot} -C openmpi'
mv %{buildroot}/usr/lib64/openmpi/include/* %{buildroot}/usr/include/openmpi-x86_64/
%endif

%if %{build_openmpi3}
scl enable %{toolset} 'make install DESTDIR=%{buildroot} -C openmpi3'
mv %{buildroot}/usr/lib64/openmpi3/include/* %{buildroot}/usr/include/openmpi3-x86_64/
%endif

%if %{build_mpich}
scl enable %{toolset} 'make install DESTDIR=%{buildroot} -C mpich'
mv %{buildroot}/usr/lib64/mpich/include/* %{buildroot}/usr/include/mpich-x86_64/
%endif

%clean
rm -rf %{buildroot}

%define _unpackaged_files_terminate_build 0

%post -n libopm-common%{opm_package_version} -p /sbin/ldconfig
%postun -n libopm-common%{opm_package_version} -p /sbin/ldconfig
%post -n python3-opm-common%{opm_package_version} -p /sbin/ldconfig
%postun -n python3-opm-common%{opm_package_version} -p /sbin/ldconfig

%if %{build_openmpi}
%post -n libopm-common-openmpi%{opm_package_version} -p /sbin/ldconfig
%postun -n libopm-common-openmpi%{opm_package_version} -p /sbin/ldconfig
%endif

%if %{build_openmpi3}
%post -n libopm-common-openmpi3%{opm_package_version} -p /sbin/ldconfig
%postun -n libopm-common-openmpi3%{opm_package_version} -p /sbin/ldconfig
%endif

%if %{build_mpich}
%post -n libopm-common-mpich%{opm_package_version} -p /sbin/ldconfig
%postun -n libopm-common-mpich%{opm_package_version} -p /sbin/ldconfig
%endif

%files
%doc README.md

%files doc
%{_docdir}/*

%files bin%{opm_package_version}
%{_bindir}/*
%{_datadir}/man/*

%files -n libopm-common%{opm_package_version}
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
/usr/lib/dunecontrol/*
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_datadir}/cmake/*
%{_datadir}/opm/*
%{_libdir}/*.so

%files -n python3-opm-common%{opm_package_version}
%{_prefix}/lib/python3.6/site-packages/opm/*

%if %{build_openmpi}
%files openmpi-bin%{opm_package_version}
%{_libdir}/openmpi/bin/*
%{_libdir}/openmpi/share/man/*

%files -n libopm-common-openmpi%{opm_package_version}
%defattr(-,root,root,-)
%{_libdir}/openmpi/lib/*.so.*

%files openmpi-devel
%defattr(-,root,root,-)
%{_libdir}/openmpi/lib/dunecontrol/*
%{_libdir}/openmpi/lib/pkgconfig/*
%{_includedir}/openmpi-x86_64/*
%{_libdir}/openmpi/share/cmake/*
%{_libdir}/openmpi/share/doc/*
%{_libdir}/openmpi/share/opm/*
%{_libdir}/openmpi/lib/*.so
%endif

%if %{build_openmpi3}
%files openmpi3-bin%{opm_package_version}
%{_libdir}/openmpi3/bin/*
%{_libdir}/openmpi3/share/man/*

%files -n libopm-common-openmpi3%{opm_package_version}
%defattr(-,root,root,-)
%{_libdir}/openmpi3/lib/*.so.*

%files openmpi3-devel
%defattr(-,root,root,-)
%{_libdir}/openmpi3/lib/dunecontrol/*
%{_libdir}/openmpi3/lib/pkgconfig/*
%{_includedir}/openmpi3-x86_64/*
%{_libdir}/openmpi3/share/cmake/*
%{_libdir}/openmpi3/share/doc/*
%{_libdir}/openmpi3/share/opm/*
%{_libdir}/openmpi3/lib/*.so
%endif

%if %{build_mpich}
%files mpich-bin%{opm_package_version}
%{_libdir}/mpich/bin/*
%{_libdir}/mpich/share/man/*

%files -n libopm-common-mpich%{opm_package_version}
%defattr(-,root,root,-)
%{_libdir}/mpich/lib/*.so.*

%files mpich-devel
%defattr(-,root,root,-)
%{_libdir}/mpich/lib/dunecontrol/*
%{_libdir}/mpich/lib/pkgconfig/*
%{_includedir}/mpich-x86_64/*
%{_libdir}/mpich/share/doc/*
%{_libdir}/mpich/share/cmake/*
%{_libdir}/mpich/share/opm/*
%{_libdir}/mpich/lib/*.so
%endif
