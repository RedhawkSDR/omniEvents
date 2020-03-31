//                            Package   : omniEvents
// ProxyPushConsumer.cc       Created   : 2003/12/04
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

#include "ProxyPushConsumer.h"
#include "ConsumerAdmin.h"
#include "Orb.h"
#include "omniEventsLog.h"
#include "PersistNode.h"

#include <assert.h>

namespace OmniEvents {

void ProxyPushConsumer_i::connect_push_supplier(
  CosEventComm::PushSupplier_ptr pushSupplier)
{
  // pushSupplier is permitted to be nil.
  if(CORBA::is_nil(pushSupplier))
      return;

  string oidstr =currentObjectId();
  Connections_t::iterator pos =_connections.find(oidstr);

  if(pos!=_connections.end())
      throw CosEventChannelAdmin::AlreadyConnected();

  Connection* newConnection =
    new Connection(
          _channelName.in(),
          oidstr,
          CosEventComm::PushSupplier::_duplicate(pushSupplier)
        );
  _connections.insert( Connections_t::value_type(oidstr,newConnection) );

  // Test to see whether pushSupplier is a ProxyPushSupplier.
  // If so, then we will aggressively try to reconnect, when we are reincarnated
  CORBA::Request_var req =pushSupplier->_request("_is_a");
  req->add_in_arg() <<= CosEventChannelAdmin::_tc_ProxyPushSupplier->id();
  req->set_return_type(CORBA::_tc_boolean);
  req->send_deferred();
  Orb::inst().deferredRequest(req._retn(),newConnection); // Register callback

  if(omniEventsLog::exists())
  {
    WriteLock log;
    newConnection->output(log.os);
  }
}


void ProxyPushConsumer_i::disconnect_push_consumer()
{
#ifdef HAVE_OMNIORB4
  DB(5,"ProxyPushConsumer_i::disconnect_push_consumer()")
  string oidstr =currentObjectId();
  Connections_t::iterator pos =_connections.find(oidstr);

  if(pos!=_connections.end())
  {
    CORBA::Request_var req =
      pos->second->_target->_request("disconnect_push_supplier");
    pos->second->_remove_ref();
    _connections.erase(pos);
    // The following line could result in a reentrant callback, if this call was
    // not made through the POA => must erase the connection BEFORE this point.
    req->send_deferred();
    Orb::inst().deferredRequest(req._retn());
    if(omniEventsLog::exists())
    {
      // Erase this connection from the log file.
      WriteLock log;
      log.os<<"-ecf/"<<_channelName.in();
      log.os<<"/SupplierAdmin/ProxyPushConsumer/"<<oidstr<<'\n';
    }
  }
#else /* Silently ignore disconnects with omniORB3 */
  DB(5,"Ignoring disconnect_push_consumer(). Upgrade to omniORB4!")
#endif
}


void ProxyPushConsumer_i::push(const CORBA::Any& event)
{
#ifdef OMNIEVENTS_REAL_TIME_PUSH
  if(!_useLocalQueue)
  {
    _consumerAdmin.send(new CORBA::Any(event));
    _useLocalQueue=true;
  }
  else
#endif
    _queue.push_back(new CORBA::Any(event));
}


ProxyPushConsumer_i::ProxyPushConsumer_i(
  PortableServer::POA_ptr p,
  list<CORBA::Any*>&      q,
  ConsumerAdmin_i&        consumerAdmin
)
: Servant(PortableServer::POA::_nil()),
  _connections(),
  _channelName(p->the_name()),
  _consumerAdmin(consumerAdmin),
  _queue(q),
  _useLocalQueue(false)
{
  _consumerAdmin._add_ref();

  using namespace PortableServer;

  // POLICIES:
  //  Lifespan          =PERSISTENT             // we can persist
  //  Assignment        =USER_ID                // write our own oid
  //  Uniqueness        =MULTIPLE_ID            // only one servant
  //  ImplicitActivation=NO_IMPLICIT_ACTIVATION // disable auto activation
  //  RequestProcessing =USE_DEFAULT_SERVANT    // only one servant
  //  ServantRetention  =NON_RETAIN             // stateless POA
  //  Thread            =SINGLE_THREAD_MODEL    // keep it simple

  CORBA::PolicyList policies;
  policies.length(7);
  policies[0]=p->create_lifespan_policy(PERSISTENT);
  policies[1]=p->create_id_assignment_policy(USER_ID);
  policies[2]=p->create_id_uniqueness_policy(MULTIPLE_ID);
  policies[3]=p->create_implicit_activation_policy(NO_IMPLICIT_ACTIVATION);
  policies[4]=p->create_request_processing_policy(USE_DEFAULT_SERVANT);
  policies[5]=p->create_servant_retention_policy(NON_RETAIN);
  policies[6]=p->create_thread_policy(SINGLE_THREAD_MODEL);

  try
  {  
    // Create a POA for this proxy type in this channel.
    string          poaName =string(_channelName.in())+".ProxyPushConsumer";
    POAManager_var  parentManager =p->the_POAManager();
    _poa=p->create_POA(poaName.c_str(),parentManager.in(),policies);
  }
  catch(POA::AdapterAlreadyExists&) // create_POA
  {
    DB(0,"ProxyPushConsumer_i::ProxyPushConsumer_i() - "
          "POA::AdapterAlreadyExists")
  }
  catch(POA::InvalidPolicy& ex) // create_POA
  {
    DB(0,"ProxyPushConsumer_i::ProxyPushConsumer_i() - "
          "POA::InvalidPolicy: "<<ex.index)
  }

  // Destroy the policy objects (Not strictly necessary in omniORB)
  for(CORBA::ULong i=0; i<policies.length(); ++i)
      policies[i]->destroy();

  // This object is the POA's default servant.
  _poa->set_servant(this);
}


ProxyPushConsumer_i::~ProxyPushConsumer_i()
{
  DB(20,"~ProxyPushConsumer_i()")
  for(Connections_t::iterator i =_connections.begin();
                              i!=_connections.end();
                            ++i)
  {
    i->second->_remove_ref();
  }
  _connections.clear();

  _consumerAdmin._remove_ref();
}


CosEventChannelAdmin::ProxyPushConsumer_ptr
ProxyPushConsumer_i::createObject()
{
  return createNarrowedReference<CosEventChannelAdmin::ProxyPushConsumer>(
           _poa.in(),
           CosEventChannelAdmin::_tc_ProxyPushConsumer->id()
         );
}


void ProxyPushConsumer_i::disconnect()
{
  // Note. We are (probably) in the EventChannel's thread.
  Connections_t::iterator curr,next=_connections.begin();
  while(next!=_connections.end())
  {
    curr=next++;
    CORBA::Request_var req =
      curr->second->_target->_request("disconnect_push_supplier");
    curr->second->_remove_ref();
    _connections.erase(curr);
    // The following line could result in a reentrant callback
    // => must erase the connection BEFORE this point.
    req->send_deferred();
    Orb::inst().deferredRequest(req._retn());
  }
}


void ProxyPushConsumer_i::reincarnate(const PersistNode& node)
{
  // Reincarnate all connections from node's children.
  for(map<string,PersistNode*>::const_iterator i=node._child.begin();
      i!=node._child.end();
      ++i)
  {
    const char* oidstr =i->first.c_str();
    string      ior( i->second->attrString("IOR") );
    bool        isProxy( i->second->attrLong("proxy") );
    assert(_connections.find(oidstr)==_connections.end());
    try
    {
      using namespace CosEventComm;
      using namespace CosEventChannelAdmin;

      PushSupplier_var supp =string_to_<PushSupplier>(ior.c_str());
      _connections.insert(Connections_t::value_type(
        oidstr,
        new Connection(_channelName.in(),oidstr,supp._retn(),isProxy)
      ));
      DB(5,"Reincarnated ProxyPushConsumer: "<<oidstr)

      // If supp is a ProxyPushSupplier, then try to reconnect.
      if(isProxy)
      {
        DB(15,"Attempting to reconnect ProxyPushConsumer: "<<oidstr)
        // This will only work if the proxy is implemented in the same way as
        // omniEvents, so connect_() automatically creates a proxy.
        ProxyPushSupplier_var proxySupp =
          string_to_<ProxyPushSupplier>(ior.c_str());
        PortableServer::ObjectId_var objectId =
          PortableServer::string_to_ObjectId(oidstr);
        CORBA::Object_var obj =
          _poa->create_reference_with_id(
            objectId.in(),
            CosEventChannelAdmin::_tc_ProxyPushConsumer->id()
          );
        PushConsumer_var thisCons =CosEventComm::PushConsumer::_narrow(obj);
        proxySupp->connect_push_consumer(thisCons.in());
        DB(7,"Reconnected ProxyPushConsumer: "<<oidstr)
      }
    }
    catch(CORBA::BAD_PARAM&) {
      // This will happen when IOR fails to narrow.
      DB(5,"Failed to reincarnate ProxyPushConsumer: "<<oidstr)
    }
    catch(CosEventChannelAdmin::AlreadyConnected&){ //connect_push_consumer()
      // The supplier doesn't need to be reconnected.
      DB(7,"Remote ProxyPushSupplier already connected: "<<oidstr)
    }
    catch(CosEventChannelAdmin::TypeError&){ // connect_push_consumer()
      // Don't know what to make of this...
      DB(2,"Remote ProxyPushSupplier threw TypeError: "<<oidstr)
    }
    catch(CORBA::OBJECT_NOT_EXIST&) {} // object 'supp' not responding.
    catch(CORBA::TRANSIENT&       ) {} // object 'supp' not responding.
    catch(CORBA::COMM_FAILURE&    ) {} // object 'supp' not responding.
  } // end loop for(i)
}


void ProxyPushConsumer_i::output(ostream& os) const
{
  for(Connections_t::const_iterator i=_connections.begin();
      i!=_connections.end();
      ++i)
  {
    i->second->output(os);
  }
}


string ProxyPushConsumer_i::currentObjectId() const
{
#ifdef HAVE_OMNIORB4
  try
  {
    using namespace PortableServer;
    ObjectId_var oid =Orb::inst()._POACurrent->get_object_id();
    CORBA::String_var oidStr =ObjectId_to_string(oid.in());
    return string(oidStr.in());
  }
  catch(PortableServer::Current::NoContext&) // get_object_id()
  {
    DB(0,"No context!!")
  }
  catch(CORBA::BAD_PARAM&) // ObjectId_to_string()
  {
    // Should never get here in omniORB, because ObjectID is a char*.
    assert(0);
  }
  return "ERROR";
#else
  throw CORBA::NO_IMPLEMENT();
#endif
}


//
//  ProxyPushConsumer_i::Connection
//

#if OMNIEVENTS__DEBUG_SERVANT
int ProxyPushConsumer_i::Connection::_objectCount =0;
#endif

ProxyPushConsumer_i::Connection::Connection(
  const char*                    channelName,
  const string&                  oidstr,
  CosEventComm::PushSupplier_ptr pushSupplier,
  bool                           isProxy
):Callback(),
  _channelName(channelName),
  _oidstr(oidstr),
  _target(pushSupplier),
  _targetIsProxy(isProxy)
{
#if OMNIEVENTS__DEBUG_SERVANT
  ++_objectCount;
  DB(21,"ProxyPushConsumer_i::Connection::Connection() count="<<_objectCount)
#endif
      CORBA::String_var _iorstr;
  _iorstr = Orb::inst()._orb->object_to_string(_target.in());  
  _target_sior=static_cast<const char*>(_iorstr);
}

ProxyPushConsumer_i::Connection::~Connection()
{
#if OMNIEVENTS__DEBUG_SERVANT
  --_objectCount;
  DB(20,"ProxyPushConsumer_i::Connection::~Connection() count="<<_objectCount)
#else
  DB(20,"ProxyPushConsumer_i::Connection::~Connection()")
#endif
}

OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(ProxyPushConsumer_i::Connection)

void ProxyPushConsumer_i::Connection::callback(CORBA::Request_ptr req)
{
  bool save =_targetIsProxy;
  if(req->return_value()>>=CORBA::Any::to_boolean(_targetIsProxy))
  {
    if(_targetIsProxy && omniEventsLog::exists())
    {
      WriteLock log;
      output(log.os);
      DB(15,"ProxyPushConsumer is federated.");
    }
  }
  else
  {
    DB(2,"ProxyPushConsumer got unexpected callback.");
    _targetIsProxy=save; // Reset it just to be sure.
  }
}

void ProxyPushConsumer_i::Connection::output(ostream& os) const
{
  os<<"ecf/"<<_channelName;
  os<<"/SupplierAdmin/ProxyPushConsumer/"<<_oidstr;

  os<<" IOR="<<_target_sior;
  if(_targetIsProxy)
      os<<" proxy=1";
  os<<" ;;\n";
}


}; // end namespace OmniEvents
