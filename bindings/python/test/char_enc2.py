# Copyright (C) 2016 D. V. Wiebe
#
##########################################################################
#
# This file is part of the GetData project.
#
# GetData is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation; either version 2.1 of the License, or (at your
# option) any later version.
#
# GetData is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with GetData; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# Test character_encoding

import re
import os
import sys
import pygetdata

# Python2/3 abstraction:

# an encoded string
def B(s):
  if sys.version[:1] == '3':
    return bytes(s, "UTF-8")
  return s

# a decoded string
def U(s):
  if sys.version[:1] == '3':
    return s
  return unicode(s)

ne = 0

def CheckOK(t):
  global ne
  ne+=1
  print ("e[", t, "] =", sys.exc_info()[0], sys.exc_value)

def CheckSimple(t,v,g):
  global ne
  if (v != g):
    ne+=1
    print ("n[", t, "] =", v, "expected", g)

def CheckEOS(t,v,g):
  global ne
  if (v[-len(g):] != g):
    ne+=1
    print ("n[", t, "] =", repr(v), "expected", repr(g))

# create the dirfile
os.system("rm -rf dirfile")
os.mkdir("dirfile")

# Encoded string (koi8-r)
estring =  B('\xF3\xD4\xD2\xCF\xCB\xC1')

# byte-escaped (used for Dirfile metadata)
xstring = B('\\xF3\\xD4\\xD2\\xCF\\xCB\\xC1')

# The decoded version
ustring = unichr(0x421) + unichr(0x442) + unichr(0x440) + unichr(0x43E) + \
    unichr(0x43A) + unichr(0x430)

# Unicode escaped, which will show up in ASCII-ified error messages
xestring = B("\\u0421\\u0442\\u0440\\u043e\\u043a\\u0430")

f=open("dirfile/format", "wb")
f.write(
        B("/ALIAS ") + xstring + B("_al ") + xstring + B("_t\n") +
        xstring + B("_s1 STRING ") + xstring + B("1\n") +
        xstring + B("_s2 STRING ") + xstring + B("2\n") +
        xstring + B("_a1 CARRAY UINT64 1 2 3\n") +
        xstring + B("_a2 CARRAY UINT64 1 2 3\n") +
        xstring + B("_c1 CONST UINT8 1\n") +
        xstring + B("_c2 CONST UINT8 1\n") +
        xstring + B("_r1 RAW UINT8 ") + xstring + B("_spf\n") +
        B("l1 LINTERP in ") + xstring + B("\n") +
        B("l1/") + xstring + B("_s1 STRING ") + xstring + B("1\n") +
        B("l1/") + xstring + B("_s2 STRING ") + xstring + B("2\n") +
        B("l1/") + xstring + B("_a1 CARRAY UINT64 1 2 3\n") +
        B("l1/") + xstring + B("_a2 CARRAY UINT64 1 2 3\n") +
        B("l1/") + xstring + B("_c1 CONST UINT8 1\n") +
        B("l1/") + xstring + B("_c2 CONST UINT8 2\n") +
        B("o1 LINCOM ") + xstring + B("_i ") + xstring + B("_m ") +
        xstring + B("_b\n") +
        B("o2 LINCOM ") +
        xstring + B("_i1 ") + xstring + B("_m1 ") + xstring + B("_b1 ") +
        xstring + B("_i2 ") + xstring + B("_m2 ") + xstring + B("_b2\n") +
        B("o3 LINCOM ") +
        xstring + B("_i1 ") + xstring + B("_m1 ") + xstring + B("_b1 ") +
        xstring + B("_i2 ") + xstring + B("_m2 ") + xstring + B("_b2 ") +
        xstring + B("_i3 ") + xstring + B("_m3 ") + xstring + B("_b3\n") +
        B("b1 BIT ") + xstring + B("_i ") +
        xstring + B("_bn ") + xstring + B("_nb\n") +
        B("e1 RECIP in ") + xstring + B("_dv\n") +
        B("p1 PHASE ") + xstring + B("_i ") + xstring + B("_ps\n") +
        B("m1 MPLEX a b ") + xstring + B("_cv ") + xstring + B("_pd\n") +
        B("y1 POLYNOM ") + xstring + B("_i ") + xstring + B("_y1 2 ") +
        xstring + B("_y3\n") +
        B("w1 WINDOW a b EQ ") + xstring + B("_t1\n") +
        B("w2 WINDOW a b SET ") + xstring + B("_t2\n") +
        B("w3 WINDOW a b GT ") + xstring + B("_t3\n") +
        B("d1 DIVIDE ") + xstring + B("_i1 ") + xstring + B("_i2\n")
        )
f.close()

try:
  D=pygetdata.dirfile("dirfile", pygetdata.RDONLY)
except pygetdata.DirfileError:
  CheckOK(0)

# Attempt 2: no encoding

try:
  D.validate(estring)
except pygetdata.DirfileError:
  CheckEOS(2,D.error_string,estring)

c = D.carrays(return_type=pygetdata.NULL)
CheckSimple(3,len(c),2)
CheckSimple(4,c[0][0],estring + "_a1")
CheckSimple(5,c[1][0],estring + "_a2")

c = D.constants(return_type=pygetdata.UINT8)
CheckSimple(6,len(c),2)
CheckSimple(7,c[0][0],estring + "_c1")
CheckSimple(8,c[1][0],estring + "_c2")

c = D.mcarrays("l1", return_type=pygetdata.NULL)
CheckSimple(9,len(c),2)
CheckSimple(10,c[0][0], estring + "_a1")
CheckSimple(11,c[1][0], estring + "_a2")

c = D.mconstants("l1", return_type=pygetdata.UINT8)
CheckSimple(12,len(c),2)
CheckSimple(13,c[0][0], estring + "_c1")
CheckSimple(14,c[1][0], estring + "_c2")

c = D.strings()
CheckSimple(15,len(c),2)
CheckSimple(16,c[0][0], estring + "_s1")
CheckSimple(17,c[0][1], estring + "1")
CheckSimple(18,c[0][0], estring + "_s1")
CheckSimple(19,c[1][1], estring + "2")

c = D.mstrings("l1")
CheckSimple(20,len(c),2)
CheckSimple(21,c[0][0], estring + "_s1")
CheckSimple(22,c[0][1], estring + "1")
CheckSimple(23,c[0][0], estring + "_s1")
CheckSimple(24,c[1][1], estring + "2")

c = D.reference
CheckSimple(25,c,estring + "_r1")

c = D.get_string(estring + "_s1")
CheckSimple(26,c,estring + "1")

c = D.alias_target(estring + "_al")
CheckSimple(27,c,estring + "_t")

c = D.entry(estring + "_r1")
CheckSimple(28,c.name,estring + "_r1")
CheckSimple(29,c.spf,estring + "_spf")
CheckSimple(30,c.parameters,(1, estring + "_spf"))

c = D.entry("l1")
CheckSimple(31,c.parameters,("in", estring))

c = D.entry("d1")
CheckSimple(32,c.in_fields,(estring + "_i1", estring + "_i2"))
CheckSimple(33,c.parameters,(estring + "_i1", estring + "_i2"))

c = D.entry("e1")
CheckSimple(35,c.dividend,estring + "_dv")
CheckSimple(36,c.parameters,("in", estring + "_dv"))

c = D.entry("p1")
CheckSimple(37,c.in_fields,(estring + "_i",))
CheckSimple(38,c.shift,estring + "_ps")
CheckSimple(39,c.parameters,(estring + "_i", estring + "_ps"))

c = D.entry("y1")
CheckSimple(40,c.a,(estring + "_y1", 2, estring + "_y3"))
CheckSimple(41,c.parameters,(estring + "_i",
  (estring + "_y1", 2, estring + "_y3")))

c = D.entry("o1")
CheckSimple(42,c.in_fields,(estring + "_i",))
CheckSimple(43,c.m,(estring + "_m",))
CheckSimple(44,c.b,(estring + "_b",))
CheckSimple(45,c.parameters,((estring + "_i",), (estring + "_m",),
  (estring + "_b",)))

c = D.entry("o2")
CheckSimple(46,c.in_fields,(estring + "_i1",estring + "_i2"))
CheckSimple(47,c.m,(estring + "_m1",estring + "_m2"))
CheckSimple(48,c.b,(estring + "_b1",estring + "_b2"))
CheckSimple(49,c.parameters,((estring + "_i1",estring + "_i2"),
  (estring + "_m1",estring + "_m2"),(estring + "_b1",estring + "_b2")))

c = D.entry("o3")
CheckSimple(50,c.in_fields,(estring + "_i1",estring + "_i2",estring + "_i3"))
CheckSimple(51,c.m,(estring + "_m1",estring + "_m2",estring + "_m3"))
CheckSimple(52,c.b,(estring + "_b1",estring + "_b2",estring + "_b3"))
CheckSimple(53,c.parameters,((estring + "_i1",estring + "_i2",estring + "_i3"),
  (estring + "_m1",estring + "_m2",estring + "_m3"),
  (estring + "_b1",estring + "_b2",estring + "_b3")))

c = D.entry("w1")
CheckSimple(54,c.parameters,("a", "b", pygetdata.WINDOP_EQ, estring + "_t1"))

c = D.entry("w2")
CheckSimple(55,c.parameters,("a", "b", pygetdata.WINDOP_SET, estring + "_t2"))

c = D.entry("w3")
CheckSimple(56,c.parameters,("a", "b", pygetdata.WINDOP_GT, estring + "_t3"))

c = D.entry("m1")
CheckSimple(57,c.count_val, estring + "_cv")
CheckSimple(58,c.period, estring + "_pd")
CheckSimple(59,c.parameters,("a", "b", estring + "_cv", estring + "_pd"))

D.discard()
del D
os.system("rm -rf dirfile")

if (ne > 0):
  print ("ne = ",ne)
  sys.exit(1)
  
