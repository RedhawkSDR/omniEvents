//                            Package   : omniEvents
// ProxyPushSupplier.h        Created   : 2003/12/04
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

#ifndef OMNIEVENTS__PROXYPUSHSUPPLIER_H
#define OMNIEVENTS__PROXYPUSHSUPPLIER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "Callback.h"
#include "EventQueue.h"
#include "ProxyManager.h"

#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class ProxyPushSupplierManager
: public ProxyManager,
  public omni_thread
{
public: // CORBA interface methods
  PortableServer::Servant incarnate(
    const PortableServer::ObjectId& oid,
    PortableServer::POA_ptr         poa
  );
  /** Pauses the thread, and then calls the parent's implementation. */
  void etherealize(
    const PortableServer::ObjectId& oid,
    PortableServer::POA_ptr         adapter,
    PortableServer::Servant         serv,
    CORBA::Boolean                  cleanup_in_progress,
    CORBA::Boolean                  remaining_activations
  );
public:
  ProxyPushSupplierManager(PortableServer::POA_ptr parentPoa,EventQueue& q);
  ~ProxyPushSupplierManager();
  CosEventChannelAdmin::ProxyPushSupplier_ptr createObject();

  /** Send disconnect_push_consumer() to all connected PushConsumers. */    
  void disconnect();

  void* run_undetached(void*);
  void _add_ref();
  void _remove_ref(); ///< Shutdown the thread when refCount reaches zero.

  omni_mutex     _lock;
  omni_condition _condition;

  /** Helper class that locks ProxyPushSupplier upon construction, and
   * wakes it up on destruction. By contrast, just locking & unlocking the mutex
   * pauses the thread, but doesn't wake it up any faster than it would have
   * woken anyway. */
  class PauseThenWake
  {
    ProxyPushSupplierManager* _p;
    PauseThenWake(const PauseThenWake&); ///< Dummy, no implementation.
    PauseThenWake(); ///< Dummy, no implementation.
  public:
    inline PauseThenWake(ProxyPushSupplierManager* p);
    inline ~PauseThenWake();
  };

private:
  EventQueue&    _queue;
  int            _refCount;
};


class ProxyPushSupplier_i
: public virtual POA_CosEventChannelAdmin::ProxyPushSupplier,
  public Proxy,
  public EventQueue::Reader,
  public Callback
{
public: // CORBA interface methods
  void connect_push_consumer(CosEventComm::PushConsumer_ptr pushConsumer);
  void disconnect_push_supplier();
public:
  ProxyPushSupplier_i(PortableServer::POA_ptr poa, EventQueue& q);
  ~ProxyPushSupplier_i();
  OMNIEVENTS__DEBUG_REF_COUNTS__DECL

  /** Sets 'busy' if some work was done.
   *  Sets 'waiting' if there is an outstanding request.
   */
  inline void trigger(bool& busy, bool& waiting);

  /** Sets _targetIsProxy, if it is. */
  void callback(CORBA::Request_ptr req);
  void reincarnate(const string& oid, const PersistNode& node);
  void output(ostream &os);
private:
  CosEventComm::PushConsumer_var _target;
  bool _targetIsProxy; ///< TRUE if _target is a ProxyPushConsumer.
};


//
// Inline Implementations.
//

inline ProxyPushSupplierManager::PauseThenWake::PauseThenWake(
  ProxyPushSupplierManager* p
):_p(p)
{
  if(_p)
     _p->_lock.lock();
}

inline ProxyPushSupplierManager::PauseThenWake::~PauseThenWake()
{
  if(_p)
  {
    _p->_lock.unlock();
    _p->_condition.signal(); // Wake up the thread if it's asleep.
  }
}


}; // end namespace OmniEvents

#endif // OMNIEVENTS__PROXYPUSHSUPPLIER_H
