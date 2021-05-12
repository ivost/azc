cmake_minimum_required(VERSION 3.10)
project(azc)

include_directories(. ./src ./include)
link_directories(./lib)

add_executable(azc 
	src/azc.c
	src/certs.c
		src/main.c)

target_link_libraries(azc
    iothub_client_mqtt_transport
    iothub_client
    umqtt
    aziotsharedutil
    parson
    crypto
    ssl
    curl
    pthread
    m
   )
