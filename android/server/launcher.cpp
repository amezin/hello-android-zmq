#include <cassert>
#include <string>

#include <jni.h>

#include "util.hpp"

namespace
{

class jni_local_frame : public noncopyable
{
public:
    jni_local_frame(JNIEnv *jni, int size)
        : jni_(jni), popped_(false)
    {
        jni_->PushLocalFrame(size);
    }

    jobject pop(jobject keep = nullptr)
    {
        assert(!popped_);
        if (!popped_) {
            popped_ = true;
            return jni_->PopLocalFrame(nullptr);
        }
    }

    ~jni_local_frame()
    {
        if (!popped_) {
            pop(nullptr);
        }
    }

private:
    JNIEnv *jni_;
    bool popped_;
};

class jni_utf_chars : public noncopyable
{
public:
    jni_utf_chars(JNIEnv *jni, jstring str)
        : jni_(jni), str_(str), chars_(jni_->GetStringUTFChars(str_, nullptr))
    {
    }

    ~jni_utf_chars()
    {
        if (chars_) {
            jni_->ReleaseStringUTFChars(str_, chars_);
        }
    }

    const char *chars() const
    {
        return chars_;
    }

private:
    JNIEnv *jni_;
    jstring str_;
    const char *chars_;
};

std::string jni_string_get(JNIEnv *jni, jstring str)
{
    jni_utf_chars chars(jni, str);
    if (!chars.chars()) {
        return "Can't get chars from jstring";
    }
    return chars.chars();
};

std::string jni_current_exception(JNIEnv *jni)
{
    auto exception = jni->ExceptionOccurred();
    if (!exception) {
        return "No exception";
    }
    jni->ExceptionDescribe();
    jni->ExceptionClear();

    jni_local_frame j_frame(jni, 10);

    auto j_class = jni->GetObjectClass(exception);
    if (!j_class) {
        jni->ExceptionClear();
        return "Can't get exception class";
    }
    auto to_string = jni->GetMethodID(j_class, "toString", "()Ljava/lang/String;");
    if (!to_string) {
        jni->ExceptionClear();
        return "Can't find toString() exception method";
    }
    auto as_string = static_cast<jstring>(jni->CallObjectMethod(exception, to_string));
    if (!as_string) {
        jni->ExceptionClear();
        return "Can't convert exception to string";
    }
    return jni_string_get(jni, as_string);
}

}

std::string launch_app(JNIEnv *jni, jobject instance, const char *identifier)
{
    LOGI("Launching %s", identifier);

    /* Java:
    Intent launchIntent = instance.getPackageManager().getLaunchIntentForPackage(identifier);
    instance.startActivity(launchIntent);
    */

    for (auto c = identifier; *c; c++) {
        if (c < 0) {
            return "Invalid character in identifier";
        }
    }

    jni_local_frame j_frame(jni, 100);

    auto j_identifier = jni->NewStringUTF(identifier);
    if (!j_identifier) {
        LOGE("NewStringUTF failed");
        return jni_current_exception(jni);
    }

    auto j_class = jni->GetObjectClass(instance);
    if (!j_class) {
        LOGE("GetObjectClass (instance) failed");
        return jni_current_exception(jni);
    }

    auto get_package_manager = jni->GetMethodID(j_class, "getPackageManager",
                                                "()Landroid/content/pm/PackageManager;");
    if (!get_package_manager) {
        LOGE("GetMethodID (getPackageManager) failed");
        return jni_current_exception(jni);
    }

    auto package_manager = jni->CallObjectMethod(instance, get_package_manager);
    if (!package_manager) {
        LOGE("CallObjectMethod (getPackageManager) failed");
        return jni_current_exception(jni);
    }

    auto package_manager_class = jni->GetObjectClass(package_manager);
    if (!package_manager_class) {
        LOGE("GetObjectClass (package_manager) failed");
        return jni_current_exception(jni);
    }

    auto get_launch_intent_for_package = jni->GetMethodID(package_manager_class,
                                                          "getLaunchIntentForPackage",
                                                          "(Ljava/lang/String;)Landroid/content/Intent;");
    if (!get_launch_intent_for_package) {
        LOGE("GetMethodID (getLaunchIntentForPackage) failed");
        return jni_current_exception(jni);
    }

    auto launch_intent = jni->CallObjectMethod(package_manager,
                                               get_launch_intent_for_package,
                                               j_identifier);
    if (!launch_intent) {
        LOGE("CallObjectMethod (getLaunchIntentForPackage) failed");
        if (!jni->ExceptionCheck()) {
            return "Package " + std::string(identifier) + " hasn't been found";
        }
        return jni_current_exception(jni);
    }

    auto start_activity = jni->GetMethodID(j_class, "startActivity",
                                           "(Landroid/content/Intent;)V");
    if (!start_activity) {
        LOGE("GetMethodID (startActivity) failed");
        return jni_current_exception(jni);
    }

    jni->CallVoidMethod(instance, start_activity, launch_intent);
    if (jni->ExceptionCheck()) {
        LOGE("CallVoidMethod (startActivity) failed");
        return jni_current_exception(jni);
    }

    return "OK";
}
