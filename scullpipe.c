#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>


#define MIN(a,b) ((a)<(b)?a:b)
dev_t dev;

static struct file_operations scull_p_fops={
     .open=,
     .read=,
     .write=,
     .release=,
};



struct scull_pipe{
     wait_queue_head_t inq,outq; /*read and write queues*/
     char* buffer,*end; /*begin and end of buffer*/
     int buffersize; /*arithmatic*/
     char* rp,*wp; /*similar to inode read and write pointer*/
     int nreaders,nwriters; /*no of opening for r/w*/
     struct fasync_struct* async_queue; /*asynchronous readers*/
     struct semaphore sem; /*mutual exclusion on semaphore*/
     struct cdev dev; /*char device strcuture*/
};

static ssize_t scull_p_read(struct file* filp,char __user* buf,size_t count,loff_t* fpos){
     struct scull_pipe* pdev=filp->private_data;

     if(down_interruptible(&pdev->sem))
          return -ERESTARTSYS;
     /*
          pipe empty
     */
     while(pdev->rp==pdev->wp){
          up(&pdev->sem);
          if(filp->flags & O_NONBLOCK){
               return -EAGAIN;
          }
          PDEBUG("\"%s\" reading: going to sleep\n",current->comm);
          if(wait_event_interruptible(pdev->inq,(pdev->rp!=pdev->wp)))
               return -ESTARTSYS;
          if(down_interruptible(&pdev->sem)){ /*havrat pana nako , check again, it may have been acquired by another process*/
               /*
                    remember getblk() continue scenario
               */
               
               return -ERESTARTSYS}
     }
     /* 
          something present
          mhanje write jevdha pudhe tevdha read karayche
          */
     if(pdev->wp > pdev->rp){ 
          count=MIN(count,(size_t)(pdev->wp-pdev->rp));
     }
     else{
          /*buffer full lihun zhalay mhanun, write parat 0 la gela ahe*/
          count=MIN(count,(size_t)(pdev->end-pdev->rp));
     }
     if(copy_to_user(buf,pdev->rp,count)){
        
          up(&pdev->sem);
          return -EFAULT;  
     }
     pdev->rp+=count;
     if(pdev->rp==pdev->end){
          /*
               if read has reached end move it to start again
          */
          pdev->rp=pdev->buffer; 
          
     }
     up(&pdev->sem);

     /*
          wakeup the writers
     */
     wake_up_interruptible(&pdev->outq);
     PDEBUG("\"%s\" did read %li bytes\n",current->comm,(long)count);
     return count;

}


static ssize_t scull_p_write(struct file* filp,char __user* buf,size_t count,loff_t* fpos){
     struct scull_pipe* pdev=filp->private_data;

     if(down_interruptible(&pdev->sem))
          return -ERESTARTSYS;
     /*
          pipe empty
     */
   

     // while(pdev->rp==pdev->wp){
     //      up(&pdev->sem);
     //      if(filp->flags & O_NONBLOCK){
     //           return -EAGAIN;
     //      }
     //      PDEBUG("\"%s\" reading: going to sleep\n",current->comm);
     //      if(wait_event_interruptible(pdev->inq,(pdev->rp!=pdev->wp)))
     //           return -ESTARTSYS;
     //      if(down_interruptible(&pdev->sem)){ /*havrat pana nako , check again, it may have been acquired by another process*/
     //           /*
     //                remember getblk() continue scenario
     //           */
               
     //           return -ERESTARTSYS}
     // }
     /* 
          something present
          mhanje write jevdha pudhe tevdha read karayche
          */
   
     count=MIN(count,(size_t)(pdev->end-pdev->wp));
     if(copy_from_user(pdev->rp,buf,count)){
        
          up(&pdev->sem);
          return -EFAULT;  
     }
     pdev->wp+=count;
     if(pdev->wp==pdev->end){
          /*
               if read has reached end move it to start again
          */
          pdev->wp=pdev->buffer; 
          
     }
     up(&pdev->sem);

     /*
          wakeup the writers
     */
     wake_up_interruptible(&pdev->inq);
     PDEBUG("\"%s\" did write %li bytes\n",current->comm,(long)count);
     return count;

}