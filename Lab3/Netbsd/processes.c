#include <sys/module.h>
#include <sys/proc.h>

MODULE(MODULE_CLASS_MISC, processes, NULL);
static int processes_modcmd(modcmd_t cmd, void* arg) {
    printf("List of processes:\n");
    struct proc *it;
    PROCLIST_FOREACH(it, &allproc)
        printf("%s %d\n, it->p_comm, it->p_pid");
    return 0;
}
