LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := modbus

LOCAL_SRC_FILES := \
	../src/modbus-data.c \
	../src/modbus-rtu.c \
	../src/modbus-tcp.c \
	../src/modbus.c \


# Prepare config.h and modbus-version.h
$(shell python $(LOCAL_PATH)/configure.py)

include $(BUILD_SHARED_LIBRARY)
