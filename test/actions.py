import time,CORBA
import CosEventComm
import CosEventChannelAdmin

timeout=5

def passed(test):
  print "passed:",test

def failed(test):
  print "FAILED:",test


## ############################################################################
##
## actions

# EventChannel

def for_consumers(channel):
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    try:
      result=channel.for_consumers()
      assert(result._is_a(CosEventChannelAdmin._tc_ConsumerAdmin.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.1a (for_consumers raised %s)"%ex.__class__.__name__)
      raise ex

def for_suppliers(channel):
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    try:
      result=channel.for_suppliers()
      assert(result._is_a(CosEventChannelAdmin._tc_SupplierAdmin.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.1b (for_suppliers raised %s)"%ex.__class__.__name__)
      raise ex

def destroy(channel):
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    try:
      channel.destroy()
    except CORBA.UserException, ex:
      failed("2.3.1c (destroy raised %s)"%ex.__class__.__name__)
      raise ex

# ConsumerAdmin

def obtain_push_supplier(consumerAdmin):
    assert(consumerAdmin._is_a(CosEventChannelAdmin._tc_ConsumerAdmin.id()))
    try:
      result=consumerAdmin.obtain_push_supplier()
      assert(result._is_a(CosEventChannelAdmin._tc_ProxyPushSupplier.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.2a (obtain_push_supplier raised %s)"%ex.__class__.__name__)
      raise ex

def obtain_pull_supplier(consumerAdmin):
    assert(consumerAdmin._is_a(CosEventChannelAdmin._tc_ConsumerAdmin.id()))
    try:
      result=consumerAdmin.obtain_pull_supplier()
      assert(result._is_a(CosEventChannelAdmin._tc_ProxyPullSupplier.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.2b (obtain_pull_supplier raised %s)"%ex.__class__.__name__)
      raise ex

# SupplierAdmin

def obtain_push_consumer(supplierAdmin):
    assert(supplierAdmin._is_a(CosEventChannelAdmin._tc_SupplierAdmin.id()))
    try:
      result=supplierAdmin.obtain_push_consumer()
      assert(result._is_a(CosEventChannelAdmin._tc_ProxyPushConsumer.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.3a (obtain_push_consumer raised %s)"%ex.__class__.__name__)
      raise ex

def obtain_pull_consumer(supplierAdmin):
    assert(supplierAdmin._is_a(CosEventChannelAdmin._tc_SupplierAdmin.id()))
    try:
      result=supplierAdmin.obtain_pull_consumer()
      assert(result._is_a(CosEventChannelAdmin._tc_ProxyPullConsumer.id()))
      return result
    except CORBA.UserException, ex:
      failed("2.3.3b (obtain_pull_consumer raised %s)"%ex.__class__.__name__)
      raise ex

# ProxyPushConsumer

def connect_push_supplier(proxyPushConsumer,pushSupplier):
    assert(proxyPushConsumer._is_a(CosEventChannelAdmin._tc_ProxyPushConsumer.id()))
    if(pushSupplier):
      assert(pushSupplier._is_a(CosEventComm._tc_PushSupplier.id()))
    try:
      proxyPushConsumer.connect_push_supplier(pushSupplier)
    except CosEventChannelAdmin.AlreadyConnected, ex:
      raise ex
    except CORBA.Exception, ex:
      failed("2.3.4a (connect_push_supplier raised %s)"%ex.__class__.__name__)
      if(pushSupplier is None):
        failed("2.3.4b (connect_push_supplier(NIL) raised %s)"%ex.__class__.__name__)
      raise ex

# ProxyPullSupplier

def connect_pull_consumer(proxyPullSupplier,pullConsumer):
    assert(proxyPullSupplier._is_a(CosEventChannelAdmin._tc_ProxyPullSupplier.id()))
    if(pullConsumer):
      assert(pullConsumer._is_a(CosEventComm._tc_PullConsumer.id()))
    try:
      proxyPullSupplier.connect_pull_consumer(pullConsumer)
    except CosEventChannelAdmin.AlreadyConnected, ex:
      raise ex
    except CORBA.Exception, ex:
      failed("2.3.5a (connect_pull_consumer raised %s)"%ex.__class__.__name__)
      if(pullConsumer is None):
        failed("2.3.5b (connect_pull_consumer(NIL) raised %s)"%ex.__class__.__name__)
      raise

# ProxyPullConsumer

def connect_pull_supplier(proxyPullConsumer,pullSupplier):
    assert(proxyPullConsumer._is_a(CosEventChannelAdmin._tc_ProxyPullConsumer.id()))
    if(pullSupplier):
      assert(pullSupplier._is_a(CosEventComm._tc_PullSupplier.id()))
    try:
      proxyPullConsumer.connect_pull_supplier(pullSupplier)
    except CosEventChannelAdmin.AlreadyConnected, ex:
      raise ex
    except CosEventChannelAdmin.TypeError, ex:
      raise ex
    except CORBA.UserException, ex:
      failed("2.3.6a (connect_pull_supplier raised %s)"%ex.__class__.__name__)
      raise ex

# ProxyPushSupplier

def connect_push_consumer(proxyPushSupplier,pushConsumer):
    assert(proxyPushSupplier._is_a(CosEventChannelAdmin._tc_ProxyPushSupplier.id()))
    if(pushConsumer):
      assert(pushConsumer._is_a(CosEventComm._tc_PushConsumer.id()))
    try:
      proxyPushSupplier.connect_push_consumer(pushConsumer)
    except CosEventChannelAdmin.AlreadyConnected, ex:
      raise ex
    except CosEventChannelAdmin.TypeError, ex:
      raise ex
    except CORBA.BAD_PARAM, ex:
      raise ex
    except CORBA.Exception, ex:
      failed("2.3.7a (connect_push_consumer raised %s)"%ex.__class__.__name__)
      raise ex

# PushConsumer

def push(pushConsumer,event):
    assert(pushConsumer._is_a(CosEventComm._tc_PushConsumer.id()))
    try:
      pushConsumer.push(event)
    except CosEventComm.Disconnected, ex:
      raise ex
    except CORBA.Exception, ex:
      failed("2.1.1b (push raised %s)"%ex.__class__.__name__)
      raise ex

def disconnect_push_consumer(pushConsumer):
    assert(pushConsumer._is_a(CosEventComm._tc_PushConsumer.id()))
    try:
      pushConsumer.disconnect_push_consumer()
    except CORBA.Exception, ex:
      failed("2.1.1c (disconnect_push_consumer raised %s)"%ex.__class__.__name__)
      raise ex

# PushSupplier

def disconnect_push_supplier(pushSupplier):
    assert(pushSupplier._is_a(CosEventComm._tc_PushSupplier.id()))
    try:
      pushSupplier.disconnect_push_supplier()
    except CORBA.UserException, ex:
      failed("2.1.2a (disconnect_push_supplier raised %s)"%ex.__class__.__name__)
      raise ex

# PullSupplier

def pull(pullSupplier,until):
    assert(pullSupplier._is_a(CosEventComm._tc_PullSupplier.id()))
    result=None
    while(1):
      try:
        result = pullSupplier.pull()
        break
      except CORBA.TRANSIENT, ex:
        if(time.time()>until):
          raise ex
      except CosEventComm.Disconnected, ex:
        raise ex
      except CORBA.Exception, ex:
        failed("2.1.3a (pull raised %s)"%ex.__class__.__name__)
        raise ex
      #time.sleep(0.1)
    return result

def try_pull(pullSupplier,until):
    assert(pullSupplier._is_a(CosEventComm._tc_PullSupplier.id()))
    result=None
    hasEvent=0
    try:
      while(1):
        result,hasEvent = pullSupplier.try_pull()
        if(hasEvent or time.time()>until):
          break
        else:
          time.sleep(0.1)
    except CosEventComm.Disconnected, ex:
      raise ex
    except CORBA.Exception, ex:
      failed("2.1.3e (try_pull raised %s)"%ex.__class__.__name__)
      raise ex
    return result,hasEvent

def disconnect_pull_supplier(pullSupplier):
    assert(pullSupplier._is_a(CosEventComm._tc_PullSupplier.id()))
    try:
      pullSupplier.disconnect_pull_supplier()
    except CORBA.UserException, ex:
      failed("2.1.3i (disconnect_pull_supplier raised %s)"%ex.__class__.__name__)
      raise ex

# PullConsumer

def disconnect_pull_consumer(pullConsumer):
    assert(pullConsumer._is_a(CosEventComm._tc_PullConsumer.id()))
    try:
      pullConsumer.disconnect_pull_consumer()
    except CORBA.UserException, ex:
      failed("2.1.4a (disconnect_pull_consumer raised %s)"%ex.__class__.__name__)
      raise ex

## end actions
## 
## ############################################################################
