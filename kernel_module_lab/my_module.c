// 必备头函数
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/pid.h>

// 该模块的LICENSE
MODULE_LICENSE("GPL");
// 该模块的作者
MODULE_AUTHOR("yuhaowei");
// 该模块的说明
MODULE_DESCRIPTION("test module/n");

// 该模块需要传递的参数
static int func = -1;
module_param(func, int, 0644);

static int pid_num = -1;
module_param(pid_num, int, 0644);

// 定时器
struct timer_list my_timer_list;

// 定时器服务函数
void timmer_function(struct timer_list *t)
{
    struct task_struct *task;
    int count = 0;
    for_each_process(task)
    {
        if (task->mm == NULL)
            count++;
    }
    printk(KERN_INFO "The number of kernel process is %d", count);

    my_timer_list.expires = jiffies + HZ;
    mod_timer(&my_timer_list, my_timer_list.expires);
}

// 初始化入口
// 模块安装时执行
// 这里的__init 同样是宏定义，主要的目的在于
// 告诉内核，加载该模块之后，可以回收init.text的区间
static int __init my_module_init(void)
{
    // 输出信息，类似于printf()
    // printk适用于内核模块
    printk(KERN_ALERT " my module init!\n");
    switch (func)
    {
    case 1:
    {
        // ps
        struct task_struct *task;
        printk(KERN_INFO "PID\tSTATE\tCOMMAND");
        for_each_process(task)
        {
            printk(KERN_INFO "%d\t%ld\t%s", task->pid, task->state, task->comm);
        }
        break;
    }
    case 2:
    {
        // 定时任务
        timer_setup(&my_timer_list, timmer_function, 0);
        my_timer_list.expires = HZ + jiffies;
        add_timer(&my_timer_list);
        printk(KERN_INFO "my_timer init");
        break;
    }
    case 3:
    {
        // 查看指定进程的进程树
        struct pid *target_pid = NULL;
        target_pid = find_get_pid(pid_num);
        if (target_pid != NULL)
        {
            struct task_struct *target_task_struct = NULL;
            target_task_struct = pid_task(target_pid, 0);
            if (target_task_struct != NULL)
            {
                struct task_struct *target_parent_task_struct = target_task_struct->parent;
                if (target_parent_task_struct != NULL)
                {
                    printk(KERN_INFO "His father is : pid=%d, state=%ld, comm=%s", target_parent_task_struct->pid,
                           target_parent_task_struct->state, target_parent_task_struct->comm);
                }
                struct list_head *curr;
                list_for_each(curr, &target_task_struct->children)
                {
                    struct task_struct *target_children_task_struct = list_entry(curr, struct task_struct, sibling);
                    printk(KERN_INFO "His children is : pid=%d, state=%ld, comm=%s", target_children_task_struct->pid,
                           target_children_task_struct->state, target_children_task_struct->comm);
                }

                list_for_each(curr, &target_task_struct->sibling)
                {
                    struct task_struct *target_sibling_task_struct = list_entry(curr, struct task_struct, sibling);
                    printk(KERN_INFO "His sibling is : pid=%d, state=%ld, comm=%s", target_sibling_task_struct->pid,
                           target_sibling_task_struct->state, target_sibling_task_struct->comm);
                }
                struct task_struct *p = target_task_struct;
                printk(KERN_INFO "His thread is : pid=%d, state=%ld, comm=%s", p->pid, p->state, p->comm);
                while_each_thread(target_task_struct, p)
                {
                    printk(KERN_INFO "His thread is : pid=%d, state=%ld, comm=%s", p->pid, p->state, p->comm);
                }
            }
        }
    }
    default:
        break;
    }
    return 0;
}

// 模块卸载时执行
// 同上
static void __exit my_module_exit(void)
{
    switch (func)
    {
    case 1:

        break;
    case 2:
        del_timer_sync(&my_timer_list);
        printk(KERN_INFO "del my_timer");
        break;
    default:
        break;
    }
    printk(KERN_ALERT " my module has exitedi!\n");
}

// 模块初始化宏，用于加载该模块
module_init(my_module_init);
// 模块卸载宏，用于卸载该模块
module_exit(my_module_exit);