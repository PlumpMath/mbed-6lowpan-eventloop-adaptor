include ../makefile_defines.txt

COMPONENT_NAME = cs_nvm_unit

SRC_FILES = ../../../../source/cs_nvm.c

TEST_SRC_FILES = main.cpp \
	csnvmtest.cpp \
	test_cs_nvm.c \
	../stubs/nsdynmemLIB_stub.c \
	../stubs/ns_trace_stub.c \
	../stubs/timeout_stub.c \
	../stubs/configuration_store_stub.c

include ../MakefileWorker.mk

CPPUTESTFLAGS += -DFEA_TRACE_SUPPORT -DTARGET_LIKE_X86_LINUX_NATIVE

