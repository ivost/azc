source /usr/local/oecore-x86_64/environment-setup-aarch64-oe-linux

export CROSS_COMPILE="aarch64-oe-linux-gnueabi"

# echo $CC

$CC src/azc.c src/certs.c -c -fPIC \
   -I include -I src -I deps/umock_c/inc

$CC azc.o certs.o -shared -o libazc.so \
   -L lib \
   -l iothub_client_mqtt_transport -l iothub_client \
   -l umqtt -l aziotsharedutil -l parson

$CC src/main.c src/hub.c src/upload.c \
  -o azc610 \
  -I include -I src -I deps/umock_c/inc  \
  -L . \
  -L lib \
  -Wl,-rpath-link=lib \
  -l azc \
  -l crypto -l ssl -l curl -l pthread -l m -l rt

file azc610
scp -P222 azc610 libazc.so $UTBR:/data/root

