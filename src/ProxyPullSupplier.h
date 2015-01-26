//                            Package   : omniEvents
// ProxyPullSupplier.h        Created   : 2003/12/04
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

#ifndef OMNIEVENTS__PROXYPULLSUPPLIER_H
#define OMNIEVENTS__PROXYPULLSUPPLIER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "ProxyManager.h"
#include "EventQueue.h"

#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class EventChannel_i;

class ProxyPullSupplierManager
: public ProxyManager,
  public PortableServer::RefCountServantBase
{
public: // CORBA interface methods
  PortableServer::Servant incarnate(
    const PortableServer::ObjectId& oid,
    PortableServer::POA_ptr         poa
  );
public:
  ProxyPullSupplierManager(
    const EventChannel_i&   channel,
    PortableServer::POA_ptr parentPoa,
    EventQueue&             q
  );
  ~ProxyPullSupplierManager();
  OMNIEVENTS__DEBUG_REF_COUNTS__DECL
  CosEventChannelAdmin::ProxyPullSupplier_ptr createObject();

  /** Send disconnect_pull_consumer() to all connected PullConsumers. */    
  void disconnect();

private:
  const EventChannel_i& _channel;
  EventQueue& _queue; ///< Reference to queue shared with ProxyPushSuppliers.
  int         _maxNumProxies; ///< Upper limit on number of proxies.
};


/** Servant for ProxyPullSupplier interface. Does not properly implement the
 * blocking pull() method - to do so would need us to create and park a thread
 * for each waiting pull() method. Instead pull() throws TRANSIENT if the
 * event queue is empty.
 */
class ProxyPullSupplier_i
: public virtual POA_CosEventChannelAdmin::ProxyPullSupplier,
  public Proxy,
  public EventQueue::Reader
{
public: // CORBA interface methods
  void connect_pull_consumer(CosEventComm::PullConsumer_ptr pullConsumer);
  void disconnect_pull_supplier();
  CORBA::Any* pull();
  CORBA::Any* try_pull(CORBA::Boolean& has_event);
public:
  ProxyPullSupplier_i(PortableServer::POA_ptr poa, EventQueue& q);
  ~ProxyPullSupplier_i();
  void reincarnate(const string& oid, const PersistNode& node);
  void output(ostream& os);
  inline unsigned long timestamp() const {return _timestamp;}
private:
  CosEventComm::PullConsumer_var _target;
  
  /** Can't use _target to keep track of whether this object is connected,
   * because it is legal to connect with a nil target. Use this bool instead.
   */
  bool _connected;
  /** Keep track of when this proxy was last contacted. */
  unsigned long _timestamp;
  /** Update the _timestamp to the current moment. */
  inline void touch();
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__PROXYPULLSUPPLIER_H
