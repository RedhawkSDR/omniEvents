//                            Package   : omniEvents
// ProxyPullSupplier.cc       Created   : 2003/12/04
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

#include "ProxyPullSupplier.h"
#include "EventChannel.h"
#include "Orb.h"
#include "omniEventsLog.h"
#include "PersistNode.h"
#include <assert.h>

namespace OmniEvents {

//
//  ProxyPullSupplierManager
//

PortableServer::Servant ProxyPullSupplierManager::incarnate(
  const PortableServer::ObjectId& oid,
  PortableServer::POA_ptr         poa
)
{
  // Evict the oldest proxy servant, if we have reached the maximum number.
  if(_servants.size()>=_channel.maxNumProxies())
  {
    ProxyPullSupplier_i* oldest =NULL;
    unsigned long        age    =0;
    for(set<Proxy*>::iterator i=_servants.begin(); i!=_servants.end(); ++i)
        if(!oldest || dynamic_cast<ProxyPullSupplier_i*>(*i)->timestamp()<age)
        {
          oldest=dynamic_cast<ProxyPullSupplier_i*>(*i);
          age=oldest->timestamp();
        }
    DB(5,"Evicting oldest ProxyPullSupplier to make space for a new one")
    try{ oldest->disconnect_pull_supplier(); }catch(CORBA::OBJECT_NOT_EXIST&){}
  }
  // Make a new servant.
  ProxyPullSupplier_i* result =new ProxyPullSupplier_i(_managedPoa,_queue);
  _servants.insert(result);
  return result;
}

ProxyPullSupplierManager::ProxyPullSupplierManager(
  const EventChannel_i&   channel,
  PortableServer::POA_ptr parentPoa,
  EventQueue&             q
)
: ProxyManager(parentPoa),
  _queue(q),
  _channel(channel)
{
  ProxyManager::activate("ProxyPullSupplier");
}

ProxyPullSupplierManager::~ProxyPullSupplierManager()
{
  DB(20,"~ProxyPullSupplierManager()")
}

OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(ProxyPullSupplierManager)

CosEventChannelAdmin::ProxyPullSupplier_ptr
ProxyPullSupplierManager::createObject()
{  
  return createNarrowedReference<CosEventChannelAdmin::ProxyPullSupplier>(
           _managedPoa.in(),
           CosEventChannelAdmin::_tc_ProxyPullSupplier->id()
         );
}

void ProxyPullSupplierManager::disconnect()
{
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    ProxyPullSupplier_i* pps =dynamic_cast<ProxyPullSupplier_i*>(*i);
    // We are in the EventChannel's thread.
    // Make sure all calls go though the ProxyPullSupplier POA.
    CosEventChannelAdmin::ProxyPullSupplier_var ppsv =pps->_this(); 
    ppsv->disconnect_pull_supplier();

  }
}


//
//  ProxyPullSupplier_i
//

// CORBA interface methods

void ProxyPullSupplier_i::connect_pull_consumer(
  CosEventComm::PullConsumer_ptr pullConsumer
)
{
  if(_connected || !CORBA::is_nil(_target) || !CORBA::is_nil(_req))
      throw CosEventChannelAdmin::AlreadyConnected();
  touch();
  _connected=true;
  if(!CORBA::is_nil(pullConsumer))
      _target=CosEventComm::PullConsumer::_duplicate(pullConsumer);

  if(omniEventsLog::exists())
  {
    WriteLock log;
    output(log.os);
  }
}

void ProxyPullSupplier_i::disconnect_pull_supplier()
{
  DB(5,"ProxyPullSupplier_i::disconnect_pull_supplier()");
  touch();
  eraseKey("ConsumerAdmin/ProxyPullSupplier");
  deactivateObject();
  if(!_connected)
  {
    throw CORBA::OBJECT_NOT_EXIST(
      IFELSE_OMNIORB4(omni::OBJECT_NOT_EXIST_NoMatch,0),
      CORBA::COMPLETED_NO
    );
  }
  else if(!CORBA::is_nil(_target))
  {
    CORBA::Request_var req=_target->_request("disconnect_pull_consumer");
    _target=CosEventComm::PullConsumer::_nil();
    req->send_deferred();
    Orb::inst().deferredRequest(req._retn());
  }
}

CORBA::Any* ProxyPullSupplier_i::pull()
{
  if(!_connected)
      throw CosEventComm::Disconnected();
  touch();
  if(moreEvents())
      return new CORBA::Any(*nextEvent());
  else
      throw CORBA::TRANSIENT(
        IFELSE_OMNIORB4(omni::TRANSIENT_CallTimedout,0),
        CORBA::COMPLETED_NO
      );
}

CORBA::Any* ProxyPullSupplier_i::try_pull(CORBA::Boolean& has_event)
{
  if(!_connected)
      throw CosEventComm::Disconnected();
  touch();
  if(moreEvents())
  {
    has_event=1;
    return new CORBA::Any(*nextEvent());
  }
  else
  {
    has_event=0;
    return new CORBA::Any();
  }
}

//

ProxyPullSupplier_i::ProxyPullSupplier_i(
  PortableServer::POA_ptr poa,
  EventQueue& q
)
: Proxy(poa),
  EventQueue::Reader(q),
  _target(CosEventComm::PullConsumer::_nil()),
  _connected(false),
  _timestamp(0)
{
  touch();
}

ProxyPullSupplier_i::~ProxyPullSupplier_i()
{
  DB(20,"~ProxyPullSupplier_i()")
}

void ProxyPullSupplier_i::reincarnate(
  const string&      oid,
  const PersistNode& node
)
{
  CosEventComm::PullConsumer_var pullConsumer =
    string_to_<CosEventComm::PullConsumer>(node.attrString("IOR").c_str());
  // Do not activate until we know that we have read a valid target.
  activateObjectWithId(oid.c_str());
  connect_pull_consumer(pullConsumer.in());
}

void ProxyPullSupplier_i::output(ostream& os)
{
  basicOutput(os,"ConsumerAdmin/ProxyPullSupplier",_target.in());
}

inline void ProxyPullSupplier_i::touch()
{
  unsigned long nsec; // dummy
  omni_thread::get_time(&_timestamp,&nsec);
}

}; // end namespace OmniEvents
