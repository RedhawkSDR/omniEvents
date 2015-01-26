//                            Package   : omniEvents
// Filter.h                   Created   : 2004/04/30
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

#ifndef OMNIEVENTS__FILTER_H
#define OMNIEVENTS__FILTER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Orb.h"

#include <string.h>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif


namespace OmniEvents {

/** Event filter interface. */
class Filter
{
public:
  Filter(){}
  virtual ~Filter(){}

  /** Returns TRUE if the event passes the filter and FALSE if the event should
   * be discarded. Called by filter() once for each event.
   */
  virtual bool keep(const CORBA::Any& event) const =0;

  virtual void output(ostream& os) const =0;
};

/** The most basic event filter allows only events of a certain CORBA TCKind to
 * pass. Usually used to filter basic types.
 */
class FilterByTCKind: public Filter
{
public:
  FilterByTCKind(CORBA::TCKind kind):_kind(kind){}
  virtual ~FilterByTCKind(){}
  bool keep(const CORBA::Any& event) const
  {
    CORBA::TypeCode_var tc=event.type();
    return( tc->kind()==_kind );
  }
  void output(ostream& os) const { os<<"\n FilterKind="<<_kind; }
private:
  CORBA::TCKind _kind;
};

/** Allows only events of a certain CORBA RepositoryId to pass. Only passes
 *  events whose type matches exactly.
 */
class FilterByRepositoryId: public Filter
{
public:
  FilterByRepositoryId(const char* rid):_rid(rid){}
  virtual ~FilterByRepositoryId(){}
  bool keep(const CORBA::Any& event) const;
  void output(ostream& os) const { os<<"\n FilterId="<<_rid; }
private:
  CORBA::RepositoryId_var _rid;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__FILTER_H
