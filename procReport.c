//Patrick Stevens

#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

struct PROC {
	int parent_pid;
	char *parent_name;
	int child_num;
	int child_pid;
	char *child_name;
} procList[100];
//For printing
int unrunnable;
int runnable;
int stopped;
int procNum;

void proc_data(void) {
	struct task_struct *task;
	for_each_process(task) {
    	struct task_struct *child;
   	    struct list_head *list;
    	int firstChild = 1;
    	long report = (long)task->state;
    	if(report == 0) {
        	runnable++;
    	}else if(report == -1) {
            unrunnable++;
    	}else {
        	stopped++;
    	}
    	procList[procNum].parent_pid = (int)task->pid;
    	procList[procNum].parent_name = (char*)task->comm;
    	list_for_each(list, &task->children) {
        	child = list_entry(list, struct task_struct, sibling);
        	if (firstChild == 1) {
                procList[procNum].child_name = (char*)child->comm;
            	procList[procNum].child_pid = (int)child->pid;
            	firstChild = 0;
        	}
        	procList[procNum].child_num++;
    	}
    	procNum++;
	}
}

int proc_reporter(struct seq_file *m, void *v) {
	seq_printf(m, "PROCESS REPORTER:\n");
	seq_printf(m, "Unrunnable:%d\n", unrunnable);
	seq_printf(m, "Runnable:%d\n", runnable);
	seq_printf(m, "Stopped:%d\n", stopped);
	int index;
	for (index =0; index < procNum; index++) {
    	if(procList[index].child_num > 0) {
        	seq_printf(m, "Process ID=%d Name=%s number_of_children=%d first_child_pid=%d first_child_name=%s\n", procList[index].parent_pid, procList[index].parent_name, procList[index].child_num, procList[index].child_pid, procList[index].child_name);
    	}else {
        	seq_printf(m, "Process ID=%d Name=%s *No Children\n", procList[index].parent_pid, procList[index].parent_name);
    	}
	}
	return 0;
}

int proc_open(struct inode *inode, struct file *file) {
	return single_open(file, proc_reporter, NULL);
}

static const struct file_operations proc_report_fops = {
    .owner = THIS_MODULE,
    .open = proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

int proc_init(void) {
    printk(KERN_INFO "procReport: Kernel module initialized\n");
    proc_data();
    proc_create("proc_report", 0, NULL, &proc_report_fops);
    return 0;
}

void proc_cleanup(void) {
	printk(KERN_INFO "procReport: Performing cleanup of module\n");
    remove_proc_entry("proc_report", NULL);
}
MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);





