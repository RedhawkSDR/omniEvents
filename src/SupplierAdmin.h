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

#ifndef OMNIEVENTS__SUPPLIERADMIN_H
#define OMNIEVENTS__SUPPLIERADMIN_H

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
#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class EventChannel_i;
class ProxyPushConsumer_i;
class ProxyPullConsumerManager;
class PersistNode;

class SupplierAdmin_i
: public virtual POA_CosEventChannelAdmin::SupplierAdmin,
  public PortableServer::RefCountServantBase,
  public Servant
{
public: // CORBA interface methods
  CosEventChannelAdmin::ProxyPushConsumer_ptr obtain_push_consumer();
  CosEventChannelAdmin::ProxyPullConsumer_ptr obtain_pull_consumer();

public:
  SupplierAdmin_i(const EventChannel_i& channel, PortableServer::POA_ptr poa);
  virtual ~SupplierAdmin_i();
  OMNIEVENTS__DEBUG_REF_COUNTS__DECL

  /** Collects all events that have arrived since the last call. */
  void collect(list<CORBA::Any*>& events);

  /** Send disconnect_XXX_supplier() to all connected consumers. */    
  void disconnect();

  /** Populate this servant from log information. */
  void reincarnate(const PersistNode& node);

  /** Save this object's state to a stream. */
  void output(ostream& os);

  long  queueLength();
private:
  const EventChannel_i&     _channel;
  ProxyPushConsumer_i*      _pushConsumer;
  ProxyPullConsumerManager* _pullConsumer;
  list<CORBA::Any*>         _queue; ///< Incoming queue for the PushConsumer.

  /** Next time to retry pull (sec,nsec).
   * Set to (0,0) when we should pull every cycle. */
  pair<unsigned long,unsigned long> _nextPull;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__SUPPLIERADMIN_H
