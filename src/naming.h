// -*- Mode: C++; -*-
//                            Package   : omniEvents
// naming.h                   Created   : 1/10/99
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2004 Alex Tingle.
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
// Description:
//

/*
  $Log: naming.h,v $
  Revision 1.3.2.1  2005/05/10 14:28:11  alextingle
  Updated copyrights to 2005.

  Revision 1.3  2004/07/26 16:22:25  alextingle
  New method: str2name() parses a stringified naming service name info a CosNaming::Name.

  Revision 1.2  2004/04/21 10:01:34  alextingle
  Removed unused code. Now silently fails if the Orb has no naming service ref.

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.2  2003/11/03 22:33:49  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.

  Revision 1.1.1.1  2002/09/25 19:00:32  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.1  2000/09/04 03:41:20  naderp
  Changed headers.

  Revision 1.0  1999/11/01 16:48:11  naderp
  Initial revision

*/

#ifndef _NAMING_H_
#define _NAMING_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

ostream& operator<<(ostream& os, const CosNaming::Name &n);

/** Converts stringified name to naming service name.
 *
 * Format for name: [<context_id>[.<context_kind>]/]*<object_id>[.<object_kind>]
 * 
 * E.g. foo, foo.bar, foo.bar/baz/qux, foo/bar/baz.qux
 */
CosNaming::Name str2name(const char* namestr);

/** Binds CosNaming::Name to object in the naming service. */
int bindName2Object(
  CosNaming::NamingContext_ptr  namingContext,
  const CosNaming::Name &       name,
  CORBA::Object_ptr             obj
);

#endif /* _NAMING_H_ */
