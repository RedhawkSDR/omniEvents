//                            Package   : omniEvents
// SupplierAdmin.h            Created   : 2003/12/04
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

#include "SupplierAdmin.h"

#include "EventChannel.h"
#include "ProxyPushConsumer.h"
#include "ProxyPullConsumer.h"
#include "Orb.h"
#include "PersistNode.h"

#define MILLION 1000000
#define BILLION 1000000000

namespace OmniEvents {

CosEventChannelAdmin::ProxyPushConsumer_ptr
SupplierAdmin_i::obtain_push_consumer()
{
  return _pushConsumer->createObject();
}


CosEventChannelAdmin::ProxyPullConsumer_ptr
SupplierAdmin_i::obtain_pull_consumer()
{
  if(!_pullConsumer)
      _pullConsumer=new ProxyPullConsumerManager(_poa,_queue);
  return _pullConsumer->createObject();
}


SupplierAdmin_i::SupplierAdmin_i(
  const EventChannel_i&   channel,
  PortableServer::POA_ptr poa
)
: Servant(poa),
  _channel(channel),
  _pushConsumer(NULL),
  _pullConsumer(NULL),
  _queue(),
  _nextPull(0,0)
{
  // Initialise _nextPull. Only set it if the cycle period is LESS than the
  // pull retry period - otherwise just pull every cycle.
  if(_channel.pullRetryPeriod_ms() > (_channel.cyclePeriod_ns()/MILLION))
  {
    omni_thread::get_time(&(_nextPull.first),&(_nextPull.second));
  }

  // Always create the ProxyPushConsumer_i default servant. This allows
  // lazy clients to connect suppliers without having to go through the
  // proper procedure - they can make up an appropriate ObjectId, call push()
  // and it will just work (TM).
  // Note: A SupplierAdmin_i is always created by the EventChannel to allow this
  // behaviour.
  _pushConsumer=new ProxyPushConsumer_i(_poa,_queue,_channel.consumerAdmin());

  activateObjectWithId("SupplierAdmin");
}


SupplierAdmin_i::~SupplierAdmin_i()
{
  DB(20,"~SupplierAdmin_i()")
  if(_pullConsumer)
  {
    _pullConsumer->_remove_ref();
    _pullConsumer=NULL;
  }
  if(_pushConsumer)
  {
    delete _pushConsumer;
    _pushConsumer=NULL;
  }
  for(list<CORBA::Any*>::iterator i=_queue.begin(); i!=_queue.end(); ++i)
      delete *i;
}


OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(SupplierAdmin_i)


void SupplierAdmin_i::collect(list<CORBA::Any*>& events)
{
  if(_pullConsumer)
  {
    _pullConsumer->collect();
    if(0==_nextPull.first)
    { // No delay between pulls.
      _pullConsumer->triggerRequest();
    }
    else
    { // Only trigger new pull() calls if `pullRetry' ms have passed.
      pair<unsigned long,unsigned long> now;
      omni_thread::get_time(&(now.first),&(now.second));
      if(now>=_nextPull)
      {
        _pullConsumer->triggerRequest();

        CORBA::ULong p =_channel.pullRetryPeriod_ms();
        do{
          _nextPull.second += (p%1000)*MILLION;                    // nsec
          _nextPull.first  +=  p/1000 + _nextPull.second/BILLION;  // sec
          _nextPull.second %= BILLION;                             // nsec
        } while(now>=_nextPull);
      }
    }
  }
  _pushConsumer->trigger();
  // Pick up events from both pull & push consumers.
  events=_queue;
  _queue.clear();
}


void SupplierAdmin_i::disconnect()
{
  if(_pushConsumer)
     _pushConsumer->disconnect();
  if(_pullConsumer)
     _pullConsumer->disconnect();
}


void SupplierAdmin_i::reincarnate(const PersistNode& node)
{
  // Build Push Consumer proxies
  PersistNode* pushcNode =node.child("ProxyPushConsumer");
  if(pushcNode && !pushcNode->_child.empty())
  {
    assert(_pushConsumer!=NULL);
    _pushConsumer->reincarnate(*pushcNode);
  }

  // Build Pull Consumer proxies
  PersistNode* pullcNode =node.child("ProxyPullConsumer");
  if(pullcNode && !pullcNode->_child.empty())
  {
    if(!_pullConsumer)
        _pullConsumer=new ProxyPullConsumerManager(_poa,_queue);
    _pullConsumer->reincarnate(*pullcNode);
  }
}


void SupplierAdmin_i::output(ostream& os)
{
  if(_pushConsumer)
     _pushConsumer->output(os);
  if(_pullConsumer)
     _pullConsumer->output(os);
}


  long SupplierAdmin_i::queueLength() {
    return _queue.size();
  }


}; // end namespace OmniEvents
