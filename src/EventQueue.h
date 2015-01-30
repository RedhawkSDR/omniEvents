//                            Package   : omniEvents
// EventQueue.h               Created   : 2003/12/04
//                            Author    : Alex Tingle
//
//    Copyright (C) 2003 Alex Tingle.
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

#ifndef OMNIEVENTS__EVENTQUEUE_H
#define OMNIEVENTS__EVENTQUEUE_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Orb.h"
#include "Filter.h"

#include <memory>

#ifdef HAVE_STD_STL
using namespace std;
#endif


namespace OmniEvents {


/** The EventQueue is a circular buffer, that contains _size-1 events. Events
 * are stored as pointers to CORBA::Any. The Reader class is a mix-in for
 * classes that need to consume events from the queue. Readers that cannot keep
 * up with the flow of messages into the queue will skip _size events
 * whenever the back of the queue catches up with them. This implements the
 * usual 'fresh business first' pattern for deciding what to discard.
 *
 * Events are not scavenged from the queue. This means that once the queue has
 * filled up, it will never contain fewer than _size-1 events. This strategy is
 * usually somewhat wasteful of memory, but guarantees that the worst-case
 * memory usage will never rise above the defined maximum.
 */
class EventQueue
{
public:
  class Reader
  {
  public:
    Reader(EventQueue& eventQueue);
    bool moreEvents() const;
    CORBA::Any* nextEvent();
  private:
    EventQueue& _eventQueue;
    int         _next; ///< Points to the next event to read.
  };

  EventQueue(long size=1023);
  virtual ~EventQueue();
  void setFilter(Filter* filter)
  {
    auto_ptr<Filter> f(filter);
    _filter=f; // MS VC++ 6 has no auto_ptr::reset()
  }

  inline void append(CORBA::Any* event)
  {
    if(!_filter.get() || _filter->keep(*event))
    {
      delete _queue[_next];
      _queue[_next]=event;
      _next=(_next+1)%_size;
    }
  }

  inline long size() { return _size;};

private:
  /** Always points to the next slot to which an event will be written. */
  long             _next;
  long             _size;
  CORBA::Any**     _queue;
  auto_ptr<Filter> _filter;

  friend class Reader;
};

}; // end namespace OmniEvents

#endif // OMNIEVENTS__EVENTQUEUE_H
