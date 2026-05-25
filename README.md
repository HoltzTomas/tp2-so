# TP2 - Sistemas Operativos

## Construcción del Núcleo de un Sistema Operativo

Kernel monolítico de 64 bits con:
- **Memory Management**: Free List y Buddy System (seleccionable en compilación)
- **Procesos**: Multitasking preemptivo con Round Robin y prioridades
- **Sincronización**: Semáforos con nombres, sin busy waiting
- **IPC**: Pipes unidireccionales bloqueantes
- **Shell**: Intérprete de comandos con soporte para pipes (`|`) y background (`&`)

## Compilación

### Requisitos
- `x86_64-linux-gnu-gcc` (cross-compiler)
- `nasm` (assembler)
- `qemu-system-x86_64` (emulador)
- `qemu-img` (herramienta de imágenes)

### Compilar con Free List Memory Manager (default)
```bash
make
```

### Compilar con Buddy System
```bash
make buddy
```

### Limpiar
```bash
make clean
```

### Ejecutar
```bash
./run.sh
```

## Comandos de la Shell

| Comando | Descripción |
|---------|-------------|
| `help` | Lista de comandos disponibles |
| `mem` | Estado de la memoria |
| `ps` | Lista de procesos |
| `loop` | Imprime PID periódicamente |
| `kill <pid>` | Mata un proceso |
| `nice <pid> <prio>` | Cambia prioridad |
| `block <pid>` | Bloquea/desbloquea proceso |
| `cat` | Imprime stdin |
| `wc` | Cuenta líneas del input |
| `filter` | Filtra vocales |
| `mvar <w> <r>` | Problema de lectores/escritores |
| `clear` | Limpia la pantalla |

### Tests
| Test | Descripción |
|------|-------------|
| `test_mm <maxmem>` | Test del memory manager |
| `test_proc <maxproc>` | Test de procesos |
| `test_prio <maxval>` | Test de prioridades |
| `test_sync <n> <sem>` | Test de sincronización (sem=1 usa semáforos) |

### Funcionalidades de la Shell
- **Background**: Agregar `&` al final del comando
- **Pipes**: Conectar dos procesos con `|` (ej: `cat | filter`)
- **Ctrl+C**: Matar proceso en foreground
- **Ctrl+D**: Enviar EOF

## Estructura del Proyecto

```
├── Kernel/
│   ├── asm/            # Interrupciones y funciones assembly
│   ├── drivers/        # Teclado y video
│   ├── include/        # Headers
│   ├── ipc/            # Pipes y semáforos
│   ├── memory/         # Memory managers (Free List + Buddy)
│   ├── process/        # Procesos y scheduler
│   └── syscalls/       # Dispatcher de syscalls
├── Userland/
│   └── SampleCodeModule/
│       ├── asm/        # Invocación de syscalls
│       ├── include/    # Headers
│       ├── tests/      # Tests provistos
│       └── programs/   # Programas de usuario
├── Bootloader/         # Pure64 bootloader
├── Image/              # Generación de imagen de disco
└── Toolchain/          # ModulePacker
```
