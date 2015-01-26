import sys,time,CORBA
import CosEventComm
import CosEventChannelAdmin
from actions import *
import servant

def test(channel):
    # Prepare
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    consumer_i=servant.PullConsumer_i()
    consumer_i.connect(channel)
    supplier_i=servant.PushSupplier_i()
    supplierAdmin=for_suppliers(channel)

    test="PPshC-1" # ----------------------------------------------------------
    proxyPushConsumer=obtain_push_consumer(supplierAdmin)
    connect_push_supplier(proxyPushConsumer,CosEventComm.PushSupplier._nil)
    push(proxyPushConsumer,CORBA.Any(CORBA.TC_string,test))
    result,hasEvent =try_pull(consumer_i.pullSupplier,time.time()+timeout)
    if(hasEvent):
      if(result.value(CORBA.TC_string)==test):
        passed(test)
      else:
        failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
    else:
      failed(test+": timed out")
    disconnect_push_consumer(proxyPushConsumer) # Tidy!

    test="PPshC-3" # ----------------------------------------------------------
    proxyPushConsumer=obtain_push_consumer(supplierAdmin)
    connect_push_supplier(proxyPushConsumer,supplier_i._this())
    try:
      connect_push_supplier(proxyPushConsumer,supplier_i._this())
      failed(test+": no exception")
    except CosEventChannelAdmin.AlreadyConnected, ex:
      passed(test)
    except:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="PshC-1" # -----------------------------------------------------------
    push(proxyPushConsumer,CORBA.Any(CORBA.TC_string,test))
    result,hasEvent =try_pull(consumer_i.pullSupplier,time.time()+timeout)
    if(hasEvent):
      if(result.value(CORBA.TC_string)==test):
        passed(test)
      else:
        failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
    else:
      failed(test+": timed out")

    test="PshC-2" # -----------------------------------------------------------
    disconnect_push_consumer(proxyPushConsumer)
    try:
      push(proxyPushConsumer,CORBA.Any(CORBA.TC_string,test))
    except:
      pass
    result,hasEvent =try_pull(consumer_i.pullSupplier,time.time()+timeout)
    if(hasEvent and result.value(CORBA.TC_string)==test):
      failed(test+": value arrived ("+result.value(CORBA.TC_string)+")")
    else:
      passed(test)

    test="PPshC-2" # & PshC-4 # -----------------------------------------------
    time.sleep(0.1)
    if(supplier_i.disconnected):
      passed(test+", PshC-4")
    else:
      failed(test)

    test="PshC-5" # -----------------------------------------------------------
    try:
      disconnect_push_consumer(proxyPushConsumer)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test)
    except:
      failed(test+": wrong exception")

    #disconnect_pull_supplier(consumer_i.pullSupplier)
    consumer_i.disconnect_pull_consumer()
    time.sleep(1)
#end testProxyPushConsumer() --------------------------------------------------


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
