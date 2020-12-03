
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>	
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/timekeeping.h>


#define BUFSIZE  100
#define KBD_IRQ             1       /* IRQ number for keyboard (i8042) */
 
MODULE_AUTHOR("Piotr Bak");
 
static int counter_key = 0; 
module_param(counter_key,int, 0660);

static char my_timestamp[BUFSIZE];
  
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos);
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos);

static void do_gettimeofday(struct timeval *tv)
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec;
}

static struct proc_dir_entry *ent;
 
static struct file_operations myops =
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
};

static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	char buf[BUFSIZE];
	int len=0;
	
	if(*ppos > 0 || count < BUFSIZE)
		return 0;
		
	len += sprintf(buf + len,"counter = %d\n", counter_key);
	len += sprintf(buf + len,"%s", my_timestamp);
	
	if(copy_to_user(ubuf,buf,len))
		return -EFAULT;
		
	*ppos = len;
	
	return len;
}

static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	int c;
	char buf[BUFSIZE];
	struct timeval t;
	struct tm broken;
	
	if(*ppos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf,ubuf,count))
		return -EFAULT;

	if (strcmp(buf,"Set zero") == 0u)
	{
		counter_key = 0;

		do_gettimeofday(&t);
		time64_to_tm(t.tv_sec, 0, &broken);
		sprintf(my_timestamp,"%02d:%02d:%02d:%ld\n", broken.tm_hour, broken.tm_min,
								 broken.tm_sec, t.tv_usec);
	}

	*ppos = strlen(buf);

	return c;
}
 
 
static irqreturn_t kbd3_isr(int irq, void *dev_id)
{    
    counter_key = counter_key+1;

    return IRQ_HANDLED;
} 
 
static int simple_init(void)
{
	struct timeval t;
	struct tm broken;

	ent=proc_create("mydev",0660,NULL,&myops);
	printk(KERN_ALERT "hello...\n");

	do_gettimeofday(&t);
	time64_to_tm(t.tv_sec, 0, &broken);
	sprintf(my_timestamp,"%02d:%02d:%02d:%ld\n", broken.tm_hour, broken.tm_min, broken.tm_sec, t.tv_usec);

	return request_irq(KBD_IRQ, kbd3_isr, IRQF_SHARED, "kbd3", (void *)kbd3_isr);
}
 
static void simple_cleanup(void)
{
    printk(KERN_WARNING "bye ...\n");
	proc_remove(ent);
	free_irq(KBD_IRQ, (void *)kbd3_isr);
}
 
module_init(simple_init);
module_exit(simple_cleanup);

