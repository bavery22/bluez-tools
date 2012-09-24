LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bluetooth
LOCAL_SRC_FILES := lib/uuid.c \
	lib/bluetooth.c \
	lib/sdp.c \
	lib/hci.c

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := hcitool
LOCAL_SRC_FILES := tools/hcitool.c
LOCAL_STATIC_LIBARIES := bluetooth

include $(BUILD_EXECUTABLE)
