#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "fcntl.h"
#include "file.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_mmap(void)
{
  struct proc *p = myproc();

  int length;
  int prot, flags, fd;

  if (argint(1, &length) < 0 || argint(2, &prot) < 0 || argint(3, &flags) < 0 || argint(4, &fd) < 0)
    return 0xffffffffffffffff;

  struct file *f = p->ofile[fd];

  if(!f->writable && (prot&PROT_WRITE) && flags == MAP_SHARED) 
    return -1;
  if (p->originalsize == -1)
    p->originalsize = p->sz;

  int index = 0;

  for (; index < 16; index++){
    if (p->vma[index].used == 0)
      break;
  }

  if (index != 16){
    filedup(f);
    uint64 va = (p->sz);
    p->sz = va + length;
    p->vma[index].addr = va;
    p->vma[index].end = PGROUNDUP(va + length);
    p->vma[index].prot = prot;
    p->vma[index].flags = flags;
    p->vma[index].offset = 0;
    p->vma[index].pf = f;
    p->vma[index].used = 1;
    return va;
  }
  else
    return 0xffffffffffffffff;
}

uint64
sys_munmap(void)
{
  struct proc *p = myproc();

  uint64 addr;
  int len;

  if (argaddr(0, &addr) < 0 || argint(1, &len) < 0)
    return -1;

  int index = 0;

  for (; index < 16; index++){
    if (p->vma[index].used == 1 && p->vma[index].addr <= addr && addr<p->vma[index].end){  
      if(p->vma[index].flags==MAP_SHARED){
        begin_op();
        ilock(p->vma[index].pf->ip);
        writei(p->vma[index].pf->ip,1,addr,addr - p->vma[index].addr,len);
        iunlock(p->vma[index].pf->ip);
        end_op();
      }
      uvmunmap(p->pagetable,addr,len/PGSIZE,1);
      if(addr==p->vma[index].addr){
        if(p->vma[index].end==addr+len){
          fileclose(p->vma[index].pf);
          p->vma[index].used = 0;
        }else{
            p->vma[index].addr = addr+len;
        }
      }else if( addr+len == p->vma[index].end){
        p->vma[index].end = addr+len;
      }
      return 0;
    }
  }

  return -1;
}