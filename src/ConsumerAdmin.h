//                            Package   : omniEvents
// src/ConsumerAdmin.h        Created   : 2003/12/04
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

#ifndef OMNIEVENTS__CONSUMERADMIN_H
#define OMNIEVENTS__CONSUMERADMIN_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <list>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "Servant.h"
#include "EventQueue.h"
#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class EventChannel_i;
class ProxyPushSupplierManager;
class ProxyPullSupplierManager;
class PersistNode;

class ConsumerAdmin_i
: public virtual POA_CosEventChannelAdmin::ConsumerAdmin,
  public PortableServer::RefCountServantBase,
  public Servant
{
public: // CORBA interface methods
  CosEventChannelAdmin::ProxyPushSupplier_ptr obtain_push_supplier();
  CosEventChannelAdmin::ProxyPullSupplier_ptr obtain_pull_supplier();

public:
  ConsumerAdmin_i(const EventChannel_i& channel, PortableServer::POA_ptr poa);
  virtual ~ConsumerAdmin_i();
  OMNIEVENTS__DEBUG_REF_COUNTS__DECL

  /** Queues a single event for sending to consumers. Takes ownership of the
   * event.
   */
  void send(CORBA::Any* event);

  /** Queues up events for sending to consumers. Takes ownership of the
   * events. On exit, the 'events' parameter is empty.
   */
  void send(list<CORBA::Any*>& events);

  /** Send disconnect_XXX_consumer() to all connected consumers. */
  void disconnect();

  /** Populate this servant from log information. */
  void reincarnate(const PersistNode& node);

  /** Save this object's state to a stream. */
  void output(ostream& os);

  long  queueLength();

private:
  const EventChannel_i&     _channel;
  EventQueue                _queue;
  ProxyPushSupplierManager* _pushSupplier;
  ProxyPullSupplierManager* _pullSupplier;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__CONSUMERADMIN_H
