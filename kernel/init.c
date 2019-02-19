#include <arch.h>
#include <driver/ps2.h>
#include <driver/vga.h>
#include <exc.h>
#include <intr.h>
#include <page.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
#include <zjunix/log.h>
#include <zjunix/pc.h>
#include <zjunix/slab.h>
#include <zjunix/syscall.h>
#include <zjunix/time.h>
#include "../usr/ps.h"

void machine_info() 
{
    int row;
    int col;
    kernel_printf("\n%s\n", "Windows X");
    row = cursor_row;
    col = cursor_col;
    cursor_row = 29;
    kernel_printf("%s", "Created by.");
    cursor_row = row;
    cursor_col = col;
    kernel_set_cursor();
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void create_startup_process() 
{
	if (create_pc("shell", SHELL_ID, ps, 0, 0))
	{
		kernel_printf("Create startup process failed!\n");
	}
	else
	{
		kernel_printf("Shell create success!\n");
	}
}
#pragma GCC pop_options

void init_kernel() 
{
    kernel_clear_screen(31);
    // Exception
    init_exception();
    // Page table
    init_pgtable();
    // Drivers
    init_vga();
    init_ps2();
    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
    log(LOG_END, "Memory Modules.");
    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");
    // System call
    log(LOG_START, "System Calls.");
    init_syscall();
    log(LOG_END, "System Calls.");
    // Process control
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    log(LOG_START, "Process Control Module.");
    init_pc();
    create_startup_process();
    log(LOG_END, "Process Control Module.");
    // Init finished
    machine_info();
    *GPIO_SEG = 0x11223344;
    // disable_interrupts();
    // print_all_pcs();
    // enable_interrupts();
    // Enter shell
    while (1)
        ;
}