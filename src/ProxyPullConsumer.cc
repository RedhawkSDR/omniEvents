//                            Package   : omniEvents
// ProxyPullConsumer.cc       Created   : 2003/12/04
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

#include "ProxyPullConsumer.h"
#include "Orb.h"
#include "omniEventsLog.h"
#include "PersistNode.h"
#include <assert.h>

namespace OmniEvents {

//
//  ProxyPullConsumerManager
//

PortableServer::Servant
ProxyPullConsumerManager::incarnate(
  const PortableServer::ObjectId& oid,
  PortableServer::POA_ptr         poa
)
{
  DB(20,"ProxyPullConsumerManager::incarnate()")
  ProxyPullConsumer_i* result =new ProxyPullConsumer_i(_managedPoa,_queue);
  _servants.insert(result);
  return result;
}

ProxyPullConsumerManager::ProxyPullConsumerManager(
  PortableServer::POA_ptr parentPoa,
  list<CORBA::Any*>&      q
)
: ProxyManager(parentPoa),
  _queue(q)
{
  ProxyManager::activate("ProxyPullConsumer");
}

ProxyPullConsumerManager::~ProxyPullConsumerManager()
{
  DB(20,"~ProxyPullConsumerManager()")
}

OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(ProxyPullConsumerManager)

CosEventChannelAdmin::ProxyPullConsumer_ptr
ProxyPullConsumerManager::createObject()
{
  return createNarrowedReference<CosEventChannelAdmin::ProxyPullConsumer>(
           _managedPoa.in(),
           CosEventChannelAdmin::_tc_ProxyPullConsumer->id()
         );
}

void ProxyPullConsumerManager::collect()
{
  // Collect events from each servant in turn.
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    ProxyPullConsumer_i* proxy=dynamic_cast<ProxyPullConsumer_i*>(*i);
    proxy->collect();
  }
}

void ProxyPullConsumerManager::triggerRequest()
{
  // Trigger each servant in turn.
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    ProxyPullConsumer_i* proxy=dynamic_cast<ProxyPullConsumer_i*>(*i);
    proxy->triggerRequest();
  }
}

void ProxyPullConsumerManager::disconnect()
{
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    Proxy* p =*i; // Sun's CC requires this temporary.
    ProxyPullConsumer_i* ppc =static_cast<ProxyPullConsumer_i*>(p);
    // We are in the EventChannel's thread.
    // Make sure all calls go though the ProxyPullConsumer POA.
    CosEventChannelAdmin::ProxyPullConsumer_var ppcv =ppc->_this(); 
    ppcv->disconnect_pull_consumer();
  }
}


//
//  ProxyPullConsumer_i
//

// CORBA interface methods

void ProxyPullConsumer_i::connect_pull_supplier(
  CosEventComm::PullSupplier_ptr pullSupplier
)
{
  if(CORBA::is_nil(pullSupplier))
      throw CORBA::BAD_PARAM();
  if(!CORBA::is_nil(_target) || !CORBA::is_nil(_req))
      throw CosEventChannelAdmin::AlreadyConnected();
  _target=CosEventComm::PullSupplier::_duplicate(pullSupplier);

  if(omniEventsLog::exists())
  {
    WriteLock log;
    output(log.os);
  }
}

void ProxyPullConsumer_i::disconnect_pull_consumer()
{
  DB(5,"ProxyPullConsumer_i::disconnect_pull_consumer()");
  eraseKey("SupplierAdmin/ProxyPullConsumer");
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
    CORBA::Request_var req=_target->_request("disconnect_pull_supplier");
    _target=CosEventComm::PullSupplier::_nil();
    req->send_deferred();
    Orb::inst().deferredRequest(req._retn());
  }
}

//

ProxyPullConsumer_i::ProxyPullConsumer_i(
  PortableServer::POA_ptr poa,
  list<CORBA::Any*>&      q
)
: Proxy(poa),
  _target(CosEventComm::PullSupplier::_nil()),
  _queue(q),
  _mode(Pull), // Prefer 'pull' method calls.
  _exceptionCount(0)
{}

ProxyPullConsumer_i::~ProxyPullConsumer_i()
{
  DB(20,"~ProxyPullConsumer_i()")
}

void ProxyPullConsumer_i::collect()
{
  if(!CORBA::is_nil(_req) && _req->poll_response()) ////// RESPONSE HAS ARRIVED
  {
    const char* opname =_req->operation();
    assert(opname);
    CORBA::Environment_ptr env =_req->env(); // No need to release environment.

    if(!CORBA::is_nil(env) && env->exception()) //////////// EXCEPTION OCCURRED
    {
      CORBA::Exception* ex =env->exception(); // No need to free exception.
      DB(10,"ProxyPullConsumer got exception"
           IF_OMNIORB4(<<": "<<ex->_name())<<", op:"<<opname);
      if(0==strcmp("pull",opname) || 0==strcmp("try_pull",opname))
      {
        ++_exceptionCount;
        _mode=( _mode==Pull? TryPull: Pull ); // Try something else next time.
      }
      else
          DB(2,"Ignoring unrecognised response. operation:"<<opname);
      if(_exceptionCount>=4)
      {
        Orb::inst().reportObjectFailure(HERE,_target.in(),ex);

        // Try to notify the Supplier that the connection is closing.
        CORBA::Request_var req=_target->_request("disconnect_pull_supplier");
        req->send_deferred();
        Orb::inst().deferredRequest(req._retn());

        _target=CosEventComm::PullSupplier::_nil(); // disconnected
        eraseKey("SupplierAdmin/ProxyPullConsumer");
        deactivateObject();
      }
    }
    else  //////////////////////////////////////////////// METHOD CALL RETURNED
    {
      // Do we have an event?
      bool hasEvent=false;
      if(0==strcmp("pull",opname))
      {
        hasEvent=true;
      }
      else if(0==strcmp("try_pull",opname))
      {
        CORBA::NVList_ptr args=_req->arguments(); // No need to release args.
        if(args->count()==1)
        {
          CORBA::NamedValue_var hasEventArg=args->item(0);
          if(0==strcmp(hasEventArg->name(),"has_event"))
          {
            CORBA::Any* a =hasEventArg->value();
            CORBA::Boolean b;
            CORBA::Any::to_boolean tb(b); //MS VC++6 is on drugs!
            hasEvent=(((*a)>>=tb) && b);
          }
        }
      }
      // Pick up an event, if we have one.
      if(hasEvent)
      {
        CORBA::Any* event =new CORBA::Any();
        _req->return_value() >>= (*event);
        _queue.push_back(event);
      }
      // Reset the exception count.
      _exceptionCount=0;
    }
    _req=CORBA::Request::_nil();
  }
} // ProxyPullConsumer_i::end collect()

void ProxyPullConsumer_i::triggerRequest()
{
  if(CORBA::is_nil(_req) && !CORBA::is_nil(_target))
  {
    switch(_mode)
    {
      case Pull:
          _req=_target->_request("pull");
          break;
      case TryPull:
          _req=_target->_request("try_pull");
          _req->add_out_arg("has_event")<<=CORBA::Any::from_boolean(1);
          break;
      default:
          assert(0);
    }
    _req->set_return_type(CORBA::_tc_any);
    _req->send_deferred();
  }
}

void ProxyPullConsumer_i::reincarnate(
  const string&      oid,
  const PersistNode& node
)
{
  CosEventComm::PullSupplier_var pullSupplier =
    string_to_<CosEventComm::PullSupplier>(node.attrString("IOR").c_str());
  // Do not activate until we know that we have read a valid target.
  activateObjectWithId(oid.c_str());
  connect_pull_supplier(pullSupplier.in());
}

void ProxyPullConsumer_i::output(ostream& os)
{
  basicOutput(os,"SupplierAdmin/ProxyPullConsumer",_target.in());
}

}; // end namespace OmniEvents
