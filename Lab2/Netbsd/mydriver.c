#include <sys/module.h>

MODULE(MODULE_CLASS_MISC, mydrive, NULL);
static int mydrive_modcmd(modcmd_t cmd, void* arg) {
    printf("VSEVOLOD");
    return 0;
}
