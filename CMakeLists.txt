cmake_minimum_required(VERSION 3.10)
project(azc)

include_directories(. ./src ./include)
link_directories(./lib)

add_executable(azc
        src/azc.c
        src/msgq.c
        src/watch.c
        src/Uploader.cpp
        src/certs.c
        src/main.c)

target_link_libraries(azc
    iothub_client_mqtt_transport
    iothub_client
	parson
    umqtt
    aziotsharedutil
    crypto
    ssl
    curl
    pthread
	rt
    m
   )
