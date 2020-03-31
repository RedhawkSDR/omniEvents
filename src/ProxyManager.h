//                            Package   : omniEvents
// ProxyManager.h             Created   : 2003/12/04
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

#ifndef OMNIEVENTS__PROXYMANAGER_H
#define OMNIEVENTS__PROXYMANAGER_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <set>
#include <string>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "Servant.h"

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

namespace OmniEvents {

class Proxy;
class PersistNode;

/** Base class for ServantActivator classes that manage Proxy servants. Each
 * ProxyManager manages its own POA (_managedPoa), with policies: PERSISTENT,
 * USER_ID, NO_IMPLICIT_ACTIVATION, USE_SERVANT_MANAGER, SINGLE_THREAD_MODEL.
 * This POA is only used to contain objects of a single Proxy type.
 * Specific subclasses must implement incarnate() & createObject() methods.
 */
class ProxyManager
: public virtual POA_PortableServer::ServantActivator,
  public Servant
{
public: // CORBA interface methods
  /** Implements etherealize() method from ServantActivator interface. */
  void etherealize(
    const PortableServer::ObjectId& oid,
    PortableServer::POA_ptr         adapter,
    PortableServer::Servant         serv,
    CORBA::Boolean                  cleanup_in_progress,
    CORBA::Boolean                  remaining_activations
  );

public:
  /** Re-create servants from information saved in the log file. */
  void reincarnate(const PersistNode& node);
  /** Save this object's state to a stream. */
  void output(ostream& os);

protected:
  /** @param poa   parent POA. */
  ProxyManager(PortableServer::POA_ptr poa);
  virtual ~ProxyManager();
  
  /** Creates the Proxy-type's POA, and registers this object as its
   *  ServantManager.
   * 
   *  @param name  e.g. "ProxyPushSupplier".
   */
  void activate(const char* name);

  /** The set of all active Proxies in this object's _managedPoa. */
  set<Proxy*>             _servants;
  /** The POA owned & managed by this object. Don't confuse it with _poa
   * (inherited from class Servant) which points to the POA in which THIS
   * object resides.
   */
  PortableServer::POA_var _managedPoa;
};


/** Base class for three of the four Proxy servants. Proxy servants are stored
 * in a ProxyManager. There is one Proxy servant for each CORBA proxy object.
 * (Compare with ProxyPushConsumer_i, where all objects are implemented by a
 * single servant.)
 */
class Proxy
: public virtual PortableServer::ServantBase,
  public Servant
{
public:
  virtual ~Proxy();
  /** Re-create a servant from information saved in the log file. */
  virtual void reincarnate(const string& oid, const PersistNode& node)=0;
  /** Save this object's state to a stream. */
  virtual void output(ostream &os)=0;
protected:
  Proxy(PortableServer::POA_ptr poa);

  /** Helper method for constructing persistency output. */
  void keyOutput(ostream& os, const char* name);
  /** Helper method for constructing persistency output. */
  void eraseKey(const char* name);
  /** Helper method for constructing persistency output. */
  void basicOutput(
    ostream& os, const char* name,
    CORBA::Object_ptr target=CORBA::Object::_nil(),
    const char* extraAttributes=NULL
  );

  void basicOutput(
    ostream& os, const char* name,
    const std::string &ior,
    const char* extraAttributes=NULL
  );
  
 
  CORBA::Request_var _req;
private:
  Proxy(const Proxy&); ///< NO IMPLEMENTATION
};


}; // end namespace OmniEvents

#endif // OMNIEVENTS__PROXYMANAGER_H
