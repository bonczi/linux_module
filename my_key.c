
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


#define BUFSIZE      100
#define KBD_IRQ      1       /* IRQ number for keyboard (i8042) */
#define MON_AMOUNT   12
#define DAY_AMOUNT   7

#define SUCCESS_FAIL 0u
#define SUCCESS_OK   1u
#define STRCMP_OK    0u

#define YEAR_OFFSET 1900
#define HOUR_OFFSET 1
 
MODULE_AUTHOR("Piotr Bak");
 
static int counter_key = 0; 
module_param(counter_key,int, 0660);

static char my_timestamp[BUFSIZE];
static char * daystr = "Mon";
static char * monstr = "Jan";
  
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos);
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos);
static int convert_dayweek(int day, char ** str);
static int convert_month(int day, char ** str);

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
    struct tm data;

    if(*ppos > 0 || count > BUFSIZE)
        return -EFAULT;
    if(copy_from_user(buf,ubuf,count))
        return -EFAULT;

    if (strcmp(buf,"Set zero") == STRCMP_OK)
    {
        counter_key = 0;

        do_gettimeofday(&t);
        time64_to_tm(t.tv_sec, 0, &data);

        if (convert_dayweek(data.tm_wday, &daystr) && convert_month(data.tm_mon, &monstr))
        {
            sprintf(my_timestamp,"Ostatnie resetowanie licznika:   %s %s  %d %02d:%02d:%02d %ld\n",
                    daystr, monstr, data.tm_mday, data.tm_hour+HOUR_OFFSET, data.tm_min, data.tm_sec, data.tm_year+YEAR_OFFSET);
        }
    }

    *ppos = strlen(buf);

    return c;
}
 
 
static irqreturn_t kbd3_isr(int irq, void *dev_id)
{    
    counter_key = counter_key+1;

    return IRQ_HANDLED;
} 
 
static int keyboard_module_init(void)
{
    struct timeval t;
    struct tm data;

    ent=proc_create("mydev",0660,NULL,&myops);
    printk(KERN_ALERT "keyboard_module_counter opening\n");

    do_gettimeofday(&t);
    time64_to_tm(t.tv_sec, 0, &data);

    if (convert_dayweek(data.tm_wday, &daystr) && convert_month(data.tm_mon, &monstr))
    {
        sprintf(my_timestamp,"Last file modification:   %s %s  %d %02d:%02d:%02d %ld\n",
                daystr, monstr, data.tm_mday, data.tm_hour+HOUR_OFFSET, data.tm_min, data.tm_sec, data.tm_year+YEAR_OFFSET);
    }

    return request_irq(KBD_IRQ, kbd3_isr, IRQF_SHARED, "kbd3", (void *)kbd3_isr);
}
 
static void keyboard_module_cleanup(void)
{
    printk(KERN_WARNING "keyboard_module_counter closing\n");
    proc_remove(ent);
    free_irq(KBD_IRQ, (void *)kbd3_isr);
}
 
static int convert_dayweek(int day, char ** str)
{
    int success = SUCCESS_FAIL;

    if ( (str != NULL) && (day >= 0) && (day < DAY_AMOUNT) )
    {
        switch (day)
        {
            case (0u):
                    *str = "Sun";
                    break;
            case (1u):
                    *str = "Mon";
                    break;
            case (2u):
                    *str = "Tue";
                    break;
            case (3u):
                    *str = "Wed";
                    break;
            case (4u):
                    *str = "Thu";
                    break;
            case (5u):
                    *str = "Fri";
                    break;
            case (6u):
                    *str = "Sat";
                    break;
            default:
                break;
        }
        success = SUCCESS_OK;
    }

    return success;
}

static int convert_month(int mon, char ** str)
{
    int success = SUCCESS_FAIL;

    if ( (str != NULL) && (mon >= 0) && (mon < MON_AMOUNT) )
    {
        switch (mon)
        {
            case (0u):
                    *str = "Jan";
                    break;
            case (1u):
                    *str = "Feb";
                    break;
            case (2u):
                    *str = "Mar";
                    break;
            case (3u):
                    *str = "Apr";
                    break;
            case (4u):
                    *str = "May";
                    break;
            case (5u):
                    *str = "Jun";
                    break;
            case (6u):
                    *str = "Jul";
                    break;
            case (7u):
                    *str = "Aug";
                    break;
            case (8u):
                    *str = "Sep";
                    break;
            case (9u):
                    *str = "Oct";
                    break;
            case (10u):
                    *str = "Nov";
                    break;
            case (11u):
                    *str = "Dec";
                    break;
            default:
                break;
        }
        success = SUCCESS_OK;
    }

    return success;
}

module_init(keyboard_module_init);
module_exit(keyboard_module_cleanup);

