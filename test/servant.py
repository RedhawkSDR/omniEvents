import CORBA
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin
from actions import *

class PullConsumer_i(CosEventComm__POA.PullConsumer):
  def __init__(self,pullSupplier=None):
      self.disconnected=0
      self.pullSupplier=pullSupplier
  def disconnect_pull_consumer(self):
      disconnect=(0==self.disconnected)
      self.disconnected=1
      if(disconnect and self.pullSupplier):
        try: disconnect_pull_supplier(self.pullSupplier)
        except: pass
  def connect(self,channel):
      consumerAdmin=for_consumers(channel)
      self.pullSupplier=obtain_pull_supplier(consumerAdmin)
      connect_pull_consumer(self.pullSupplier,self._this())
#end class PullConsumer_i


class PushConsumer_i(CosEventComm__POA.PushConsumer):
  def __init__(self,pushSupplier=None):
      self.disconnected=0
      self.pushSupplier=pushSupplier
      self.event=None
  def disconnect_push_consumer(self):
      disconnect=(0==self.disconnected)
      self.disconnected=1
      if(disconnect and self.pushSupplier):
        try: disconnect_push_supplier(self.pushSupplier)
        except: pass
  def push(self,event):
      self.event=event
  def connect(self,channel):
      consumerAdmin=for_consumers(channel)
      self.pushSupplier=obtain_push_supplier(consumerAdmin)
      connect_push_consumer(self.pushSupplier,self._this())
  def receive(self,until):
      while(not self.event and not time.time()>until):
        time.sleep(0.1)
      result=self.event
      self.event=None
      return result
#end class PushConsumer_i


class PushSupplier_i(CosEventComm__POA.PushSupplier):
  def __init__(self,pushConsumer=None):
      self.disconnected=0
      self.pushConsumer=pushConsumer
  def disconnect_push_supplier(self):
      disconnect=(0==self.disconnected)
      self.disconnected=1
      if(disconnect and self.pushConsumer):
        try: disconnect_push_consumer(self.pushConsumer)
        except: pass
  def connect(self,channel):
      supplierAdmin=for_suppliers(channel)
      self.pushConsumer=obtain_push_consumer(supplierAdmin)
      connect_push_supplier(self.pushConsumer,self._this())
#end class PushSupplier_i


class PullSupplier_i(CosEventComm__POA.PullSupplier):
  def __init__(self,pullConsumer=None):
      self.disconnected=0
      self.pullConsumer=pullConsumer
      self.event=None
  def disconnect_pull_supplier(self):
      disconnect=(0==self.disconnected)
      self.disconnected=1
      if(disconnect and self.pullConsumer):
        try: disconnect_pull_consumer(self.pullConsumer)
        except: pass
  def pull(self):
      if(self.event):
        result=self.event
        self.event=None
        return result
      else:
        raise CORBA.TRANSIENT(0,CORBA.COMPLETED_NO)
  def try_pull(self):
      if(self.event):
        result=self.event
        self.event=None
        return (result,1)
      else:
        return (CORBA.Any(CORBA.TC_long,0),0)
  def connect(self,channel):
      supplierAdmin=for_suppliers(channel)
      self.pullConsumer=obtain_pull_consumer(supplierAdmin)
      connect_pull_supplier(self.pullConsumer,self._this())
#end class PullSupplier_i
