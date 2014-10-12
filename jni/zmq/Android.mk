LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= zmq

LOCAL_SRC_FILES:= \
	zmqserver.c \
	zmq4.x/src/address.cpp \
	zmq4.x/src/clock.cpp \
	zmq4.x/src/ctx.cpp \
	zmq4.x/src/curve_client.cpp \
	zmq4.x/src/curve_server.cpp \
	zmq4.x/src/dealer.cpp \
	zmq4.x/src/devpoll.cpp \
	zmq4.x/src/dist.cpp \
	zmq4.x/src/epoll.cpp \
	zmq4.x/src/err.cpp \
	zmq4.x/src/fq.cpp \
	zmq4.x/src/io_object.cpp \
	zmq4.x/src/io_thread.cpp \
	zmq4.x/src/ipc_address.cpp \
	zmq4.x/src/ipc_connecter.cpp \
	zmq4.x/src/ipc_listener.cpp \
	zmq4.x/src/ip.cpp \
	zmq4.x/src/kqueue.cpp \
	zmq4.x/src/lb.cpp \
	zmq4.x/src/mailbox.cpp \
	zmq4.x/src/mechanism.cpp \
	zmq4.x/src/msg.cpp \
	zmq4.x/src/mtrie.cpp \
	zmq4.x/src/null_mechanism.cpp \
	zmq4.x/src/object.cpp \
	zmq4.x/src/options.cpp \
	zmq4.x/src/own.cpp \
	zmq4.x/src/pair.cpp \
	zmq4.x/src/pgm_receiver.cpp \
	zmq4.x/src/pgm_sender.cpp \
	zmq4.x/src/pgm_socket.cpp \
	zmq4.x/src/pipe.cpp \
	zmq4.x/src/plain_mechanism.cpp \
	zmq4.x/src/poll.cpp \
	zmq4.x/src/poller_base.cpp \
	zmq4.x/src/precompiled.cpp \
	zmq4.x/src/proxy.cpp \
	zmq4.x/src/pub.cpp \
	zmq4.x/src/pull.cpp \
	zmq4.x/src/push.cpp \
	zmq4.x/src/random.cpp \
	zmq4.x/src/raw_decoder.cpp \
	zmq4.x/src/raw_encoder.cpp \
	zmq4.x/src/reaper.cpp \
	zmq4.x/src/rep.cpp \
	zmq4.x/src/req.cpp \
	zmq4.x/src/router.cpp \
	zmq4.x/src/select.cpp \
	zmq4.x/src/session_base.cpp \
	zmq4.x/src/signaler.cpp \
	zmq4.x/src/socket_base.cpp \
	zmq4.x/src/stream.cpp \
	zmq4.x/src/stream_engine.cpp \
	zmq4.x/src/sub.cpp \
	zmq4.x/src/tcp_address.cpp \
	zmq4.x/src/tcp_connecter.cpp \
	zmq4.x/src/tcp.cpp \
	zmq4.x/src/tcp_listener.cpp \
	zmq4.x/src/thread.cpp \
	zmq4.x/src/trie.cpp \
	zmq4.x/src/v1_decoder.cpp \
	zmq4.x/src/v1_encoder.cpp \
	zmq4.x/src/v2_decoder.cpp \
	zmq4.x/src/v2_encoder.cpp \
	zmq4.x/src/xpub.cpp \
	zmq4.x/src/xsub.cpp \
	zmq4.x/src/zmq.cpp \
	zmq4.x/src/zmq_utils.cpp 

LOCAL_LDLIBS +=  -llog -lz -ldl -landroid -lstdc++ 
LOCAL_CFLAGS  +=  -Wall \
		-O3 
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/zmq4.x/include \
	$(LOCAL_PATH)/zmq4.x/src
	
LOCAL_STATIC_LIBRARIES := libzmq

include $(BUILD_EXECUTABLE)
