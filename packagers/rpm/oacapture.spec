Name:           oacapture
Version:        1.1.0
Release:        1
Summary:        planetary capture application
License:        GPL-3
URL:            http://www.openastroproject.org/
Prefix:         %{_prefix}
Provides:       oacapture = %{version}-%{release}
Obsoletes:      oacapture <= 1.0.0
Requires:       libtiff
Requires:       libdc1394
Requires:       systemd
Requires:       cfitsio
Requires:       qt
Requires:       qt-x11
Requires:       xz-libs
Requires:       libjpeg-turbo
Requires:       libftdi
Requires:       libasicamera
Requires:       libuvc
Requires:       libpng
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  systemd-devel
BuildRequires:  libv4l-devel
BuildRequires:  libtiff-devel
BuildRequires:  libdc1394-devel
BuildRequires:  systemd-devel
BuildRequires:  cfitsio-devel
BuildRequires:  xz-devel
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  libftdi-devel
BuildRequires:  libasicamera-devel
BuildRequires:  libuvc-devel
BuildRequires:  libpng-devel
BuildRequires:  qt
BuildRequires:  qt-config
BuildRequires:  qt-devel
BuildRequires:  qt-x11
BuildRequires:  yasm
BuildRequires:  autoconf
BuildRequires:  libtool
Source:         oacapture-%{version}.tar.bz2

%description
An application and associated tools for controlling cameras for planetary
capture and related astronomy imaging

%undefine _hardened_build
%define _unpackaged_files_terminate_build 0

%{?systemd_requires}
BuildRequires: systemd
%prep
%setup -q

%build
%configure
%make_build

%install
%make_install

%files
/usr/bin/oacapture
/lib/udev/rules.d/*
/lib/firmware/qhy/*

%post
%systemd_post udev.service

%postun
%systemd_postun udev.service