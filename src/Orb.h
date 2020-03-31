
//                            Package   : omniEvents
// Orb.h                      Created   : 2003/12/04
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

#ifndef OMNIEVENTS__ORB_H
#define OMNIEVENTS__ORB_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <list>

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  define IFELSE_OMNIORB4(omniORB4_code,default_code) omniORB4_code
#  define IF_OMNIORB4(omniORB4_code) omniORB4_code
#else
#  define IFELSE_OMNIORB4(omniORB4_code,default_code) default_code
#  define IF_OMNIORB4(omniORB4_code)
#endif

#define DB(l,x) \
  {if(omniORB::trace(l)){omniORB::logger log("omniEvents: ");log<<x<<"\n";}}

#define NP_MINORSTRING(systemException) \
  ((systemException).NP_minorString()?(systemException).NP_minorString():"??")

#define AS_STR_2(x) #x
#define AS_STR_1(x) AS_STR_2(x)
/** Generates a string literal that describes the filename and line number. */
#define HERE __FILE__ ":" AS_STR_1(__LINE__)

#ifdef HAVE_STD_STL
using namespace std;
#include <string>
#endif

namespace OmniEvents {

class Callback;

/** Singleton class that owns the ORB and various initial references. */
class Orb
{
private:
  static Orb _inst;
  typedef pair<CORBA::Request_ptr,Callback*> RequestCallback_t;
  list<RequestCallback_t> _deferredRequests;
  omni_mutex _deferredRequestsLock;
  bool _shutdownRequested;
  Orb():_shutdownRequested(false){}
  friend void OmniEvents_Orb_shutdown(int);

public:
  inline static Orb& inst()
  {
    return _inst;
  }
  /** Destructor needs to be public to keep MS VC++6 happy. */
  ~Orb();

  CORBA::ORB_var               _orb;
  PortableServer::POA_var      _RootPOA;
  PortableServer::POA_var      _omniINSPOA;
  CosNaming::NamingContext_var _NameService;
#ifdef HAVE_OMNIORB4
  PortableServer::Current_ptr  _POACurrent;
#endif

  /** _orb must already have been initialized before this method is called. */
  void resolveInitialReferences();
  
  /** Parks the main thread, but also picks up (and ignores) responses from
   * orphan requests. If _shutdownRequested is set, then run() shuts down the
   * orb and returns.
   */
  void run();
  
  /** Adopts the request and then stores it in _deferredRequests.
   * run() later picks up the responses and forwards them to 'callback', if it
   * is set.
   */
  void deferredRequest(CORBA::Request_ptr req, Callback* callback=NULL);

  /** Called by Callback objects when they are destroyed. */
  void cancelCallback(const Callback* callback);

  /** Called by omniEvents when an object has failed (fatal exception).
   * The failure is logged as an omniORB message with traceLevel zero.
   */
  void reportObjectFailure(
    const char*       here,
    CORBA::Object_ptr obj,
    CORBA::Exception* ex
  );
  void reportObjectFailure(
    const char*       here,
    const std::string &iorstr,
    omniIOR          *ior,
    CORBA::Exception* ex
  );
  

  /** Sets _shutdownRequested. The parameter is ignored.
   * This method may be used as a signal handler.
   */
  void shutdown(int) { _shutdownRequested=true; }
}; // end class Orb


/** Converts a string to a narrowed reference. */
template<class T>
typename T::_ptr_type string_to_(const char* oidStr)
{
  CORBA::Object_var obj =Orb::inst()._orb->string_to_object(oidStr);
  if(CORBA::is_nil(obj.in()))
    throw CORBA::BAD_PARAM();

#ifdef HAVE_OMNIORB4
  typename T::_var_type result =T::_unchecked_narrow(obj);
#else
  typename T::_var_type result =T::_narrow(obj);
#endif
  if(CORBA::is_nil(result.in()))
    throw CORBA::BAD_PARAM();

  return result._retn();
}

}; // end namespace OmniEvents

#endif // OMNIEVENTS__ORB_H
