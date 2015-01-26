//                            Package   : omniEvents
// Servant.h                  Created   : 2003/12/04
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

#ifndef OMNIEVENTS__SERVANT_H
#define OMNIEVENTS__SERVANT_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

//
// Debug definitions for servants.
//

#if OMNIEVENTS__DEBUG_ALL
#  define OMNIEVENTS__DEBUG_REF_COUNTS 1
#  define OMNIEVENTS__DEBUG_SERVANT 1
#else
   /** Define the macro OMNIEVENTS__DEBUG_REF_COUNTS to 1 in order to
    *  compile in debug versions of _add/remove_ref(). */
#  define OMNIEVENTS__DEBUG_REF_COUNTS 0
   /** Define the macro OMNIEVENTS__DEBUG_SERVANT to 1 in order to
    *  compile in servant object count checks. */
#  define OMNIEVENTS__DEBUG_SERVANT 0
#endif

#if OMNIEVENTS__DEBUG_REF_COUNTS
#  define OMNIEVENTS__DEBUG_REF_COUNTS__DECL void _add_ref();void _remove_ref();
#  define OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(C) \
   void C::_add_ref() { \
     DB(20,#C "::_add_ref()") \
     PortableServer::RefCountServantBase::_add_ref(); \
   } \
   void C::_remove_ref() { \
     DB(20,#C "::_remove_ref()") \
     PortableServer::RefCountServantBase::_remove_ref(); \
   }
#else
/** Declares debug versions of _add/remove_ref(). */
#  define OMNIEVENTS__DEBUG_REF_COUNTS__DECL
/** Defines debug versions of _add/remove_ref() for class C. */
#  define OMNIEVENTS__DEBUG_REF_COUNTS__DEFN(C)
#endif


namespace OmniEvents {

/** Helper method called by createNarrowedReference().
 *
 * @param poa  POA to own new object.
 * @param repositoryId Identifies the type of object to make.
 *          e.g. _tc_ProxyPushSupplier->id().
 */
CORBA::Object_ptr
createReference(PortableServer::POA_ptr poa, const char* repositoryId);

/** Helper method that creates a new CORBA object and then narrows it to the
 * appropriate type. Wrapper around POA::create_reference_with_id.
 * The type T *must* match the repositoryId parameter. Called by a class'
 * createObject() method.
 *
 * @param poa  POA to own new object.
 * @param repositoryId Identifies the type of object to make.
 *          e.g. _tc_ProxyPushSupplier->id().
 */
template<class T>
typename T::_ptr_type
createNarrowedReference(PortableServer::POA_ptr poa, const char* repositoryId)
{
  CORBA::Object_var obj =createReference(poa,repositoryId);
#ifdef HAVE_OMNIORB4
  return T::_unchecked_narrow(obj.in());
#else
  return T::_narrow(obj.in());
#endif
}

/** Generates a unique object ID string, based upon the current PID and time. */
char* newUniqueId();


/** Base class for servants. Stores the servant's POA. Provides some
 *  useful helper methods.
 */
class Servant : public virtual PortableServer::ServantBase
{
public:
  virtual PortableServer::POA_ptr _default_POA();
  virtual ~Servant();

#if OMNIEVENTS__DEBUG_SERVANT
  static int _objectCount;
#endif

protected:
  Servant(PortableServer::POA_ptr poa);

  /** Calls activate_object_with_id() to activate this servant in its POA. */
  void activateObjectWithId(const char* oidStr);
  /** Calls deactivate_object() to deactivate this servant in its POA. */
  void deactivateObject();

  PortableServer::POA_var _poa;

private:
  /** No default constructor. */
  Servant();
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__SERVANT_H
