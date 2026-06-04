# TP2 - Sistemas Operativos

Kernel x86-64 bare-metal con multitasking preemptivo, memory management, sincronización y IPC.

## Requisitos

- Docker (imagen `agodio/itba-so-multiarch:3.1`)
- QEMU (para ejecución)

## Compilación

```bash
# Memory Manager: Free-list (K&R)
make all

# Memory Manager: Buddy System
make buddy

# Limpiar
make clean
```

La compilación se hace dentro del container Docker:

```bash
docker run --rm -v $(pwd):/root -w /root agodio/itba-so-multiarch:3.1 make all
```

## Ejecución

```bash
# Con QEMU
./run.sh

# Manualmente
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512
```

## Estructura del Proyecto

```
├── Bootloader/          # Pure64 + BMFS (no se modifica)
├── Kernel/
│   ├── asm/
│   │   ├── interrupts.asm  # IDT, handlers, context switch, _initialize_stack_frame
│   │   └── libasm.asm      # cpuVendor, utilidades ASM
│   ├── drivers/
│   │   ├── keyboard.c      # Driver de teclado con buffer circular
│   │   └── video.c         # Driver de video VGA 80x25
│   ├── memory/
│   │   ├── memoryManager.c       # Free-list (K&R first-fit)
│   │   └── buddyMemoryManager.c  # Buddy system
│   ├── process/
│   │   ├── process.c       # PCB, creación/destrucción de procesos
│   │   └── scheduler.c     # Round Robin con prioridades
│   ├── ipc/
│   │   ├── semaphore.c     # Semáforos nombrados sin busy-wait
│   │   └── pipe.c          # Pipes unidireccionales bloqueantes
│   ├── kernel.c            # Entry point, inicialización, exception handler
│   ├── syscallDispatcher.c # Dispatcher de syscalls (INT 0x80)
│   └── include/            # Headers del kernel
├── Userland/
│   └── SampleCodeModule/
│       ├── shell.c         # Shell con soporte pipe (|) y background (&)
│       ├── libc.c          # printf, gets, memset, strcmp, etc.
│       ├── tests/
│       │   ├── test_mm.c   # Test de memory manager
│       │   ├── test_proc.c # Test de procesos
│       │   ├── test_sync.c # Test de sincronización (semáforos)
│       │   ├── test_prio.c # Test de prioridades
│       │   └── test_util.c # Utilidades para tests
│       ├── asm/
│       │   └── syscalls.asm # Wrappers INT 0x80
│       └── include/         # Headers de userland
├── Toolchain/              # ModulePacker
├── Image/                  # Imagen de disco generada
└── Makefile                # Makefile raíz
```

## Arquitectura

### Memory Management

Dos implementaciones intercambiables en tiempo de compilación:

- **Free-list (K&R)**: Lista enlazada de bloques libres con first-fit y coalescing. Se selecciona con `make all`.
- **Buddy System**: Árbol binario de potencias de 2. Se selecciona con `make buddy`.

Heap base: `0x600000`, tamaño: 64 MB.

### Procesos

- **PCB** con: PID, nombre, estado (READY/RUNNING/BLOCKED/ZOMBIE), prioridad, RSP, stack, FDs
- **Context switch** en IRQ0: `push_state → schedule(rsp) → mov rsp, rax → pop_state → iretq`
- **`_initialize_stack_frame`**: arma el stack de un proceso nuevo con frame de iretq + registros
- **Idle process** (PID 0): `sti + hlt` loop, siempre disponible

### Scheduler

Round Robin con prioridades. Quantum = prioridad + 1 ticks del timer.

Prioridades de 0 (lowest) a 4 (highest). Se puede cambiar con `nice`.

### Semáforos

- Nombrados: `sem_open(name, value)`, `sem_wait(name)`, `sem_post(name)`, `sem_close(name)`
- Sin busy-wait: `sem_wait` bloquea el proceso si value ≤ 0
- Cola FIFO: los procesos se despiertan en orden de llegada
- Protección con spinlock (`__sync_lock_test_and_set`) solo para la estructura interna
- Reference counting: se destruye cuando refCount llega a 0

### Pipes

- Buffer circular de 1024 bytes
- Blocking I/O: read bloquea si buffer vacío y write-end abierto; write bloquea si buffer lleno
- EOF: cerrar write-end causa que read retorne 0
- FDs base 100 para distinguir de STDIN(0)/STDOUT(1)

### Syscalls (INT 0x80)

| # | Syscall | Descripción |
|---|---------|-------------|
| 0 | sys_read | Leer de FD (stdin o pipe) |
| 1 | sys_write | Escribir a FD (stdout o pipe) |
| 2 | sys_malloc | Allocar memoria en heap |
| 3 | sys_free | Liberar memoria |
| 4 | sys_create_process | Crear proceso |
| 5 | sys_exit | Terminar proceso actual |
| 6 | sys_getpid | Obtener PID actual |
| 7 | sys_kill | Matar proceso |
| 8 | sys_block | Bloquear proceso |
| 9 | sys_unblock | Desbloquear proceso |
| 10 | sys_yield | Ceder CPU |
| 11 | sys_wait | Esperar que un proceso termine |
| 12 | sys_nice | Cambiar prioridad |
| 13 | sys_sem_open | Abrir/crear semáforo |
| 14 | sys_sem_wait | Wait en semáforo |
| 15 | sys_sem_post | Post en semáforo |
| 16 | sys_sem_close | Cerrar semáforo |
| 17 | sys_pipe_create | Crear pipe |
| 18 | sys_pipe_close | Cerrar extremo de pipe |
| 19 | sys_list_processes | Listar procesos |
| 20 | sys_mem_info | Info de memoria |
| 21 | sys_sleep | Sleep en ms |
| 22 | sys_get_ticks | Obtener ticks del timer |

### Exception Handlers

Maneja las excepciones críticas del CPU:
- **#DE (0)**: Division by zero
- **#UD (6)**: Invalid Opcode
- **#GP (13)**: General Protection Fault
- **#PF (14)**: Page Fault

Cuando ocurre una excepción en un proceso de usuario (PID > 1), se mata el proceso y se fuerza un context switch. Si ocurre en el kernel/shell, se detiene el sistema.

## Comandos de la Shell

### Builtins (no crean proceso)

| Comando | Descripción |
|---------|-------------|
| `help` | Lista de comandos |
| `clear` | Limpiar pantalla |
| `mem` | Info de memoria (total/used/free) |
| `ps` | Lista de procesos |
| `kill <pid>` | Matar proceso |
| `nice <pid> <prio>` | Cambiar prioridad (0-4) |
| `block <pid>` | Bloquear/desbloquear proceso |

### Aplicaciones (crean proceso)

| Comando | Descripción |
|---------|-------------|
| `loop` | Imprime PID en loop infinito |
| `cat` | Echo de stdin a stdout |
| `wc` | Cuenta líneas/palabras/caracteres |
| `filter` | Elimina vocales minúsculas |
| `test_mm <size>` | Test de memory manager |
| `test_proc` | Test de procesos |
| `test_sync` | Test de sincronización |
| `test_prio` | Test de prioridades |

### Operadores

- `cmd &` — Ejecutar en background
- `cmd1 | cmd2` — Pipe entre procesos
- `Ctrl+D` — EOF (terminar input)

## Tests

```bash
# En la shell del SO:
test_mm 1048576    # Testea MM con 1MB máximo
test_proc          # Crea/mata/bloquea procesos
test_sync          # Verifica sincronización con semáforos (resultado debe ser 0)
test_prio          # Muestra que el scheduler respeta prioridades
```

## Autores

Grupo - ITBA - Sistemas Operativos (72.11)
