#include "Uploader.h"

extern "C"
char * echo (char *s) {
    Uploader u;
    std::string ss = u.echo(std::string(s));
    return (char *) ss.c_str();
}

std::string Uploader::echo(std::string s) {
    return s;
}