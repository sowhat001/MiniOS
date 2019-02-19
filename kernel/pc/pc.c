// #include <zjunix/syscall.h>
// #include <intr.h>
// #include <driver/ps2.h>
// #include <arch.h>
// #include <zjunix/utils.h>
// #include <zjunix/log.h>
// #include <zjunix/slab.h>
// #include <driver/vga.h>
// #include <page.h>
// # include "zjunix/pc.h"

// //找出这个8位数最低位的1在哪一位
// const u_byte where_lowest1_table_for_8[256] =
// {
// 	0u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x00 to 0x0F                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x10 to 0x1F                   */
// 	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x20 to 0x2F                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x30 to 0x3F                   */
// 	6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x40 to 0x4F                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x50 to 0x5F                   */
// 	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x60 to 0x6F                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x70 to 0x7F                   */
// 	7u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x80 to 0x8F                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x90 to 0x9F                   */
// 	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xA0 to 0xAF                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xB0 to 0xBF                   */
// 	6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xC0 to 0xCF                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xD0 to 0xDF                   */
// 	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xE0 to 0xEF                   */
// 	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u  /* 0xF0 to 0xFF                   */
// };

// //三位数为几，对应的那一位就置1
// const u_byte only_indexbit_is1_table_for_3[8] =
// {
// 	1, 2, 4, 8, 16, 32, 64, 128
// };

// list wait_list;
// pc_union *cur_pc = 0;
// pc_union *all_pcs[MAX_LEVEL];
// pc_union *shell = 0;
// int TIME_SLOT = 0;

// u_byte ready_table[MAX_LEVEL / 8];
// u_byte ready_group;

// //初始化进程模块
// //将进程调度所用的bitmap全部初始化为0
// //初始化时没有当前执行进程，shell也未创建
// //然后创建idle进程，并进入进程调度。
// void init_pc()
// {
// 	ready_group = 0;
// 	for (int i = 0; i < MAX_LEVEL / 8; i++)
// 	{
// 		ready_table[i] = 0;
// 	}
// 	INIT_LIST_HEAD(&wait_list);
// 	for (int i = 0; i < MAX_LEVEL; i++)
// 	{
// 		all_pcs[i] = 0;
// 	}
	
// 	cur_pc = 0;
// 	shell = 0;

// 	//将调度函数注册到7号中断（时间中断）与5号系统调用
// 	register_syscall(10, pc_schedule_sys);
// 	register_interrupt_handler(7, pc_schedule_intr);
// 	//kernel_printf("Success register syscall!\n");

// 	asm volatile(
// 	"li $v0, 500000\n\t"
// 	"mtc0 $v0, $11\n\t"
// 	"mtc0 $zero, $9");
// }

// //初始优先级为7，变为20之后，新的进程优先级确实不能是20
// //新的进程相应的id也不是20
// //原来如果有id为20的，他的优先级只要没改，是不能变过去的
// //外界调用这个函数创建新进程，创建好了之后触发进程调度
// int exec_from_kernel(u_int argc, void *args, int wait, u_byte new_id)
// {
// 	// kernel_printf("new_id: %d\n", new_id);
// 	// kernel_printf("entry_test: %x\n", entry_test);
// 	if (create_pc(args, new_id, entry_test, 0, 0))
// 	{
// 		kernel_printf("Create process failed!\n");
// 		return 1;
// 	}
// 	else
// 	{
// 		if (wait)
// 		{
// 			wait_for_newpc(new_id);
// 		}
// 		else
// 		{
// 			if(new_id != SHELL_ID)
// 			{
// 				// kernel_printf("here new_id: %d\n", new_id);
// 				asm volatile(
// 				"li $v0, 10\n\t"
// 				"syscall\n\t"
// 				"nop\n\t");
// 			}
// 		}
// 	}
// }

// //创建进程函数，传入进程名字，进程ID以及进程入口函数地址
// int create_pc(char *name, u_byte new_id, void(*entry)(u_int argc, void *args), u_int argc, void *args)
// {
// 	u_int init_gp;
// 	pc_union *new_pc_union;

// 	new_pc_union = kmalloc(sizeof(pc_union));
// 	//kernel_printf("pc_union_addr: %x\n", new_pc_union);
// 	if(new_pc_union)
// 	{
// 		new_pc_union->pc.state = P_INIT;
// 		kernel_strcpy(new_pc_union->pc.name, name);
// 		if ((!all_pcs[new_id] && !find_by_id(new_id)) || (new_id == SHELL_ID))
// 		{
// 			if (cal_prio(&(new_pc_union->pc), new_id))
// 			{//初始化id失败，传入的id不合法
// 				return 1;
// 			}
// 			else
// 			{
// 				new_pc_union->pc.id = new_id;
// 				if(cur_pc)
// 				{
// 					new_pc_union->pc.parent_id = cur_pc->pc.id;
// 				}
// 				else
// 				{
// 					new_pc_union->pc.parent_id = new_id;
// 				}
// 				kernel_memset(&(new_pc_union->pc.context), 0, sizeof(context));
// 				new_pc_union->pc.context.epc = (u_int)entry;
// 				//kernel_printf("new_pc_union_epc: %x\n", new_pc_union->pc.context.epc);
// 				new_pc_union->pc.context.sp = (u_int)new_pc_union + KERNEL_STACK_SIZE;
// 				asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
// 				new_pc_union->pc.context.gp = init_gp;
// 				new_pc_union->pc.context.a0 = argc;
// 				new_pc_union->pc.context.a1 = (u_int)args;
// 				//new_pc_union->pc.pc_files = 0;

// 				INIT_LIST_HEAD(&(new_pc_union->pc.node));
// 				if (new_id == SHELL_ID)
// 				{
// 					shell = new_pc_union;
// 					//kernel_printf("shell: %x\n", shell);
// 				}
// 				else
// 				{
// 					all_pcs[new_pc_union->pc.prio] = new_pc_union;
// 				}
// 				turn_to_ready(&(new_pc_union->pc));
// 				kernel_printf("Create %d process success!", new_pc_union->pc.id);
// 				//print_all_pcs();
// 				return 0;
// 			}
// 		}
// 		else
// 		{
// 			kernel_printf("This process exists!\n");
// 			return 1;
// 		}
// 	}
// 	else
// 	{
// 		kernel_printf("init new_pc_union error!\n");
// 	}
// }

// //强制通过id结束进程
// int kill_pc(u_byte kill_id)
// {
// 	pc_union *to_kill = 0;
// 	list *pos;
// 	pc_union *next;
// 	if(kill_id == SHELL_ID)
// 	{
// 		kernel_printf("Shell can not be killed!\n");
// 		return 1;
// 	}
// 	else
// 	{
// 		to_kill = find_by_id(kill_id);
// 		//kernel_printf("tokill: %x\n", to_kill);
// 		if (to_kill && cur_pc)
// 		{
// 			if (to_kill->pc.id == cur_pc->pc.id)
// 			{
// 				kernel_printf("Current process can not be killed!\n");
// 				return 1;
// 			}

// 			disable_interrupts();
// 			if (to_kill->pc.state < 0)
// 			{
// 				list_for_each(pos, &wait_list)
// 				{
// 					next = container_of(pos, pc_union, pc.node);
// 					if (next->pc.id == to_kill->pc.id)
// 					{
// 						list_del(&(next->pc.node));
// 						INIT_LIST_HEAD(&(next->pc.node));
// 						break;
// 					}
// 				}
// 			}

// 			turn_to_unready(&(to_kill->pc), P_END);
// 			// if (to_kill->pc.pc_files != 0)
// 			// {
// 			// 	pc_files_delete(&(to_kill->pc));
// 			// }
// 			// kernel_printf("kill_pc->prio: %d", to_kill->pc.prio);
// 			// all_pcs[to_kill->pc.prio] = 0;
// 			kfree(to_kill);
// 			enable_interrupts();
// 			asm volatile(
// 				"li $v0, 10\n\t"
// 				"syscall\n\t"
// 				"nop\n\t");
// 			return 0;
// 		}
// 		else
// 		{
// 			kernel_printf("ID not found!\n");
// 			return 1;
// 		}
// 	}
// }

// //删除进程打开的文件
// void pc_files_delete(pc* pc) 
// {
// 	// fs_close(pc->pc_files);
// 	// kfree(&(pc->pc_files));
// }

// //打印所有进程
// void print_all_pcs()
// {
// 	for(int i = 0; i < MAX_LEVEL;i++)
// 	{
// 		if(!(i % 8))
// 		{
// 			kernel_printf("\n");
// 		}
// 		if(all_pcs[i] == 0)
// 		{
// 			kernel_printf("null | ");
// 		}
// 		if(all_pcs[i] != 0)
// 		{
// 			kernel_printf("%d %d | ", all_pcs[i]->pc.state, all_pcs[i]->pc.id);
// 		}
// 	}
// 	kernel_printf("\n");
// }

// //打印等待链表
// void print_wait() 
// {
//     pc_union *next;
//     list *pos;
//     kernel_printf("wait list:\n");
// 	list_for_each(pos, &wait_list)
// 	{
// 		next = container_of(pos, pc_union, pc.node);
// 		if(next)
// 		{
// 			kernel_printf("%d ", next->pc.id);
// 		}
// 		else
// 		{
// 			kernel_printf("wait_null\n");
// 		}
// 	}
// 	kernel_printf("\n");
// }

// //测试函数
// void entry_test(unsigned int argc, void *args)
// {
// 	kernel_printf("\n============number %d process============\n", cur_pc->pc.id);
// 	int count = 40000000;
// 	int count_exit = 0;
// 	while(1)
// 	{
// 		if(count == 40000000)
// 		{
// 			kernel_printf("%d process is running!\n", cur_pc->pc.id);
// 			count = 0;
// 			count_exit++;
// 			if(cur_pc->pc.id == 1 && count_exit == 10)
// 			{
// 				break;
// 			}

// 			if(cur_pc->pc.id == 2 && count_exit == 1)
// 			{
// 				print_all_pcs();
// 				print_wait();
// 			}
// 			if(cur_pc->pc.id == 2 && count_exit == 3)
// 			{
// 				break;
// 			}

// 			if(cur_pc->pc.id == 6 && count_exit == 1)
// 			{
// 				print_all_pcs();
// 				print_wait();
// 			}
// 			if(cur_pc->pc.id == 6 && count_exit == 3)
// 			{
// 				break;
// 			}

// 			if(cur_pc->pc.id == 7 && count_exit == 3)
// 			{
// 				change_prio(cur_pc, 20);
// 			}
// 			if(cur_pc->pc.id == 7 && count_exit == 8)
// 			{
// 				exec_from_kernel(0, "wait process", 1, 2);
// 			}

// 			if(cur_pc->pc.id == 8 && count_exit == 6)
// 			{
// 				change_prio(cur_pc, 4);
// 			}
// 		}
// 		count++;
// 	}
// 	end_pc();
//     kernel_printf("Error: entry\n");
// }

// //等待这个进程，只要你所等待的进程还在运行，就需要等它结束
// void wait_for_newpc(u_byte id)
// {
// 	if (id < MAX_LEVEL)
// 	{
// 		pc_union *target = find_by_id(id);
// 		disable_interrupts();
// 		if (target && target->pc.state != P_END)
// 		{
// 			turn_to_unready(&(cur_pc->pc), -id);
// 			list_add_tail(&(cur_pc->pc.node), &wait_list);
// 			enable_interrupts();
// 			asm volatile(
// 			"li $v0, 10\n\t"
// 			"syscall\n\t"
// 			"nop\n\t");
// 		}
// 		else
// 		{
// 			kernel_printf("This process you wait has exited!\n");
// 			enable_interrupts();
// 		}
// 	}
// 	else
// 	{
// 		kernel_printf("Illegal process ID!\n");
// 		enable_interrupts();
// 	}
// }

// //进程自然结束
// void end_pc()
// {
// 	if (cur_pc->pc.id == SHELL_ID)
// 	{
// 		kernel_printf("Shell process can not be exited!\n");
// 		return;
// 	}
// 	turn_to_unready(&(cur_pc->pc), P_END);
// 	// if(cur_pc->pc.pc_files != 0)
// 	// {
// 	// 	pc_files_delete(&(cur_pc->pc));
// 	// }
// 	//kernel_printf("cur_pc_prio: %d\n", cur_pc->pc.prio);
// 	//all_pcs[cur_pc->pc.prio] = 0;
// 	kfree(cur_pc);
// 	asm volatile(
// 		"li $v0, 10\n\t"
// 		"syscall\n\t"
// 		"nop\n\t");
// }

// //时间中断触发的调度函数
// void pc_schedule_intr(unsigned int state, unsigned int cause, context *pt_context)
// {
// 	// 每5个时间片调用一次shell
// 	if (TIME_SLOT == 5) 
// 	{		
// 		TIME_SLOT = 0;
// 		if(shell->pc.state == P_READY)
// 		{
// 			disable_interrupts();
// 			//当前是后台进程，切换成shell
// 			if(cur_pc != shell && cur_pc)
// 			{
// 				copy_context(pt_context, &(cur_pc->pc.context));
// 				cur_pc = shell;
// 				copy_context(&(cur_pc->pc.context), pt_context);
// 			}
// 			//当前是初始化的时候，没有进程，切换为shell
// 			else if(!cur_pc)
// 			{
// 				cur_pc = shell;		
// 				copy_context(&(cur_pc->pc.context), pt_context);
// 			}
// 			// kernel_printf("cur_pc_name: %s\n", cur_pc->pc.name);
// 			enable_interrupts();
// 		}
// 	}
// 	//刚运行过shell，应当从shell切向后台进程，或者刚开机
// 	else if(TIME_SLOT == 0)
// 	{
// 		pc_union *next = find_next(0);
// 		TIME_SLOT++;
// 		if(next)
// 		{
// 			disable_interrupts();
// 			if(cur_pc)
// 			{
// 				copy_context(pt_context, &(cur_pc->pc.context));
// 			}
// 			cur_pc = next;	
// 			copy_context(&(cur_pc->pc.context), pt_context);
// 			enable_interrupts();
// 		}
// 	}
// 	//时间中断进来，但是时间片还不够
// 	else
// 	{
// 		TIME_SLOT++;
// 		if(TIME_SLOT > 5)
// 		{
// 			TIME_SLOT = 0;
// 		}
// 	}
// 	enable_interrupts();
// 	asm volatile("mtc0 $zero, $9\n\t");
// }

// //系统调用触发的调度函数
// void pc_schedule_sys(unsigned int state, unsigned int cause, context *pt_context)
// {
// 	//kernel_printf("sys_schedule\n");
// 	pc_union *next = find_next(1);
// 	// kernel_printf("next from sys: %x\n", next);
// 	if(next)
// 	{
// 		disable_interrupts();
// 		if(cur_pc)
// 		{
// 			copy_context(pt_context, &(cur_pc->pc.context));
// 		}
// 		cur_pc = next;	
// 		copy_context(&(cur_pc->pc.context), pt_context);
// 		enable_interrupts();
// 	}
// 	else
// 	{
// 		disable_interrupts();
// 		if(cur_pc)
// 		{
// 			copy_context(pt_context, &(cur_pc->pc.context));
// 		}
// 		cur_pc = shell;	
// 		copy_context(&(cur_pc->pc.context), pt_context);
// 		enable_interrupts();
// 	}
// }

// //计算优先级，计算low3, high3, 并判别优先级是否合法
// //第一个参数为需要计算优先级的pcb
// //第二个参数优先级
// int cal_prio(pc *target, u_byte prio)
// {
// 	if(target)
// 	{
// 		disable_interrupts();
// 		if(prio == SHELL_ID && !kernel_strcmp("shell", target->name))
// 		{//shell进程
// 			target->prio = prio;
// 			target->high3 = prio >> 3;
// 			target->low3 = prio & 0x07;
// 			enable_interrupts();
// 			return 0;
// 		}
// 		else
// 		{//normal进程
// 			if(prio < MAX_LEVEL && prio >= 0)
// 			{
// 				target->prio = prio;
// 				target->high3 = prio >> 3;
// 				target->low3 = prio & 0x07;
// 				enable_interrupts();
// 				return 0;
// 			}
// 			else
// 			{
// 				kernel_printf("Error priority!");
// 				enable_interrupts();
// 				return 1;
// 			}
// 		}
// 	}
// }

// //进程变为ready的状态，修改位图
// void turn_to_ready(pc* target)
// {
// 	if(target)
// 	{
// 		disable_interrupts();
// 		if(target->id != SHELL_ID)
// 		{
// 			ready_group |= only_indexbit_is1_table_for_3[target->high3];
// 			ready_table[target->high3] |= only_indexbit_is1_table_for_3[target->low3];
// 		}
// 		target->state = P_READY;
// 		enable_interrupts();
// 	}
// }

// //进程脱离ready的状态，修改位图
// void turn_to_unready(pc* target, int state)
// {
// 	list *pos;
// 	pc_union *next;
// 	disable_interrupts();
// 	ready_table[target->high3] &= ~only_indexbit_is1_table_for_3[target->low3];
// 	if(ready_table[target->high3] == 0)
// 	{
// 		ready_group &= ~only_indexbit_is1_table_for_3[target->high3];
// 	}
// 	target->state = state;
// 	//如果进程结束，不管如何结束，则要唤醒因它而被挂起的进程
// 	if(state == P_END)
// 	{
// 		list_for_each(pos, &wait_list)
// 		{
// 			next = container_of(pos, pc_union, pc.node);
// 			if (next->pc.state == -(target->id))
// 			{
// 				kernel_printf("here1\n");
// 				turn_to_ready(&(next->pc));
// 				disable_interrupts();
// 				kernel_printf("here2\n");
// 				list_del(&(next->pc.node));
// 				kernel_printf("here3\n");
// 				INIT_LIST_HEAD(&(next->pc.node));
// 				kernel_printf("here4\n");
// 				break;
// 			}
// 		}
// 		kernel_printf("here5\n");
// 		all_pcs[target->prio] = 0;
// 	}
// 	enable_interrupts();
// }

// //从位图中找到下一个优先级最高的进程
// pc_union *find_next(int debug)
// {

// 	int y = where_lowest1_table_for_8[ready_group];
// 	u_byte prio = (u_byte) ((y << 3) + where_lowest1_table_for_8[ready_table[y]]);
// 	if (debug)
// 	{
// 		kernel_printf("prio: %d\n", prio);
// 	}

// 	if(prio == 0 && all_pcs[prio] == 0)
// 	{
// 		return shell;
// 	}
// 	else
// 	{
// 		return all_pcs[prio];
// 	}
// }

// //通过id来寻找对应进程
// pc_union *find_by_id(u_byte id)
// {
// 	if(id == SHELL_ID)
// 	{
// 		return shell;
// 	}
// 	else
// 	{
// 		for(int i = 0; i<MAX_LEVEL; i++)
// 		{
// 			if(all_pcs[i] && all_pcs[i]->pc.id == id)
// 			{
// 				return all_pcs[i];
// 			}
// 		}
// 		return 0;
// 	}
// }

// //改变进程的优先级
// void change_prio(pc_union *target, int new_prio)
// {
// 	if(target)
// 	{
// 		disable_interrupts();
// 		int old_prio = target->pc.prio;
// 		ready_table[target->pc.high3] &= ~only_indexbit_is1_table_for_3[target->pc.low3];
// 		if(ready_table[target->pc.high3] == 0)
// 		{
// 			ready_group &= ~only_indexbit_is1_table_for_3[target->pc.high3];
// 		}

// 		if(new_prio >= 0 && new_prio < SHELL_ID && !all_pcs[new_prio])
// 		{	
// 			if(!cal_prio(&(target->pc), new_prio))
// 			{
// 				ready_group |= only_indexbit_is1_table_for_3[target->pc.high3];
// 				ready_table[target->pc.high3] |= only_indexbit_is1_table_for_3[target->pc.low3];
// 				all_pcs[old_prio] = 0;
// 				all_pcs[new_prio] = target;
// 				enable_interrupts();
// 				asm volatile(
// 				"li $v0, 10\n\t"
// 				"syscall\n\t"
// 				"nop\n\t");
// 			}
// 			else
// 			{
// 				enable_interrupts();
// 			}
// 		}
// 		else
// 		{
// 			kernel_printf("This priority has existed!\n");
// 		}
// 	}
// }

// //上下文切换，寄存器赋值
// static void copy_context(context* src, context* dest)
// {
// 	dest->epc = src->epc;
// 	dest->at = src->at;
// 	dest->v0 = src->v0;
// 	dest->v1 = src->v1;
// 	dest->a0 = src->a0;
// 	dest->a1 = src->a1;
// 	dest->a2 = src->a2;
// 	dest->a3 = src->a3;
// 	dest->t0 = src->t0;
// 	dest->t1 = src->t1;
// 	dest->t2 = src->t2;
// 	dest->t3 = src->t3;
// 	dest->t4 = src->t4;
// 	dest->t5 = src->t5;
// 	dest->t6 = src->t6;
// 	dest->t7 = src->t7;
// 	dest->s0 = src->s0;
// 	dest->s1 = src->s1;
// 	dest->s2 = src->s2;
// 	dest->s3 = src->s3;
// 	dest->s4 = src->s4;
// 	dest->s5 = src->s5;
// 	dest->s6 = src->s6;
// 	dest->s7 = src->s7;
// 	dest->t8 = src->t8;
// 	dest->t9 = src->t9;
// 	dest->hi = src->hi;
// 	dest->lo = src->lo;
// 	dest->gp = src->gp;
// 	dest->sp = src->sp;
// 	dest->fp = src->fp;
// 	dest->ra = src->ra;
// }

#include <zjunix/syscall.h>
#include <intr.h>
#include <driver/ps2.h>
#include <arch.h>
#include <zjunix/utils.h>
#include <zjunix/log.h>
#include <zjunix/slab.h>
#include <driver/vga.h>
#include <page.h>
# include "zjunix/pc.h"

//找出这个8位数最低位的1在哪一位
const u_byte where_lowest1_table_for_8[256] =
{
	0u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x00 to 0x0F                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x10 to 0x1F                   */
	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x20 to 0x2F                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x30 to 0x3F                   */
	6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x40 to 0x4F                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x50 to 0x5F                   */
	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x60 to 0x6F                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x70 to 0x7F                   */
	7u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x80 to 0x8F                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x90 to 0x9F                   */
	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xA0 to 0xAF                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xB0 to 0xBF                   */
	6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xC0 to 0xCF                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xD0 to 0xDF                   */
	5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xE0 to 0xEF                   */
	4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u  /* 0xF0 to 0xFF                   */
};

//三位数为几，对应的那一位就置1
const u_byte only_indexbit_is1_table_for_3[8] =
{
	1, 2, 4, 8, 16, 32, 64, 128
};

list wait_list;
pc_union *cur_pc = 0;
pc_union *all_pcs[MAX_LEVEL];
pc_union *shell = 0;
int TIME_SLOT = 0;

u_byte ready_table[MAX_LEVEL / 8];
u_byte ready_group;

//初始化进程模块
//将进程调度所用的bitmap全部初始化为0
//初始化时没有当前执行进程，shell也未创建
//然后创建idle进程，并进入进程调度。
void init_pc()
{
	ready_group = 0;
	for (int i = 0; i < MAX_LEVEL / 8; i++)
	{
		ready_table[i] = 0;
	}
	INIT_LIST_HEAD(&wait_list);
	for (int i = 0; i < MAX_LEVEL; i++)
	{
		all_pcs[i] = 0;
	}
	
	cur_pc = 0;
	shell = 0;

	//将调度函数注册到7号中断（时间中断）与5号系统调用
	register_syscall(10, pc_schedule_sys);
	register_interrupt_handler(7, pc_schedule_intr);
	//kernel_printf("Success register syscall!\n");

	asm volatile(
	"li $v0, 1000000\n\t"
	"mtc0 $v0, $11\n\t"
	"mtc0 $zero, $9");
}

//初始优先级为7，变为20之后，新的进程优先级确实不能是20
//新的进程相应的id也不是20
//原来如果有id为20的，他的优先级只要没改，是不能变过去的
//外界调用这个函数创建新进程，创建好了之后触发进程调度
int exec_from_kernel(u_int argc, void *args, int wait, u_byte new_id)
{
	// kernel_printf("new_id: %d\n", new_id);
	// kernel_printf("entry_test: %x\n", entry_test);
	if (create_pc(args, new_id, entry_test, 0, 0))
	{
		kernel_printf("Create process failed!\n");
		return 1;
	}
	else
	{
		if (wait)
		{
			wait_for_newpc(new_id);
		}
		else
		{
			if(new_id != SHELL_ID)
			{
				// kernel_printf("here new_id: %d\n", new_id);
				asm volatile(
				"li $v0, 10\n\t"
				"syscall\n\t"
				"nop\n\t");
			}
		}
	}
}

//创建进程函数，传入进程名字，进程ID以及进程入口函数地址
int create_pc(char *name, u_byte new_id, void(*entry)(u_int argc, void *args), u_int argc, void *args)
{
	u_int init_gp;
	pc_union *new_pc_union;

	new_pc_union = kmalloc(sizeof(pc_union));
	//kernel_printf("pc_union_addr: %x\n", new_pc_union);
	if(new_pc_union)
	{
		new_pc_union->pc.state = P_INIT;
		kernel_strcpy(new_pc_union->pc.name, name);
		if ((!all_pcs[new_id] && !find_by_id(new_id)) || (new_id == SHELL_ID))
		{
			if (cal_prio(&(new_pc_union->pc), new_id))
			{//初始化id失败，传入的id不合法
				return 1;
			}
			else
			{
				new_pc_union->pc.id = new_id;
				if(cur_pc)
				{
					new_pc_union->pc.parent_id = cur_pc->pc.id;
				}
				else
				{
					new_pc_union->pc.parent_id = new_id;
				}
				kernel_memset(&(new_pc_union->pc.context), 0, sizeof(context));
				new_pc_union->pc.context.epc = (u_int)entry;
				//kernel_printf("new_pc_union_epc: %x\n", new_pc_union->pc.context.epc);
				new_pc_union->pc.context.sp = (u_int)new_pc_union + KERNEL_STACK_SIZE;
				asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
				new_pc_union->pc.context.gp = init_gp;
				new_pc_union->pc.context.a0 = argc;
				new_pc_union->pc.context.a1 = (u_int)args;
				//new_pc_union->pc.pc_files = 0;
				disable_interrupts();

				INIT_LIST_HEAD(&(new_pc_union->pc.node));
				if (new_id == SHELL_ID)
				{
					shell = new_pc_union;
					//kernel_printf("shell: %x\n", shell);
				}
				else
				{
					all_pcs[new_pc_union->pc.prio] = new_pc_union;
				}
				turn_to_ready(&(new_pc_union->pc));
				kernel_printf("Create %d process success!", new_pc_union->pc.id);
				enable_interrupts();
				//print_all_pcs();
				return 0;
			}
		}
		else
		{
			kernel_printf("This process exists!\n");
			return 1;
		}
	}
	else
	{
		kernel_printf("init new_pc_union error!\n");
	}
}

//强制通过id结束进程
int kill_pc(u_byte kill_id)
{
	pc_union *to_kill = 0;
	list *pos;
	pc_union *next;
	disable_interrupts();
	if(kill_id == SHELL_ID)
	{
		kernel_printf("Shell can not be killed!\n");
		enable_interrupts();
		return 1;
	}
	else
	{
		to_kill = find_by_id(kill_id);
		//kernel_printf("tokill: %x\n", to_kill);
		if (to_kill && cur_pc)
		{
			if (to_kill->pc.id == cur_pc->pc.id)
			{
				kernel_printf("Current process can not be killed!\n");
				return 1;
			}

			//disable_interrupts();
			if (to_kill->pc.state < 0)
			{
				list_for_each(pos, &wait_list)
				{
					next = container_of(pos, pc_union, pc.node);
					if (next->pc.id == to_kill->pc.id)
					{
						list_del(&(next->pc.node));
						kernel_printf("");
						INIT_LIST_HEAD(&(next->pc.node));
						kernel_printf("");
						break;
					}
				}
			}

			turn_to_unready(&(to_kill->pc), P_END);
			// if (to_kill->pc.pc_files != 0)
			// {
			// 	pc_files_delete(&(to_kill->pc));
			// }
			// kernel_printf("kill_pc->prio: %d", to_kill->pc.prio);
			// all_pcs[to_kill->pc.prio] = 0;
			//kfree(to_kill);
			enable_interrupts();
			asm volatile(
				"li $v0, 10\n\t"
				"syscall\n\t"
				"nop\n\t");
			return 0;
		}
		else
		{
			kernel_printf("ID not found!\n");
			return 1;
		}
	}
}

//删除进程打开的文件
void pc_files_delete(pc* pc) 
{
	// fs_close(pc->pc_files);
	// kfree(&(pc->pc_files));
}

//打印所有进程
void print_all_pcs()
{
	disable_interrupts();
	if(shell)
		kernel_printf("shell: %d %d", shell->pc.state, shell->pc.id);
	for(int i = 0; i < MAX_LEVEL;i++)
	{
		if(!(i % 8))
		{
			kernel_printf("\n");
		}
		if(all_pcs[i] == 0)
		{
			kernel_printf("null | ");
		}
		if(all_pcs[i] != 0)
		{
			kernel_printf("%d %d | ", all_pcs[i]->pc.state, all_pcs[i]->pc.id);
		}
	}
	kernel_printf("\n");
	enable_interrupts();
}

//打印等待链表
void print_wait() 
{
	disable_interrupts();
    pc_union *next;
    list *pos;
    kernel_printf("wait list:\n");
	list_for_each(pos, &wait_list)
	{
		next = container_of(pos, pc_union, pc.node);
		if(next)
		{
			kernel_printf("%d ", next->pc.id);
		}
		else
		{
			kernel_printf("wait_null\n");
		}
	}
	kernel_printf("\n");
	enable_interrupts();
}

//测试函数
void entry_test(unsigned int argc, void *args)
{
	kernel_printf("\n============number %d process============\n", cur_pc->pc.id);
	int count = 40000000;
	int count_exit = 0;
	while(1)
	{
		if(count == 40000000)
		{
			kernel_printf("%d process is running!\n", cur_pc->pc.id);
			count = 0;
			count_exit++;
			if(cur_pc->pc.id == 1 && count_exit == 10)
			{
				break;
			}

			if(cur_pc->pc.id == 2 && count_exit == 1)
			{
				print_all_pcs();
				print_wait();
				//print_table();
			}
			if(cur_pc->pc.id == 2 && count_exit == 3)
			{
				break;
			}

			if(cur_pc->pc.id == 6 && count_exit == 1)
			{
				print_all_pcs();
				print_wait();
				//print_table();
			}
			if(cur_pc->pc.id == 6 && count_exit == 3)
			{
				break;
			}

			if(cur_pc->pc.id == 7 && count_exit == 3)
			{
				change_prio(cur_pc, 20);
			}
			if(cur_pc->pc.id == 7 && count_exit == 8)
			{
				exec_from_kernel(0, "wait process", 1, 2);
			}

			if(cur_pc->pc.id == 8 && count_exit == 6)
			{
				change_prio(cur_pc, 4);
			}
		}
		count++;
	}
	end_pc();
    kernel_printf("Error: entry\n");
}

//等待这个进程，只要你所等待的进程还在运行，就需要等它结束
void wait_for_newpc(u_byte id)
{
	if (id < MAX_LEVEL)
	{
		pc_union *target = find_by_id(id);
		if (target && target->pc.state != P_END)
		{
			disable_interrupts();
			turn_to_unready(&(cur_pc->pc), -id);
			list_add_tail(&(cur_pc->pc.node), &wait_list);
			enable_interrupts();
			asm volatile(
			"li $v0, 10\n\t"
			"syscall\n\t"
			"nop\n\t");
		}
		else
		{
			kernel_printf("This process you wait has exited!\n");
			return;
		}
	}
	else
	{
		kernel_printf("Illegal process ID!\n");
		return;
	}
}

//进程自然结束
void end_pc()
{
	if (cur_pc->pc.id == SHELL_ID)
	{
		kernel_printf("Shell process can not be exited!\n");
		return;
	}
	disable_interrupts();
	turn_to_unready(&(cur_pc->pc), P_END);
	// if(cur_pc->pc.pc_files != 0)
	// {
	// 	pc_files_delete(&(cur_pc->pc));
	// }
	//kernel_printf("cur_pc_prio: %d\n", cur_pc->pc.prio);
	//all_pcs[cur_pc->pc.prio] = 0;
	//kfree(cur_pc);
	enable_interrupts();
	asm volatile(
		"li $v0, 10\n\t"
		"syscall\n\t"
		"nop\n\t");
}

//时间中断触发的调度函数
void pc_schedule_intr(unsigned int state, unsigned int cause, context *pt_context)
{
	disable_interrupts();
	// 每5个时间片调用一次shell
	if (TIME_SLOT == 5) 
	{		
		TIME_SLOT = 0;
		if(shell->pc.state == P_READY)
		{
			//当前是后台进程，切换成shell
			if(cur_pc != shell && cur_pc)
			{
				copy_context(pt_context, &(cur_pc->pc.context));
				cur_pc = shell;
				copy_context(&(cur_pc->pc.context), pt_context);
			}
			//当前是初始化的时候，没有进程，切换为shell
			else if(!cur_pc)
			{
				cur_pc = shell;		
				copy_context(&(cur_pc->pc.context), pt_context);
			}
			// kernel_printf("cur_pc_name: %s\n", cur_pc->pc.name);
		}
	}
	//刚运行过shell，应当从shell切向后台进程，或者刚开机
	else if(TIME_SLOT == 0)
	{
		pc_union *next = find_next(0);
		TIME_SLOT++;
		if(next)
		{
			if(cur_pc)
			{
				copy_context(pt_context, &(cur_pc->pc.context));
			}
			cur_pc = next;	
			copy_context(&(cur_pc->pc.context), pt_context);
		}
	}
	//时间中断进来，但是时间片还不够
	else
	{
		TIME_SLOT++;
		if(TIME_SLOT > 5)
		{
			TIME_SLOT = 0;
		}
	}
	enable_interrupts();
	asm volatile("mtc0 $zero, $9\n\t");
}

//系统调用触发的调度函数
void pc_schedule_sys(unsigned int state, unsigned int cause, context *pt_context)
{
	disable_interrupts();
	//kernel_printf("sys_schedule\n");
	pc_union *next = find_next(1);
	// kernel_printf("next from sys: %x\n", next);
	if(next)
	{
		if(cur_pc)
		{
			copy_context(pt_context, &(cur_pc->pc.context));
		}
		cur_pc = next;	
		copy_context(&(cur_pc->pc.context), pt_context);
	}
	else
	{
		if(cur_pc)
		{
			copy_context(pt_context, &(cur_pc->pc.context));
		}
		cur_pc = shell;	
		copy_context(&(cur_pc->pc.context), pt_context);
	}
	enable_interrupts();
}

// void print_table()
// {
// 	disable_interrupts();
// 	for(int i = 0;i<8;i++)
// 	{
// 		kernel_printf("i: %x\n", ready_table[i]);
// 	}
// 	enable_interrupts();
// }

//计算优先级，计算low3, high3, 并判别优先级是否合法
//第一个参数为需要计算优先级的pcb
//第二个参数优先级
int cal_prio(pc *target, u_byte prio)
{
	disable_interrupts();
	if(prio == SHELL_ID && !kernel_strcmp("shell", target->name))
	{//shell进程
		target->prio = prio;
		target->high3 = prio >> 3;
		target->low3 = prio & 0x07;
		enable_interrupts();
		return 0;
	}
	else
	{//normal进程
		if(prio < MAX_LEVEL && prio >= 0)
		{
			target->prio = prio;
			target->high3 = prio >> 3;
			target->low3 = prio & 0x07;
			enable_interrupts();
			return 0;
		}
		else
		{
			kernel_printf("Error priority!");
			enable_interrupts();
			return 1;
		}
	}
}

//进程变为ready的状态，修改位图
void turn_to_ready(pc* target)
{
	disable_interrupts();
	if(target->id != SHELL_ID)
	{
		ready_group |= only_indexbit_is1_table_for_3[target->high3];
		ready_table[target->high3] |= only_indexbit_is1_table_for_3[target->low3];
	}
	target->state = P_READY;
	enable_interrupts();
	//print_table();
}

//进程脱离ready的状态，修改位图
void turn_to_unready(pc* target, int state)
{
	list *pos;
	pc_union *next;
	kernel_printf("");
	disable_interrupts();
	kernel_printf("");
	ready_table[target->high3] &= ~only_indexbit_is1_table_for_3[target->low3];
	kernel_printf("");
	if(ready_table[target->high3] == 0)
	{
		kernel_printf("");
		ready_group &= ~only_indexbit_is1_table_for_3[target->high3];
		kernel_printf("");
	}
	kernel_printf("");
	target->state = state;
	kernel_printf("");
	//如果进程结束，不管如何结束，则要唤醒因它而被挂起的进程
	if(state == P_END)
	{
		kernel_printf("");
		list_for_each(pos, &wait_list)
		{
			kernel_printf("");
			next = container_of(pos, pc_union, pc.node);
			kernel_printf("");
			if (next->pc.state == -(target->id))
			{
				kernel_printf("");
				turn_to_ready(&(next->pc));
				kernel_printf("");
				disable_interrupts();
				kernel_printf("");
				list_del(&(next->pc.node));
				kernel_printf("");
				INIT_LIST_HEAD(&(next->pc.node));
				kernel_printf("");
				break;
				kernel_printf("");
			}
		}
		kernel_printf("");
		all_pcs[target->prio] = 0;
	}
	kernel_printf("");
	enable_interrupts();
	//print_table();
}

//从位图中找到下一个优先级最高的进程
pc_union *find_next(int debug)
{
	disable_interrupts();
	int y = where_lowest1_table_for_8[ready_group];
	u_byte prio = (u_byte) ((y << 3) + where_lowest1_table_for_8[ready_table[y]]);
	if (debug)
	{
		kernel_printf("prio: %d\n", prio);
	}

	if(prio == 0 && all_pcs[prio] == 0)
	{
		enable_interrupts();
		return shell;
	}
	else
	{
		enable_interrupts();
		return all_pcs[prio];
	}
}

//通过id来寻找对应进程
pc_union *find_by_id(u_byte id)
{
	disable_interrupts();
	if(id == SHELL_ID)
	{
		enable_interrupts();
		return shell;
	}
	else
	{
		for(int i = 0; i<MAX_LEVEL; i++)
		{
			if(all_pcs[i] && all_pcs[i]->pc.id == id)
			{
				enable_interrupts();
				return all_pcs[i];
			}
		}
		enable_interrupts();
		return 0;
	}
}

//改变进程的优先级
void change_prio(pc_union *target, int new_prio)
{
	//
	int old_prio = target->pc.prio;
	disable_interrupts();
	ready_table[target->pc.high3] &= ~only_indexbit_is1_table_for_3[target->pc.low3];
	if(ready_table[target->pc.high3] == 0)
	{
		ready_group &= ~only_indexbit_is1_table_for_3[target->pc.high3];
	}

	if(new_prio >= 0 && new_prio < SHELL_ID && !all_pcs[new_prio])
	{	
		if(!cal_prio(&(target->pc), new_prio))
		{
			ready_group |= only_indexbit_is1_table_for_3[target->pc.high3];
			ready_table[target->pc.high3] |= only_indexbit_is1_table_for_3[target->pc.low3];
			all_pcs[old_prio] = 0;
			all_pcs[new_prio] = target;
			enable_interrupts();
			asm volatile(
			"li $v0, 10\n\t"
			"syscall\n\t"
			"nop\n\t");
		}
		else
		{
			enable_interrupts();
		}
	}
	else
	{
		kernel_printf("This priority has existed!\n");
		enable_interrupts();
	}
}

//上下文切换，寄存器赋值
static void copy_context(context* src, context* dest)
{
	dest->epc = src->epc;
	dest->at = src->at;
	dest->v0 = src->v0;
	dest->v1 = src->v1;
	dest->a0 = src->a0;
	dest->a1 = src->a1;
	dest->a2 = src->a2;
	dest->a3 = src->a3;
	dest->t0 = src->t0;
	dest->t1 = src->t1;
	dest->t2 = src->t2;
	dest->t3 = src->t3;
	dest->t4 = src->t4;
	dest->t5 = src->t5;
	dest->t6 = src->t6;
	dest->t7 = src->t7;
	dest->s0 = src->s0;
	dest->s1 = src->s1;
	dest->s2 = src->s2;
	dest->s3 = src->s3;
	dest->s4 = src->s4;
	dest->s5 = src->s5;
	dest->s6 = src->s6;
	dest->s7 = src->s7;
	dest->t8 = src->t8;
	dest->t9 = src->t9;
	dest->hi = src->hi;
	dest->lo = src->lo;
	dest->gp = src->gp;
	dest->sp = src->sp;
	dest->fp = src->fp;
	dest->ra = src->ra;
}