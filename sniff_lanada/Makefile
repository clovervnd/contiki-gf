CONTIKI = ../

all: sniff_test sensor base

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

MODULES += core/net/mac/cxmac

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
