#include "ps.h"
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
#include <zjunix/slab.h>
#include <zjunix/time.h>
#include <zjunix/utils.h>
#include "../usr/ls.h"
#include "exec.h"
#include "myvi.h"

char ps_buffer[64];
int ps_buffer_index;
int *addrArr[10000];


unsigned int atoi(const char *src)
{
    unsigned int s = 0;
    if(*src < '0' || *src > '9') {
        s = 2147483647;
        return s;
    }
    while(*src == ' ') {
        src++; 
    }
    while(*src != '\0' && *src >= '0' && *src <= '9'){
        s = s * 10 + *src - '0';
        src++;
    }
    return s;
 }

unsigned int atox(const char *src)
{
    unsigned int s = 0;
    if(!((*src >= '0'&&*src <= '9')||(*src >= 'A'&&*src <= 'F')||(*src >= 'a'&&*src <= 'f'))) {
        s = 2147483647;
        return s;
    }
    while(*src == ' ') {
        src++; 
    }
    while(*src != '\0' && (*src >= '0'&&*src <= '9')||(*src >= 'A'&&*src <= 'F')||(*src >= 'a'&&*src <= 'f')){
        if(*src >= '0'&&*src <= '9'){
            s = s * 16 + *src - '0';
        }
        else if(*src >= 'A'&&*src <= 'F'){
            s = s * 16 + *src - 'A' + 10;
        }
        else if(*src >= 'a'&&*src <= 'f'){
            s = s * 16 + *src - 'a' + 10;
        }
        src++;
    }
    return s;
 }


void ps() {
    kernel_printf("Press any key to enter shell.\n");
    kernel_getchar();
    char c;
    ps_buffer_index = 0;
    ps_buffer[0] = 0;
    kernel_clear_screen(31);
    kernel_puts("PowerShell\n", 0xfff, 0);
    kernel_puts("PS>", 0xfff, 0);
    while (1) {
        c = kernel_getchar();
        if (c == '\n') {
            ps_buffer[ps_buffer_index] = 0;
            if (kernel_strcmp(ps_buffer, "exit") == 0) {
                ps_buffer_index = 0;
                ps_buffer[0] = 0;
                kernel_printf("\nPowerShell exit.\n");
            } else
                parse_cmd();
            ps_buffer_index = 0;
            kernel_puts("PS>", 0xfff, 0);
        } else if (c == 0x08) {
            if (ps_buffer_index) {
                ps_buffer_index--;
                kernel_putchar_at(' ', 0xfff, 0, cursor_row, cursor_col - 1);
                cursor_col--;
                kernel_set_cursor();
            }
        } else {
            if (ps_buffer_index < 63) {
                ps_buffer[ps_buffer_index++] = c;
                kernel_putchar(c, 0xfff, 0);
            }
        }
    }
}

void parse_cmd() {
    unsigned int result = 0;
    char dir[32];
    char c;
    kernel_putchar('\n', 0, 0);
    char sd_buffer[8192];
    int i = 0;
    char *param;
    for (i = 0; i < 63; i++) {
        if (ps_buffer[i] == ' ') {
            ps_buffer[i] = 0;
            break;
        }
    }
    if (i == 63)
        param = ps_buffer;
    else
        param = ps_buffer + i + 1;
    if (ps_buffer[0] == 0) {
        return;
    } else if (kernel_strcmp(ps_buffer, "clear") == 0) {
        kernel_clear_screen(31);
    } else if (kernel_strcmp(ps_buffer, "echo") == 0) {
        kernel_printf("%s\n", param);
    } else if (kernel_strcmp(ps_buffer, "gettime") == 0) {
        char buf[10];
        get_time(buf, sizeof(buf));
        kernel_printf("%s\n", buf);
    } else if (kernel_strcmp(ps_buffer, "sdwi") == 0) {
        for (i = 0; i < 512; i++)
            sd_buffer[i] = i;
        sd_write_block(sd_buffer, 7, 1);
        kernel_puts("sdwi\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdr") == 0) {
        sd_read_block(sd_buffer, 7, 1);
        for (i = 0; i < 512; i++) {
            kernel_printf("%d ", sd_buffer[i]);
        }
        kernel_putchar('\n', 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdwz") == 0) {
        for (i = 0; i < 512; i++) {
            sd_buffer[i] = 0;
        }
        sd_write_block(sd_buffer, 7, 1);
        kernel_puts("sdwz\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "mminfo") == 0) {
        bootmap_info("bootmm");
        buddy_info();
    } else if (kernel_strcmp(ps_buffer, "mmtest") == 0) {
        kernel_printf("kmalloc : %x, size = 1KB\n", kmalloc(1024));
    } 

     else if (kernel_strcmp(ps_buffer, "cat") == 0) {
        result = fs_cat(param);
        kernel_printf("cat return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "ls") == 0) {
        result = ls(param);
        kernel_printf("ls return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "vi") == 0) {
        result = myvi(param);
        kernel_printf("vi return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "exec") == 0) {
        result = exec(param);
        kernel_printf("exec return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "ps") == 0) 
    {
        disable_interrupts();
        print_all_pcs();
        enable_interrupts();
    } 
    else if (kernel_strcmp(ps_buffer, "kill") == 0) 
    {
        int id = param[0] - '0';
        kernel_printf("Killing process %d\n", id);
        result = kill_pc(id);
        kernel_printf("Kill return with %d\n", result);
        //print_all_pcs();
    } 
    else if (kernel_strcmp(ps_buffer, "execk8") == 0) 
    {
        kernel_printf("Create task with priority 8. Than increase it to 4.\n");
        result = exec_from_kernel(0, (void*)param, 0, 8);
        kernel_printf("execk return with %d\n", result);
        //print_all_pcs();
    }
    else if (kernel_strcmp(ps_buffer, "execk7") == 0) 
    {
        kernel_printf("Create task with priority 7. Than decrease it to 20. Then create a son process with priority 2.\n");
        result = exec_from_kernel(0, (void*)param, 0, 7);
        kernel_printf("execk return with %d\n", result);
        //print_all_pcs();
    } 
    else if (kernel_strcmp(ps_buffer, "execk6") == 0) 
    {
        kernel_printf("Create task with priority 6. It blocks shell.\n");
        result = exec_from_kernel(0, (void*)param, 1, 6);
        kernel_printf(ps_buffer, "execk return with %d\n",result);
        //print_all_pcs();
    }
    else if (kernel_strcmp(ps_buffer, "execk1") == 0) 
    {
        kernel_printf("Create task with priority 1. And end by itself.\n");
        result = exec_from_kernel(0, (void*)param, 0, 1);
        kernel_printf("execk return with %d\n", result);
        //print_all_pcs();
    }else if (kernel_strcmp(ps_buffer, "km") == 0) {
        unsigned int msize = atoi(param);
        if(msize != 2147483647){
            kernel_printf("kmalloc(%d)=%x\n", msize, kmalloc(msize));
        }
    } else if (kernel_strcmp(ps_buffer, "kf") == 0) {
        unsigned int msize = atox(param);
        if(msize != 2147483647){
            kfree((void*)msize);
            kernel_printf("kfree(%x)\n", msize);
        }
    } 
    else if (kernel_strcmp(ps_buffer, "mts") == 0) {
        unsigned int total = atoi(param);
        if(total != 2147483647){
            int i;
            
	        int size = 32;
            int before_allocate = free_size();
            for (i=0; i<total; i++){
                //kernel_printf("%d\n",i);
                addrArr[i] = kmalloc(size);
            } 
            kernel_clear_screen(31);
            kernel_printf("Before Allocate: %d\n",before_allocate);
            kernel_printf("After Allocate: %d\n", free_size());
            //for (i=total-1; i>=0; i--){
            for (i=0; i<total; i++){
                //kernel_printf("%d\n",i);
                kfree(addrArr[i]);
            } 
            kernel_printf("After Free: %d\n", free_size());
        }
    } 
    else if (kernel_strcmp(ps_buffer, "mtb") == 0) {
        unsigned int total = atoi(param);
        if(total != 2147483647){
            int i;            
	        int size = 4096;
            int before_allocate = free_size();
            for (i=0; i<total; i++){
                //kernel_printf("%d\n",i);
                addrArr[i] = kmalloc(size);
            } 
            kernel_clear_screen(31);
            kernel_printf("Before Allocate: %d\n",before_allocate);
            kernel_printf("After Allocate: %d\n", free_size());
            //for (i=total-1; i>=0; i--){
            for (i=0; i<total; i++){
                //kernel_printf("%d\n",i);
                kfree(addrArr[i]);
            } 
            kernel_printf("After Free: %d\n", free_size());
        }
    } 
     else 
     {
        kernel_puts(ps_buffer, 0xfff, 0);
        kernel_puts(": command not found\n", 0xfff, 0);
    }
}
