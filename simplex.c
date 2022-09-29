#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/delay.h>


#define BUFF_SIZE 100
#define NUMOFVAR 3
#define NUMOFSLACK NUMOFVAR
#define ROWSIZE (NUMOFSLACK+1)
#define COLSIZE (NUMOFSLACK+NUMOFVAR+1)
#define SIZE (ROWSIZE*COLSIZE+2)

MODULE_AUTHOR("Vladimir Vincan");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("SIMPLEX IP core driver");

#define DRIVER_NAME "SIMPLEX"

// ------------------------------------------
// DECLARATIONS
// ------------------------------------------

static dev_t my_dev_id;
static struct class *my_class;
static struct cdev  *my_cdev;

static int simplex_probe     (struct platform_device *pdev);
static int simplex_remove    (struct platform_device *pdev);
static int simplex_open      (struct inode *pinode, struct file *pfile);
static int simplex_close     (struct inode *pinode, struct file *pfile);
static ssize_t simplex_read  (struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
static ssize_t simplex_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

static int  __init simplex_init(void);
static void __exit simplex_exit(void);
//user functions
static void pivoting(void);
static uint64_t mymul32x32(uint32_t a, uint32_t b,int time);
static uint32_t multi(uint32_t a,uint32_t b,int time);


struct device_info
{
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;
};

static struct device_info *fft2    = NULL;
static struct device_info *bram_re = NULL;
static struct device_info *bram_im = NULL;

static struct of_device_id device_of_match[] = {
  { .compatible = "xlnx,fft2", },
  { .compatible = "xlnx,bram_re", },
  { .compatible = "xlnx,bram_im", },
  { /* end of list */ }
};

MODULE_DEVICE_TABLE(of, device_of_match);

static struct platform_driver my_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= device_of_match,
  },
  .probe = simplex_probe,
  .remove	= simplex_remove,
};

struct file_operations my_fops =
{
	.owner   = THIS_MODULE,
	.open    = simplex_open,
	.read    = simplex_read,
	.write   = simplex_write,
	.release = simplex_close,
};
////////global var
uint32_t bram[SIZE];
int p=0,q=0;
int ready=1,start=0;

int i = 0;
int endRead = 0;
// ------------------------------------------
// PROBE & REMOVE
// ------------------------------------------

int device_fsm = 0;

static int simplex_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		printk(KERN_ALERT "invalid address\n");
		return -ENODEV;
	}
  printk(KERN_INFO "Starting probing.\n");

  switch (device_fsm)
    {
    case 0: // device fft2
      fft2 = (struct device_info *) kmalloc(sizeof(struct device_info), GFP_KERNEL);
      if (!fft2)
        {
          printk(KERN_ALERT "Cound not allocate fft2 device\n");
          return -ENOMEM;
        }
      fft2->mem_start = r_mem->start;
      fft2->mem_end   = r_mem->end;
      if(!request_mem_region(fft2->mem_start, fft2->mem_end - fft2->mem_start+1, DRIVER_NAME))
        {
          printk(KERN_ALERT "Couldn't lock memory region at %p\n",(void *)fft2->mem_start);
          rc = -EBUSY;
          goto error1;
        }
      fft2->base_addr = ioremap(fft2->mem_start, fft2->mem_end - fft2->mem_start + 1);
      if (!fft2->base_addr)
        {
          printk(KERN_ALERT "[PROBE]: Could not allocate fft2 iomem\n");
          rc = -EIO;
          goto error2;
        }
      ++device_fsm;
      printk(KERN_INFO "[PROBE]: Finished probing fft2.\n");
      return 0;
      error2:
        release_mem_region(fft2->mem_start, fft2->mem_end - fft2->mem_start + 1);
      error1:
        return rc;
      break;

    case 1: // device bram_re
      bram_re = (struct device_info *) kmalloc(sizeof(struct device_info), GFP_KERNEL);
      if (!bram_re)
        {
          printk(KERN_ALERT "Cound not allocate bram_re device\n");
          return -ENOMEM;
        }
      bram_re->mem_start = r_mem->start;
      bram_re->mem_end   = r_mem->end;
      if(!request_mem_region(bram_re->mem_start, bram_re->mem_end - bram_re->mem_start+1, DRIVER_NAME))
        {
          printk(KERN_ALERT "Couldn't lock memory region at %p\n",(void *)bram_re->mem_start);
          rc = -EBUSY;
          goto error3;
        }
      bram_re->base_addr = ioremap(bram_re->mem_start, bram_re->mem_end - bram_re->mem_start + 1);
      if (!bram_re->base_addr)
        {
          printk(KERN_ALERT "[PROBE]: Could not allocate bram_re iomem\n");
          rc = -EIO;
          goto error4;
        }
      ++device_fsm;
      printk(KERN_INFO "[PROBE]: Finished probing bram_re.\n");
      return 0;
      error4:
        release_mem_region(bram_re->mem_start, bram_re->mem_end - bram_re->mem_start + 1);
      error3:
        return rc;
      break;

    case 2: // device bram_im
      bram_im = (struct device_info *) kmalloc(sizeof(struct device_info), GFP_KERNEL);
      if (!bram_im)
        {
          printk(KERN_ALERT "Cound not allocate bram_im device\n");
          return -ENOMEM;
        }
      bram_im->mem_start = r_mem->start;
      bram_im->mem_end   = r_mem->end;
      if(!request_mem_region(bram_im->mem_start, bram_im->mem_end - bram_im->mem_start+1, DRIVER_NAME))
        {
          printk(KERN_ALERT "Couldn't lock memory region at %p\n",(void *)bram_im->mem_start);
          rc = -EBUSY;
          goto error5;
        }
      bram_im->base_addr = ioremap(bram_im->mem_start, bram_im->mem_end - bram_im->mem_start + 1);
      if (!bram_im->base_addr)
        {
          printk(KERN_ALERT "[PROBE]: Could not allocate bram_im iomem\n");
          rc = -EIO;
          goto error6;
        }
      printk(KERN_INFO "[PROBE]: Finished probing bram_im.\n");
      return 0;
      error6:
        release_mem_region(bram_im->mem_start, bram_im->mem_end - bram_im->mem_start + 1);
      error5:
        return rc;
      break;

    default:
      printk(KERN_INFO "[PROBE] Device FSM in illegal state. \n");
      return -1;
    }
  printk(KERN_INFO "Succesfully probed driver\n");
  return 0;
}

static int simplex_remove(struct platform_device *pdev)
{
  switch (device_fsm)
    {
    case 0: //device fft2
      printk(KERN_ALERT "fft2 device platform driver removed\n");
      iowrite32(0, fft2->base_addr);
      iounmap(fft2->base_addr);
      release_mem_region(fft2->mem_start, fft2->mem_end - fft2->mem_start + 1);
      kfree(fft2);
      break;

    case 1: //device bram_re
      printk(KERN_ALERT "bram_re platform driver removed\n");
      iowrite32(0, bram_re->base_addr);
      iounmap(bram_re->base_addr);
      release_mem_region(bram_re->mem_start, bram_re->mem_end - bram_re->mem_start + 1);
      kfree(bram_re);
      --device_fsm;
      break;

    case 2: //device bram_im
      printk(KERN_ALERT "bram_im platform driver removed\n");
      iowrite32(0, bram_im->base_addr);
      iounmap(bram_im->base_addr);
      release_mem_region(bram_im->mem_start, bram_im->mem_end - bram_im->mem_start + 1);
      kfree(bram_im);
      --device_fsm;
      break;

    default:
      printk(KERN_INFO "[REMOVE] Device FSM in illegal state. \n");
      return -1;
    }
  printk(KERN_INFO "Succesfully removed driver\n");
  return 0;
}

// ------------------------------------------
// OPEN & CLOSE
// ------------------------------------------

static int simplex_open(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

static int simplex_close(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}
/////////////////////
////////USER FUNCTIONS
static void pivoting(void)
{
	ready=0;
	int c,d;
	uint32_t newRow[COLSIZE];
	uint32_t temp=0;
	uint32_t sub=0;
	uint32_t pivotColVal[ROWSIZE];
	int pivotRow=0;
	int pivotCol=(int)(bram[SIZE-2]>>21);
	uint32_t pivot=bram[SIZE-1];
	uint32_t m1=0,m2=0;	
	uint32_t m3,m4=0;
	printk(KERN_INFO "Pivoting\n");
	for(c=0;c<COLSIZE;c++)
	{
		//printk(KERN_INFO "%u,%u\n",bram[pivot*COLSIZE+c],pivot);
		m1=bram[pivotRow*COLSIZE+c];
		m2=pivot;
		newRow[c]=multi(m1,m2,c);
		//printk(KERN_INFO "multi=%u,m1=%u,m2=%u\n",newRow[c],m1,m2);
		bram[pivotRow*COLSIZE+c]=newRow[c];
		
		
	}
	printk(KERN_INFO "---------Phase1\n");
	for(d=1;d<ROWSIZE;d++)
	{
		pivotColVal[d]=bram[d*COLSIZE+pivotCol];
		//printk(KERN_INFO "pivotcolval,%u\n",pivotColVal[d]);
		
	}
	printk(KERN_INFO "---------Phase2\n");
	for(d=1;d<ROWSIZE;d++)
	{
		for(c=0;c<COLSIZE;c++)
		{
			m3=newRow[c];
			m4=pivotColVal[d];
			temp=multi(m3,m4,d*COLSIZE+c);
			//if(d==ROWSIZE-1 && c==0)
				printk(KERN_INFO "temp=%u",temp);
				printk(KERN_INFO "temp=%#010x",temp);
			sub=bram[d*COLSIZE+c]-temp;
			//if(d==ROWSIZE-1 && c==0)
			//printk(KERN_INFO "%d,%d,%u,%u,%u,%u,%u,\n",d,c,sub,bram[d*COLSIZE+c],temp,m3,m4);
			bram[d*COLSIZE+c]=sub;
			printk(KERN_INFO "sub=%#010x->",sub);
			printk(KERN_INFO "%dend",d*COLSIZE+c);
			
		}
	}
	printk(KERN_INFO "---------Phase3\n");
	
	ready=1;
}
/////// unsigned 32bit multiplication
uint64_t mymul32x32(uint32_t a, uint32_t b,int time)
{
  uint16_t va_high = (a >> 16) & 0xFFFF;
  uint16_t va_low = a & 0xFFFF;

  uint16_t vb_high = (b >> 16) & 0xFFFF;
  uint16_t vb_low = b & 0xFFFF;

  uint64_t mul_high_high = (uint64_t)((uint32_t)(va_high * vb_high))<< 32;
  uint64_t mul_high_low = (uint64_t)((u32)(va_high << 16)) * vb_low;
  uint64_t mul_low_high = (uint64_t)((u32)(vb_high << 16)) * va_low;
  uint64_t mul_low_low = (uint64_t)(va_low * vb_low);

  uint64_t res = 0;

  res = mul_high_high;
  res += mul_high_low;
  res += mul_low_high;
  res += mul_low_low;

  return res;
}
//////////// signed 32bit multiplication
static uint32_t multi(uint32_t a,uint32_t b,int time)
{
	printk(KERN_INFO "%dbegin",time);
    int nega=0;
    int negb=0;
    uint64_t rez=0;
    uint64_t del=0;
    uint32_t r;
    if(a >= 2147483648)
    {
        printk(KERN_INFO "%#010x",a);
        a=~a;
        a++;
        printk(KERN_INFO "%#010x",a);
        nega=1;
    }
    if(b >= 2147483648)
    {
       printk(KERN_INFO "%#010x",b);
        b=~b;
        b++;
        printk(KERN_INFO "%#010x",b);
        negb=1;
    }
    printk(KERN_INFO "a=%#010x\n",a);
    printk(KERN_INFO "b=%#010x\n",b);
    rez=mymul32x32(a,b,time); //unsigned 32bit multiplication
   printk(KERN_INFO "rez=%#018x\n,%llu",rez,rez);
   //rez=rez& 0x00000000ffffffff;
    del=(rez/2097152)& 0x00000000ffffffff; printk(KERN_INFO "del=%#018x\n,%llu",del,del);
    r=(uint32_t)del& 0xffffffff; printk(KERN_INFO "r=%#010x\n,%u",r,r);
    if(nega ^ negb)
    {
        printk(KERN_INFO "%#010x->",r);
        r=~r;
        r++;
       printk(KERN_INFO "%#010x\n",r);
    }
    
    return r;
}
////////////


/////////////////
//////////////
// ------------------------------------------
// READ & WRITE
// ------------------------------------------




ssize_t simplex_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
  char buff[BUFF_SIZE];
  long int len=0;
  u32 bram_val=0;
 
  
  int minor = MINOR(pfile->f_inode->i_rdev);

  /* printk(KERN_INFO "FFT2 READ entered \n"); */
  //printk(KERN_INFO "i = %d, len = %ld, end_read = %d\n", i, len, end_read);
  if (endRead){
	printk(KERN_INFO "Reading complete \n");
	endRead = 0;
	return 0;
}

  switch (minor)
    {
    case 0://citanje registri
     
  
      len=scnprintf(buff,BUFF_SIZE,"%d %d\n",start,ready);
      
      if(copy_to_user(buffer,buff,len))
	{
		printk(KERN_ERR "Copy to user does not work\n");
		return -EFAULT;
	}
	printk(KERN_INFO "Read from pivot,Start=%d,Ready=%d\n",start,ready);
      endRead = 1;
      break;

    case 1: //device bram
      //bram_val=bram[p];
      ////////citanje sve odjednom
	/*if(p<SIZE)
	{
		if(p==SIZE-1)
		
			len=scnprintf(buff,BUFF_SIZE,"%u\n",bram[p]);
		else
			len=scnprintf(buff,BUFF_SIZE,"%u ",bram[p]);
		printk(KERN_INFO "%d,Succesfully read %#010x,lu\n",p,bram[p],len);
		p++;
		if(copy_to_user(buffer,buff,len))
		{
			printk(KERN_ERR "Copy to user does not work\n");
			return -EFAULT;
		}
	}
	else
	{
		printk(KERN_INFO "Succesfully read from bram\n");
		p=0;
		endRead=1;
	}
	break;*/
	//offset+=len;
	//endRead=1;
	///////citanje vrstu po vrstu
	printk(KERN_INFO "%d,%d\n",q,p);
	if(p<COLSIZE)
	{
		if(p==COLSIZE-1)
			len=scnprintf(buff,BUFF_SIZE,"%u\n",bram[q*COLSIZE+p]);
		else
			len=scnprintf(buff,BUFF_SIZE,"%u ",bram[q*COLSIZE+p]);
			//printk(KERN_INFO "%d,%d,Succesfully read %#010x,%lu\n",q,p,bram[q*COLSIZE+p],len);
		p++;
		printk(KERN_INFO "-%d,%d\n",q,p);
		if(copy_to_user(buffer,buff,len))
		{
			printk(KERN_ERR "Copy to user does not work\n");
			return -EFAULT;
		}
	}
	if(p==COLSIZE)
	{
		
		if(q<ROWSIZE)
		{
			printk(KERN_INFO "--%d,%d\n",q,p);
			q++;
			p=0;
		}
		if(q==ROWSIZE)
		{
			printk(KERN_INFO "---%d,%d\n",q,p);
			p=0;
			q=0;
			printk(KERN_INFO "Succesfully read from bram\n");
		}
		endRead=1;
	}
      break;

    

    default:
      printk(KERN_ERR "[READ] Invalid minor. \n");
      endRead = 1;
    }

  return len;
}

ssize_t simplex_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
  //char buf[length+1];
  int minor = MINOR(pfile->f_inode->i_rdev);
  int start_val=0;
  char buff[BUFF_SIZE];
	int addr;
	int ret;
	u32 bram_val;
  if (copy_from_user(buff, buffer, length))
    return -EFAULT;
  buff[length]='\0';

  switch (minor)
    {
    case 0: //upis u star registar
      sscanf(buff,"%d",&start_val);
      if(start_val==1)
      {
      	if(ready==1)
	{
		start=start_val;
		pivoting();
        }
      }
      else
      	start=start_val;
      printk(KERN_INFO "Wrote succesfully to start register value %u\n",start_val);
      break;

    case 1: //upis u bram
      ret = sscanf(buff,"%d,%u",&addr,&bram_val);
	
	
	if((addr<SIZE) &&
	(addr>=0)) 
	{
		
		if(ret==2)//two parameter parsed in sscanf
		{
			//printk(KERN_INFO "Succesfully wrote value %#010x to %d", bram_val,addr); 
			bram[addr] = bram_val; 
	
		}
		else
		{
			printk(KERN_WARNING "Wrong command format\n");
		}
	}
	else
	{
		printk(KERN_WARNING "bram is full\n"); 
	}
      break;

    

    default:
      printk(KERN_INFO "[WRITE] Invalid minor. \n");
  }

  return length;
}

// ------------------------------------------
// INIT & EXIT
// ------------------------------------------

static int __init simplex_init(void)
{
   printk(KERN_INFO "\n");
   printk(KERN_INFO "simplex driver starting insmod.\n");
	
	int i=0;
	for(i=0;i<ROWSIZE*COLSIZE+1;i++)
	{
		bram[i]=0;
	}
	start=0;
	ready=1;
	
	
	
   if (alloc_chrdev_region(&my_dev_id, 0, 2, "simplex_region") < 0){
      printk(KERN_ERR "failed to register char device\n");
      return -1;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "simplex_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");

   if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),0), NULL, "pivot") == NULL) {
      printk(KERN_ERR "failed to create device pivot\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created - pivot\n");

   if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),1), NULL, "bram") == NULL) {
     printk(KERN_ERR "failed to create device bram\n");
     goto fail_2;
   }
   printk(KERN_INFO "device created - bram\n");

  

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;

	if (cdev_add(my_cdev, my_dev_id, 2) == -1)
	{
      printk(KERN_ERR "failed to add cdev\n");
      goto fail_3;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "simplex driver initialized.\n");

   return platform_driver_register(&my_driver);

    
    fail_3:
     device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	  fail_2:
     device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit simplex_exit(void)
{
  printk(KERN_INFO "simplex driver starting rmmod.\n");
	platform_driver_unregister(&my_driver);
	cdev_del(my_cdev);
 
  device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
  device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
  class_destroy(my_class);
  unregister_chrdev_region(my_dev_id,1);
  printk(KERN_INFO "simplex driver exited.\n");
}


module_init(simplex_init);
module_exit(simplex_exit);
