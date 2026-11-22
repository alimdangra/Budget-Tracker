# JNI build overlay for an official SDL2 Android Studio template.
# Copy Budgetly_Portable.c and BudgetTracker_Core.inc beside this makefile.
# The SDL2 and SDL2_ttf libraries must be included/built in the Android project.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_SRC_FILES := Budgetly_Portable.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../SDL/include $(LOCAL_PATH)/../SDL2_ttf
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf
LOCAL_LDLIBS := -lGLESv2 -llog -landroid -lm
include $(BUILD_SHARED_LIBRARY)
