//                            Package   : omniEvents
// EventChannel.cc            Created   : 2003/12/04
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

#include "EventChannel.h"
#include "ConsumerAdmin.h"
#include "SupplierAdmin.h"
#include "omniEventsLog.h"
#include "Orb.h"

#include <list>

namespace OmniEvents {

// CORBA interface methods
CosEventChannelAdmin::ConsumerAdmin_ptr EventChannel_i::for_consumers()
{
  if(!_consumerAdmin || _shutdownRequested)
      throw CORBA::OBJECT_NOT_EXIST();
  return _consumerAdmin->_this();
}


CosEventChannelAdmin::SupplierAdmin_ptr EventChannel_i::for_suppliers()
{
  if(!_supplierAdmin || _shutdownRequested)
      throw CORBA::OBJECT_NOT_EXIST();
  return _supplierAdmin->_this();
}


void EventChannel_i::destroy()
{
  if(_shutdownRequested)
      throw CORBA::OBJECT_NOT_EXIST();

  // Prevent further incoming connections.
  _shutdownRequested=true;

  DB(5,"EventChannel_i::destroy check state poaMananger--> state:" << _poaManager->get_state() );
  if (  _poaManager->get_state() ==  PortableServer::POAManager::HOLDING ) {
    _poaManager->activate();
    DB(5,"EventChannel_i::destroy  activate poaMananger--> state:" << _poaManager->get_state() );
  }

  DB(5,"EventChannel_i::destroy()")

  // Send disconnect messages to connected clients.
  if(_consumerAdmin)
     _consumerAdmin->disconnect();
  if(_supplierAdmin)
     _supplierAdmin->disconnect();
}


EventChannel_i::EventChannel_i(EventChannelStore* store)
: Servant(PortableServer::POA::_nil()),
  _eventChannelStore(store),
  _consumerAdmin(NULL),
  _supplierAdmin(NULL),
  _poaManager(),
  _shutdownRequested(false),
  _properties(),
  _mapper(NULL),
  _lock(),
  _refCount(1),
  _name("")
{}


void EventChannel_i::activate(
  const char*        channelName,
  const PersistNode* node
)
{
  // The order of these various initialization methods is very important.
  // I've documented dependencies as 'REQUIRES' comments.
  _name = channelName;
  createPoa(channelName);

  if(node)
      _properties._attr=node->_attr;

  // REQUIRES: _properties
  _consumerAdmin=new ConsumerAdmin_i(*this,_poa);

  // REQUIRES: _consumerAdmin, _properties
  _supplierAdmin=new SupplierAdmin_i(*this,_poa);

  if(node)
  {
    PersistNode* saNode =node->child("SupplierAdmin");
    if(saNode)
        _supplierAdmin->reincarnate(*saNode);

    PersistNode* caNode =node->child("ConsumerAdmin");
    if(caNode)
        _consumerAdmin->reincarnate(*caNode);
  }

  activateObjectWithId("EventChannel");

  // Remove the constructor's reference. This object will now be destroyed when
  // the POA releases it.
  _remove_ref();

  // REQUIRES: activate() ...since it uses _this().
  setInsName(_properties.attrString("InsName"));

  // Start the channel's thread running.
  start_undetached();
}


EventChannel_i::~EventChannel_i()
{
  DB(20,"~EventChannel_i()")
  // Destroy the mapper object, even when the EventChannel is being shut down
  // without a call to destroy(). This can happen if the channel is
  // implemented through libomniEvents - the channel could be shut down and
  // later reincarnated in the same process. The Mapper's lifecycle should
  // match that of the EventChannel.
  if(_mapper)
  {
    _mapper->destroy();
    _mapper=NULL;
  }
  if(_consumerAdmin)
  {
    _consumerAdmin->_remove_ref();
    _consumerAdmin=NULL;
  }
  if(_supplierAdmin)
  {
    _supplierAdmin->_remove_ref();
    _supplierAdmin=NULL;
  }
}


void* EventChannel_i::run_undetached(void*)
{
  // Ensure that activate() is called before start()/run().
  assert(!CORBA::is_nil(_poa));

  const char* action="";
  try
  {
    if(_eventChannelStore)
    {
      action="add this object to the store";
      _eventChannelStore->insert(this);
    }

    if(omniEventsLog::exists())
    {
      action="create this object in the persistency database";
      WriteLock log;
      output(log.os);
    }

    // Process events until the channel is destroyed.
    action="run main loop";
    mainLoop();

    if(_eventChannelStore)
    {
      action="remove this object from the store";
      _eventChannelStore->erase(this);
    }

    if(_shutdownRequested)
    {
      if(omniEventsLog::exists())
      {
        action="remove record from persistency database";
        CORBA::String_var poaName =_poa->the_name();
        WriteLock log;
        log.os<<"-ecf/"<<poaName.in()<<'\n';
      }
      action="destroy POA";
      _poa->destroy(
        CORBA::Boolean(1) /* etherealize_objects */,
        CORBA::Boolean(0) /* wait_for_completion */
      );
      _poa=PortableServer::POA::_nil();

    } // end if(_shutdownRequested)

  }
  catch(PortableServer::POAManager::AdapterInactive& ex) {
    DB(0,"EventChannel_i::run_undetached() - failed to "<<action<<
       ", POA deactivated from the outside.")
  }
  catch (CORBA::SystemException& ex) {
    DB(0,"EventChannel_i::run_undetached() - failed to "<<action<<
       ", System exception: "<<ex._name()<<" ("<<NP_MINORSTRING(ex)<<")")
  }
  catch (CORBA::Exception& ex) {
    DB(0,"EventChannel_i::run_undetached() - failed to "<<action<<
       ", CORBA exception: "<<ex._name())
  }

  // Thread now exits, and this object is deleted.
  return NULL;
}


void EventChannel_i::mainLoop()
{
  _poaManager->activate();
  unsigned long localCyclePeriod_ns=cyclePeriod_ns();
  while(_refCount>0 && !_shutdownRequested)
  {
    //
    // TRANSFER PHASE - transfer events from SupplierAdmin to ConsumerAdmin.
    _poaManager->hold_requests(CORBA::Boolean(1) /* wait_for_completion */);

    if(_shutdownRequested) break;

    list<CORBA::Any*> events;
    _supplierAdmin->collect(events);
    _consumerAdmin->send(events);
    assert(events.empty());

    _poaManager->activate();

    //
    // COMMUNICATION PHASE - talk with clients' suppliers & consumers.
    // Note: On Linux the resolution of nanosleep is a huge 10ms.
    omni_thread::sleep(0,localCyclePeriod_ns);
  }
}


void EventChannel_i::_add_ref()
{
#if OMNIEVENTS__DEBUG_REF_COUNTS
  DB(20,"EventChannel_i::_add_ref()")
#endif
  omni_mutex_lock pause(_lock);
  ++_refCount;
}


void EventChannel_i::_remove_ref()
{
#if OMNIEVENTS__DEBUG_REF_COUNTS
  DB(20,"EventChannel_i::_remove_ref()")
#endif
  int myref;
  {
    omni_mutex_lock pause(_lock);
    myref = --_refCount;
  }

  if(myref<0)
  {
    DB(2,"EventChannel has negative ref count! "<<myref)
  }
  else if(myref==0)
  {
    DB(15,"EventChannel has zero ref count -- shutdown.")
    join(NULL);
  }
}


void EventChannel_i::output(ostream& os)
{
  CORBA::String_var poaName =_poa->the_name();
  string name =string("ecf/")+poaName.in();
  _properties.output(os,name);
  if(_supplierAdmin)
     _supplierAdmin->output(os);
  if(_consumerAdmin)
     _consumerAdmin->output(os);
}


void EventChannel_i::setInsName(const string v)
{

  std::cout << " setInstName  v: " << v << std::endl;
  Mapper* newMapper =NULL;
  try
  {

    // If _insName is set, then create a mapper object to allow clients to
    // find this object with a `corbaloc' string.
    if(!v.empty())
    {
      // !! Throws when there is already an object named 'v' in the INSPOA.
      CORBA::Object_var obj( _this() );
      newMapper=new Mapper(v.c_str(),obj.in());
    }
    // Deactivate the old _mapper object.
    if(_mapper)
       _mapper->destroy();
    _mapper=newMapper;

  }
  catch(...)
  {
    // Can't use an auto_ptr, because MS VC++ 6 has no auto_ptr::reset()
    delete newMapper;
    throw;
  }
}


  std::string EventChannel_i::name( ) {
    std::string retval("");
    if (!CORBA::is_nil(_poa)) {
        CORBA::String_var poaName =_poa->the_name();
        retval=poaName.in();
    }
    else {
      retval = _name;
    }
    return retval;
  }

void EventChannel_i::createPoa(const char* channelName)
{
  using namespace PortableServer;
  POA_ptr p=Orb::inst()._RootPOA.in();

  // POLICIES:
  //  Lifespan          =PERSISTENT             // we can persist
  //  Assignment        =USER_ID                // write our own oid
  //  Uniqueness        =[default] UNIQUE_ID    // one servant per object
  //  ImplicitActivation=[default] IMPLICIT_ACTIVATION // auto activation
  //  RequestProcessing =[default] USE_ACTIVE_OBJECT_MAP_ONLY
  //  ServantRetention  =[default] RETAIN       // stateless POA
  //  Thread            =SINGLE_THREAD_MODEL    // keep it simple

  CORBA::PolicyList policies;
  policies.length(3);
  policies[0]=p->create_lifespan_policy(PERSISTENT);
  policies[1]=p->create_id_assignment_policy(USER_ID);
  policies[2]=p->create_thread_policy(SINGLE_THREAD_MODEL);

  try // finally
  {
      try
      {
        // Create a new POA (and new POAManager) for this channel.
        // The POAManager will be used for all of this channel's POAs.
        if ( channelName )
          std::cout << "createPoa name:"  << channelName << std::endl;
        _poa=p->create_POA(channelName,POAManager::_nil(),policies);
        _poaManager=_poa->the_POAManager();
      }
      catch(POA::AdapterAlreadyExists& ex) // create_POA
      {
        DB(0,"EventChannel_i::createPoa() - POA::AdapterAlreadyExists")
        throw;
      }
      catch(POA::InvalidPolicy& ex) // create_POA
      {
        DB(0,"EventChannel_i::createPoa() - POA::InvalidPolicy: "<<ex.index)
        throw;
      }
  }
  catch(...) // finally
  {
    // Destroy the policy objects (Not strictly necessary in omniORB)
    for(CORBA::ULong i=0; i<policies.length(); ++i)
        policies[i]->destroy();
    throw;
  }

  // Destroy the policy objects (Not strictly necessary in omniORB)
  for(CORBA::ULong i=0; i<policies.length(); ++i)
      policies[i]->destroy();
}


//
// class EventChannelStore
//


EventChannelStore::EventChannelStore()
:_channels(),_lock()
{}

EventChannelStore::~EventChannelStore()
{
  // ?? IMPLEMENT ME
}

void EventChannelStore::insert(EventChannel_i* channel)
{
  omni_mutex_lock l(_lock);
  bool insertOK =_channels.insert(channel).second;
  if(!insertOK)
      DB(2,"Attempted to store an EventChannel, when it is already stored.");
}

void EventChannelStore::erase(EventChannel_i* channel)
{
  omni_mutex_lock l(_lock);
  set<EventChannel_i*>::iterator pos =_channels.find(channel);
  if(pos==_channels.end())
      DB(2,"Failed to erase unknown EventChannel.")
  else
      _channels.erase(pos);
}

void EventChannelStore::output(ostream &os)
{
  omni_mutex_lock l(_lock);
  for(set<EventChannel_i*>::iterator i=_channels.begin();
      i!=_channels.end();
      ++i)
  {
    (*i)->output(os);
  }
}


void EventChannelStore::empty()
{
  omni_mutex_lock l(_lock);
  for(set<EventChannel_i*>::iterator i=_channels.begin();
      i!=_channels.end();
      ++i)
  {
    (*i)->destroy();
  }
}



  bool EventChannelStore::exists( const std::string &cname )
{
  bool retval=false;
  omni_mutex_lock l(_lock);
  for(set<EventChannel_i*>::iterator i=_channels.begin();
      i!=_channels.end();
      ++i)
  {
    if ( cname == (*i)->name() ) { retval=true; break;}
  }
  DB(5,"EventChannel_i::exists channel:" << cname.c_str() << " exists" << retval );
  return retval;
}



  void EventChannelStore::list( omniEvents::EventChannelInfoList &clist  )
{

  DB(5,"EventChannel_i::list num channels:" << _channels.size() );
  clist.length(_channels.size());
  int i;
  omni_mutex_lock l(_lock);
  set<EventChannel_i*>::iterator iter;
  for(i=0, iter=_channels.begin();
      iter!=_channels.end();
      ++iter, i++)
  {
    EventChannel_i *ech=*iter;
    try {
      omniEvents::EventChannelInfo eci;
      std::string ename = ech->name();
      DB(5,"EventChannel_i::list i:" << i << " name:" << ename.c_str() );
      eci.channel_name = ename.c_str();
      eci.has_mapper = ech->hasMapper();
      eci.pull_retry_period = ech->pullRetryPeriod_ms();
      eci.max_queue_length = ech->maxQueueLength();
      eci.max_num_proxies = ech->maxNumProxies();
      eci.cycle_period = ech->cyclePeriod_ns();
      if(ech->properties().hasAttr("FilterId")) {
        string rid = ech->properties().attrString("FilterId");
        eci.filter_id = rid.c_str();
      }
      clist[i]=eci;
    }
    catch(...) {
    }
  }
}






}; // end namespace OmniEvents

