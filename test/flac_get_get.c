/* Copyright (C) 2015 D. V. Wiebe
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

int main(void)
{
#if !defined USE_FLAC || !defined TEST_FLAC
  return 77; /* skip test */
#else
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  const char *flacdata = "dirfile/data.flac";
  const char *format_data = "data RAW UINT16 8\n";
  uint16_t c1[8], c2[8];
  char command[4096];
  uint16_t data_data[256];
  int fd, i, n1, e1, e2, n2, e3, r = 0;
  DIRFILE *D;

  memset(c1, 0, 16);
  memset(c2, 0, 16);
  rmdirfile();
  mkdir(filedir, 0777);

  for (fd = 0; fd < 256; ++fd)
    data_data[fd] = (unsigned char)fd;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  write(fd, data_data, 256 * sizeof(uint16_t));
  close(fd);

  snprintf(command, 4096,
      "%s --endian=little --silent --sample-rate=1 --channels=1 --bps=16 "
      "--sign=signed --delete-input-file %s > /dev/null", FLAC, data);
  if (gd_system(command))
    return 1;

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  n1 = gd_getdata(D, "data", 5, 0, 1, 0, GD_UINT16, c1);
  CHECKI(n1, 8);
  e1 = gd_error(D);
  CHECKI(e1, 0);

  e2 = gd_close(D);
  CHECKI(e2, 0);

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  n2 = gd_getdata(D, "data", 5, 0, 1, 0, GD_UINT16, c2);
  e3 = gd_error(D);
  CHECKI(e3, 0);
  CHECKI(n2, 8);
  for (i = 0; i < 8; ++i) {
    CHECKIi(i,c1[i], 40 + i);
    CHECKIi(i,c2[i], 40 + i);
  }

  gd_discard(D);

  unlink(flacdata);
  unlink(format);
  rmdir(filedir);

  return r;
#endif
}
