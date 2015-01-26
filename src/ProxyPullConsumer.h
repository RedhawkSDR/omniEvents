//                            Package   : omniEvents
// ProxyPullCOnsumer.h        Created   : 2003/12/04
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

#ifndef OMNIEVENTS__PROXYPULLCONSUMER_H
#define OMNIEVENTS__PROXYPULLCONSUMER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <list>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "ProxyManager.h"

#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class ProxyPullConsumerManager
: public ProxyManager,
  public PortableServer::RefCountServantBase
{
public: // CORBA interface methods
  PortableServer::Servant incarnate(
    const PortableServer::ObjectId& oid,
    PortableServer::POA_ptr         poa
  );
public:
  ProxyPullConsumerManager(
    PortableServer::POA_ptr parentPoa,
    list<CORBA::Any*>& q
  );
  ~ProxyPullConsumerManager();
  OMNIEVENTS__DEBUG_REF_COUNTS__DECL
  CosEventChannelAdmin::ProxyPullConsumer_ptr createObject();

  /** Collects events that have arrived at connected proxies. For each proxy:
   *
   * - If an exception has arrived, increment the exception count, and switch
   *     to the other operation (pull/try_pull).
   * - If the exception count is 4+, then deactivate this object.
   * - If a method has returned, then decide whether we have an event (hasEvent).
   *     If so, the read the event and push it onto the queue.
   *
   * Should be called BEFORE triggerRequest().
   */
  void collect();

  /** For each connected proxy, if there is no request in progress, send a new
   *  request to the current operation (pull or try_pull).
   *  Should be called AFTER collect().
   */
  void triggerRequest();

  /** Send disconnect_pull_supplier() to all connected PullSuppliers. */    
  void disconnect();

private:
  list<CORBA::Any*>& _queue;
};


/** Implementation of the ProxyPullConsumer interface. Tries to get messages by
 * using pull() then try_pull() method calls, but prefers pull(). Switches
 * between pull() & try_pull() if it gets an exception. Only gives up when both
 * methods have returned two consecutive exceptions.
 *
 * Note, our own implementation of ProxyPullSupplier::pull() is very basic.
 */
class ProxyPullConsumer_i
: public virtual POA_CosEventChannelAdmin::ProxyPullConsumer,
  public Proxy
{
public: // CORBA interface methods
  void connect_pull_supplier(CosEventComm::PullSupplier_ptr pullSupplier);
  void disconnect_pull_consumer();
public:
  ProxyPullConsumer_i(PortableServer::POA_ptr poa, list<CORBA::Any*>& q);
  ~ProxyPullConsumer_i();

  /** Collects responses since the last trigger. */
  void collect();
  /** When _req is NIL, sends out a new pull() or try_pull() call. */
  void triggerRequest();

  void reincarnate(const string& oid, const PersistNode& node);
  void output(ostream& os);
private:
  CosEventComm::PullSupplier_var _target;
  list<CORBA::Any*>&             _queue;
  
  /** This proxy can call out in either pull() or try_pull() mode. */
  enum Mode { Pull=0, TryPull=1 };
  Mode _mode;

  /** Only when two consecutive exceptions have been received from each mode,
   * do we consider the connection dead.
   */
  int _exceptionCount;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__PROXYPULLCONSUMER_H
