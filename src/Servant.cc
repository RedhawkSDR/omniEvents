//                            Package   : omniEvents
// Servant.cc                 Created   : 2003/12/04
//                            Author    : Alex Tingle
//
//    Copyright (C) 2003-2005 Alex Tingle.
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

#include "Servant.h"
#include "Orb.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h> // getpid
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>    // getpid
#elif defined(HAVE_PROCESS_H)
# include <process.h>
#endif

#include <stdio.h>     // sprintf
#include <assert.h>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {


CORBA::Object_ptr createReference(
  PortableServer::POA_ptr poa,          // POA to own new object
  const char*             repositoryId  // e.g. _tc_ProxyPushSupplier->id()
)
{
  CORBA::String_var oidStr =newUniqueId();

  PortableServer::ObjectId_var oid =
    PortableServer::string_to_ObjectId(oidStr.in());

  CORBA::Object_var obj =
    poa->create_reference_with_id(oid.in(),repositoryId);

  assert(!CORBA::is_nil(obj));
  return obj._retn();
}

char* newUniqueId()
{
  static long  count=0;
  static omni_mutex  mutex;
  int  mypid =getpid(); // MS VC++6 doesn't have type pid_t!
  unsigned long  sec,nsec;
  omni_thread::get_time(&sec,&nsec); // More portable than just time().
  char buf[128];
  {
    omni_mutex_lock l(mutex);
    sprintf(buf,"%lx.%d.%lx",++count,mypid,sec);
  }
  return CORBA::string_dup(buf);
}


//
//  Servant
//


#if OMNIEVENTS__DEBUG_SERVANT
#  define OMNIEVENTS__ADDR "["<<long(this)<<"] "
int Servant::_objectCount =0;
#else
#  define OMNIEVENTS__ADDR
#endif


Servant::Servant(PortableServer::POA_ptr poa)
: _poa(PortableServer::POA::_duplicate(poa))
{
#if OMNIEVENTS__DEBUG_SERVANT
  ++_objectCount;
  DB(21,OMNIEVENTS__ADDR "Servant::Servant() count="<<_objectCount)
#endif
}


Servant::~Servant()
{
#if OMNIEVENTS__DEBUG_SERVANT
  --_objectCount;
  DB(20,OMNIEVENTS__ADDR "Servant::~Servant() count="<<_objectCount)
#endif
}


PortableServer::POA_ptr Servant::_default_POA()
{
  return PortableServer::POA::_duplicate(_poa.in());
}


void Servant::activateObjectWithId(const char* oidStr)
{
  using namespace PortableServer;
  CORBA::String_var poaName =_poa->the_name();
  DB(5,OMNIEVENTS__ADDR "Activating object "<<poaName.in()<<"/"<<oidStr);
  try
  {
    ObjectId_var oid =string_to_ObjectId(oidStr);
    _poa->activate_object_with_id(oid.in(),this);
  }
  catch(CORBA::BAD_PARAM& ex)
  {
    DB(0,"Can't activate "<<oidStr<<": "
      "BAD_PARAM" IF_OMNIORB4(" ("<<NP_MINORSTRING(ex)<<")") )
    throw;
  }
  catch(POA::ServantAlreadyActive& ex)
  {
    DB(0,"Can't activate "<<oidStr<<": Servant is already active.")
    throw;
  }
  catch(POA::ObjectAlreadyActive& ex)
  {
    DB(0,"Can't activate "<<oidStr<<": Object is already active.")
    throw;
  }
  catch(POA::WrongPolicy& ex)
  {
    DB(0,"Can't activate "<<oidStr<<": POA '"<<poaName.in()
        <<"' has wrong policy for activate_object_with_id().")
    exit(1); // Programming error - so quit.
  }
}


void Servant::deactivateObject()
{
  using namespace PortableServer;
  CORBA::String_var poaName =_poa->the_name();

  ObjectId_var oid;
  try
  {
    oid=_poa->servant_to_id(this);
  }
  catch(POA::ServantNotActive& ex)
  {
    DB(0,"Can't deactivate servant: POA '"<<poaName.in()
        <<"' says it is not active.")
    return;
  }
  catch(POA::WrongPolicy& ex)
  {
    DB(0,"Can't deactivate servant: POA '"<<poaName.in()
        <<"' has wrong policy for servant_to_id().")
    exit(1); // Programming error - so quit.
  }

  CORBA::String_var oidStr;
  try
  {
    oidStr=ObjectId_to_string(oid.in());
  }
  catch(CORBA::BAD_PARAM& ex)
  {
    DB(0,"Can't deactivate servant. ObjectId looks bad: "
      "BAD_PARAM" IF_OMNIORB4(" ("<<NP_MINORSTRING(ex)<<")") )
    return;
  }

  try
  {
    DB(7,OMNIEVENTS__ADDR "Deactivating object "<<poaName<<"/"<<oidStr.in());
    _poa->deactivate_object(oid.in());
  }
  catch(POA::ObjectNotActive& ex)
  {
    DB(0,"Can't deactivate "<<oidStr<<": Object is not active.")
    return;
  }
  catch(POA::WrongPolicy& ex)
  {
    DB(0,"Can't deactivate "<<oidStr<<": POA '"<<poaName.in()
        <<"' has wrong policy for deactivate_object().")
    exit(1); // Programming error - so quit.
  }
}

}; // end namespace OmniEvents
