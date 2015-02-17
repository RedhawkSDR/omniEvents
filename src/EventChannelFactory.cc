// -*- Mode: C++; -*-
//                            Package   : omniEvents
// EventChannelFactory_i.cc   Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2004 Alex Tingle.
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
// Description:
//      Implementation of the COSS Event Services Event Channel Factory
//	

#include "EventChannelFactory.h"

#include "Orb.h"
#include "EventChannel.h"
#include "PersistNode.h"
#include "Iterator.h"
#include <memory>
#include <sstream>

#ifdef HAVE_OMNIORB4
#  define STR_MATCH(s1,s2) omni::strMatch((s1),(s2))
#else
#  define STR_MATCH(s1,s2) (0==::strcmp((s1),(s2)))
#endif


namespace OmniEvents {


typedef  OmniEvents::Iterator<  omniEvents::EventChannelInfo,
                                   omniEvents::EventChannelInfo_out,
                                   omniEvents::EventChannelInfoList,
                                   omniEvents::EventChannelInfoList_out,
                                   omniEvents::EventChannelInfoIterator,
                                   POA_omniEvents::EventChannelInfoIterator >   EventChannelInfoIter;


//------------------------------------------------------------------------
//           Event Channel Factory Interface Implementation
//------------------------------------------------------------------------
EventChannelFactory_i::EventChannelFactory_i(const PersistNode& node)
: Servant(Orb::inst()._omniINSPOA.in()),
  _port(node.attrLong("port",11169)),
  _endPointNoListen(node.attrString("endPointNoListen")),
  _channels()
{
  // Create event channels
  for(map<string,PersistNode*>::const_iterator i=node._child.begin();
      i!=node._child.end();
      ++i)
  {
    EventChannel_i* channel =new EventChannel_i(&_channels);
    channel->activate(
      i->first.c_str(), // channelName
      i->second         // node
    );
  }
  activateObjectWithId("omniEvents");
}


EventChannelFactory_i::~EventChannelFactory_i()
{
  DB(20, "EventChannelFactory_i::~EventChannelFactory_i()");
}


CORBA::Boolean
EventChannelFactory_i::supports(const CosLifeCycle::Key &k)
{
  if((k.length() == 1) &&
     (strcmp(k[0].id, "EventChannel") == 0) &&
     (strcmp(k[0].kind, "object interface") == 0))
    return 1;
  else
    return 0;
}


CORBA::Object_ptr
EventChannelFactory_i::create_object(
  const CosLifeCycle::Key& k,
  const CosLifeCycle::Criteria& criteria
)
{
  // Check the key
  if(!this->supports(k))
      throw CosLifeCycle::NoFactory(k);

  // Process criteria !! MAY THROW !!
  auto_ptr<PersistNode> criteriaNode( parseCriteria(criteria) );

  CORBA::String_var channelId;
  if(criteriaNode->hasAttr("InsName"))
      channelId=criteriaNode->attrString("InsName").c_str();
  else
      channelId=newUniqueId();

  // Create the channel.
  // We place it into an auto_ptr - this will automatically clean up if anything
  // goes wrong.
  auto_ptr<EventChannel_i> channel( new EventChannel_i(&_channels) );
  try
  {
    channel->activate(channelId.in(),criteriaNode.get()); // !! MAY THROW !!
  }
  catch(PortableServer::POA::ObjectAlreadyActive& ex)
  {
    throw CosLifeCycle::InvalidCriteria(criteria); //??
  }
  catch(PortableServer::POA::AdapterAlreadyExists& ex) // create_POA
  {
    throw CosLifeCycle::InvalidCriteria(criteria); //??
  }

  // We release() the pointer, as the thread will delete it when it stops.
  return channel.release()->_this();
}


CosEventChannelAdmin::EventChannel_ptr
EventChannelFactory_i::create_channel(const char* channel_name)
{
  CosEventChannelAdmin::EventChannel_var result;

  CosLifeCycle::Key key;
  key.length(1);
  key[0].id  ="EventChannel";
  key[0].kind="object interface";

  CosLifeCycle::Criteria criteria;
  criteria.length(1);
  criteria[0].name    = "InsName";
  criteria[0].value <<= channel_name;

  try
  {
    CORBA::Object_var obj=create_object(key,criteria);
    result=CosEventChannelAdmin::EventChannel::_narrow(obj.in());
  }
  catch(CosLifeCycle::InvalidCriteria& ex)
  {
    if(ex.invalid_criteria.length()>0 &&
       STR_MATCH(ex.invalid_criteria[0].name,"InsName"))
    {
      throw event::NameAlreadyUsed();
    }
    else
    {
      DB(10,"Failed to create_channel."
        " Converting InvalidCriteria exception into UNKNOWN.")
      throw CORBA::UNKNOWN();
    }
  }
  catch(CORBA::UserException& ex)
  {
    DB(2,"Failed to create_channel. Converting UserException"
      IFELSE_OMNIORB4(" '"<<ex._name()<<"'",<<) " into UNKNOWN.")
    throw CORBA::UNKNOWN();
  }
  return result._retn();
}


CosEventChannelAdmin::EventChannel_ptr
EventChannelFactory_i::join_channel(const char* channel_name)
{
  using namespace PortableServer;
  CosEventChannelAdmin::EventChannel_var result;
  try
  {
    ObjectId_var oid =PortableServer::string_to_ObjectId(channel_name);
    CORBA::Object_var obj =Orb::inst()._omniINSPOA->id_to_reference(oid.in());
    result=CosEventChannelAdmin::EventChannel::_narrow(obj.in());
  }
  catch(POA::ObjectNotActive&)
  {
    DB(10,"Failed to join_channel. Object not active.")
    throw event::EventChannelNotFound();
  }
  catch(CORBA::UserException& ex)
  {
    DB(2,"Failed to join_channel. Converting UserException"
      IFELSE_OMNIORB4(" '"<<ex._name()<<"'",<<) " into UNKNOWN.")
    throw CORBA::UNKNOWN();
  }
  return result._retn();
}


PersistNode* EventChannelFactory_i::parseCriteria(
  const CosLifeCycle::Criteria &criteria
) const
{
  using namespace CosLifeCycle;
  auto_ptr<PersistNode> result( new PersistNode() );

  for(CORBA::ULong i=0; i<criteria.length(); i++)
  {
    if(strcmp(criteria[i].name, "PullRetryPeriod_ms") == 0)
    {
      CORBA::ULong pullRetryPeriod_ms;
      if(! (criteria[i].value >>= pullRetryPeriod_ms))
          throw InvalidCriteria(extract("PullRetryPeriod_ms",criteria));
      if(pullRetryPeriod_ms <= 0)
          throw CannotMeetCriteria(extract("PullRetryPeriod_ms",criteria));
      result->addattr("PullRetryPeriod_ms",pullRetryPeriod_ms);
    }
    else if(strcmp(criteria[i].name, "PullRetryPeriod") == 0)
    {
      // This criterion has been deprecated in favour of PullRetryPeriod_ms.
      // Don't overwrite any value provided by the latter.
      if(!result->hasAttr("PullRetryPeriod_ms"))
      {
        CORBA::ULong pullRetryPeriod;
        if(! (criteria[i].value >>= pullRetryPeriod))
            throw InvalidCriteria(extract("PullRetryPeriod",criteria));
        if(pullRetryPeriod <= 0)
            throw CannotMeetCriteria(extract("PullRetryPeriod",criteria));
        result->addattr("PullRetryPeriod_ms",pullRetryPeriod*1000);
      }
    }
    else if(strcmp(criteria[i].name, "MaxQueueLength") == 0)
    {
      CORBA::ULong maxQueueLength;
      if(! (criteria[i].value >>= maxQueueLength))
          throw InvalidCriteria(extract("MaxQueueLength",criteria));
      if(maxQueueLength > 0)
          result->addattr("MaxQueueLength",maxQueueLength);
      else
          DB(10,"Ignoring CosLifeCycle criterion: MaxQueueLength=0");
    }
    else if(strcmp(criteria[i].name, "MaxNumProxies") == 0)
    {
      CORBA::ULong maxNumProxies;
      if(! (criteria[i].value >>= maxNumProxies))
          throw InvalidCriteria(extract("MaxNumProxies",criteria));
      if(maxNumProxies > 0)
          result->addattr("MaxNumProxies",maxNumProxies);
      else
          DB(10,"Ignoring CosLifeCycle criterion: MaxNumProxies=0");
    }
    else if(strcmp(criteria[i].name, "CyclePeriod_ns") == 0)
    {
      CORBA::ULong cyclePeriod_ns;
      if(! (criteria[i].value >>= cyclePeriod_ns))
          throw InvalidCriteria(extract("CyclePeriod_ns",criteria));
      if(cyclePeriod_ns > 0)
          result->addattr("CyclePeriod_ns",cyclePeriod_ns);
      else
          DB(10,"Ignoring CosLifeCycle criterion: CyclePeriod_ns=0");
    }
    else if(strcmp(criteria[i].name, "InsName") == 0)
    {
      const char* insName;
      if(! (criteria[i].value >>= insName))
          throw InvalidCriteria(extract("InsName",criteria));
      if(insName && insName[0])
          result->addattr(string("InsName=")+insName);
      else
          DB(10,"Ignoring empty CosLifeCycle criterion: InsName");
    }
    else if(strcmp(criteria[i].name, "FilterId") == 0)
    {
      const char* repositoryId;
      if(! (criteria[i].value >>= repositoryId))
          throw InvalidCriteria(extract("FilterId",criteria));
      if(repositoryId && repositoryId[0])
          result->addattr(string("FilterId=")+repositoryId);
      else
          DB(10,"Ignoring empty CosLifeCycle criterion: FilterId");
    }
    else if(strcmp(criteria[i].name, "MaxEventsPerConsumer") == 0)
    {
      DB(10,"Ignoring obsolete CosLifeCycle criterion: MaxEventsPerConsumer");
    }
    else
    {
      DB(10,"Ignoring unknown CosLifeCycle criterion: "<<criteria[i].name);
    }
  } // end loop for(i)
  
  return result.release();
}


CosLifeCycle::Criteria EventChannelFactory_i::extract(
  const char*                   name,
  const CosLifeCycle::Criteria& from
) const
{
  CosLifeCycle::Criteria result;
  result.length(0);
  for(CORBA::ULong i=0; i<from.length(); i++)
  {
    if(strcmp(from[i].name,name) == 0)
    {
      result.length(1);
      result[0]=from[i];
      break;
    }
  }
  return result;
}


void
EventChannelFactory_i::output(ostream &os)
{
  os<<"ecf port="<<_port;
  if(!_endPointNoListen.empty())
      os<<" endPointNoListen="<<_endPointNoListen;
  os<<" ;;\n";
  _channels.output(os);
}




void 
EventChannelFactory_i::delete_all()
{
  // request destroy operation against all managed event
  // channels....
  std::ostringstream os;
  os << "EventChannelFactory_i::delete_all() - Request to delete ALL CHANNELS :  "<< _channels.size();
  DB(5, os.str().c_str() );
  _channels.empty();

}



CORBA::Boolean
EventChannelFactory_i::exists( const char *channel_name)
{
  CORBA::Boolean retval=_channels.exists( channel_name );

  return retval;
}


CosEventChannelAdmin::EventChannel_ptr 
EventChannelFactory_i::get_channel( const char *channel_name) 
{

  using namespace PortableServer;
  CosEventChannelAdmin::EventChannel_var result;
  try
    {
      ObjectId_var oid =PortableServer::string_to_ObjectId(channel_name);
      CORBA::Object_var obj =Orb::inst()._omniINSPOA->id_to_reference(oid.in());
      result=CosEventChannelAdmin::EventChannel::_narrow(obj.in());
    }
  catch(POA::ObjectNotActive&)
    {
      DB(10,"Failed to join_channel. Object not active.")
        throw event::EventChannelNotFound();
    }
  catch(CORBA::UserException& ex)
    {
      DB(2,"Failed to join_channel. Converting UserException"
         IFELSE_OMNIORB4(" '"<<ex._name()<<"'",<<) " into UNKNOWN.")
        throw CORBA::UNKNOWN();
    }
  return result._retn();
}



void 
EventChannelFactory_i::list_channels( const CORBA::ULong  how_many,
                                             omniEvents::EventChannelInfoList_out elist,
                                             omniEvents::EventChannelInfoIterator_out eiter)
{
    uint64_t size = _channels.size();
    std::ostringstream os;
    os << "EventChannelFactory::_list_channels - Listing all channels : " <<  size;
    DB(5, os.str().c_str() );

    // create copy of entire table...
    omniEvents::EventChannelInfoList* all = new omniEvents::EventChannelInfoList(size);
    all->length(size);
    _channels.list(*all);
    eiter = EventChannelInfoIter::list( how_many, elist, all );
}



}; // end namespace OmniEvents
