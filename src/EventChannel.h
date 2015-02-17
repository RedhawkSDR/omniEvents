//                            Package   : omniEvents
// EventsChannel.h            Created   : 2003/12/04
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

#ifndef OMNIEVENTS__EVENTCHANNEL_H
#define OMNIEVENTS__EVENTCHANNEL_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif
#include <string>
#include "Servant.h"
#include "PersistNode.h"
#include "omniEvents.hh"
#include "Mapper.h"
#include "defaults.h"

#include <set>
#include <assert.h>

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

namespace OmniEvents {

class SupplierAdmin_i;
class ConsumerAdmin_i;
class EventChannelStore;

/** Servant for CosEventChannelAdmin::EventChannel objects, also inherits from
 * omni_thread. Each EventChannel contains five POAs, one for each of the proxy
 * types, and one for the XXXAdmins and EventChannel itself. This POA also
 * contains the ProxyManager objects that are used to manage three of the
 * four proxy object types.
 *
 * Here's a summary of the POAs, and their contents:
 *
 * <pre>
 *     +-POA: EventChannel-----------------------------------------------+
 *     |                                                                 |
 *     |                      Obj: EventChannel_i                        |
 *     |                                                                 |
 *     |       Obj: SupplierAdmin_i           Obj: ConsumerAdmin_i       |
 *     |                                                                 |
 *     |                                 Obj: ProxyPushSupplierManager   |
 *     |                                                                 |
 *     |   +-POA: ProxyPushConsumer---+   +-POA: ProxyPushSupplier---+   |
 *     |   |                          |   |                          |   |
 *     |   | Obj: ProxyPushConsumer_i |   | Obj: ProxyPushSupplier_i |   |
 *     |   | (DEFAULT SERVANT)        |   | Obj: ProxyPushSupplier_i |   |
 *     |   |                          |   | Obj: ProxyPushSupplier_i |   |
 *     |   |                          |   | .                        |   |
 *     |   |                          |   | .                        |   |
 *     |   |                          |   | .                        |   |
 *     |   |                          |   |                          |   |
 *     |   +--------------------------+   +--------------------------+   |
 *     |                                                                 |
 *     |  Obj: ProxyPullConsumerManager  Obj: ProxyPullSupplierManager   |
 *     |                                                                 |
 *     |   +-POA: ProxyPullConsumer---+   +-POA: ProxyPullSupplier---+   |
 *     |   |                          |   |                          |   |
 *     |   | Obj: ProxyPullConsumer_i |   | Obj: ProxyPullSupplier_i |   |
 *     |   | Obj: ProxyPullConsumer_i |   | Obj: ProxyPullSupplier_i |   |
 *     |   | Obj: ProxyPullConsumer_i |   | Obj: ProxyPullSupplier_i |   |
 *     |   | .                        |   | .                        |   |
 *     |   | .                        |   | .                        |   |
 *     |   | .                        |   | .                        |   |
 *     |   |                          |   |                          |   |
 *     |   +--------------------------+   +--------------------------+   |
 *     |                                                                 |
 *     +-----------------------------------------------------------------+
 * </pre>
 *
 * All five POAs are single threaded, and managed by the same POAmanager. This
 * enables the top level event loop (EventChannel::run()) to hold all incoming
 * calls while it passes events from the ProxyConsumers over to the
 * ProxySuppliers. This single threaded model simplifies[*] the implementation
 * and avoids all of the locking overheads of a multi-threaded solution.
 *
 * [*] That's the theory anyway... Actually the single threaded model
 * complicates the underlying implementation, because omniORB treats single
 * threaded POAs as a special case on top of multi-threaded ones, rather than
 * the other way round! Ironic!
 */
class EventChannel_i
: public POA_omniEvents::EventChannel,
  public Servant,
  public omni_thread
{
public: // CORBA interface methods
  CosEventChannelAdmin::ConsumerAdmin_ptr for_consumers();
  CosEventChannelAdmin::SupplierAdmin_ptr for_suppliers();
  void destroy();
  /** 'ping' method inherited from FT::PullMonitorable. */
  CORBA::Boolean is_alive() { return 1; }

public:
  EventChannel_i(EventChannelStore* store=NULL);

  /** Cleans up the _poa, if this object is deleted before its thread starts. */
  ~EventChannel_i();

  /** Creates the channel's POA, and any child objects. Must to be called
   *  just after construction, and before start()/run().
   */
  void activate(const char* channelName, const PersistNode* node =NULL);

  /** Warn about interface change. */
  void start(){DB(0,"It is no longer necessary to call EventChannel::start().")}

  /** Entry point for the channel's thread. Calls mainLoop() and waits for
   * it to exit. Handles any exceptions, and shuts down the channel once the
   * main loop has finished.
   */
  void* run_undetached(void*);

  /** The main loop for a channel. Work is strictly separated into two phases.
   * Most of the time, all of the POAs are active, and receiving incoming calls.
   * Periodically, incoming calls are held and the channel collects new events
   * from the consumers and sends them to the suppliers.
   *
   * Incoming call handlers are all designed to complete in the absolute minimum
   * time. This enables the POAs to be single threaded, and to hold incoming
   * calls without having to wait a long time for ongoing invokations to
   * complete. Sadly, it's not possible to implement ProxyPullSupplier::pull()
   * 'properly' without blocking, so our version just raises TRANSIENT if there
   * is no event immediately available.
   *
   * Outgoing calls are always sent as deferred requests, to avoid blocking
   * while we wait for them to return.
   *
   */
  void mainLoop();

  void _add_ref();
  void _remove_ref(); ///< Shutdown the thread when refCount reaches zero.

  void output(ostream& os);

  std::string  name();

  //
  // Accessors
  ConsumerAdmin_i& consumerAdmin() const
    {assert(_consumerAdmin!=NULL);return *_consumerAdmin;}
  const PersistNode& properties() const
    {return _properties;}

  //
  // Values stored in _properties member.
  CORBA::ULong  pullRetryPeriod_ms() const
    {return _properties.attrLong("PullRetryPeriod_ms",PULL_RETRY_PERIOD_MS);}
  CORBA::ULong  maxQueueLength() const
    {return _properties.attrLong("MaxQueueLength",MAX_QUEUE_LENGTH);}
  CORBA::ULong  maxNumProxies() const
    {return _properties.attrLong("MaxNumProxies",MAX_NUM_PROXIES);}
  unsigned long cyclePeriod_ns() const
    {return _properties.attrLong("CyclePeriod_ns",CYCLE_PERIOD_NS);}

  bool hasMapper() { return ( _mapper != NULL ); };


private:
  /** Construct a new Mapper object, and registers it in the INSPOA. */
  void setInsName(const string v);

  /** Constructs the main POA for this channel.
   * Policies are: PERSISTENT, USER_ID, SINGLE_THREAD_MODEL. POA name is set to
   * channelName.
   */
  void createPoa(const char* channelName);

private:
  EventChannelStore*             _eventChannelStore;
  SupplierAdmin_i*               _supplierAdmin;
  ConsumerAdmin_i*               _consumerAdmin;
  PortableServer::POAManager_var _poaManager;
  bool                           _shutdownRequested;
  PersistNode                    _properties;
  Mapper*                        _mapper;
  omni_mutex                     _lock; //< Protects _refCount
  int                            _refCount;
  std::string                    _name;
};


/** Container for Event Channels. */
class EventChannelStore
{
public:
  EventChannelStore();
  ~EventChannelStore();
  void insert(EventChannel_i* channel);
  void erase(EventChannel_i* channel);
  void output(ostream &os);

  // extend store to support new EventChannelFactoryExt interface
  bool exists( const std::string &cname);
  void empty();
  uint64_t  size() { return _channels.size(); };
  void list( omniEvents::EventChannelInfoList &clist  );
private:
  set<EventChannel_i*> _channels;
  omni_mutex           _lock;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__EVENTCHANNEL_H
