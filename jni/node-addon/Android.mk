LOCAL_PATH:= $(call my-dir)

# any additional addons that want to use the node/v8 headers
# should add the prebuilt/include  to their LOCAL_C_INCLUDES


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    BT.cpp \
    device.cpp \
    glibHandler.cpp \
    att.c \
    utils.c \
    gattrib.c \
    gatt.c \
    ../btio/btio.c \
    scanner.cpp



LOCAL_MODULE:= libbtle_node
LOCAL_MODULE_CLASS    := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := eng
LOCAL_C_INCLUDES := frameworks/base/services \
	external/skia/include/core \
	bionic \
	external/stlport/stlport \
	external/node/src \
	external/node/deps/uv/include \
	external/node/deps/v8/include \
	external/jpeg \
	external/libpng \
	external/zlib \
	frameworks/base/include/surfaceflinger \
	$(LOCAL_PATH)/../glib/ \
	$(LOCAL_PATH)/../btio/ \
	$(LOCAL_PATH)/../ \
	external/aminolang/prebuilt/include


LOCAL_CFLAGS = -DKLAATU -DBUILDING_NODE_EXTENSION -fpermissive 	-DNEED_PPOLL -D__ANDROID__

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_STATIC_LIBRARIES := glib bluetooth
TARGET_CUSTOM_DEBUG_CFLAGS := ­O0 -g 

LOCAL_SHARED_LIBRARIES := \
    libEGL libGLESv2 libui libgui \
    libutils libstlport libinput \
    libjpeg \
    libv8 \
    libmedia libbinder libcutils \
    libz

#building a shared lib because this is for a NodeJS addon
include $(BUILD_SHARED_LIBRARY)


# and the fixup shell script
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	btle_node-dependencyForcer.c

LOCAL_SHARED_LIBRARIES := \
	libbtle_node

LOCAL_MODULE:= btle_node-dependencyForcer
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := EXECUTABLES
intermediates:= $(local-intermediates-dir)
GEN := $(LOCAL_PATH)/btle_node-dependencyForcer.c
$(GEN): PRIVATE_INPUT_FILE := $(LOCAL_PATH)/AndroidFixup.sh
$(GEN): PRIVATE_CUSTOM_TOOL = bash $(PRIVATE_INPUT_FILE) $@ 
$(GEN): $(LOCAL_PATH)/AndroidFixup.sh 
	$(transform-generated-source)
$(GEN): libbtle_node
.PHONY: $(GEN)
#LOCAL_GENERATED_SOURCES += $(GEN)



include $(BUILD_EXECUTABLE)
