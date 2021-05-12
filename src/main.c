
#include "azc.h"
#include "azure_c_shared_utility/threadapi.h"

int main(int argc, char *argv[]) {
    azc_init();

    for (int i=0; i<10; i++) {
        azc_send();
        ThreadAPI_Sleep(1000);
    }

    azc_reset();
}
