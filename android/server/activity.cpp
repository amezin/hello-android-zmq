#include <android/looper.h>
#include <android/native_activity.h>
#include <android/input.h>

#include <zmq.hpp>

#include "util.hpp"

std::string launch_app(JNIEnv *jni, jobject instance, const char *identifier);

namespace
{

class native_activity_impl : public noncopyable
{
public:
    native_activity_impl(ANativeActivity *activity);
    ~native_activity_impl();

private:

    static native_activity_impl *from_void(void *ptr);
    static native_activity_impl *from_activity(ANativeActivity *activity);

    static void destroy_callback(ANativeActivity* activity);
    static int looper_event_callback(int, int, void *data);
    static int looper_request_callback(int, int, void *data);
    static void input_queue_created_callback(ANativeActivity *activity, AInputQueue *queue);
    static void input_queue_destroyed_callback(ANativeActivity *activity, AInputQueue *);

    void set_input_queue(AInputQueue *input_queue);
    void handle_events();
    void handle_requests();

    ANativeActivity *activity_;
    AInputQueue *input_queue_;
    ALooper *looper_;
    zmq::context_t zmq_context_;
    zmq::socket_t socket_;
    int socket_fd_;
};

native_activity_impl::native_activity_impl(ANativeActivity *activity)
    : activity_(activity),
      input_queue_(nullptr),
      looper_(ALooper_forThread()),
      zmq_context_(1),
      socket_(zmq_context_, zmq::socket_type::rep)
{
    socket_.bind("tcp://*:4653");
    LOGI("Listening on tcp://*:4653. Don't forget to forward the port.");

    auto fd_size = sizeof(socket_fd_);
    socket_.getsockopt(ZMQ_FD, &socket_fd_, &fd_size);
    ALooper_addFd(looper_,
                  socket_fd_,
                  ALOOPER_POLL_CALLBACK,
                  ALOOPER_EVENT_INPUT,
                  &looper_request_callback,
                  this);

    activity_->callbacks->onInputQueueCreated = &input_queue_created_callback;
    activity_->callbacks->onInputQueueDestroyed = &input_queue_destroyed_callback;
    activity_->callbacks->onDestroy = &destroy_callback;
}

native_activity_impl::~native_activity_impl()
{
    ALooper_removeFd(looper_, socket_fd_);
    set_input_queue(nullptr);
    LOGI("Server stopped");
}

native_activity_impl *native_activity_impl::from_void(void *ptr)
{
    return static_cast<native_activity_impl *>(ptr);
}

native_activity_impl *native_activity_impl::from_activity(ANativeActivity *activity)
{
    if (!activity) {
        return nullptr;
    }
    return from_void(activity->instance);
}

void native_activity_impl::destroy_callback(ANativeActivity* activity)
{
    try {
        delete from_activity(activity);
    } catch (const std::exception &ex) {
        LOGE("Exception when deleting native_activity_impl: %s", ex.what());
    }
}

int native_activity_impl::looper_event_callback(int, int, void *data)
{
    try {
        from_void(data)->handle_events();
    } catch (const std::exception &ex) {
        LOGE("Exception from handle_events(): %s", ex.what());
    }
    return 1;
}

int native_activity_impl::looper_request_callback(int, int, void *data)
{
    try {
        from_void(data)->handle_requests();
    } catch (const std::exception &ex) {
        LOGE("Exception from handle_requests(): %s", ex.what());
    }
    return 1;
}

void native_activity_impl::input_queue_created_callback(ANativeActivity *activity, AInputQueue *queue)
{
    try {
        from_activity(activity)->set_input_queue(queue);
    } catch (const std::exception &ex) {
        LOGE("Exception from set_input_queue(%p): %s", queue, ex.what());
    }
}

void native_activity_impl::input_queue_destroyed_callback(ANativeActivity *activity, AInputQueue *)
{
    try {
        from_activity(activity)->set_input_queue(nullptr);
    } catch (const std::exception &ex) {
        LOGE("Exception from set_input_queue(NULL): %s", ex.what());
    }
}

void native_activity_impl::set_input_queue(AInputQueue *input_queue)
{
    if (input_queue == input_queue_) {
        return;
    }
    if (input_queue_) {
        AInputQueue_detachLooper(input_queue_);
    }
    input_queue_ = input_queue;
    if (input_queue) {
        AInputQueue_attachLooper(input_queue_,
                                 looper_,
                                 ALOOPER_POLL_CALLBACK,
                                 &looper_event_callback,
                                 this);
    }
}

void native_activity_impl::handle_events()
{
    AInputEvent *input_event(nullptr);
    while (AInputQueue_getEvent(input_queue_, &input_event) >= 0) {
        if (!AInputQueue_preDispatchEvent(input_queue_, input_event)) {
            AInputQueue_finishEvent(input_queue_, input_event, false);
        }
    }
}

void native_activity_impl::handle_requests()
{
    zmq::message_t message;
    while (socket_.recv(&message, ZMQ_DONTWAIT)) {
        std::string identifier(message.data<const char>(), message.size());
        auto result = launch_app(activity_->env, activity_->clazz, identifier.c_str());
        socket_.send(result.c_str(), result.size());
    }
}

}

extern "C" {

void ANativeActivity_onCreate(ANativeActivity* activity,
                              void* savedState, size_t savedStateSize)
{
    try {
        activity->instance = new native_activity_impl(activity);
    } catch (const std::exception &ex) {
        LOGE("Exception when creating native_activity_impl: %s", ex.what());
    }
}

}
