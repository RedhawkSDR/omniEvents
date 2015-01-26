//                            Package   : omniEvents
// EventQueue.cc              Created   : 2003/12/04
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

#include "EventQueue.h"
#include <string.h> // memset
#include <assert.h>

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


EventQueue::EventQueue(long size)
: _next(0),
  _size(size+1), // Always need an `empty' entry at the head of the buffer.
  _queue(new CORBA::Any*[_size]),
  _filter(NULL)
{
  DB(5,"MaxQueueLength="<<size)
  assert(_size>1);
  // Explicitly clear the queue with memset, because MS VC++ doesn't like
  // it as an array initializer.
  memset(_queue,0,_size*sizeof(CORBA::Any*));
}


EventQueue::~EventQueue()
{
  for(long i=0; i<_size; ++i)
      delete _queue[i];
  delete[] _queue;
}


//
//  EventQueue::Reader
//


EventQueue::Reader::Reader(EventQueue& eventQueue)
: _eventQueue(eventQueue),
  _next(eventQueue._next)
{
  // pass
}


bool EventQueue::Reader::moreEvents() const
{
  return( _next!=_eventQueue._next );
}


CORBA::Any* EventQueue::Reader::nextEvent()
{
  CORBA::Any* result =_eventQueue._queue[_next];
  _next=(_next+1)%_eventQueue._size;
  return result;
}


}; // end namespace OmniEvents
