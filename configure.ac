AC_PREREQ([2.63])
AC_INIT([eos-codecs-manager], [1.0.0], [mario@endlessm.com], [eos-codecs-manager], [http://www.endlessm.com/])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 -Wno-portability foreign no-define tar-ustar no-dist-gzip dist-xz])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

PKG_PROG_PKG_CONFIG


PKG_CHECK_EXISTS([systemd], [],
	[AC_MSG_ERROR([systemd required to build $PACKAGE_NAME])])

AC_ARG_WITH([systemdunitdir],
	[AS_HELP_STRING([--with-systemdunitdir],
			[systemd unit file directory])],
	[systemdunitdir=$withval],
	[systemdunitdir=`$PKG_CONFIG --variable=systemdsystemunitdir systemd`])
AS_IF([test "x$systemdunitdir" = x],
	[AC_MSG_ERROR([no path for systemd unit directory])])
AC_SUBST([systemdunitdir])

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT

echo "
    Endless Codecs Manager $VERSION
"
