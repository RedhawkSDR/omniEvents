//                          Package   : omniMapper
// Mapper.h                 Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//    Copyright (C) 2004 Alex Tingle
//
//  This file is part of omniEvents. It is based upon omniMapper.
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

#include "Servant.h"
#include "Orb.h"
#include <string.h>

namespace OmniEvents {

/** A dummy servant that installs itself into the INSPOA and redirects all
 *  calls to the real destination. Copied from the omniMapper application.
 */
class Mapper :
  public Servant,
  public PortableServer::RefCountServantBase
{
public:

  Mapper(const char* id, CORBA::Object_ptr obj)
    : Servant(Orb::inst()._omniINSPOA.in()),
      id_( CORBA::string_dup(id) ),
      obj_( CORBA::Object::_duplicate(obj) )
  {
    DB(1,"Initialising Mapper `"<<id<<"'.")
    activateObjectWithId(id);
    _remove_ref();
  }
  ~Mapper() { DB(20,"~Mapper()") }
  void destroy() { deactivateObject(); }
  CORBA::Boolean _is_a(const char* id) { do_redir(); return 1; }

#ifdef HAVE_OMNIORB4
  CORBA::Boolean _dispatch(omniCallHandle&) { do_redir(); return 1; }
#else
  CORBA::Boolean _dispatch(GIOP_S&        ) { do_redir(); return 1; }
#endif

  void do_redir()
  {
    DB(20,"Mapping `"<<id_.in()<<"'")
#ifdef HAVE_OMNIORB4
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
#else
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_)  );
#endif
  }
  
  string id() const {return string(id_.in());}

private:
  CORBA::String_var id_;
  CORBA::Object_var obj_;
};

}; // end namespace OmniEvents
