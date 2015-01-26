// -*- Mode: C++; -*-
//                            Package   : omniEvents
// omniEventsLog.h            Created   : 1/10/99
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2005 Alex Tingle.
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
//

/*
  $Log: omniEventsLog.h,v $
  Revision 1.6.2.4  2005/05/10 14:56:06  alextingle
  All private members are now protected. Allows for flexible subclassing.

  Revision 1.6.2.3  2005/05/10 14:28:13  alextingle
  Updated copyrights to 2005.

  Revision 1.6.2.2  2005/04/27 20:49:31  alextingle
  Merge across changes from HEAD branch (see CHANGES_262. Change version number ready for release 2.6.2.

  Revision 1.6.2.1  2004/11/16 21:46:11  alextingle
  Made several methods virtual to allow users of libomniEvents to override
  the default persistency behaviour. (Dirk O. Siebnich)

  Revision 1.6  2004/09/11 23:08:39  alextingle
  WriteLock now non-copyable.

  Revision 1.5  2004/07/26 16:27:08  alextingle
  Support for NT service on windows: main() moved into daemon.cc.
  New (laxer) start up syntax. Port is now set with -p (not -s). There is no
  special cold start mode.
  More flexible naming service name option -N. (No more -K option).

  Revision 1.4  2004/04/20 17:16:17  alextingle
  Corrected openOfstream() arg name/comments.

  Revision 1.3  2004/03/28 01:03:58  alextingle
  Refactored class omniEventsLog to allow for more EventChannelFactory parameters.\nNew omniEvents params: -v, -a (alternate endPoint).

  Revision 1.2  2004/01/11 16:57:26  alextingle
  New persistancy log file format, implemented by PersistNode.h/cc. The new format enables new nodes to be added and old ones erased by appending a single line to the file, rather than by re-persisting the whole application. This is much more efficient when lots of proxies are being created all at once. It's also a much simpler solution, with far fewer lines of code.

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.4  2003/11/14 13:54:48  alextingle
  New output() members functions. Eliminates the need for friend ostream
  functions that are problematic on earlier versions of Microsoft
  VisualC++.

  Revision 1.3  2003/11/03 22:35:08  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.
  Added private helper functions initializeFileNames(), setFilename() &
  openOfstream() to simplify the implementation.
  Removed member `logdir', as it's only used during object construction.
  Renamed configuration macro LOGDIR_ENV_VAR to OMNIEVENTS_LOGDIR_ENV_VAR
  for consistency with other configuration macros.

  Revision 1.1.1.1.2.1  2002/09/28 22:20:51  shamus13
  Added ifdefs to enable omniEvents to compile
  with both omniORB3 and omniORB4. If __OMNIORB4__
  is defined during compilation, omniORB4 headers
  and command line option syntax is used, otherwise
  fall back to omniORB3 style.

  Revision 1.1.1.1  2002/09/25 19:00:32  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.3  2000/08/30 04:21:56  naderp
  Port to omniORB 3.0.1.

  Revision 1.2  2000/03/02 04:19:17  naderp
  Passing factory by reference to init() for initialisation.

  Revision 1.1  1999/11/02 13:40:56  naderp
  Rearranged data member definitions to avoid compiler warnings during
  initialisation.

  Revision 1.0  1999/11/01 16:48:21  naderp
  Initial revision

*/

#ifndef _OMNIEVENTSLOG_H_
#define _OMNIEVENTSLOG_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#  include <fstream>
#else
#  include <iostream.h>
#  include <fstream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

namespace OmniEvents {

class EventChannelFactory_i;
class PersistNode;

#ifndef OMNIEVENTS_LOGDIR_ENV_VAR
#  define OMNIEVENTS_LOGDIR_ENV_VAR "OMNIEVENTS_LOGDIR"
#endif

class omniEventsLog
{
public:
   /** Library code may create Event Service objects without the need for
    * persistency. We use this method to check for the log object, before
    * attempting to write out state changes.
    */
   static bool exists() { return NULL!=omniEventsLog::theLog; }

   omniEventsLog(const char* logdir=NULL);
   virtual ~omniEventsLog();

   bool fileExists(const char* filename) const;
   const char* activeFilename() const { return _activeFilename; }
   const char* backupFilename() const { return _backupFilename; }

   /** Creates an initialState from its arguments. Used when the server is
    *  cold started with no saved state. Aborts with an error if there are any
    *  logfiles in the log directory.
    */
   PersistNode* bootstrap(int port, const char* endPointNoListen);

   /** Creates an initialState from the logfile. Used when the server is warm
    *  started. Aborts with an error if there is no logfile in the log
    *  directory.
    */
   virtual PersistNode* parse();
   
   /** Constructs the EventChannelFactory from the information in the
    *  initialState parameter.
    */
   void incarnateFactory(PersistNode* initialState);

   /** Kicks off the worker thread that periodically checkpoints the
    *  persistency logfile.
    */
   virtual void runWorker();
   
   /** accessor method */
   EventChannelFactory_i* factory() {return _factory;}
   
   /** Entry point used by the omniEventsLogWorker to perform checkpointing.
    *  The active logfile is moved to backup, and a new active logfile is
    *  created.
    */
   void checkpoint(void);

   virtual void output(ostream& os);

public:
  class IOError {};

protected:
  virtual void initializeFileNames(const char* logdir);
  void setFilename(
    char*&      filename, ///< OUT parameter.
    const char* logdir,
    const char* sep,
    const char* logname,
    const char* hostname,
    const char* ext
  );
  virtual void openOfstream(
    ofstream&   s,
    const char* filename,
    int         flags=0,
    int*        fd=NULL
  );

  static omniEventsLog*  theLog;

  ofstream               _logstream;
  char*                  _activeFilename;
  char*                  _backupFilename;
  char*                  _checkpointFilename;
  omni_thread*           _workerThread;       ///< In charge of checkpoints.
  EventChannelFactory_i* _factory;
  bool                   _checkpointNeeded;
  omni_mutex             _lock;
  
  friend class WriteLock;
};

class omniEventsLogWorker : public omni_thread
{
public:
    typedef void (omniEventsLog::*Method)(void);
    omniEventsLogWorker(
      omniEventsLog* object,
      Method         method,
      priority_t     priority=PRIORITY_NORMAL
    );
    void* run_undetached(void *);
    ~omniEventsLogWorker();
private:
    omniEventsLog* _object;
    Method         _method;
    omniEventsLogWorker(); ///< No default construction allowed.
};


/** Obtains an output stream to the active persistancy logfile, and
 * locks it for exclusive access. The lock is released when the object is
 * destructed.
 */
class WriteLock
{
public:
  WriteLock():
    os(omniEventsLog::theLog->_logstream),
    l(omniEventsLog::theLog->_lock)
  {}
  ~WriteLock()
  {
    os.flush();
    omniEventsLog::theLog->_checkpointNeeded=true;
  }
  ostream& os;
private:
  omni_mutex_lock l;
  WriteLock(const WriteLock&); ///< No implementation
};

}; // end namespace OmniEvents

#endif /* _OMNIEVENTSLOG_H_ */
