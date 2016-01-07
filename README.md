Requirements:

- CMake
- Android SDK
- Android NDK
- Ant (preferrably in PATH)
- ZeroMQ

Installation on Mac OS X:
```
$ brew install cmake android-ndk ant zeromq
```

ZeroMQ for host machine should be [built and] installed manually.
ZeroMQ for Android will be downloaded&compiled automagically.

Build:

```
$ git clone git@github.com:amezin/hello-android-zmq.git
$ cd hello-android-zmq
$ git submodule init
$ git submodule update
$ cd ..

$ mkdir hello-android-zmq-build
$ cd hello-android-zmq-build
$ cmake ../hello-android-zmq
$ make
```

Run:

- Launch Android emulator (x86 or ARM)
- `$ make install`
- `$ adb forward tcp:4653 tcp:4653`
- Launch 'HelloAndroidZMQ' in the emulator.

```
$ client/client tcp://localhost:4653 com.android.calculator2
Connecting to tcp://localhost:4653
Sending com.android.calculator2
Waiting for response
OK
```
