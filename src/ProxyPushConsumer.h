//                            Package   : omniEvents
// ProxyPushConsumer.h        Created   : 2003/12/04
//                            Author    : Alex Tingle
//
//    Copyright (C) 2003,2005 Alex Tingle.
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

#ifndef OMNIEVENTS__PROXYPUSHCONSUMER_H
#define OMNIEVENTS__PROXYPUSHCONSUMER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>
#include <map>
#include <list>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "Callback.h"
#include "Servant.h"

#include "CosEventChannelAdmin.hh"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class PersistNode;
class ConsumerAdmin_i;

/** Default servant for ProxyPushConsumer objects. All objects are implemented
 * by a single instance of this servant class. This enables us to deal
 * statelessly with anonymous connections (where connect_push_supplier() is
 * called with a NIL callback reference).
 */
class ProxyPushConsumer_i
: public virtual POA_CosEventChannelAdmin::ProxyPushConsumer,
  public Servant
{
public: // CORBA interface methods
  /** If pushSupplier is provided, then it is stored in _connections. Otherwise
   * this method does nothing.
   */
  void connect_push_supplier(CosEventComm::PushSupplier_ptr pushSupplier);

  /** We may not have a record of the supplier, so this method must accept
   * calls from any supplier without complaint.
   */
  void disconnect_push_consumer();

  /** Accepts events from any supplier, not just those stored in _connections.*/
  void push(const CORBA::Any& event);

public:
  ProxyPushConsumer_i(
    PortableServer::POA_ptr parentPoa,
    list<CORBA::Any*>&      q,
    ConsumerAdmin_i&        consumerAdmin
  );
  virtual ~ProxyPushConsumer_i();
  
  void trigger() {_useLocalQueue=false;}

  /** Constructs a new object.
   * This method is almost completely stateless. It makes a new
   * objectId, and returns it to the caller. But, only when it's
   * USED the first time (connect_push_consumer) do we store it in
   * _connections. (Note, The POA doesn't store objects either.)
   */
  CosEventChannelAdmin::ProxyPushConsumer_ptr createObject();

  /** Send disconnect_push_supplier() to all connected PushSuppliers. */    
  void disconnect();

  /** Re-create all servants from information saved in the log file. */
  void reincarnate(const PersistNode& node);
  /** Save this object's state to a stream. */
  void output(ostream& os) const;

private:
  string currentObjectId() const;
  struct Connection : public Callback
  {
    const char*                    _channelName;
    string                         _oidstr;
    CosEventComm::PushSupplier_var _target;
    std::string                    _target_sior;
    bool _targetIsProxy; ///< TRUE if _target is a ProxyPushSupplier

    /** Constructor adopts 'pushSupplier' parameter. */
    Connection(
      const char*                    channelName,
      const string&                  oidstr,
      CosEventComm::PushSupplier_ptr pushSupplier,
      bool                           isProxy=false
    );
    virtual ~Connection();
    OMNIEVENTS__DEBUG_REF_COUNTS__DECL
    /** Sets _targetIsProxy, if it is. */
    void callback(CORBA::Request_ptr req);
    /** Save this object's state to a stream. */
    void output(ostream& os) const;
  private:
    Connection(); ///< NO IMPLEMENTATION
#if OMNIEVENTS__DEBUG_SERVANT
    static int _objectCount;
#endif
  };

  typedef map<string,Connection*> Connections_t;
  Connections_t      _connections;
  CORBA::String_var  _channelName;
  ConsumerAdmin_i&   _consumerAdmin;
  list<CORBA::Any*>& _queue;
  bool               _useLocalQueue; ///< Switch between RT/chunked modes.
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__PROXYPUSHCONSUMER_H
