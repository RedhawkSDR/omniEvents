//                            Package   : omniEvents
// Callback.h                 Created   : 2004/07/10
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004-2005 Alex Tingle.
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

#ifndef OMNIEVENTS__CALLBACK_H
#define OMNIEVENTS__CALLBACK_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

namespace OmniEvents {

/** Interface for classes that wish to receive callbacks from deferred
 * requests. Implementations must override the callback() method. */
class Callback:
  public virtual PortableServer::ServantBase,
  public PortableServer::RefCountServantBase
{
public:
  /** Invoked when the CORBA::Request has returned.
   * This method is never invoked when req has returned an exception. */
  virtual void callback(CORBA::Request_ptr req) =0;

  Callback():PortableServer::RefCountServantBase()
    {}
  virtual ~Callback()
    {}
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__CALLBACK_H
