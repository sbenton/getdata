// (C) 2008-2010 D. V. Wiebe
//
///////////////////////////////////////////////////////////////////////////
//
// This file is part of the GetData project.
//
// GetData is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// GetData is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with GetData; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef GETDATA_PHASEENTRY_H
#define GETDATA_PHASEENTRY_H

#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 64
#endif

#define GD_NO_LEGACY_API
#define GD_C89_API

extern "C" {
#include <getdata.h>
}
#include <getdata/entry.h>

namespace GetData {

  class Dirfile;

  class PhaseEntry : public Entry {
    friend class Dirfile;

    public:
      PhaseEntry() : Entry::Entry() { E.field_type = GD_PHASE_ENTRY; };

      PhaseEntry(const char* field_code, const char* in_field, gd_shift_t shift,
          int fragment_index = 0);

      virtual const char *Input() {
        return E.in_fields[0];
      };

      virtual gd_shift_t Shift() {
        return E.shift;
      };

      int SetInput(const char* field);
      int SetShift(gd_shift_t shift);
      int SetShift(const char* shift);

      const char *Scalar() {
        return E.scalar[0];
      };

    private:
      PhaseEntry(GetData::Dirfile *dirfile, const char* field_code) :
        Entry(dirfile, field_code) { };
  };
}

#endif
