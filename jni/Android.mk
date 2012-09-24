LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bluetooth
LOCAL_SRC_FILES := lib/uuid.c \
	lib/bluetooth.c \
	lib/sdp.c \
	lib/hci.c

LOCAL_CFLAGS:= \
	-DVERSION=\"4.93\" \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-DNEED_PPOLL \
	-D__ANDROID__ \
	-DCONFIGDIR=\"/etc/bluetooth\" \
	-DSERVICEDIR=\"/system/bin\" \
	-DPLUGINDIR=\"/system/lib/bluez-plugin\" \
	-DANDROID_SET_AID_AND_CAP \
	-DANDROID_EXPAND_NAME \
	-DOUIFILE=\"/data/misc/bluetoothd/ouifile\"

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := bluetoothd
LOCAL_SRC_FILES := src/oui.c \
	src/textfile.c

LOCAL_CFLAGS:= \
	-DVERSION=\"4.93\" \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-DNEED_PPOLL \
	-D__ANDROID__ \
	-DCONFIGDIR=\"/etc/bluetooth\" \
	-DSERVICEDIR=\"/system/bin\" \
	-DPLUGINDIR=\"/system/lib/bluez-plugin\" \
	-DANDROID_SET_AID_AND_CAP \
	-DANDROID_EXPAND_NAME \
	-DOUIFILE=\"/data/misc/bluetoothd/ouifile\"

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	glib/gdir.c \
	glib/gerror.c \
	glib/giochannel.c \
	glib/gkeyfile.c \
	glib/gmain.c \
	glib/gmem.c \
	glib/goption.c \
	glib/gslice.c \
	glib/gslist.c \
	glib/gstring.c \
	glib/gstrfuncs.c \
	glib/gtimer.c \
	glib/giounix.c \
	glib/gmessages.c \
	glib/gutf8.c \
	glib/gfileutils.c \
	glib/gconvert.c \
	glib/gdataset.c \
	glib/gtestutils.c \
	glib/ghash.c \
	glib/glist.c \
	glib/gthread.c \
	glib/garray.c \
	glib/gutils.c \
	glib/gatomic.c \
	glib/gprintf.c \
	glib/gpattern.c \
	glib/guniprop.c \
	glib/gpoll.c \
	glib/grand.c \
	glib/gunidecomp.c \
	glib/gqsort.c \
	glib/gstdio.c \
	glib/gqueue.c \

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/glib/
#	$(LOCAL_PATH)/../ \
#	$(LOCAL_PATH)

LOCAL_CFLAGS:= \
	-DANDROID_STUB -fno-strict-aliasing

LOCAL_MODULE := glib

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := hcitool
LOCAL_SRC_FILES := tools/hcitool.c
LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := gatttool
LOCAL_SRC_FILES := attrib/gatttool.c \
	attrib/gatt.c \
	attrib/gattrib.c \
	attrib/att.c \
	attrib/utils.c

LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd glib
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/ \
	$(LOCAL_PATH)/btio/ \
	$(LOCAL_PATH)/glib/

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)
