
rm *.o
rm libazc.so
rm azcX86

gcc src/azc.c src/certs.c -c -fPIC \
   -I include -I src -I deps/umock_c/inc

gcc azc.o certs.o -shared -o libazc.so \
   -l iothub_client_mqtt_transport -l iothub_client \
   -l umqtt -l aziotsharedutil -l parson 

gcc src/main.c \
 -o azcX86 \
 -I include -I src -I deps/umock_c/inc  \
 -L . \
 -l azc \
 -l crypto -l ssl -l curl -l pthread -l m

 file azcX86

 LD_LIBRARY_PATH=. ./azcX86
 