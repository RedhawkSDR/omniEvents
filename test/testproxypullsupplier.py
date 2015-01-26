import sys,time,CORBA
import CosEventComm
import CosEventChannelAdmin
from actions import *
import servant

def test(channel):
    # Prepare
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    consumer_i=servant.PullConsumer_i()
    supplier_i=servant.PushSupplier_i()
    supplier_i.connect(channel)
    consumerAdmin=for_consumers(channel)

    test="PPulS-1" # ----------------------------------------------------------
    proxyPullSupplier=obtain_pull_supplier(consumerAdmin)
    try:
      connect_pull_consumer(proxyPullSupplier,CosEventComm.PullConsumer._nil)
      push(supplier_i.pushConsumer,CORBA.Any(CORBA.TC_string,test))
      result,hasEvent =try_pull(proxyPullSupplier,time.time()+timeout)
      if(hasEvent):
        if(result.value(CORBA.TC_string)==test):
          passed(test)
        else:
          failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
      else:
        failed(test+": timed out")
      disconnect_pull_supplier(proxyPullSupplier) # Tidy up
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)

    test="PPulS-3" # ----------------------------------------------------------
    proxyPullSupplier=obtain_pull_supplier(consumerAdmin)
    connect_pull_consumer(proxyPullSupplier,consumer_i._this())
    try:
      connect_pull_consumer(proxyPullSupplier,consumer_i._this())
      failed(test+": no exception")
    except CosEventChannelAdmin.AlreadyConnected, ex:
      passed(test)
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)

    #test="PulS-1" # -----------------------------------------------------------
    #try:
    #  result=pull(proxyPullSupplier,time.time()+timeout) # ?? Assumes pull will time out.
    #  failed(test+": pull returned value("+result.value(CORBA.TC_string)+")")
    #except:
    #  passed(test)

    test="PulS-2" # -----------------------------------------------------------
    push(supplier_i.pushConsumer,CORBA.Any(CORBA.TC_string,test))
    try:
      result=pull(proxyPullSupplier,time.time()+timeout)
      if(result.value(CORBA.TC_string)==test):
        passed(test)
      else:
        failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)

    test="PulS-3" # -----------------------------------------------------------
    push(supplier_i.pushConsumer,CORBA.Any(CORBA.TC_string,test))
    try:
      result,hasEvent =try_pull(proxyPullSupplier,time.time()+timeout)
      if(hasEvent):
        if(result.value(CORBA.TC_string)==test):
          passed(test)
        else:
          failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
      else:
        failed(test+": timed out")
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)

    test="PulS-4" # -----------------------------------------------------------
    try:
      result,hasEvent =try_pull(proxyPullSupplier,time.time()+timeout)
      if(hasEvent):
        failed(test+": got event ("+result.value(CORBA.TC_string)+")")
      else:
        passed(test)
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)

    test="PPulS-2" # ----------------------------------------------------------
    disconnect_pull_supplier(proxyPullSupplier)
    time.sleep(1)
    if(consumer_i.disconnected):
      passed(test)
    else:
      failed(test)

    test="PulS-5" # -----------------------------------------------------------
    try:
      result=pull(proxyPullSupplier,time.time()+timeout)
      failed(test+": got event ("+result.value(CORBA.TC_string)+")")
    except CORBA.TRANSIENT, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)

    test="PulS-6" # -----------------------------------------------------------
    try:
      result,hasEvent =try_pull(proxyPullSupplier,time.time()+timeout)
      if(hasEvent):
        failed(test+": got event ("+result.value(CORBA.TC_string)+")")
      else:
        failed(test+": returned (has_event=0)")
    except CORBA.TRANSIENT, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)

    test="PPulS-8" # ----------------------------------------------------------
    try:
      disconnect_pull_supplier(proxyPullSupplier)
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": got exception %s"%ex.__class__.__name__)
#end test


################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
  poa = orb.resolve_initial_references("RootPOA")
  poaManager = poa._get_the_POAManager()
  poaManager.activate()
  obj = orb.resolve_initial_references("Channel")
  channel = obj._narrow(CosEventChannelAdmin.EventChannel)
  
  test(channel)
  time.sleep(3)
##
