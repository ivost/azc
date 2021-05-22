source /usr/local/oecore-x86_64/environment-setup-aarch64-oe-linux

export CROSS_COMPILE="aarch64-oe-linux-gnueabi"
rm -f *.o libazc.so
# echo $CC

$CC src/azc.c src/certs.c -c -fPIC \
   -I include -I src -I deps/umock_c/inc

$CC azc.o certs.o -shared -o libazc.so \
   -L lib \
   -l iothub_client \
   -l iothub_client_amqp_transport \
   -l iothub_client_amqp_ws_transport \
   -l iothub_client_mqtt_transport \
   -l aziotsharedutil \
   -l uamqp \
   -l umqtt \
   -l parson 

   # -l iothub_client_http_transport \
   # -l uhttp \

   # -l prov_auth_client \
   # -l hsm_security_client \

$CC src/main.c src/watch.c src/msgq.c src/Uploader.cpp \
  -o azc610 \
  -I include -I src -I deps/umock_c/inc  \
  -L . \
  -L lib \
  -Wl,-rpath-link=lib \
  -l azc \
  -l stdc++ \
  -l crypto -l ssl -l curl -l pthread \
  -l m -l rt -l uuid

# file azc610

scp -P222 azc610 libazc.so $UTBR:/data/root

