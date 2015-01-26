import sys,time,CORBA
import CosNaming
import CosEventComm
import CosEventChannelAdmin
import EventChannelAdmin
from actions import *
import servant

def makeNVP(name,typecode,value):
  return CosLifeCycle.NameValuePair(name,CORBA.Any(typecode,value))

def create_channel(factory):
    assert(factory._is_a(EventChannelAdmin._tc_EventChannelFactory.id()))
    key=[CosNaming.NameComponent("EventChannel","object interface")]
    criteria=[]
    #criteria.append(makeNVP("PullRetryPeriod",CORBA.TC_ulong,1))
    #criteria.append(makeNVP("MaxQueueLength" ,CORBA.TC_ulong,1023))
    obj=factory.create_object(key,criteria)
    assert(obj is not None)
    channel=obj._narrow(CosEventChannelAdmin.EventChannel)
    assert(channel is not None)
    return channel

def test(factory):
    # Prepare
    assert(factory._is_a(EventChannelAdmin._tc_EventChannelFactory.id()))
    pushconsumer_i=servant.PushConsumer_i()
    pushsupplier_i=servant.PushSupplier_i()
    pullconsumer_i=servant.PullConsumer_i()
    pullsupplier_i=servant.PullSupplier_i()
    
    channel=create_channel(factory)

    consumeradmin=for_consumers(channel)
    supplieradmin=for_suppliers(channel)
    pushconsumer_i.connect(channel)
    pushsupplier_i.connect(channel)
    pullconsumer_i.connect(channel)
    pullsupplier_i.connect(channel)

    # Test EventChannel::destroy()
    try:
      destroy(channel)
      passed('destroy worked')
    except CORBA.UserException, ex:
      pass #  User Exceptions are reported as a failure
    except CORBA.SystemException, ex:
      # Strictly this passes, as there is no requirement for destroy() to
      # to actually DO anything.
      passed('destroy raised %s. (STRICT PASS)'%ex.__class__.__name__)
    time.sleep(2)

    test="EvCh-1" # -----------------------------------------------------------
    try:
      for_suppliers(channel)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-2" # -----------------------------------------------------------
    try:
      for_consumers(channel)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-3" # -----------------------------------------------------------
    try:
      destroy(channel)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-4" # -----------------------------------------------------------
    try:
      obtain_push_supplier(consumeradmin)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-5" # -----------------------------------------------------------
    try:
      obtain_pull_supplier(consumeradmin)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-6" # -----------------------------------------------------------
    try:
      obtain_push_consumer(supplieradmin)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-7" # -----------------------------------------------------------
    try:
      obtain_pull_consumer(supplieradmin)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="EvCh-8" # -----------------------------------------------------------
    if(pushsupplier_i.disconnected):
      passed(test)
    else:
      failed(test)

    test="EvCh-9" # -----------------------------------------------------------
    if(pullconsumer_i.disconnected):
      passed(test)
    else:
      failed(test)

    test="EvCh-10" # -----------------------------------------------------------
    if(pullsupplier_i.disconnected):
      passed(test)
    else:
      failed(test)

    test="EvCh-11" # -----------------------------------------------------------
    if(pushconsumer_i.disconnected):
      passed(test)
    else:
      failed(test)
#end test


################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
  poa = orb.resolve_initial_references("RootPOA")
  poaManager = poa._get_the_POAManager()
  poaManager.activate()
  obj = orb.resolve_initial_references("EventChannelFactory")
  factory = obj._narrow(EventChannelAdmin.EventChannelFactory)
  
  test(factory)
  time.sleep(3)
##
