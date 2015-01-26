// -*- Mode: C++; -*-
//                            Package   : omniEvents
//   events.cc                Created   : 2004/05/02
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle
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
//    Push Model streamer.
//	

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_GETOPT
#  include <unistd.h>
extern char* optarg;
extern int optind;
#else
#  include "getopt.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <stdio.h>

#if defined HAVE_UNISTD_H
#  include <unistd.h> // read(), write()
#elif defined __WIN32__
#  include <io.h>
#  define write(fd,buf,count) _write(fd,buf,count)
#  define read(fd,buf,count)  _read(fd,buf,count)
#  define ssize_t int
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include "CosEventComm.hh"
#include "CosEventChannelAdmin.hh"
#include "naming.h"

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
# define STDOUT_FILENO 1
#endif

CORBA::ORB_ptr orb;

static void usage(int argc, char **argv);

//
// Time
//

#define BILLION 1000000000

class Time;
class Time
{
private:
  CORBA::ULong _sec;
  CORBA::ULong _nano;
public:
  static Time current()
  {
    Time result;
    unsigned long sec,nano;
    omni_thread::get_time(&sec,&nano);
    result._sec=sec;
    result._nano=nano;
    return result;
  }
  static void sleepUntil(const Time& futureTime)
  {
    Time now =current();
    if(now<futureTime)
    {
      Time offset=futureTime-now;
      omni_thread::sleep(offset._sec,offset._nano);
    }
  }
  //
  Time():_sec(0),_nano(0){}
  Time(CORBA::ULong sec,CORBA::ULong nano):_sec(sec),_nano(nano){}
  Time(const Time& right):_sec(right._sec),_nano(right._nano){}
  Time& operator=(const Time& right)
  {
    if(this!=&right)
    {
      _sec =right._sec;
      _nano=right._nano;
    }
    return *this;
  }
  bool operator<(const Time& right) const
  {
    if(_sec==right._sec)
        return _nano<right._nano;
    else
        return _sec<right._sec;
  }
  Time& operator+=(const Time& right)
  {
    _sec +=right._sec;
    _nano+=right._nano;
    if(_nano>BILLION)
    {
      _nano=_nano%BILLION;
      ++_sec;
    }
    return *this;
  }
  Time operator+(const Time& right) const
  {
    Time result(*this);
    result+=right;
    return result;
  }
  Time& operator-=(const Time& right)
  {
    if(operator<(right))
    {
      cerr<<"Negative time!"<<endl;
      throw CORBA::BAD_PARAM();
    }
    _sec-=right._sec;
    if(_nano<right._nano)
    {
      _nano+=BILLION;
      --_sec;
    }
    _nano-=right._nano;
    return *this;
  }
  Time operator-(const Time& right) const
  {
    Time result(*this);
    result-=right;
    return result;
  }
  void operator>>=(cdrMemoryStream& s) const
  {
    _sec>>=s;
    _nano>>=s;
  }
  void operator<<=(cdrMemoryStream& s)
  {
    _sec<<=s;
    _nano<<=s;
  }
  bool is_nil() const { return(_sec==0 && _nano==0); }
}; // end class Time


//
// Consumer_i
//

class Consumer_i : virtual public POA_CosEventComm::PushConsumer
{
public:
  Consumer_i(long disconnect=0): _memstream() {}
  void push(const CORBA::Any& data)
  {
    // Record the event timestamp.
    Time now=Time::current();
    now>>=_memstream;
    // stream event data.
    data>>=_memstream;
    // Write to file.
    write(STDOUT_FILENO,_memstream.bufPtr(),_memstream.bufSize());
    // Reset.
    _memstream.rewindPtrs();
  }
  void disconnect_push_consumer()
  {
    cout<<"disconnected"<<endl;
    orb->shutdown(0);
  }
  void consume(
    CosEventChannelAdmin::EventChannel_ptr channel,
    const char*& action)
  {
    action="get ConsumerAdmin";
    CosEventChannelAdmin::ConsumerAdmin_var consumer_admin =
      channel->for_consumers();

    action="get ProxyPushSupplier";
    CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier =
      consumer_admin->obtain_push_supplier();

    action="connect to ProxyPushSupplier";
    proxy_supplier->connect_push_consumer(_this());
  }
private:
  cdrMemoryStream _memstream;
};


//
// Supplier_i
//

class Supplier_i : virtual public POA_CosEventComm::PushSupplier
{
public:
  Supplier_i(): _connected(true) {}
  void disconnect_push_supplier()
  {
    cout<<"disconnected"<<endl;
    _connected=false;
  }
  void supply(
    CosEventChannelAdmin::EventChannel_ptr channel,
    const char*& action)
  {
    action="get SupplierAdmin";
    CosEventChannelAdmin::SupplierAdmin_var supplier_admin =
      channel->for_suppliers();

    action="get ProxyPushConsumer";
    CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer =
      supplier_admin->obtain_push_consumer();

    action="connect to ProxyPushConsumer";
    proxy_consumer->connect_push_supplier(_this());

    char buf[1024];
    ssize_t len;
    action="read standard input";
    // Stream start time (seconds,nanoseconds)
    Time offsetTime;
    while(_connected && (len=read(STDIN_FILENO,buf,1024)))
    {
      CORBA::Any any;
      cdrMemoryStream memstr;
      action="put_octet_array";
      memstr.put_octet_array( (_CORBA_Octet*)buf, (int)len );
      while(_connected && memstr.currentInputPtr()<memstr.bufSize())
      {
        action="unmarshal";
        Time eventTime;
        eventTime<<=memstr;
        any<<=memstr;

        if(offsetTime.is_nil()) // first time special.
           offsetTime=Time::current()-eventTime;
        Time::sleepUntil(eventTime+offsetTime);

        action="push";
        proxy_consumer->push(any);
      }
    }
  }
private:
  bool _connected;
};


//
// main()
//

int main(int argc, char **argv)
{
  //
  // Start orb.
#if defined(HAVE_OMNIORB4)
  orb=CORBA::ORB_init(argc,argv,"omniORB4");
#else
  orb=CORBA::ORB_init(argc,argv,"omniORB3");
#endif

  // Process Options
  bool supplierMode =false;
  const char* channelName ="EventChannel";

  int c;
  while ((c = getopt(argc,argv,"shn:")) != EOF)
  {
     switch (c)
     {
        case 's': supplierMode=true;
                  break;

        case 'n': channelName = optarg;
                  break;

        case 'h': usage(argc,argv);
                  exit(0);
        default : usage(argc,argv);
                  exit(-1);
     }
  }

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
  // Ignore broken pipes
  signal(SIGPIPE, SIG_IGN);
#endif

  const char* action=""; // Use this variable to help report errors.
  try {
    CORBA::Object_var obj;

    action="resolve initial reference 'RootPOA'";
    obj=orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var rootPoa =PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(rootPoa))
        throw CORBA::OBJECT_NOT_EXIST();

    action="activate the RootPOA's POAManager";
    PortableServer::POAManager_var pman =rootPoa->the_POAManager();
    pman->activate();

    //
    // Obtain object reference to EventChannel
    // (from command-line argument or from the Naming Service).
    if(optind<argc)
    {
      action="convert URI from command line into object reference";
      obj=orb->string_to_object(argv[optind]);
    }
    else
    {
      action="resolve initial reference 'NameService'";
      obj=orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var rootContext=
        CosNaming::NamingContext::_narrow(obj);
      if(CORBA::is_nil(rootContext))
          throw CORBA::OBJECT_NOT_EXIST();

      action="find EventChannel in NameService";
      cout << action << endl;
      obj=rootContext->resolve(str2name(channelName));
    }

    action="narrow object reference to event channel";
    CosEventChannelAdmin::EventChannel_var channel =
      CosEventChannelAdmin::EventChannel::_narrow(obj);
    if(CORBA::is_nil(channel))
    {
       cerr << "Failed to narrow Event Channel reference." << endl;
       exit(1);
    }

    if(supplierMode)
    {
      action="construct PushSupplier";
      Supplier_i* supplier =new Supplier_i();
      supplier->supply(channel,action);
    }
    else
    {
      action="construct PushConsumer";
      Consumer_i* consumer =new Consumer_i();
      consumer->consume(channel,action);

      action="run ORB";
      orb->run();
    }

    return 0;

  }
  catch(CORBA::ORB::InvalidName& ex) { // resolve_initial_references
     cerr<<"Failed to "<<action<<". ORB::InvalidName"<<endl;
  }
  catch(CosNaming::NamingContext::InvalidName& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::InvalidName"<<endl;
  }
  catch(CosNaming::NamingContext::NotFound& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::NotFound"<<endl;
  }
  catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::CannotProceed"<<endl;
  }
  catch(CORBA::TRANSIENT& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". TRANSIENT"<<endl;
  }
  catch(CORBA::OBJECT_NOT_EXIST& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". OBJECT_NOT_EXIST"<<endl;
  }
  catch(CORBA::SystemException& ex) {
     cerr<<"Failed to "<<action<<"."
#if defined(HAVE_OMNIORB4)
       " "<<ex._name()<<" ("<<ex.NP_minorString()<<")"
#endif
       <<endl;
  }
  catch(CORBA::Exception& ex) {
     cerr<<"Failed to "<<action<<"."
#if defined(HAVE_OMNIORB4)
       " "<<ex._name()
#endif
       <<endl;
  }

  return 1;
}

static void usage(int argc, char **argv)
{
  cerr<<
"\nStream events from a channel to stdout, or (-s) from stdin to a channel.\n"
"syntax: "<<(argc?argv[0]:"events")<<" OPTIONS [CHANNEL_URI]\n"
"\n"
"CHANNEL_URI: The event channel may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -s       supply mode. Read events from stdin.\n"
" -n NAME  channel name (if URI is not specified)  [\"EventChannel\"]\n"
" -h       display this help text\n" << endl;
}
