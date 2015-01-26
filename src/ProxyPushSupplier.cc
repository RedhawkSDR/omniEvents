//                            Package   : omniEvents
// ProxyPushSupplier.cc       Created   : 2003/12/04
//                            Author    : Alex Tingle
//
//    Copyright (C) 2003,2005 Alex Tingle.
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

#include "ProxyPushSupplier.h"
#include "Orb.h"
#include "omniEventsLog.h"
#include "PersistNode.h"
#include <assert.h>

namespace OmniEvents {

/** The opposite of omni_mutex_lock, unlocks the mutex upon construction and
 * re-locks it upon destruction.
 */
class omni_mutex_kcol {
    omni_mutex& mutex;
public:
    omni_mutex_kcol(omni_mutex& m) : mutex(m) { mutex.unlock(); }
    ~omni_mutex_kcol(void) { mutex.lock(); }
private:
    // dummy copy constructor and operator= to prevent copying
    omni_mutex_kcol(const omni_mutex_kcol&);
    omni_mutex_kcol& operator=(const omni_mutex_kcol&);
};


//
//  ProxyPushSupplierManager
//

PortableServer::Servant
ProxyPushSupplierManager::incarnate(
  const PortableServer::ObjectId& oid,
  PortableServer::POA_ptr         poa
)
{
  ProxyPushSupplier_i* result =new ProxyPushSupplier_i(_managedPoa,_queue);
  PauseThenWake p(this);
  _servants.insert(result);
  return result;
}

void
ProxyPushSupplierManager::etherealize(
  const PortableServer::ObjectId& oid,
  PortableServer::POA_ptr         adapter,
  PortableServer::Servant         serv,
  CORBA::Boolean                  cleanup_in_progress,
  CORBA::Boolean                  remaining_activations
)
{
  // This etherealize method needs a special implementation because
  // ProxyPushSupplier_i objects are freed with _remove_ref() rather than
  // delete.
  // Otherwise, this method strongly resembles ProxyManager::etherealize().
  omni_mutex_lock pause(_lock);
  ProxyPushSupplier_i* narrowed =dynamic_cast<ProxyPushSupplier_i*>(serv);
  assert(narrowed!=NULL);
  set<Proxy*>::iterator pos =_servants.find(narrowed);
  if(pos!=_servants.end())
  {
    _servants.erase(pos);
    narrowed->_remove_ref();
  }
  else
  {
    DB(1,"\t\teh? - POA attempted to etherealize unknown servant.");
  }
}

ProxyPushSupplierManager::ProxyPushSupplierManager(
  PortableServer::POA_ptr parentPoa,
  EventQueue& q
)
: ProxyManager(parentPoa),
  omni_thread(NULL,PRIORITY_HIGH),
  _queue(q),
  _lock(),_condition(&_lock),
  _refCount(1)
{
  ProxyManager::activate("ProxyPushSupplier");
  start_undetached();
}

ProxyPushSupplierManager::~ProxyPushSupplierManager()
{
  DB(20,"~ProxyPushSupplierManager()")
}

CosEventChannelAdmin::ProxyPushSupplier_ptr
ProxyPushSupplierManager::createObject()
{  
  return createNarrowedReference<CosEventChannelAdmin::ProxyPushSupplier>(
           _managedPoa.in(),
           CosEventChannelAdmin::_tc_ProxyPushSupplier->id()
         );
}

void ProxyPushSupplierManager::disconnect()
{
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    Proxy* p =*i; // Sun's CC requires this temporary.
    ProxyPushSupplier_i* pps =static_cast<ProxyPushSupplier_i*>(p);
    // We are in the EventChannel's thread.
    // Make sure all calls go though the ProxyPushSupplier POA.
    CosEventChannelAdmin::ProxyPushSupplier_var ppsv =pps->_this(); 
    ppsv->disconnect_push_supplier();
  }
}

void*
ProxyPushSupplierManager::run_undetached(void*)
{
  // This loop repeatedly triggers all of the servants in turn. As long as
  // something happens each time, then we loop as fast as we can.
  // As soon as activity dries up, we start to wait longer and longer between
  // loops (up to a maximum). When there is no work to do, just block until
  // a new event arrives.
  //
  // Rationale: The faster we loop the more events we can deliver to each
  // consumer per second. However, when nothing is happening, this busy loop
  // just soaks up CPU and kills performance. The optimum sleep time varies
  // wildly from platform to platform, and also depends upon the typical ping
  // time to the consumers.
  //
  // This dynamic approach should deliver reasonable performance when things
  // are hectic, but not soak up too much CPU when not much is happening.
  //
  const unsigned long sleepTimeNanosec0 =0x8000;   // 33us (doubled before use)
  const unsigned long maxSleepNanosec   =0x800000; // 8.4ms
  unsigned long sleepTimeNanosec =sleepTimeNanosec0;

  omni_mutex_lock conditionLock(_lock);
  while(true)
  {
    try {
      if(_refCount<1)
          break;

      bool busy=false;
      bool waiting=false;

      // Trigger each servant in turn.
      for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
      {
        Proxy* p =*i; // Sun's CC requires this temporary.
        ProxyPushSupplier_i* pps =static_cast<ProxyPushSupplier_i*>(p);
        pps->trigger(busy,waiting);
      }

      if(busy)
      {
        // Something happened last time round. So we'll be optimistic and
        // immediately go round for another go. Briefly unlock the mutex first,
        // just to let the other kids get in if they need to.
        omni_mutex_kcol l(_lock); // 'lock' reversed!
        // Reset the sleep time.
        sleepTimeNanosec=sleepTimeNanosec0;
      }
      else if(waiting)
      {
        // Nothing happened, so we'll wait for a bit and then give it another
        // go. Each time we wait for twice as long, up to the maximum.
        if(sleepTimeNanosec<maxSleepNanosec)
            sleepTimeNanosec<<=1; // (multiply by 2)
        unsigned long sec,nsec;
        omni_thread::get_time(&sec,&nsec,0,sleepTimeNanosec);
        _condition.timedwait(sec,nsec);
      }
      else
      {
        // There is nothing to do, so block until a new event arrives.
        _condition.wait();
      }

    }
    catch (CORBA::SystemException& ex) {
      DB(2,"ProxyPushSupplierManager ignoring CORBA system exception"
         IF_OMNIORB4(": "<<ex._name()<<" ("<<NP_MINORSTRING(ex)<<")") ".")
    }
    catch (CORBA::Exception& ex) {
      DB(2,"ProxyPushSupplierManager ignoring CORBA exception"
         IF_OMNIORB4(": "<<ex._name()<<) ".")
    }
    catch(...) {
      DB(2,"ProxyPushSupplierManager thread killed by unknown exception.")
      break;
    }
  }
  return NULL;
}

void ProxyPushSupplierManager::_add_ref()
{
#if OMNIEVENTS__DEBUG_REF_COUNTS
  DB(20,"ProxyPushSupplierManager::_add_ref()")
#endif
  omni_mutex_lock pause(_lock);
  ++_refCount;
}

void ProxyPushSupplierManager::_remove_ref()
{
#if OMNIEVENTS__DEBUG_REF_COUNTS
  DB(20,"ProxyPushSupplierManager::_remove_ref()")
#endif
  int myref;
  {
    PauseThenWake p(this);
    myref = --_refCount;
  }
  if(myref<0)
  {
    DB(2,"ProxyPushSupplierManager has negative ref count! "<<myref)
  }
  else if(myref==0)
  {
    DB(15,"ProxyPushSupplierManager has zero ref count -- shutdown.")
    join(NULL);
  }
}


//
//  ProxyPushSupplier_i
//

void ProxyPushSupplier_i::connect_push_consumer(
  CosEventComm::PushConsumer_ptr pushConsumer)
{
  if(CORBA::is_nil(pushConsumer))
      throw CORBA::BAD_PARAM();
  if(!CORBA::is_nil(_target) || !CORBA::is_nil(_req))
      throw CosEventChannelAdmin::AlreadyConnected();
  _target=CosEventComm::PushConsumer::_duplicate(pushConsumer);

  // Test to see whether pushSupplier is a ProxyPushSupplier.
  // If so, then we will aggressively try to reconnect, when we are reincarnated
  CORBA::Request_var req =_target->_request("_is_a");
  req->add_in_arg() <<= CosEventChannelAdmin::_tc_ProxyPushConsumer->id();
  req->set_return_type(CORBA::_tc_boolean);
  req->send_deferred();
  Orb::inst().deferredRequest(req._retn(),this); // Register for callback

  if(omniEventsLog::exists())
  {
    WriteLock log;
    output(log.os);
  }
}


void ProxyPushSupplier_i::disconnect_push_supplier()
{
  DB(5,"ProxyPushSupplier_i::disconnect_push_supplier()");
  eraseKey("ConsumerAdmin/ProxyPushSupplier");
  deactivateObject();
  if(CORBA::is_nil(_target))
  {
    throw CORBA::OBJECT_NOT_EXIST(
      IFELSE_OMNIORB4(omni::OBJECT_NOT_EXIST_NoMatch,0),
      CORBA::COMPLETED_NO
    );
  }
  else
  {
    CORBA::Request_var req=_target->_request("disconnect_push_consumer");
    _target=CosEventComm::PushConsumer::_nil();
    req->send_deferred();
    Orb::inst().deferredRequest(req._retn());
  }
}


ProxyPushSupplier_i::ProxyPushSupplier_i(
  PortableServer::POA_ptr poa,
  EventQueue&             q
)
: Proxy(poa),
  EventQueue::Reader(q),
  _target(CosEventComm::PushConsumer::_nil()),
  _targetIsProxy(false)
{
  // pass
}

ProxyPushSupplier_i::~ProxyPushSupplier_i()
{
  DB(20,"~ProxyPushSupplier_i()")
}

OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(ProxyPushSupplier_i)

inline void ProxyPushSupplier_i::trigger(bool& busy, bool& waiting)
{
  if(!CORBA::is_nil(_req) && _req->poll_response()) // response has arrived
  {
    CORBA::Environment_ptr env=_req->env(); // No need to free environment.
    if(!CORBA::is_nil(env) && env->exception())
    {
      // Shut down the connection
      CORBA::Exception* ex =env->exception(); // No need to free exception.
      DB(10,"ProxyPushSupplier got exception" IF_OMNIORB4(": "<<ex->_name()) );
      if (!CORBA::is_nil(_target)) {
        Orb::inst().reportObjectFailure(HERE,_target.in(),ex);
        _req=CORBA::Request::_nil();

        // Try to notify the Consumer that the connection is closing.
        CORBA::Request_var req=_target->_request("disconnect_push_consumer");
        req->send_deferred();
        Orb::inst().deferredRequest(req._retn());
      }

      _target=CosEventComm::PushConsumer::_nil(); // disconnected.
      eraseKey("ConsumerAdmin/ProxyPushSupplier");
      deactivateObject();
      return; // No more work to do
    }
    _req=CORBA::Request::_nil();
    busy=true;
  }
  if(CORBA::is_nil(_req) && !CORBA::is_nil(_target) && moreEvents())
  {
    _req=_target->_request("push");
    _req->add_in_arg() <<= *(nextEvent());
    _req->send_deferred();
    busy=true;
  }
  if(!CORBA::is_nil(_req)) // More work to do, if _req NOT nil.
      waiting=true;
}


void ProxyPushSupplier_i::callback(CORBA::Request_ptr req)
{
  if(_targetIsProxy)
  {
    // There should only ever be one of these callbacks per proxy,
    // because each proxy should only be connected once.
    DB(2,"WARNING: Multiple connections to ProxyPushSupplier.");
  }
  else if(req->return_value()>>=CORBA::Any::to_boolean(_targetIsProxy))
  {
    if(_targetIsProxy && omniEventsLog::exists())
    {
      WriteLock log;
      output(log.os);
      DB(15,"ProxyPushSupplier is federated.");
    }
  }
  else
  {
    DB(2,"ProxyPushSupplier got unexpected callback.");
    _targetIsProxy=false; // Reset it just to be sure.
  }
}


void ProxyPushSupplier_i::reincarnate(
  const string&      oid,
  const PersistNode& node
)
{
  try
  {
    using namespace CosEventChannelAdmin;

    string ior( node.attrString("IOR").c_str() );
    CosEventComm::PushConsumer_var pushConsumer =
      string_to_<CosEventComm::PushConsumer>(ior.c_str());
    // Do not activate until we know that we have read a valid target.
    activateObjectWithId(oid.c_str());
    _remove_ref();
    _target=pushConsumer._retn();
    _targetIsProxy=bool(node.attrLong("proxy"));

    // If pushConsumer is a proxy, then try to reconnect.
    if(_targetIsProxy)
    {
      DB(15,"Attempting to reconnect ProxyPushSupplier: "<<oid.c_str())
      // This will only work if the proxy is implemented in the same way as
      // omniEvents, so connect_() automatically creates a proxy.
      ProxyPushConsumer_var proxyCons =
        string_to_<ProxyPushConsumer>(ior.c_str());
      CosEventComm::PushSupplier_var thisSupp =_this();
      proxyCons->connect_push_supplier(thisSupp);
      DB(7,"Reconnected ProxyPushSupplier: "<<oid.c_str())
    }
  }
  catch(CosEventChannelAdmin::AlreadyConnected&){ // connect_push_supplier()
    // The supplier doesn't need to be reconnected.
    DB(7,"Remote ProxyPushConsumer already connected: "<<oid.c_str())
  }
  catch(CosEventChannelAdmin::TypeError&){ // connect_push_supplier()
    // Don't know what to make of this...
    DB(2,"Remote ProxyPushConsumer threw TypeError: "<<oid.c_str())
  }
  catch(CORBA::OBJECT_NOT_EXIST&) {} // object 'pushConsumer' not responding.
  catch(CORBA::TRANSIENT&       ) {} // object 'pushConsumer' not responding.
  catch(CORBA::COMM_FAILURE&    ) {} // object 'pushConsumer' not responding.
}


void ProxyPushSupplier_i::output(ostream &os)
{
  basicOutput(
    os,"ConsumerAdmin/ProxyPushSupplier",
    _target.in(),
    _targetIsProxy? " proxy=1": NULL
  );
}


}; // end namespace OmniEvents
