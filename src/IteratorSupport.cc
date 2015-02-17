/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "Orb.h"
#include "IteratorSupport.h"


bool operator== (const PortableServer::ObjectId& lhs, const PortableServer::ObjectId& rhs)
{
    if (lhs.length() != rhs.length()) {
        return false;
    }
    return (memcmp(lhs.get_buffer(), rhs.get_buffer(), lhs.length()) == 0);
}

namespace PortableServer {
    std::size_t hash_value(const ObjectId& oid) {
        const CORBA::Octet* start = oid.get_buffer();
        return boost::hash_range(start, start+oid.length());
    }
}


namespace OmniEvents {



boost::mutex GCThread::instanceMutex_;
GCThread* GCThread::instance_ = 0;

GCThread::GCThread():
    sleepTime_(boost::posix_time::seconds(1)),
    thread_(&GCThread::run, this)
{
}

GCThread::~GCThread()
{
}

void GCThread::sweep()
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    for (std::list<GCContext*>::iterator iter = contexts_.begin(); iter != contexts_.end(); ++iter) {
        (*iter)->gc_sweep();
    }
}

void GCThread::run()
{
    while (true) {
        sweep();
        boost::this_thread::sleep(sleepTime_);
    }
}

void GCThread::stop()
{
    thread_.interrupt();
    thread_.join();
}

GCThread* GCThread::instance()
{
    boost::mutex::scoped_lock lock(instanceMutex_);
    if (!instance_) {
        instance_ = new GCThread();
    }
    return instance_;
}

void GCThread::add(GCContext* context)
{
    instance()->addContext(context);
}

void GCThread::release(GCContext* context)
{
    instance()->releaseContext(context);
}

void GCThread::shutdown()
{
    boost::mutex::scoped_lock lock(instanceMutex_);
    if (!instance_) {
        return;
    }
    instance_->stop();
    delete instance_;
    instance_ = 0;
}

void GCThread::addContext(GCContext* context)
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    contexts_.push_back(context);
}

void GCThread::releaseContext(GCContext* context)
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    contexts_.remove(context);
}





GCServantLocator::GCServantLocator()
{
    GCThread::add(this);
}

GCServantLocator::~GCServantLocator()
{
    GCThread::release(this);

    // Release GC reference for any remaining servants; there is no need for
    // the mutex because there can be no more calls to gc_sweep()
    for (ServantMap::iterator iter = activeMap_.begin(); iter != activeMap_.end(); ++iter) {
        iter->second.servant->_remove_ref();
    }
}

PortableServer::Servant GCServantLocator::preinvoke(const PortableServer::ObjectId& oid,
                                                    PortableServer::POA_ptr adapter,
                                                    const char* operation,
                                                    PortableServer::ServantLocator::Cookie& the_cookie)
{
    boost::mutex::scoped_lock lock(activeMapMutex_);
    ServantMap::iterator iter = activeMap_.find(oid);
    if (iter == activeMap_.end()) {
        throw CORBA::OBJECT_NOT_EXIST();
    }
    PortableServer::Servant servant = iter->second.servant;
    if (iter->second.destroy == std::string(operation)) {
      DB(10,"GCServantLocator: untracking " << (void*)servant);
      activeMap_.erase(iter);
    } else {
      servant->_add_ref();
      iter->second.last_access = boost::get_system_time();
    }
    return servant;
}

void GCServantLocator::postinvoke(const PortableServer::ObjectId& oid,
                                  PortableServer::POA_ptr adapter,
                                  const char* operation,
                                  PortableServer::ServantLocator::Cookie the_cookie,
                                  PortableServer::Servant the_servant)
{
    the_servant->_remove_ref();
}

void GCServantLocator::register_servant(const PortableServer::ObjectId& oid,
                                        PortableServer::Servant servant,
                                        boost::posix_time::time_duration ttl,
                                        const std::string& destroy)
{
    boost::mutex::scoped_lock lock(activeMapMutex_);
    if (activeMap_.count(oid) > 0) {
        throw PortableServer::POA::ObjectAlreadyActive();
    }
    DB(10,"GCServantLocator: tracking " << (void*)servant);
    ServantEntry& entry = activeMap_[oid];
    entry.servant = servant;
    servant->_add_ref();
    entry.ttl = ttl;
    entry.last_access = boost::get_system_time();
    entry.destroy = destroy;
}

void GCServantLocator::gc_sweep()
{
    boost::mutex::scoped_lock lock(activeMapMutex_);
    for (ServantMap::iterator iter = activeMap_.begin(); iter != activeMap_.end(); ) {
        boost::posix_time::time_duration age = boost::get_system_time() - iter->second.last_access;
        if (age > iter->second.ttl) {
          DB(10,"GCServantLocator: releasing " << (void*)iter->second.servant);
            iter->second.servant->_remove_ref();
            iter = activeMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}

PortableServer::POA_ptr createGCPOA(PortableServer::POA_ptr parent, const std::string& name)
{
    CORBA::PolicyList policy_list;
    policy_list.length(2);
    policy_list[0] = parent->create_servant_retention_policy(PortableServer::NON_RETAIN);
    policy_list[1] = parent->create_request_processing_policy(PortableServer::USE_SERVANT_MANAGER);

    PortableServer::POAManager_var poa_mgr = parent->the_POAManager();
    PortableServer::POA_var poa = parent->create_POA(name.c_str(), poa_mgr, policy_list);

    for (size_t ii = 0; ii < policy_list.length(); ++ii) {
        policy_list[ii]->destroy();
    }

    GCServantLocator* manager = new GCServantLocator();
    PortableServer::ServantManager_var manager_ref = manager->_this();
    poa->set_servant_manager(manager_ref);
    manager->_remove_ref();

    return poa._retn();
}

CORBA::Object* activateGCObject(PortableServer::POA_ptr poa,
                                PortableServer::Servant servant,
                                boost::posix_time::time_duration ttl,
                                const std::string& destroy)
{

  PortableServer::POA_ptr root_poa=Orb::inst()._RootPOA.in();

  PortableServer::ServantManager_var manager = poa->get_servant_manager();
  PortableServer::Servant mgr_servant = root_poa->reference_to_servant(manager);
  GCServantLocator* gc_manager = dynamic_cast<GCServantLocator*>(mgr_servant);
  if (!gc_manager) {
    throw PortableServer::POA::WrongPolicy();
  }
  CORBA::Object_var obj = poa->create_reference(servant->_mostDerivedRepoId());
  PortableServer::ObjectId_var oid = poa->reference_to_id(obj);
  gc_manager->register_servant(oid, servant, ttl, destroy);
  return obj._retn();
}



};  /// end of OmniEvents namespace
