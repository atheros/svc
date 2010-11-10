cmake_minimum_required(VERSION 2.8)

PROJECT(SVC C)
IF("${AUDIO_API}" STREQUAL "")
	SET(AUDIO_API "portaudio")
ENDIF("${AUDIO_API}" STREQUAL "")

SET(ENV_CFLAGS $ENV{CFLAGS})
IF(NOT CFLAGS)
	SET(CFLAGS ${ENV_CFLAGS})
ENDIF(NOT CFLAGS)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c89 -Wall ${CFLAGS} -D_XOPEN_SOURCE=501L")

# tell where to look for additional cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

OPTION(BUILD_SHARED "Build libsvc as a shared library" OFF)
OPTION(BUILD_ENET_SVC "Build SVC ENet client/server" OFF)
OPTION(BUILD_TESTS "Build libsvc tests" OFF)
OPTION(BUILD_P2P "Build libsvc P2P simple client" ON)



FIND_PACKAGE(Celt REQUIRED)
FIND_PACKAGE(ENet REQUIRED)
FIND_PACKAGE(PortAudio REQUIRED)

FIND_PACKAGE(Jack REQUIRED)
#FIND_PACKAGE(OSS REQUIRED)
IF(NOT CMAKE_HOST_WIN32)
	# those scripts will fail on windows
	FIND_PACKAGE(ALSA REQUIRED)
ENDIF(NOT CMAKE_HOST_WIN32)

# correct pthread to use
IF(CMAKE_HOST_WIN32)
	find_library(PTHREAD_WIN32 pthreadGC2)
	IF(NOT PTHREAD_WIN32)
		MESSAGE(FATAL_ERROR "Win32 SVC needs pthread-win32")
	ENDIF(NOT PTHREAD_WIN32)
	
	SET(PTHREAD_LIB	"${PTHREAD_WIN32}")
ELSE(CMAKE_HOST_WIN32)
	SET(PTHREAD_LIB "pthread")
ENDIF(CMAKE_HOST_WIN32)


# check if ENet SVC client and server should be built
IF(BUILD_ENET_SVC)
	
	MESSAGE(STATUS "SVC ENet client/server enabled")
	IF(NOT ENET_LIBRARIES)
		MESSAGE(FATAL_ERROR "Can't build  SVC ENet client/server without ENet")
	ENDIF(NOT ENET_LIBRARIES)
	
ELSE(BUILD_ENET_SVC)
	
	MESSAGE(STATUS "SVC ENet client/server disabled.")
	
ENDIF(BUILD_ENET_SVC)



# Select AUDIO API
IF("${AUDIO_API}" STREQUAL "portaudio")
	MESSAGE(STATUS "Using PortAudio backend")
	
	IF(NOT PORTAUDIO_LIBRARIES)
		MESSAGE(FATAL_ERROR "Can't use PortAudio backend, libraries not found.")
	ENDIF(NOT PORTAUDIO_LIBRARIES)
	
	SET(AUDIOAPI_LIBRARIES ${PORTAUDIO_LIBRARIES})
ELSEIF("${AUDIO_API}" STREQUAL "jack")
	MESSAGE(STATUS "Using Jack backend")
	
	IF(NOT JACK_LIBRARIES)
		MESSAGE(FATAL_ERROR "Can't use Jack backed, libraries not found.")
	ENDIF(NOT JACK_LIBRARIES)
	
	SET(AUDIOAPI_LIBRARIES ${JACK_LIBRARIES})
ELSEIF("${AUDIO_API}" STREQUAL "oss")
	MESSAGE(STATUS "Using OSS backend")

	IF(OSS_INCLUDES)
		MESSAGE(STATUS "OSS_INCLUDES is '${OSS_INCLUDES}'")
	ELSE(OSS_INCLUDES)
		MESSAGE(STATUS "-DOSS_INCLUDES missing, using default include path")
	ENDIF(OSS_INCLUDES)
	
	SET(AUDIOAPI_LIBRARIES "")
ELSEIF("${AUDIO_API}" STREQUAL "alsa")
	MESSAGE(STATUS "Using ALSA backend")
	
	IF(NOT ALSA_LIBRARIES)
		MESSAGE(FATAL_ERROR "Can't use ALSA backed, libraries not found.")
	ENDIF(NOT ALSA_LIBRARIES)
	
	SET(AUDIOAPI_LIBRARIES ${ALSA_LIBRARIES})
ELSEIF("${AUDIO_API}" STREQUAL "dummy")
	MESSAGE(STATUS "Using dummy backend")
ELSE("${AUDIO_API}" STREQUAL "portaudio")
	
	MESSAGE(FATAL_ERROR "Unknown AUDIO_API [${AUDIO_API}]")
	
ENDIF("${AUDIO_API}" STREQUAL "portaudio")



INCLUDE_DIRECTORIES(
	src/common
	src/libsvc
#	${ENET_INCLUDE_DIR}
#	${CELT_INCLUDE_DIR}
#	${PORTAUDIO_INCLUDE_DIR}
	${OSS_INCLUDES}
)

# select build type
IF(BUILD_SHARED)
	MESSAGE(STATUS "Building shared libsvc")
	SET(LIBSVC_TYPE SHARED)
ELSE(BUILD_SHARED)
	MESSAGE(STATUS "Building static libsvc")
	SET(LIBSVC_TYPE STATIC)
ENDIF(BUILD_SHARED)

# build libsvc
ADD_LIBRARY(svc ${LIBSVC_TYPE}
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

# Build SVC P2P simple client
IF(BUILD_P2P)
	MESSAGE(STATUS "Simple P2P client enabled.")
	ADD_EXECUTABLE(svcp2p
		src/network/svcp2p/main.c
		src/common/thread.c
	)
	TARGET_LINK_LIBRARIES(svcp2p
		svc
		${AUDIOAPI_LIBRARIES}
		${CELT_LIBRARIES}
		${PTHREAD_LIB}
	)
ELSE(BUILD_P2P)
	MESSAGE(STATUS "Simple P2P client disabled.")
ENDIF(BUILD_P2P)


# Build SVC ENet client and server
IF(BUILD_ENET_SVC)
	ADD_EXECUTABLE(svceclient
		src/network/svcenet/client/main.c
	)
	TARGET_LINK_LIBRARIES(svceclient
		svc
		${ENET_LIBRARIES}
		${AUDIOAPI_LIBRARIES}
		${CELT_LIBRARIES}
		${PTHREAD_LIB}
	)

	ADD_EXECUTABLE(svceserver
		src/network/svcenet/server/main.c
		src/common/dstr.c
	)
	TARGET_LINK_LIBRARIES(svceserver
		${ENET_LIBRARIES}
		${PTHREAD_LIB}
	)
ENDIF(BUILD_ENET_SVC)


# Build tests
IF(BUILD_TESTS)
	
	MESSAGE(STATUS "Tests enabled.")
	
	ADD_EXECUTABLE(test_threads
		src/tests/test_threads.c
		src/common/thread.c
	)
	TARGET_LINK_LIBRARIES(test_threads
		${PTHREAD_LIB}
	)


	ADD_EXECUTABLE(test_audio_api
		src/tests/test_audio_api.c
		src/libsvc/audio.c
		src/libsvc/audio_api_${AUDIO_API}.c
		src/common/thread.c
	)
	TARGET_LINK_LIBRARIES(test_audio_api
		${AUDIOAPI_LIBRARIES}
		${CELT_LIBRARIES}
		${PTHREAD_LIB}
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
		${PTHREAD_LIB}
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
		${PTHREAD_LIB}
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
		${PTHREAD_LIB}
	)
	
	
	ADD_EXECUTABLE(test_dstr
		src/tests/test_dstr.c
		src/common/dstr.c
		src/tests/tests_utils.c
	)
	
	ADD_EXECUTABLE(test_dstrutils
		src/tests/test_dstrutils.c
		src/common/dstrutils.c
		src/common/dstr.c
		src/tests/tests_utils.c
	)
	
ELSE(BUILD_TESTS)
	
	MESSAGE(STATUS "Tests disabled.")
	
ENDIF(BUILD_TESTS)


