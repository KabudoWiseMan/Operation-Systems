#include <sys/module.h>
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <uvm/uvm.h>

MODULE(MODULE_CLASS_MISC, virtual, NULL);

#define PAGESIZE 0x1000 //4k
extern paddr_t avail_end;
vaddr_t va;
struct pglist plist;

static int virtual_modcmd(modcmd_t cmd, void* arg) {
    va = uvm_km_alloc(kernel_map, 10 * PAGESIZE, 0, UVM_KMF_VAONLY);
    if(va == 0) {
        printf("Can't allocate virtual memory\n");
        return 1;
    }
    
    int error = uvm_pglistalloc(5 * PAGESIZE, 0, avail_end, 0, 0, &plist, 5, 0);
    if(error) {
        printf("Can't create pages, err №%d\n", error);
    } else {
        printf("Success\n");
    }
    
    struct vm_page *page = TAILQ_FIRST(&plist);
    for(int i = 0; page; i++) {
        pd_entry_t *ppte;
        ppte = L2_BASE + pl2_i(va + PAGESIZE * i);
        paddr_t pa = VM_PAGE_TO_PHYS(page);
        
        printf("Page №%d\n", i + 1);
        printf("Phys addr page = 0x%lx\n", pa);
        printf("Valid:  %d\n", ((*ppte & PG_V) ? 1 : 0));
        printf("WriteThrough:  %d\n", ((*ppte & PG_WT) ? 1 : 0));
        printf("NonCachable:  %d\n", ((*ppte & PG_N) ? 1 : 0));
        printf("Used:  %d\n", ((*ppte & PG_U) ? 1 : 0));
        printf("Modified:  %d\n", ((*ppte & PG_M) ? 1 : 0));
        printf("\n");
        
        page = TAILQ_NEXT(page, pageq.queue);
    }
    
    uvm_pglistfree(&plist);
    uvm_km_free(kernel_map, va, 10 * PAGESIZE, UVM_KMF_VAONLY);
    
    return 0;
}
