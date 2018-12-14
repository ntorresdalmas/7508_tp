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
	// panic("pgfault not implemented");

	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// La direccion esta mapeada sii el bit FEC_PR esta en 1 (en err)
	bool maped_addr = (err & FEC_PR);
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

	// Alineo addr a PGSIZE
	void *addr_aligned = ROUNDDOWN(addr, PGSIZE);

	int r;
	// Reservo una nueva pagina temporal
	if ((r = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W)) < 0) {
		panic("sys_page_alloc: %e", r);
	}
	// Copio el contenido original a la nueva pagina temporal (addr --> PFTEMP)
	memmove(PFTEMP, addr_aligned, PGSIZE);

	// Mapeo la nueva pagina temporal a la original
	if ((r = sys_page_map(0, PFTEMP, 0, addr_aligned, PTE_P | PTE_U | PTE_W)) <
	    0) {
		panic("sys_page_map: %e", r);
	}
	// Elimino la nueva pagina temporal
	if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
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
	// panic("duppage not implemented");

	// Obtengo la va de la pagina pn
	uintptr_t va = (uintptr_t) pn * PGSIZE;

	// Obtengo el page table entry
	pte_t actual_pte = uvpt[pn];

	// Me quedo con los bits de permisos
	int father_perm = actual_pte & PTE_SYSCALL;

	// Inicialmente los permisos del hijo son heredados del padre
	int child_perm = father_perm;

	// Si el padre tiene activado PTE_W, se lo desactivo al hijo
	// y a su vez le activo el PTE_COW
	bool is_writeable = (father_perm & PTE_W);
	if (is_writeable) {
		child_perm &= ~PTE_W;
		child_perm |= PTE_COW;
	}

	// Si el padre tiene activado PTE_SHARE, se mantienen los permisos del
	// padre Lo de arriba (is_writeable) queda en desuso
	bool is_shareable = (father_perm & PTE_SHARE);
	if (is_shareable) {
		child_perm = father_perm;
	}

	// Mapeo en el hijo la pagina fisica en la misma va
	int r;
	if ((r = sys_page_map(0, (void *) va, envid, (void *) va, child_perm)) <
	    0) {
		panic("sys_page_map: %e", r);
	}
	// Si los permisos resultantes del hijo incluyen PTE_COW, se los paso al padre
	if (child_perm & PTE_COW) {
		if ((r = sys_page_map(0, (void *) va, 0, (void *) va, child_perm)) <
		    0) {
			panic("sys_page_map: %e", r);
		}
	}
	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;
	bool not_writeable = !(perm & PTE_W);
	// Si la pagina es de solo lectura, la comparto con el hijo
	if (not_writeable) {
		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
	} else {
		// Si no, la copio
		if ((r = sys_page_alloc(dstenv, va, PTE_P | PTE_U | PTE_W)) < 0) {
			panic("sys_page_alloc: %e", r);
		}
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, PTE_P | PTE_U | PTE_W)) <
		    0) {
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
	// Creo un nuevo proceso
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
	for (va = 0; va < UTOP; va += PGSIZE) {
		// Obtengo la direccion del page directory entry
		pde_t actual_pde = uvpd[PDX(va)];
		// Si tiene el bit de presencia --> hay una pagina mapeada
		is_maped = (actual_pde & PTE_P);

		if (is_maped) {
			// Obtengo la direccion del page table entry
			pte_t actual_pte = uvpt[PGNUM(va)];
			// Si tiene el bit de presencia --> hay una pagina mapeada
			is_maped = (actual_pte & PTE_P);
			// Si hay pagina mapeada, la comparto con el hijo
			if (is_maped) {
				dup_or_share(envid,
				             (void *) va,
				             actual_pte | PTE_SYSCALL);
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
	// panic("fork not implemented");

	// return fork_v0();

	int r;
	extern void _pgfault_upcall(void);

	// Instalo en el padre la funcion 'pgfault' como handler de page faults
	// Tambien reservo memoria para su UXSTACK
	set_pgfault_handler(pgfault);

	// Creo un nuevo proceso
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
	// Reservo memoria para el UXSTACK del hijo
	if ((r = sys_page_alloc(envid,
	                        (void *) (UXSTACKTOP - PGSIZE),
	                        PTE_U | PTE_P | PTE_W)) < 0) {
		panic("sys_page_alloc: %e", r);
	}
	// Instalo el handler de excepciones en el hijo
	sys_env_set_pgfault_upcall(envid, _pgfault_upcall);

	// Es el proceso padre
	bool is_maped;
	bool va_in_xstack;
	int va;

	for (va = 0; va < UTOP; va += PGSIZE) {
		// La region correspondiente a la pila de excepciones (UXSTACK) no se mapea
		va_in_xstack = (va >= UXSTACKTOP - PGSIZE) && (va < UXSTACKTOP);

		if (!va_in_xstack) {
			// Obtengo la direccion del page directory entry
			pde_t actual_pde = uvpd[PDX(va)];
			// Si tiene el bit de presencia --> hay una pagina mapeada
			is_maped = (actual_pde & PTE_P);

			if (is_maped) {
				// Obtengo la direccion del page table entry
				pte_t actual_pte = uvpt[PGNUM(va)];
				// Si tiene el bit de presencia --> hay una pagina mapeada
				is_maped = (actual_pte & PTE_P);
				// Si hay pagina mapeada, la comparto con el hijo
				if (is_maped) {
					duppage(envid, PGNUM(va));
				}
			}
		}
	}
	// Seteo el proceso hijo como ENV_RUNNABLE
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
