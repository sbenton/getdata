dnl Copyright (C) 2009-2010 D. V. Wiebe
dnl
dnl llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll
dnl
dnl This file is part of the GetData project.
dnl
dnl GetData is free software; you can redistribute it and/or modify it under
dnl the terms of the GNU Lesser General Public License as published by the
dnl Free Software Foundation; either version 2.1 of the License, or (at your
dnl option) any later version.
dnl
dnl GetData is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with GetData; if not, write to the Free Software Foundation, Inc.,
dnl 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl GD_PYTHON
dnl ---------------------------------------------------------------
dnl Look for python.  Then determine whether we can build extension modules.
AC_DEFUN([GD_PYTHON],
[
last_python=2.7
first_python=$1

if test "x$SEQ" == "xnot found"; then
  if test "x$JOT" == "xnot found"; then
    python_prog_list="python python2"
  else
    python_prog_list="python python2 dnl
    `$JOT -w 'python%.1f' - $last_python $first_python -0.1`" #'
  fi
else
  python_prog_list="python python2 dnl
  `$SEQ -f 'python%.1f' $last_python -0.1 $first_python`" #'
fi

dnl --without-python basically does the same as --disable-python
AC_ARG_WITH([python], AS_HELP_STRING([--with-python=PATH],
            [use the Python interpreter located in PATH [default: autodetect]]),
            [
              case "${withval}" in
                no) have_python="no" ;;
                yes) user_python= ; have_python= ;;
                *) user_python="${withval}"; have_python= ;;
              esac
            ], [ user_python=; have_python= ])

AC_ARG_WITH([python-module-dir], AS_HELP_STRING([--with-python-module-dir=PATH],
      [install the Python bindings into PATH [default: autodetect]]),
      [
        case "${withval}" in
          no) local_python_modpath= ;;
          *) local_python_modpath="${withval}"
        esac
      ], [ local_python_modpath= ])

if test "x${have_python}" != "xno"; then

dnl try to find a sufficiently new python.
if test "x$user_python" != "x"; then
  AC_MSG_CHECKING([whether $user_python version >= $first_python])
  AM_PYTHON_CHECK_VERSION([$user_python], [$first_python],
  [AC_MSG_RESULT([yes])
  PYTHON=$user_python],
  [AC_MSG_RESULT([no])
  PYTHON="not found"])
else
  AC_MSG_CHECKING([for Python interpreter version >= $first_python])
  PYTHON="not found"
  for py in $python_prog_list; do
  _AS_PATH_WALK([$PATH],
  [for exec_ext in '' $ac_executable_extensions; do
    if AS_EXECUTABLE_P(["$as_dir/$py$exec_ext"]); then
      AM_PYTHON_CHECK_VERSION( ["$as_dir/$py$exec_ext"],
      [$first_python], [ PYTHON="$as_dir/$py$exec_ext"; break 3] )
    fi
  done])
  done
  AC_MSG_RESULT([$PYTHON])
fi

if test "x$PYTHON" = "xnot found"; then
  have_python="no"
  PYTHON=
fi
AC_SUBST([PYTHON])

fi

if test "x${have_python}" != "xno"; then
dnl python version
AC_MSG_CHECKING([$PYTHON version])
PYTHON_VERSION=`$PYTHON -c "import sys; print sys.version[[:3]]"`
AC_MSG_RESULT([$PYTHON_VERSION])
AC_SUBST([PYTHON_VERSION])

dnl calculate python CPPFLAGS
AC_MSG_CHECKING([Python includes])
if test -x $PYTHON-config; then
  PYTHON_CPPFLAGS=`$PYTHON-config --includes 2>/dev/null`
else
  python_prefix=`$PYTHON -c "import sys; print sys.prefix"`
  python_exec_prefix=`$PYTHON -c "import sys; print sys.exec_prefix"`
  PYTHON_CPPFLAGS="-I${python_prefix}/include/python${PYTHON_VERSION} -I${python_exec_prefix}/include/python${PYTHON_VERSION}"
fi
AC_MSG_RESULT([$PYTHON_CPPFLAGS])

dnl figure out the platform name
AC_MSG_CHECKING([Python platform name])
PYTHON_PLATFORM=`$PYTHON -c "from distutils import util; print util.get_platform()"`
AC_MSG_RESULT([$PYTHON_PLATFORM])
AC_SUBST([PYTHON_PLATFORM])

dnl calculate the exec prefix
pyexec_prefix=$exec_prefix
test "x$pyexec_prefix" = xNONE && pyexec_prefix=$prefix
test "x$pyexec_prefix" = xNONE && pyexec_prefix=$ac_default_prefix

dnl calculate the extension module directory
AC_MSG_CHECKING([Python extension module directory])
if test "x${local_python_modpath}" = "x"; then
  pythondir=`$PYTHON -c "from distutils import sysconfig; print sysconfig.get_python_lib(1,0,prefix='${pyexec_prefix}')" 2>/dev/null || echo "${pyexec_prefix}/lib/python${PYTHON_VERSION}/site-packages"`
else
  pythondir=$local_python_modpath
fi
AC_SUBST([pythondir])
AC_MSG_RESULT([$pythondir])

fi
])
