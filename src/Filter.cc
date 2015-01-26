//                            Package   : omniEvents
// Filter.cc                  Created   : 2004/04/30
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle.
//
//    This file is part of the omniEvents application.
//
//    omniEvents is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniEvents is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "Filter.h"

#include <assert.h>

#ifdef HAVE_OMNIORB4
#  define STR_MATCH(s1,s2) omni::strMatch((s1),(s2))
#else
#  define STR_MATCH(s1,s2) (0==::strcmp((s1),(s2)))
#endif

namespace OmniEvents {

bool FilterByRepositoryId::keep(const CORBA::Any& event) const
{
  using namespace CORBA;
  CORBA::TypeCode_var tc=event.type();
  switch( tc->kind() )
  {
#ifdef HAVE_OMNIORB4
    case _np_tk_indirect: // Internal to omniORB. We should never get this.
      assert(0);
#endif

    // TCs with Repository ID:
    case tk_objref:
    case tk_struct:
    case tk_union:
    case tk_enum:
    case tk_alias:
    case tk_except:
      return STR_MATCH( _rid.in(), tc->id() );

    // Collections
    case tk_sequence:
    case tk_array:

    // Primitives
    case tk_null:
    case tk_void:
    case tk_short:
    case tk_long:
    case tk_ushort:
    case tk_ulong:
    case tk_float:
    case tk_double:
    case tk_boolean:
    case tk_char:
    case tk_octet:
    case tk_any:
    case tk_TypeCode:
    case tk_Principal:
    case tk_string:
#ifdef HAS_LongLong
    case tk_longlong:
    case tk_ulonglong:
#endif
#ifdef HAS_LongDouble
    case tk_longdouble:
#endif
#ifndef HAVE_OMNIORB3
    case tk_wchar:
    case tk_wstring:
    case tk_fixed:

    // WTF? Not implemented in omniORB?
    case tk_value:
    case tk_value_box:
    case tk_native:
    case tk_abstract_interface:
    case tk_local_interface:
#else
    default:
#endif
      break;
  } // end case. Note: no default, so that missing options are flagged by GCC.
  return false;
}

}; // end namespace OmniEvents
