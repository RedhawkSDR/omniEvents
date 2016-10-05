//                            Package   : omniEvents
// Orb.cc                     Created   : 2003/12/04
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
//    Modified by REDHAWK (United States Government) - 2015

#include "Orb.h"

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include <stdlib.h>
#include <assert.h>

#include "Callback.h"

namespace OmniEvents {

Orb Orb::_inst;


Orb::~Orb()
{
  omni_mutex_lock l(_deferredRequestsLock);
  list<RequestCallback_t>::iterator curr, next=_deferredRequests.begin();
  while(next!=_deferredRequests.end())
  {
    curr=next++;
    CORBA::release(curr->first);
    _deferredRequests.erase(curr);
  }
}


void Orb::resolveInitialReferences()
{
  assert(!CORBA::is_nil(_orb));

  const char* action=""; // Use this variable to help report errors.
  try
  {
    CORBA::Object_var obj;

    action="resolve initial reference 'RootPOA'";
    obj=_orb->resolve_initial_references("RootPOA");
    _RootPOA=PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(_RootPOA))
        throw CORBA::OBJECT_NOT_EXIST();

    action="resolve initial reference 'omniINSPOA'";
    obj=_orb->resolve_initial_references("omniINSPOA");
    _omniINSPOA=PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(_omniINSPOA))
        throw CORBA::OBJECT_NOT_EXIST();

    // The naming service is optional.
    try
    {
      action="resolve initial reference 'NameService'";
      obj=_orb->resolve_initial_references("NameService");
      _NameService=CosNaming::NamingContext::_narrow(obj);
    }
    catch(CORBA::Exception& ex)
    {
      DB(1,"Warning - failed to "<<action<<
         IFELSE_OMNIORB4(". Exception: "<<ex._name(),"."))
    }

#ifdef HAVE_OMNIORB4
    action="resolve initial reference 'POACurrent'";
    obj=_orb->resolve_initial_references("POACurrent");
    _POACurrent=PortableServer::Current::_narrow(obj);
    if(CORBA::is_nil(_POACurrent))
        throw CORBA::OBJECT_NOT_EXIST();
#endif

    return;
  }
  catch(CORBA::ORB::InvalidName& ex) // resolve_initial_references
  {
    DB(0,"Failed to "<<action<<". InvalidName")
  }
  catch(CORBA::TRANSIENT& ex) // _narrow()
  {
    DB(0,"Failed to "<<action<<". TRANSIENT")
  }
  catch(CORBA::OBJECT_NOT_EXIST& ex) // _narrow()
  {
    DB(0,"Failed to "<<action<<". OBJECT_NOT_EXIST")
  }
  catch(CORBA::SystemException& ex)
  {
    DB(0,"Failed to "<<action<<"."
      IF_OMNIORB4(" "<<ex._name()<<" ("<<NP_MINORSTRING(ex)<<")") )
  }
  catch(CORBA::Exception& ex)
  {
    DB(0,"Failed to "<<action<<"." IF_OMNIORB4(" "<<ex._name()) )
  }
  exit(1);
}


void Orb::run()
{
  while(!_shutdownRequested)
  {
    omni_thread::sleep(5);

    list<Callback*> usedCallbacks;
    {
      omni_mutex_lock l(_deferredRequestsLock);
      DB(20,"Polling "<<_deferredRequests.size()<<" deferred requests.")
      list<RequestCallback_t>::iterator curr, next=_deferredRequests.begin();
      while(next!=_deferredRequests.end())
      {
        curr=next++;
        if(curr->first->poll_response())
        {
          // With omniORB 4.2, fetching the environment throws an exception
          // where 4.1 would not. To unify the handling of exceptions, the
          // entire operation is done in a try/catch, and if the environment
          // does have an exception, it is raised so that it's handled by the
          // same catch clause.
          try
          {
            CORBA::Environment_ptr env=curr->first->env();// No need to release.
            if(!CORBA::is_nil(env) && env->exception())
            {
              CORBA::Exception* ex =env->exception(); // No need to free exception
              ex->_raise();
            }
            else if(curr->second)
            {
              DB(15,"Deferred call to "<<curr->first->operation()<<"() returned.")
              try {
                  curr->second->callback(curr->first);
              } catch (...) {
              }
            }
            else
            {
              DB(15,"Orphan call to "<<curr->first->operation()<<"() returned.")
            }
          }
          catch(CORBA::Exception& ex)
          {
            DB(10,"Deferred call to "<<curr->first->operation()
               <<"() got exception" IF_OMNIORB4(<<": "<<ex._name()))
          }
          CORBA::release(curr->first);
          if(curr->second)
             usedCallbacks.push_back( curr->second );
          _deferredRequests.erase(curr);
        }
      } // end loop while()
    }
    // _deferredRequestsLock is now unlocked: clear away used callbacks.
    // (Cannot do this while _deferredRequestsLock is held, because of the
    // following deadlock:
    //  _remove_ref() -> ~Proxy() -> Orb::deferredRequest()
    while(!usedCallbacks.empty())
    {
      usedCallbacks.front()->_remove_ref();
      usedCallbacks.pop_front();
    }
  } // end loop while(!_shutdownRequested)

  // Clean up all outstanding requests.
  omni_mutex_lock l(_deferredRequestsLock);
  while(!_deferredRequests.empty())
  {
    _deferredRequests.front().first->get_response();
    CORBA::release(_deferredRequests.front().first);
    if(_deferredRequests.front().second)
       _deferredRequests.front().second->_remove_ref();
    _deferredRequests.pop_front();
  }
}


void Orb::deferredRequest(CORBA::Request_ptr req, Callback* callback)
{
  if(_shutdownRequested)
     callback=NULL;
  // If _shutdownRequested and Orb::run() has already terminated, then
  // the request (req) will never be collected or released. This is sad, and it
  // makes omniORB complain - but at least it works:
  // Attempting to get_response() here can cause deadlock. Just releasing the
  // Request causes a SEGV when the call returns.

  if(callback)
     callback->_add_ref();
  omni_mutex_lock l(_deferredRequestsLock);
  _deferredRequests.push_back(RequestCallback_t(req,callback));
}


void Orb::reportObjectFailure(
  const char*       here,
  CORBA::Object_ptr obj,
  CORBA::Exception* ex
)
{
  assert(!CORBA::is_nil(obj));
#ifdef HAVE_OMNIORB4
  {
    // Hack! The '!' signals object failure.
    // See DaemonImpl::log() in daemon_unix.cc.
    omniORB::logger log("omniEvents! Object failure: ");
    omniIOR* ior =obj->_PR_getobj()->_getIOR();
    // Log Repository ID.
    log<<ior->repositoryID();
    // Log Object ID. (Limitation: only display the first TAG_INTERNET_IOP)
    for(CORBA::ULong i=0; i<ior->iopProfiles().length(); i++)
    {
      if (ior->iopProfiles()[i].tag == IOP::TAG_INTERNET_IOP)
      {
        IIOP::ProfileBody pBody;
        IIOP::unmarshalProfile(ior->iopProfiles()[i],pBody);
        log<<" \"";
        for(CORBA::ULong j=0; j<pBody.object_key.length(); ++j)
        {
          char c=(char)pBody.object_key[j];
          log<<( (c>=' '&&c<='~')? c: '.' ); // Log object key as text
        }
        log<<"\" at "<<(const char*)pBody.address.host<<":"<<pBody.address.port;
        break; // ONLY DISPLAY FIRST!
      }
    }
    // Log exception.
    if(!ex)
    {
      log<<" threw unknown exception\n";
    }
    else
    {
      log<<" threw "<<ex->_name();
      CORBA::SystemException* sysex =CORBA::SystemException::_downcast(ex);
      if(sysex)
          log<<" ("<<NP_MINORSTRING(*sysex)<<")";
      log<<"\n";
    }
  }
#endif
  {
    omniORB::logger log("omniEvents! Object failure detail: ");
    CORBA::String_var sior( Orb::inst()._orb->object_to_string(obj) );
    log<<sior<<" at "<<here<<"\n";
  }
}


}; // end namespace OmniEvents
