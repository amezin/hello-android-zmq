#pragma once

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "HelloZMQ", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "HelloZMQ", __VA_ARGS__))

class noncopyable
{
public:
    noncopyable() { }
private:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator =(const noncopyable &) = delete;
};
