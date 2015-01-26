channel.o: \
  channel.cc \
  ../src/scour.h \
  ../src/config.h \

## Diagnostic information starts here
# ../src/scour.h  from  ../src/config.h
# ../src/config.h  from  channel.cc

pullcons.o: \
  pullcons.cc \
  ../idl/COS_sysdep.h \
  ../src/getopt.h \
  ../idl/CosEventChannelAdmin.hh \
  ../src/scour.h \
  ../src/config.h \
  ../idl/CosEventComm.hh \
  ../src/naming.h \

## Diagnostic information starts here
# ../idl/COS_sysdep.h  from  ../idl/CosEventComm.hh
# ../src/getopt.h  from  pullcons.cc
# ../idl/CosEventChannelAdmin.hh  from  pullcons.cc
# ../src/scour.h  from  ../src/config.h
# ../src/config.h  from  pullcons.cc
# ../idl/CosEventComm.hh  from  pullcons.cc
# ../src/naming.h  from  pullcons.cc

pullsupp.o: \
  pullsupp.cc \
  ../idl/COS_sysdep.h \
  ../src/getopt.h \
  ../idl/CosEventChannelAdmin.hh \
  ../src/scour.h \
  ../src/config.h \
  ../idl/CosEventComm.hh \
  ../src/naming.h \

## Diagnostic information starts here
# ../idl/COS_sysdep.h  from  ../idl/CosEventComm.hh
# ../src/getopt.h  from  pullsupp.cc
# ../idl/CosEventChannelAdmin.hh  from  pullsupp.cc
# ../src/scour.h  from  ../src/config.h
# ../src/config.h  from  pullsupp.cc
# ../idl/CosEventComm.hh  from  pullsupp.cc
# ../src/naming.h  from  pullsupp.cc

pushcons.o: \
  pushcons.cc \
  ../idl/COS_sysdep.h \
  ../src/getopt.h \
  ../idl/CosEventChannelAdmin.hh \
  ../src/scour.h \
  ../src/config.h \
  ../idl/CosEventComm.hh \
  ../src/naming.h \

## Diagnostic information starts here
# ../idl/COS_sysdep.h  from  ../idl/CosEventComm.hh
# ../src/getopt.h  from  pushcons.cc
# ../idl/CosEventChannelAdmin.hh  from  pushcons.cc
# ../src/scour.h  from  ../src/config.h
# ../src/config.h  from  pushcons.cc
# ../idl/CosEventComm.hh  from  pushcons.cc
# ../src/naming.h  from  pushcons.cc

pushsupp.o: \
  pushsupp.cc \
  ../idl/COS_sysdep.h \
  ../src/getopt.h \
  ../idl/CosEventChannelAdmin.hh \
  ../src/scour.h \
  ../src/config.h \
  ../idl/CosEventComm.hh \
  ../src/naming.h \

## Diagnostic information starts here
# ../idl/COS_sysdep.h  from  ../idl/CosEventComm.hh
# ../src/getopt.h  from  pushsupp.cc
# ../idl/CosEventChannelAdmin.hh  from  pushsupp.cc
# ../src/scour.h  from  ../src/config.h
# ../src/config.h  from  pushsupp.cc
# ../idl/CosEventComm.hh  from  pushsupp.cc
# ../src/naming.h  from  pushsupp.cc

