// // Physical memory allocator, for user processes,
// // kernel stacks, page-table pages,
// // and pipe buffers. Allocates whole 4096-byte pages.

// #include "types.h"
// #include "param.h"
// #include "memlayout.h"
// #include "spinlock.h"
// #include "riscv.h"
// #include "defs.h"

// #define STEAL_THRESHOLD 1

// void freerange(void *pa_start, void *pa_end);

// extern char end[]; // first address after kernel.
//                    // defined by kernel.ld.

// #define MEM_PER_CPU (PHYSTOP - PGROUNDUP((uint64)end)) / NCPU

// struct run {
//   struct run *next;
// };

// // struct spinlock steal_lock;

// struct {
//   struct spinlock lock;
//   struct run *freelist;
// } kmem[NCPU];

// static char* locknames[NCPU] = {
//   "kmem_0",
//   "kmem_1",
//   "kmem_2",
//   "kmem_3",
//   "kmem_4",
//   "kmem_5",
//   "kmem_6",
//   "kmem_7",
// };

// void
// kinit_free(void *pa, int cid)
// {
//   struct run *r;

//   if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//     panic("kinit_free");

//   // Fill with junk to catch dangling refs.
//   memset(pa, 1, PGSIZE);

//   r = (struct run*)pa;

//   acquire(&kmem[cid].lock);
//   r->next = kmem[cid].freelist;
//   kmem[cid].freelist = r;
//   release(&kmem[cid].lock);
// }

// void
// kinit()
// {
//   for(int i=0; i<NCPU; ++i){
//     initlock(&kmem[i].lock, locknames[i]);
//   }
//   // initlock(&steal_lock, "steal_lock");
//   freerange(end, (void*)PHYSTOP);
// }

// void
// freerange(void *pa_start, void *pa_end)
// {
//   char *p;
//   p = (char*)PGROUNDUP((uint64)pa_start);
//   for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
//     kfree(p);
// }

// // Free the page of physical memory pointed at by v,
// // which normally should have been returned by a
// // call to kalloc().  (The exception is when
// // initializing the allocator; see kinit above.)
// void
// kfree(void *pa)
// {
//   struct run *r;

//   if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//     panic("kfree");

//   // Fill with junk to catch dangling refs.
//   memset(pa, 1, PGSIZE);

//   r = (struct run*)pa;
//   push_off();
//   int cid = cpuid();
//   acquire(&kmem[cid].lock);
//   r->next = kmem[cid].freelist;
//   kmem[cid].freelist = r;
//   release(&kmem[cid].lock);
//   pop_off();
// }

// // Allocate one 4096-byte page of physical memory.
// // Returns a pointer that the kernel can use.
// // Returns 0 if the memory cannot be allocated.
// void *
// kalloc(void)
// {
//   struct run *r;
//   push_off();
//   int cid = cpuid();

//   acquire(&kmem[cid].lock);
//   r = kmem[cid].freelist;
//   if(r){
//     kmem[cid].freelist = r->next;
//   } else{
//     // steal from other cpu
//     // acquire(&steal_lock);
//     int steals = 0;
//     struct run *steal_r;
//     for(int i=0; i<NCPU && steals < STEAL_THRESHOLD; ++i){
//       if(i == cid) continue;
//       acquire(&kmem[i].lock);
//       steal_r = kmem[i].freelist;
//       while (steal_r && steals < STEAL_THRESHOLD)
//       {
//         kmem[i].freelist = steal_r->next;
//         steal_r->next = kmem[cid].freelist;
//         kmem[cid].freelist = steal_r;
//         ++steals;
//         steal_r = kmem[i].freelist;
//       }
//       release(&kmem[i].lock);
//     }
//     // release(&steal_lock);
//   }
//   release(&kmem[cid].lock);
//   pop_off();

//   if(r)
//     memset((char*)r, 5, PGSIZE); // fill with junk
//   return (void*)r;
// }
// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define STEAL_THRESHOLD 8

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct spinlock steal_lock;

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{

  initlock(&steal_lock, "steal_lock");
  char lockname[8];
  for(int i=0; i<NCPU; ++i) {
    snprintf(lockname, sizeof lockname, "kmem_%d", i);
    initlock(&kmem[i].lock, lockname);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  int cid = cpuid();

  acquire(&kmem[cid].lock);
  r->next = kmem[cid].freelist;
  kmem[cid].freelist = r;
  release(&kmem[cid].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int cid = cpuid();

  acquire(&kmem[cid].lock);
  r = kmem[cid].freelist;
  if(!r){
    // steal from other cpu
    int steals = 0;
    struct run *steal_r;
    acquire(&steal_lock);
    for(int i=0; i<NCPU && steals < STEAL_THRESHOLD; ++i){
      if(i == cid) continue;
      acquire(&kmem[i].lock);
      steal_r = kmem[i].freelist;
      while (steal_r && steals < STEAL_THRESHOLD)
      {
        kmem[i].freelist = steal_r->next;
        steal_r->next = kmem[cid].freelist;
        kmem[cid].freelist = steal_r;
        ++steals;
        steal_r = kmem[i].freelist;
      }
      release(&kmem[i].lock);
    }
    release(&steal_lock);
    r = kmem[cid].freelist;
  }
  if(r)
    kmem[cid].freelist = r->next;
  release(&kmem[cid].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
