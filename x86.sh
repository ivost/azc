
rm -f *.o libazc.so azcX86

gcc src/azc.c src/certs.c -c -fPIC \
   -I include -I src -I deps/umock_c/inc

gcc azc.o certs.o -shared -o libazc.so \
   -l iothub_client \
   -l iothub_client_amqp_transport \
   -l iothub_client_amqp_ws_transport \
   -l iothub_client_http_transport \
   -l iothub_client_mqtt_transport \
   -l aziotsharedutil \
   -l prov_auth_client \
   -l hsm_security_client \
   -l uamqp \
   -l uhttp \
   -l umqtt \
   -l parson \
   -l uuid

#   -l hogweed -l gnutls \

gcc src/main.c src/watch.c src/msgq.c src/azblob.c src/Uploader.cpp \
 -o azcX86 \
 -I include -I src -I deps/umock_c/inc  \
 -L . \
 -l azc \
 -l crypto -l ssl -l curl -l pthread -l m -l rt -l stdc++

 file azcX86

 LD_LIBRARY_PATH=. ./azcX86
 