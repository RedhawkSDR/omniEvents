// -*- Mode: C++; -*-
//                            Package   : omniEvents
// EventChannelFactory_i.h    Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2004-2005 Alex Tingle.
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

#ifndef OMNIEVENTS_EVENTCHANNELFACTORY_H
#define OMNIEVENTS_EVENTCHANNELFACTORY_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "omniEvents.hh"
#include "Servant.h"
#include "EventChannel.h"

#ifdef HAVE_STD_STL
using namespace std;
#endif

namespace OmniEvents {

class PersistNode;

// Event Channel Factory

class EventChannelFactory_i :
  public virtual POA_omniEvents::EventChannelFactoryExt,
  public PortableServer::RefCountServantBase,
  public Servant
{
public: // CORBA METHODS
  /** Returns true if the key passed has the following contents:
   *
   * - id   : "EventChannel"
   * - kind : "object interface"
   */
  CORBA::Boolean supports(const CosLifeCycle::Key& k);
  CORBA::Object_ptr create_object(
    const CosLifeCycle::Key &k,
    const CosLifeCycle::Criteria &the_criteria
  );
  
  /** 'ping' method inherited from FT::PullMonitorable. */
  CORBA::Boolean is_alive() { return 1; }
  
  /** DO NOT USE. Only for OpenOrb compatibility. */
  CosEventChannelAdmin::EventChannel_ptr create_channel(
    const char* channel_name
  );

  /** DO NOT USE. Only for OpenOrb compatibility. */
  CosEventChannelAdmin::EventChannel_ptr join_channel(
    const char* channel_name
  );
   

  /**
     EventChannelFactoryExt methods
   */

  void                          delete_all();
  CORBA::Boolean                exists( const char *channel_name );
  CosEventChannelAdmin::EventChannel_ptr  get_channel( const char *channel_name );
  void                          list_channels( const CORBA::ULong  how_many,
                                   omniEvents::EventChannelInfoList_out elist,
                                   omniEvents::EventChannelInfoIterator_out eiter);

public:
  /** Builds an EventChannelFactory_i from the parsed logfile data. */
  EventChannelFactory_i(const PersistNode& node);
  virtual ~EventChannelFactory_i();

  /** Convert CosLifeCycle::Criteria into a PersistNode. The caller is
   * responsible for deleting the result.
   */
  PersistNode* parseCriteria(const CosLifeCycle::Criteria& criteria) const;
  
  /** Utility function: constructs a Criteria that contains a single criterion.
   *  Used to generate Invalid Criteria exceptions.
   */
  CosLifeCycle::Criteria extract(
    const char*                   name,
    const CosLifeCycle::Criteria& from
  ) const;

  void output(ostream& os);

private:
  /** The EventChannelFactory listens on this TCP port. Equivalent to
   * ORB parameter: endPoint = giop:::<_port>
   */
  unsigned int      _port;

  /** Stores the value of the endPointNoListen ORB parameter. omniORB4 allows
   * us to encode backup server addresses into our object references.
   */
  string            _endPointNoListen;

  EventChannelStore _channels;
};

}; // namespace OmniEvents

#endif // OMNIEVENTS_EVENTCHANNELFACTORY_H
