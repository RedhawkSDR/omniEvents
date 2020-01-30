//                            Package   : omniEvents
// ConsumerAdmin.cc           Created   : 2003/12/04
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

#include "ConsumerAdmin.h"

#include "EventChannel.h"
#include "ProxyPushSupplier.h"
#include "ProxyPullSupplier.h"
#include "Orb.h"
#include "PersistNode.h"
#include "Filter.h"

namespace OmniEvents {


CosEventChannelAdmin::ProxyPushSupplier_ptr
ConsumerAdmin_i::obtain_push_supplier()
{
  if(!_pushSupplier)
      _pushSupplier=new ProxyPushSupplierManager(_poa,_queue);
  return _pushSupplier->createObject();
}


CosEventChannelAdmin::ProxyPullSupplier_ptr
ConsumerAdmin_i::obtain_pull_supplier()
{
  if(!_pullSupplier)
      _pullSupplier=new ProxyPullSupplierManager(_channel,_poa,_queue);
  return _pullSupplier->createObject();
}


ConsumerAdmin_i::ConsumerAdmin_i(
  const EventChannel_i&   channel,
  PortableServer::POA_ptr poa
)
: Servant(poa),
  _channel(channel),
  _queue(channel.maxQueueLength()),
  _pushSupplier(NULL),
  _pullSupplier(NULL)
{
  if(_channel.properties().hasAttr("FilterId"))
  {
    string rid =_channel.properties().attrString("FilterId");
    _queue.setFilter(new FilterByRepositoryId(rid.c_str()));
  }
  else if(_channel.properties().hasAttr("FilterKind"))
  {
    CORBA::TCKind kind =
      CORBA::TCKind(_channel.properties().attrLong("FilterKind"));
    _queue.setFilter(new FilterByTCKind(kind));
  }

  activateObjectWithId("ConsumerAdmin");
}


ConsumerAdmin_i::~ConsumerAdmin_i()
{
  DB(20,"~ConsumerAdmin_i()")
  if(_pushSupplier)
  {
    _pushSupplier->_remove_ref(); // terminates thread.
    _pushSupplier=NULL;
  }
  if(_pullSupplier)
  {
    _pullSupplier->_remove_ref();
    _pullSupplier=NULL;
  }
}


OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(ConsumerAdmin_i)


void ConsumerAdmin_i::send(CORBA::Any* event)
{
  ProxyPushSupplierManager::PauseThenWake p(_pushSupplier);
  _queue.append(event);
}


void ConsumerAdmin_i::send(list<CORBA::Any*>& events)
{
  if(!events.empty())
  {
    ProxyPushSupplierManager::PauseThenWake p(_pushSupplier);
    for(list<CORBA::Any*>::iterator i=events.begin(); i!=events.end(); ++i)
        _queue.append( *i );
    events.clear();
  }
}


void ConsumerAdmin_i::disconnect()
{
  if(_pushSupplier)
     _pushSupplier->disconnect();
  if(_pullSupplier)
     _pullSupplier->disconnect();
}


void ConsumerAdmin_i::reincarnate(const PersistNode& node)
{
  // Build Push Supplier proxies
  PersistNode* pushsNode =node.child("ProxyPushSupplier");
  if(pushsNode && !pushsNode->_child.empty())
  {
    _pushSupplier=new ProxyPushSupplierManager(_poa,_queue);
    _pushSupplier->reincarnate(*pushsNode);
  }

  // Build Pull Supplier proxies
  PersistNode* pullsNode =node.child("ProxyPullSupplier");
  if(pullsNode && !pullsNode->_child.empty())
  {
    _pullSupplier=new ProxyPullSupplierManager(_channel,_poa,_queue);
    _pullSupplier->reincarnate(*pullsNode);
  }
}


void ConsumerAdmin_i::output(ostream& os)
{
  if(_pushSupplier)
  {
    _pushSupplier->output(os);
  }
  if(_pullSupplier)
  {
    _pullSupplier->output(os);
  }
}



  long ConsumerAdmin_i::queueLength() {
    return _queue.size();
  }

}; // end namespace OmniEvents
