//                            Package   : omniEvents
// ProxyManager.cc            Created   : 2003/12/04
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

#include "ProxyManager.h"
#include "PersistNode.h"
#include "Orb.h"
#include "omniEventsLog.h"

#include <string>
#include <map>
#include <assert.h>
#include <memory>

namespace OmniEvents {

//
//  ProxyManager
//

void
ProxyManager::etherealize(
  const PortableServer::ObjectId& oid,
  PortableServer::POA_ptr         adapter,
  PortableServer::Servant         serv,
  CORBA::Boolean                  cleanup_in_progress,
  CORBA::Boolean                  remaining_activations
)
{
  auto_ptr<Proxy> narrowed( dynamic_cast<Proxy*>(serv) );
  assert(narrowed.get()!=NULL);
  set<Proxy*>::iterator pos =_servants.find(narrowed.get());
  if(pos!=_servants.end())
      _servants.erase(pos);
  else
      DB(1,"\t\teh? - POA attempted to etherealize unknown servant.");
  // memory freed when narrowed goes out of scope.
}


void ProxyManager::reincarnate(const PersistNode& node)
{
  // Reincarnate all connections from node's children.
  for(map<string,PersistNode*>::const_iterator i=node._child.begin();
      i!=node._child.end();
      ++i)
  {
    assert(i->second!=NULL);
    PortableServer::Servant serv =
      this->incarnate(PortableServer::ObjectId(),_managedPoa);
    Proxy* proxy =dynamic_cast<Proxy*>(serv);
    assert(proxy!=NULL);
    try
    {
      proxy->reincarnate(i->first,*(i->second));
    }
    catch(CORBA::BAD_PARAM& ex)
    {
      // This will happen when IOR fails to narrow.
      DB(5,"Failed to reincarnate proxy: "<<i->first.c_str());
      _servants.erase(proxy);
      delete proxy;
    }
  }
}


void ProxyManager::output(ostream& os)
{
  for(set<Proxy*>::iterator i =_servants.begin(); i!=_servants.end(); ++i)
  {
    (*i)->output(os);
  }
}


ProxyManager::ProxyManager(PortableServer::POA_ptr p)
: Servant(p),
  _servants(),
  _managedPoa(PortableServer::POA::_nil())
{}


void ProxyManager::activate(const char* name)
{
  using namespace PortableServer;

  // POLICIES:
  //  Lifespan          =PERSISTENT             // we can persist
  //  Assignment        =USER_ID                // write our own oid
  //  Uniqueness        =[default] UNIQUE_ID    // one servant per object
  //  ImplicitActivation=NO_IMPLICIT_ACTIVATION // disable auto activation
  //  RequestProcessing =USE_SERVANT_MANAGER
  //  ServantRetention  =[default] RETAIN
  //  Thread            =SINGLE_THREAD_MODEL    // keep it simple

  CORBA::PolicyList policies;
  policies.length(5);
  policies[0]=_poa->create_lifespan_policy(PERSISTENT);
  policies[1]=_poa->create_id_assignment_policy(USER_ID);
  policies[2]=_poa->create_implicit_activation_policy(NO_IMPLICIT_ACTIVATION);
  policies[3]=_poa->create_request_processing_policy(USE_SERVANT_MANAGER);
  policies[4]=_poa->create_thread_policy(SINGLE_THREAD_MODEL);

  try
  {  
    // Create a POA for this proxy type in this channel.
    CORBA::String_var parentName    =_poa->the_name();
    string            poaName       =string(parentName.in())+"."+name;
    POAManager_var    parentManager =_poa->the_POAManager();
    _managedPoa=_poa->create_POA(poaName.c_str(),parentManager.in(),policies);
  }
  catch(POA::AdapterAlreadyExists& ex) // create_POA
  {
    DB(0,"ProxyManager::ProxyManager() - POA::AdapterAlreadyExists")
  }
  catch(POA::InvalidPolicy& ex) // create_POA
  {
    DB(0,"ProxyManager::ProxyManager() - POA::InvalidPolicy: "<<ex.index)
  }

  // Destroy the policy objects (Not strictly necessary in omniORB)
  for(CORBA::ULong i=0; i<policies.length(); ++i)
      policies[i]->destroy();

  string oidStr =string(name)+"Manager";
  activateObjectWithId(oidStr.c_str());
  PortableServer::ServantManager_var manager(_this());
  _managedPoa->set_servant_manager(manager);
}


ProxyManager::~ProxyManager()
{
  // pass
}


//
// Proxy
//


Proxy::~Proxy()
{
  if(!CORBA::is_nil(_req))
  {
    Orb::inst().deferredRequest(_req._retn());
    _req=CORBA::Request::_nil();
  }
}

Proxy::Proxy(PortableServer::POA_ptr poa)
: Servant(poa),
  _req(CORBA::Request::_nil())
{
  // pass
}

void Proxy::keyOutput(ostream& os, const char* name)
{
  PortableServer::POA_var parentPoa=_poa->the_parent();
  CORBA::String_var channelName=parentPoa->the_name();

  PortableServer::ObjectId_var oid=_poa->servant_to_id(this);
  CORBA::String_var oidStr =PortableServer::ObjectId_to_string(oid.in());
  os<<"ecf/"<<channelName.in()<<"/"<<name<<"/"<<oidStr.in();
}

void Proxy::eraseKey(const char* name)
{
  if(omniEventsLog::exists())
  {
    // Remove this key from the persistency logfile.
    WriteLock log;
    log.os<<"-";
    keyOutput(log.os,name);
    log.os<<'\n';
  }
}

void Proxy::basicOutput(
  ostream&          os,
  const char*       name,
  CORBA::Object_ptr target,
  const char*       extraAttributes
)
{
  keyOutput(os,name);
  if(!CORBA::is_nil(target))
  {
    CORBA::String_var iorstr =Orb::inst()._orb->object_to_string(target);
    os<<" IOR="<<iorstr.in();
    if(extraAttributes)
        os<<extraAttributes;
  }
  os<<" ;;\n";
}

void Proxy::basicOutput(
  ostream&          os,
  const char*       name,
  const std::string &iorstr,
  const char*       extraAttributes
)
{
  keyOutput(os,name);
  os<<" IOR="<<iorstr;
  if(extraAttributes)
      os<<extraAttributes;
  os<<" ;;\n";
}
    


}; // end namespace OmniEvents
