/* Attempt to read FLOAT32 with the opposite endianness */
#include "../src/getdata.h"

#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int BigEndian(void)
{
  union {
    long int li;
    char ch[sizeof(long int)];
  } un;
  un.li = 1;
  return (un.ch[sizeof(long int) - 1] == 1);
}

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";
  const char* data = __TEST__ "dirfile/data";
  char format_data[1000];
  union {
    float f;
    char b[4];
  } u;
  const char x[4] = {0x41, 0x36, 0x40, 0x00};
  u.f = 0;
  float data_data[128];
  int fd, i;
  const int big_endian = BigEndian();

  mkdir(filedir, 0777); 

  sprintf(format_data, "data RAW FLOAT32 1\nENDIAN %s\n", (big_endian)
      ? "little" : "big");

  data_data[0] = 1.5;
  for (fd = 1; fd < 128; ++fd)
    data_data[fd] = data_data[fd - 1] * 1.5;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, data_data, 128 * sizeof(float));
  close(fd);

  DIRFILE* D = dirfile_open(filedir, GD_RDONLY);
  int n = getdata(D, "data", 5, 0, 1, 0, GD_FLOAT32, &u.f);

  if (D->error)
    return 1;
  if (n != 1)
    return 1;

  dirfile_close(D);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  for (i = 0; i < 4; ++i)
    if (x[(big_endian) ? 3 - i : i] != u.b[i])
      return 1;

  return 0;
}
