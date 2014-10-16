/* Copyright (C) 2013 D. V. Wiebe
 *
 ***************************************************************************
 *
 * This file is part of the GetData project.
 *
 * GetData is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * GetData is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with GetData; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "test.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  const char *table = "dirfile/table";
  const char *format_data = "linterp LINTERP data ./table\ndata RAW UINT8 1\n";
  unsigned char c = 0;
  unsigned char data_data[64];
  int fd, n1, error1, n2, error2, r = 0;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0777);

  for (fd = 0; fd < 64; ++fd)
    data_data[fd] = (unsigned char)fd;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(table, O_CREAT | O_EXCL | O_WRONLY, 0666);
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  write(fd, data_data, 64);
  close(fd);

  D = gd_open(filedir, GD_RDONLY);
  n1 = gd_getdata(D, "linterp", 5, 0, 1, 0, GD_UINT8, &c);
  error1 = gd_error(D);
  n2 = gd_getdata(D, "linterp", 5, 0, 1, 0, GD_UINT8, &c);
  error2 = gd_error(D);

  gd_discard(D);

  unlink(data);
  unlink(format);
  unlink(table);
  rmdir(filedir);

  CHECKI(n1, 0);
  CHECKI(error1, GD_E_LUT);
  CHECKI(n2, 0);
  CHECKI(error2, GD_E_LUT);

  return r;
}
