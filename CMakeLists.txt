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

OPTION(BUILD_SHARED	"Build libsvc as a shared library" OFF)
OPTION(BUILD_ENET_SVC	"Build SVC ENet client/server" OFF)
OPTION(BUILD_TESTS	"Build libsvc tests" OFF)
OPTION(BUILD_P2P	"Build libsvc P2P simple client" ON)

OPTION(BUILD_SVC	"Build all SVC applications" ON)
OPTION(BUILD_SVCD	"Build SVC daemon" ${BUILD_SVC})
OPTION(BUILD_SVCDC	"Build SVC daemon cli controller" ${BUILD_SVC})
OPTION(BUILD_SVCC	"Build SVC client" ${BUILD_SVC})
OPTION(BUILD_SVCCC	"Build SVC client cli controller" ${BUILD_SVC})


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
	jimtcl
# FIXME: this is only for SVC[D/DC/C/CC] targets!
#	src/network/svc/common
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



# Build SVC main daemon
IF(BUILD_SVCD)
	MESSAGE(STATUS "Main SVC daemon build enabled.")
	ADD_EXECUTABLE(svcd
		src/network/svc/server/main.c
		src/network/svc/server/peer.c
		src/network/svc/server/channels.c
		src/network/svc/server/protocol.c
		src/common/dstr.c
		src/common/dstrutils.c
	)
	
	TARGET_LINK_LIBRARIES(svcd
		${ENET_LIBRARIES}
	)
ELSE(BUILD_SVCD)
	MESSAGE(STATUS "Main SVC daemon build disabled.")
ENDIF(BUILD_SVCD)



# Build SVC main client daemon
IF(BUILD_SVCC)
	MESSAGE(STATUS "Main SVC client build enabled.")
	ADD_EXECUTABLE(svcc
		src/network/svc/client/main.c
		src/common/dstr.c
		src/common/dstrutils.c
	)
	
	TARGET_LINK_LIBRARIES(svcc
		svc
		${ENET_LIBRARIES}
	)
ELSE(BUILD_SVCC)
	MESSAGE(STATUS "Main SVC client build disabled.")
ENDIF(BUILD_SVCC)



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





ADD_CUSTOM_COMMAND(OUTPUT libjim.a COMMAND sh compile_jim.sh)
ADD_CUSTOM_TARGET(jim_lib DEPENDS libjim.a)
SET(JIM_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/libjim.a" "dl")

LINK_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Build SVC ENet client and server
IF(BUILD_ENET_SVC)

	
	MESSAGE(STATUS ${CELT_LIBRARIES})

	ADD_EXECUTABLE(svceclient
		src/network/svcenet/client/main.c
		src/common/dstr.c
		src/common/dstrutils.c
	)
	
	ADD_DEPENDENCIES(svceclient 
		jim_lib
	)
	
	TARGET_LINK_LIBRARIES(svceclient
		svc
		${JIM_LIBRARIES}
		${ENET_LIBRARIES}
		${AUDIOAPI_LIBRARIES}
		${CELT_LIBRARIES}
		${PTHREAD_LIB}
	)

	ADD_EXECUTABLE(svceserver
		src/network/svcenet/server/main.c
		src/common/dstr.c
		src/common/dstrutils.c
	)
	TARGET_LINK_LIBRARIES(svceserver
		${JIM_LIBRARIES}
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

SET_SOURCE_FILES_PROPERTIES(src/libsvc/audio_api_oss.c
	PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wextra")
