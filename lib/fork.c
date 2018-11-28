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
	//panic("pgfault not implemented");

	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// La direccion esta mapeada sii el bit FEC_PR esta en 1 (en addr)
	bool maped_addr = ((uint32_t) addr & FEC_PR);
	// El error ocurrio por una escritura sii el bit FEC_WR esta en 1 (en err)
	bool is_write = (err & FEC_WR);
	// La pagina mapeada esta marcada como copy-on-write
	pte_t actual_pte = uvpt[PGNUM(addr)];
	bool is_cow = (actual_pte & PTE_COW);

	// Tienen que cumplirse las 3 condiciones
	if ((!maped_addr) || (!is_write) || (!is_cow)) {
		panic("pgfault panic");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	int r;
	if ((r = sys_page_alloc(thisenv->env_id, addr, PTE_P|PTE_U|PTE_W)) < 0) {
			panic("sys_page_alloc: %e", r);
	}
	if ((r = sys_page_map(thisenv->env_id, addr, 0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0) {
		panic("sys_page_map: %e", r);
	}
	memmove(UTEMP, addr, PGSIZE);
	if ((r = sys_page_unmap(0, UTEMP)) < 0) {
		panic("sys_page_unmap: %e", r);
	}
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
	// LAB 4: Your code here.
	//panic("duppage not implemented");
	
	// Obtengo la va de la pagina pn
	uintptr_t va = (uintptr_t) pn * PGSIZE;

	// Compruebo permisos de la pagina pn
	pte_t actual_pte = uvpt[pn];	
	bool is_writeable = (actual_pte == (actual_pte | PTE_W));
	bool is_cow = (actual_pte == (actual_pte | PTE_COW));

	// Si es writeable o copy-on-write, el nuevo mapeo incluye PTE_COW
	int perm = (is_writeable || is_cow) ? (actual_pte | PTE_COW) : actual_pte;

	// Mapeo en el hijo la pagina fisica en la misma va
	int r;
	if ((r = sys_page_map(envid, (void *) va, 0, (void *) va, perm)) < 0) {
		panic("sys_page_map: %e", r);
	}
	// Si los permisos del hijo incluyen PTE_COW, hago lo propio con la pagina pn
	if (perm == (perm | PTE_COW)) {
		actual_pte |= PTE_COW;
	}

	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;
	bool not_writeable = !(perm == (perm | PTE_W));
	// Si la pagina es de solo lectura, la comparto con el hijo
	if (not_writeable) {
		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
	} else {
	// Si no, la copio
		if ((r = sys_page_alloc(dstenv, va, PTE_P|PTE_U|PTE_W)) < 0) {
			panic("sys_page_alloc: %e", r);
		}
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, PTE_P|PTE_U|PTE_W)) < 0) {
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
	// Aloco un nuevo proceso
	envid_t envid;
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
	bool is_maped;
	int va;
	for (va=0; va<UTOP; va+=PGSIZE) {
		// Obtengo la direccion del page directory entry
		pde_t actual_pde = uvpd[PDX(va)];
		// Si tiene el bit de presencia --> hay una pagina mapeada
		is_maped = (actual_pde == (actual_pde | PTE_P));

		if (is_maped) {
			// Obtengo la direccion del page table entry
			pte_t actual_pte = uvpt[PGNUM(va)];
			// Si tiene el bit de presencia --> hay una pagina mapeada
			is_maped = (actual_pte == (actual_pte | PTE_P));
			// Si hay pagina mapeada, la comparto con el hijo
			if (is_maped) {
				dup_or_share(envid, (void *) va, actual_pte | PTE_SYSCALL);
			}
		}
	}
	// Seteo el proceso hijo como ENV_RUNNABLE
	int r;
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

	// Aloco un nuevo proceso
	envid_t envid;
	envid = sys_exofork();

	if (envid < 0) {
		panic("sys_exofork: %e", envid);
	}
	// Es el proceso hijo
	if (envid == 0) {
		// Actualizo la variable thisenv ya que referencia al padre
		thisenv = &envs[ENVX(sys_getenvid())];

		// Instalo en el hijo el handler de excepciones
		// Tambien reservo memoria para su UXSTACK
		// TODO: ver cual es el parametro (handler) 
		set_pgfault_handler(pgfault);

		return 0;
	}
	// Es el proceso padre

	// Instalo en el padre la funcion 'pgfault' como handler de page faults
	// Tambien reservo memoria para su UXSTACK
	// TODO: ver si esto va aca o antes del sys_exofork
	set_pgfault_handler(pgfault);
	
	bool is_maped;
	bool va_in_xstack;
	int va;
	for (va=0; va<UTOP; va+=PGSIZE) {
		// La region correspondiente a la pila de excepciones (UXSTACK) no se mapea
		va_in_xstack = (va >= UXSTACKTOP - PGSIZE) && (va < UXSTACKTOP);

		if (!va_in_xstack) {
			// Obtengo la direccion del page directory entry
			pde_t actual_pde = uvpd[PDX(va)];
			// Si tiene el bit de presencia --> hay una pagina mapeada
			is_maped = (actual_pde == (actual_pde | PTE_P));

			if (is_maped) {
				// Obtengo la direccion del page table entry
				pte_t actual_pte = uvpt[PGNUM(va)];
				// Si tiene el bit de presencia --> hay una pagina mapeada
				is_maped = (actual_pte == (actual_pte | PTE_P));
				// Si hay pagina mapeada, la comparto con el hijo
				if (is_maped) {
					duppage(envid, PGNUM(va));
				}
			}
		}
	}
	// Seteo el proceso hijo como ENV_RUNNABLE
	int r;
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		panic("sys_env_set_status: %e", r);
	}
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
