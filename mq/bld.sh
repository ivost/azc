source /usr/local/oecore-x86_64/environment-setup-aarch64-oe-linux
$CC mq_posix.c -o mq_posix -lrt
file mq_posix
scp -P222 mq_posix $UTBR:

