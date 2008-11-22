/* (C) 2008 D. V. Wiebe
 *
 ***************************************************************************
 *
 * This file is part of the GetData project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GetData is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with GetData; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "internal.h"

#ifdef STDC_HEADERS
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#endif

/* encoding schemas */
const struct encoding_t encode[] = {
  { GD_UNENCODED, "", 1,
    &_GD_RawOpen, &_GD_RawClose, &_GD_GenericTouch, &_GD_RawSeek, &_GD_RawRead,
    &_GD_RawSize, &_GD_RawWrite, &_GD_RawSync, &_GD_GenericUnlink, &_GD_RawTemp
  },
  { GD_TEXT_ENCODED, ".txt", 0,
    &_GD_AsciiOpen, &_GD_AsciiClose, &_GD_GenericTouch, &_GD_AsciiSeek,
    &_GD_AsciiRead, &_GD_AsciiSize, &_GD_AsciiWrite, &_GD_AsciiSync,
    &_GD_GenericUnlink, &_GD_AsciiTemp },
  { GD_SLIM_ENCODED, ".slm", 1,
#ifdef USE_SLIMLIB
    &_GD_SlimOpen, &_GD_SlimClose, NULL /* TOUCH */, &_GD_SlimSeek,
    &_GD_SlimRead, &_GD_SlimSize, NULL /* WRITE */, NULL /* SYNC */,
    &_GD_GenericUnlink, NULL /* TEMP */
#else
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#endif
  },
  { GD_ENC_UNSUPPORTED, "", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL },
};

/* Figure out the encoding scheme */
static unsigned int _GD_ResolveEncoding(const char* name, unsigned int scheme,
    struct _gd_raw_file *file)
{
  char candidate[FILENAME_MAX];
  char* ptr;
  int i, len = strlen(name);
  struct stat64 statbuf;

  dtrace("\"%s\", 0x%08x, %p", name, scheme, e);

  strcpy(candidate, name);
  ptr = candidate + len;
  len = FILENAME_MAX - len;

  for (i = 0; encode[i].scheme != GD_ENC_UNSUPPORTED; i++) {
    if (scheme == GD_AUTO_ENCODED || scheme == encode[i].scheme) {
      strcpy(ptr, encode[i].ext);

      if (stat64(candidate, &statbuf) == 0) 
        if (S_ISREG(statbuf.st_mode)) {
          if (file != NULL)
            file->encoding = i;
          dreturn("%08x", encode[i].scheme);
          return encode[i].scheme;
        }
    }
  }

  if (scheme != 0 && file != NULL) {
    for (i = 0; encode[i].scheme != GD_ENC_UNSUPPORTED; i++)
      if (scheme == encode[i].scheme) {
        file->encoding = i;
        dreturn("0x%08x", encode[i].scheme);
        return encode[i].scheme;;
      }
  }

  dreturn("%08x", GD_AUTO_ENCODED);
  return GD_AUTO_ENCODED;
}

static inline int _GD_MissingFramework(int encoding, unsigned int funcs)
{
  dtrace("%x, %x", encoding, funcs);

  int ret =
    (funcs & GD_EF_OPEN   && encode[encoding].open   == NULL) ||
    (funcs & GD_EF_CLOSE  && encode[encoding].close  == NULL) ||
    (funcs & GD_EF_TOUCH  && encode[encoding].touch  == NULL) ||
    (funcs & GD_EF_SEEK   && encode[encoding].seek   == NULL) ||
    (funcs & GD_EF_READ   && encode[encoding].read   == NULL) ||
    (funcs & GD_EF_SIZE   && encode[encoding].size   == NULL) ||
    (funcs & GD_EF_WRITE  && encode[encoding].write  == NULL) ||
    (funcs & GD_EF_SYNC   && encode[encoding].sync   == NULL) ||
    (funcs & GD_EF_UNLINK && encode[encoding].unlink == NULL) ||
    (funcs & GD_EF_TEMP   && encode[encoding].temp   == NULL);

  dreturn("%i", ret);
  return ret;
}

int _GD_Supports(DIRFILE* D, gd_entry_t* E, unsigned int funcs)
{
  dtrace("%p, %p, %x", D, E, funcs);

  /* Figure out the dirfile encoding type, if required */
  if (D->fragment[E->fragment_index].encoding == GD_AUTO_ENCODED) {
    D->fragment[E->fragment_index].encoding =
      _GD_ResolveEncoding(E->e->filebase, GD_AUTO_ENCODED, E->e->file);
  }

  /* If the encoding scheme is unknown, complain */
  if (D->fragment[E->fragment_index].encoding == GD_AUTO_ENCODED) {
    _GD_SetError(D, GD_E_UNKNOWN_ENCODING, 0, NULL, 0, NULL);
    dreturn("%i", 0);
    return 0;
  }

  /* Figure out the encoding subtype, if required */
  if (E->e->file[0].encoding == GD_ENC_UNKNOWN)
    _GD_ResolveEncoding(E->e->filebase, D->fragment[E->fragment_index].encoding,
        E->e->file);

  /* check for our function(s) */
  if (_GD_MissingFramework(E->e->file[0].encoding, funcs)) {
    _GD_SetError(D, GD_E_UNSUPPORTED, 0, NULL, 0, NULL);
    dreturn("%i", 0);
    return 0;
  }

  dreturn("%i", 1);
  return 1;
}

int _GD_SetEncodedName(struct _gd_raw_file* file, const char* base, int temp)
{
  dtrace("%p, \"%s\", %i", file, base, temp);

  if (file->name == NULL) {
    file->name = malloc(FILENAME_MAX);
    if (file->name == NULL) {
      dreturn("%i", -1);
      return -1;
    }

    snprintf(file->name, FILENAME_MAX, "%s%s", base, temp ? "_XXXXXX" :
        encode[file->encoding].ext);
  }

  dreturn("%i", 0);
  return 0;
}

static void _GD_RecodeFragment(DIRFILE* D, unsigned int encoding, int fragment,
    int move)
{
  unsigned int i, n_raw = 0;

  dtrace("%p, %u, %i, %i\n", D, encoding, fragment, move);

  /* check protection */
  if (D->fragment[fragment].protection & GD_PROTECT_FORMAT) {
    _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
        D->fragment[fragment].cname);
    dreturnvoid();
    return;
  }

  if (move && encoding != D->fragment[fragment].encoding) {
    gd_entry_t **raw_entry = malloc(sizeof(gd_entry_t*) * D->n_entries);
    const struct encoding_t* enc_in;
    const struct encoding_t* enc_out;
    void *buffer = malloc(BUFFER_SIZE);
    size_t ns;
    ssize_t nread, nwrote;
    int subencoding = GD_ENC_UNSUPPORTED;

    if (raw_entry == NULL || buffer == NULL) {
      _GD_SetError(D, GD_E_ALLOC, 0, NULL, 0, NULL);
      dreturnvoid();
      return;
    }

    /* Figure out the subencoding scheme */
    for (i = 0; encode[i].scheme != GD_ENC_UNSUPPORTED; i++)
      if (encode[i].scheme == encoding) {
        subencoding = i;
        break;
      }

    /* Check encoding framework for output */
    if (_GD_MissingFramework(subencoding, GD_EF_OPEN | GD_EF_CLOSE |
              GD_EF_SEEK | GD_EF_WRITE | GD_EF_SYNC | GD_EF_UNLINK))
    {
      _GD_SetError(D, GD_E_UNSUPPORTED, 0, NULL, 0, NULL);
      dreturnvoid();
      return;
    }

    enc_out = encode + subencoding;

    /* Because it may fail, the move must occur out-of-place and then be copied
     * back over the affected files once success is assured */
    for (i = 0; i < D->n_entries; ++i)
      if (D->entry[i]->fragment_index == fragment &&
          D->entry[i]->field_type == GD_RAW_ENTRY)
      {
        if (!_GD_Supports(D, D->entry[i], GD_EF_OPEN | GD_EF_CLOSE |
              GD_EF_SEEK | GD_EF_READ | GD_EF_UNLINK))
          continue;

        /* check data protection */
        if (D->fragment[fragment].protection & GD_PROTECT_DATA) {
          _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
              D->fragment[fragment].cname);
          break;
        }

        enc_in = encode + raw_entry[i]->e->file[0].encoding;
        ns = BUFFER_SIZE / raw_entry[i]->e->size;

        /* add this raw field to the list */
        raw_entry[n_raw++] = D->entry[i];

        /* Create the new file */
        if ((*enc_in->open)(raw_entry[i]->e->file + 1,
              raw_entry[i]->e->filebase, GD_RDWR, 1))
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name, errno,
              NULL);
          break;
        }

        /* Open the input file, if required */
        if (raw_entry[i]->e->file[0].fp == -1 &&
            (*enc_in->open)(raw_entry[i]->e->file, raw_entry[i]->e->filebase, 0,
              0))
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[0].name, errno,
              NULL);
          break;
        }

        /* Seek both files to the beginning */
        if ((*enc_in->seek)(raw_entry[i]->e->file, 0, raw_entry[i]->data_type,
              0) == -1)
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[0].name,
              errno, NULL);
          break;
        }

        if ((*enc_in->seek)(raw_entry[i]->e->file + 1, 0,
              raw_entry[i]->data_type, 1) == -1)
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
              errno, NULL);
          break;
        }

        /* Now copy the old file to the new file */
        for (;;) {
          nread = (*enc_in->read)(raw_entry[i]->e->file, buffer,
              raw_entry[i]->data_type, ns);

          if (nread < 0) {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
          }

          if (nread == 0)
            break;

          nwrote = (*enc_out->write)(raw_entry[i]->e->file + 1, buffer,
              raw_entry[i]->data_type, nread);

          if (nwrote < nread) {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
          }
        }

        /* Well, I suppose the copy worked.  Close both files */
        if ((*enc_in->close)(raw_entry[i]->e->file) ||
            (*enc_out->sync)(raw_entry[i]->e->file + 1) ||
            (*enc_out->close)(raw_entry[i]->e->file + 1))
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
              errno, NULL);
          break;
        }
      }

    /* If successful, move the temporary file over the old file, otherwise
     * remove the temporary files */
    for (i = 0; i < n_raw; ++i)
      if ((*encode[raw_entry[i]->e->file[0].encoding].temp)
          (raw_entry[i]->e->file, (D->error) ? GD_TEMP_DESTROY : GD_TEMP_MOVE))
        _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[0].name,
            errno, NULL);

    free(raw_entry);
    free(buffer);

    if (D->error) {
      dreturnvoid();
      return;
    }
  } else {
    for (i = 0; i < D->n_entries; ++i)
      if (D->entry[i]->fragment_index == fragment &&
          D->entry[i]->field_type == GD_RAW_ENTRY)
      {
        /* close the old file */
        if (D->entry[i]->e->file[0].fp != -1 &&
            (*encode[D->entry[i]->e->file[0].encoding].close)
            (D->entry[i]->e->file))
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, D->entry[i]->e->file[1].name,
              errno, NULL);
          break;
        }
        /* reset encoding subscheme. */
        D->entry[i]->e->file[0].encoding = GD_ENC_UNKNOWN;
      }
  }

  D->fragment[fragment].encoding = encoding;
  D->fragment[fragment].modified = 1;

  dreturnvoid();
}

int put_encoding(DIRFILE* D, unsigned int encoding, int fragment, int move)
{
  int i;

  dtrace("%p, %u, %i, %i\n", D, encoding, fragment, move);

  if (D->flags & GD_INVALID) {/* don't crash */
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if ((D->flags & GD_ACCMODE) != GD_RDWR) {
    _GD_SetError(D, GD_E_ACCMODE, 0, NULL, 0, NULL);
    dreturn("%zi", -1);
    return -1;
  }

  if (fragment < GD_ALL_FRAGMENTS || fragment >= D->n_fragment) {
    _GD_SetError(D, GD_E_BAD_INDEX, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if (encoding != GD_UNENCODED && encoding != GD_TEXT_ENCODED &&
      encoding != GD_SLIM_ENCODED)
  {
    _GD_SetError(D, GD_E_UNKNOWN_ENCODING, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  if (fragment == GD_ALL_FRAGMENTS) {
    for (i = 0; i < D->n_fragment; ++i) {
      _GD_RecodeFragment(D, encoding, i, move);

      if (D->error)
        break;
    }
  } else
    _GD_RecodeFragment(D, encoding, fragment, move);

  dreturn("%i", (D->error) ? -1 : 0);
  return (D->error) ? -1 : 0;
}

unsigned int get_encoding(DIRFILE* D, int fragment)
{
  dtrace("%p, %i\n", D, fragment);

  if (D->flags & GD_INVALID) {/* don't crash */
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if (fragment < 0 || fragment >= D->n_fragment) {
    _GD_SetError(D, GD_E_BAD_INDEX, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  dreturn("%i", (long long)D->fragment[fragment].encoding);
  return D->fragment[fragment].encoding;
}

int _GD_GenericTouch(struct _gd_raw_file* file, const char* base)
{
  dtrace("%p, \"%s\"", file, base);

  if (_GD_SetEncodedName(file, base, 0)) {
    dreturn("%i", -1);
    return -1;
  }

  int fd = open(file->name, O_RDWR | O_CREAT | O_TRUNC, 0666);

  if (fd != -1)
    fd = close(fd);

  dreturn("%i", fd);
  return fd;
}

int _GD_GenericUnlink(struct _gd_raw_file* file, const char* base)
{
  dtrace("%p, \"%s\"", file, base);

  if (_GD_SetEncodedName(file, base, 0)) {
    dreturn("%i", -1);
    return -1;
  }

  int r = unlink(file->name);

  dreturn("%i", r);
  return r;
}
/* vim: ts=2 sw=2 et tw=80
*/
