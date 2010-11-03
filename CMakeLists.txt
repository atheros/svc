cmake_minimum_required(VERSION 2.8)

PROJECT(SVC C)
IF("${AUDIO_API}" STREQUAL "")
	SET(AUDIO_API "portaudio")
ENDIF("${AUDIO_API}" STREQUAL "")


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c89")

# tell where to look for additional cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

FIND_PACKAGE(Celt REQUIRED)
FIND_PACKAGE(ENet REQUIRED)


IF("${AUDIO_API}" STREQUAL "portaudio")
	MESSAGE(STATUS "Using PortAudio backend")
	FIND_PACKAGE(PortAudio REQUIRED)
	SET(AUDIOAPI_LIBRARIES ${PORTAUDIO_LIBRARIES})
ELSEIF("${AUDIO_API}" STREQUAL "jack")
	MESSAGE(STATUS "Using Jack backend")
	FIND_PACKAGE(Jack REQUIRED)
	SET(AUDIOAPI_LIBRARIES ${JACK_LIBRARIES})
ELSE("${AUDIO_API}" STREQUAL "portaudio")
	MESSAGE(FATAL_ERROR "Unknown AUDIO_API [${AUDIO_API}]")
ENDIF("${AUDIO_API}" STREQUAL "portaudio")



INCLUDE_DIRECTORIES(
	src/common
	src/libsvc
#	${ENET_INCLUDE_DIR}
#	${CELT_INCLUDE_DIR}
#	${PORTAUDIO_INCLUDE_DIR}
)

ADD_LIBRARY(svc SHARED
	src/libsvc/packet_queue.c
	src/libsvc/audio.c
	src/libsvc/audio_api_${AUDIO_API}.c
	src/libsvc/packet_cage.c
	src/common/thread.c
	src/libsvc/libsvc.c
	src/libsvc/libsvc_incoming.c
	src/libsvc/codec.c
	src/libsvc/network_packet.c
)


ADD_EXECUTABLE(svcclient
	src/network/svcd/client/main.c
)
TARGET_LINK_LIBRARIES(svcclient
	svc
	${ENET_LIBRARIES}
	${AUDIOAPI_LIBRARIES}
	${CELT_LIBRARIES}
)


ADD_EXECUTABLE(svcserver
	src/network/svcd/server/main.c
	src/common/dstr.c
)
TARGET_LINK_LIBRARIES(svcserver
	${ENET_LIBRARIES}
)


ADD_EXECUTABLE(test_threads
	src/tests/test_threads.c
	src/common/thread.c
)
TARGET_LINK_LIBRARIES(test_threads
	pthread
)


ADD_EXECUTABLE(test_audio_api
	src/tests/test_audio_api.c
	src/libsvc/audio_api_${AUDIO_API}.c
)
TARGET_LINK_LIBRARIES(test_audio_api
	${AUDIOAPI_LIBRARIES}
	${CELT_LIBRARIES}
)


ADD_EXECUTABLE(test_audio_packet_cage
	src/tests/test_audio_packet_cage.c
	src/libsvc/audio.c
	src/libsvc/audio_api_${AUDIO_API}.c
	src/libsvc/packet_cage.c
	src/common/thread.c
)
TARGET_LINK_LIBRARIES(test_audio_packet_cage
	${AUDIOAPI_LIBRARIES}
	${CELT_LIBRARIES}
	pthread
)


ADD_EXECUTABLE(test_audio_cage_queue
	src/tests/test_audio_cage_queue.c
	src/libsvc/packet_queue.c
	src/libsvc/audio.c
	src/libsvc/audio_api_${AUDIO_API}.c
	src/libsvc/packet_cage.c
	src/common/thread.c
)
TARGET_LINK_LIBRARIES(test_audio_cage_queue
	${AUDIOAPI_LIBRARIES}
	${CELT_LIBRARIES}
	pthread
)



ADD_EXECUTABLE(test_libsvc
	src/tests/test_libsvc.c
	src/libsvc/packet_queue.c
	src/libsvc/audio.c
	src/libsvc/audio_api_${AUDIO_API}.c
	src/libsvc/packet_cage.c
	src/common/thread.c
	src/libsvc/libsvc.c
	src/libsvc/libsvc_incoming.c
	src/libsvc/codec.c
	src/libsvc/network_packet.c
)
TARGET_LINK_LIBRARIES(test_libsvc
	${AUDIOAPI_LIBRARIES}
	${CELT_LIBRARIES}
	pthread
)



