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
#ifndef OMNIEVENTS_ITERATORGC_H
#define OMNIEVENTS_ITERATORGC_H

#include <list>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include "Orb.h"

namespace OmniEvents {

  class GCContext
  {
    public:
    virtual void gc_sweep() = 0;
  };


  class GCThread
    {
    public:
        static void add(GCContext* context);
        static void release(GCContext* context);

        static void shutdown();

    private:
        GCThread();
        ~GCThread();

        static GCThread* instance();
        static boost::mutex instanceMutex_;
        static GCThread* instance_;

        void addContext(GCContext* context);
        void releaseContext(GCContext* context);

        void sweep();
        void run();
        void stop();

        boost::mutex contextsMutex_;
        std::list<GCContext*> contexts_;

        boost::posix_time::time_duration sleepTime_;

        boost::thread thread_;
    };


  class GCServantLocator : public virtual POA_PortableServer::ServantLocator, private GCContext {

        public:
            GCServantLocator();
            ~GCServantLocator();

            virtual PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
                                                      PortableServer::POA_ptr adapter,
                                                      const char* operation,
                                                      PortableServer::ServantLocator::Cookie& the_cookie);

            virtual void postinvoke(const PortableServer::ObjectId& oid,
                                    PortableServer::POA_ptr adapter,
                                    const char* operation,
                                    PortableServer::ServantLocator::Cookie the_cookie,
                                    PortableServer::Servant the_servant);

            void register_servant(const PortableServer::ObjectId& oid,
                                  PortableServer::Servant servant,
                                  boost::posix_time::time_duration ttl,
                                  const std::string& destroy);

        private:
            void gc_sweep();

            struct ServantEntry {
                PortableServer::Servant servant;
                boost::posix_time::time_duration ttl;
                std::string destroy;
                boost::system_time last_access;
            };

            typedef boost::unordered_map<PortableServer::ObjectId,ServantEntry> ServantMap;
            ServantMap activeMap_;
            boost::mutex activeMapMutex_;
        };

        PortableServer::POA_ptr createGCPOA(PortableServer::POA_ptr parent, const std::string& name);

        CORBA::Object* activateGCObject(PortableServer::POA_ptr poa,
                                        PortableServer::Servant servant,
                                        boost::posix_time::time_duration ttl=boost::posix_time::seconds(60),
                                        const std::string& destroy=std::string("destroy"));
}

#endif // OMNIEVENTS_ITERATORGC_H
