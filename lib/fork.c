// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	panic("duppage not implemented");
	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	bool read_only = !(perm == (perm | PTE_W));
	int r;
	// Si la pagina es de solo lectura, la comparto con el hijo
	if (read_only) {
		// TODO: ver si estan bien los parametros
		if ((r = sys_page_map(dstenv, va, 0, va, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
	} else {
	// Si no, la copio
		if ((r = sys_page_alloc(dstenv, va, perm)) < 0) {
			panic("sys_page_alloc: %e", r);
		}
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0) {
			panic("sys_page_unmap: %e", r);
		}
	}
}

envid_t
fork_v0(void)
{
	envid_t envid;
	int r;

	envid = sys_exofork();

	if (envid < 0) {
		panic("sys_exofork: %e", envid);
	}
	// Es el proceso hijo
	if (envid == 0) {
		// Actualizo la variable thisenv ya que referencia al padre
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	// Es el proceso padre
	int va;
	for (va=0; va<UTOP; va+=PGSIZE){
		// Obtengo la direccion del page directory entry
		pde_t actual_pde = uvpd[PDX(va)];
		// Obtengo la direccion del page table entry
		pte_t actual_pte = uvpt[PTX(actual_pde)];
		// Si tiene el bit de presencia --> hay una pagina mapeada
		bool is_maped = actual_pte | PTE_P;
		// Si hay pagina mapeada, la comparto con el hijo
		if (is_maped) {
			dup_or_share(envid, (void *) va, actual_pte & PTE_SYSCALL);
		}
	}
	// Seteo el proceso hijo como ENV_RUNNABLE
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		panic("sys_env_set_status: %e", r);
	}
	return envid;	
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
	return fork_v0();
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
