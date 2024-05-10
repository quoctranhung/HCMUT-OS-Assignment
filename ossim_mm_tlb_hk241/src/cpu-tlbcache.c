/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#define CACHE 4096

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */ 

int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, BYTE* value)
{
   if(mp == NULL || value == NULL) return -1;
   int idx = pgnum % mp->maxsz;
   for (int i = 0; i < CACHE; i++)
   {
      /* code */
      if(mp->cache[i].pid == pid && mp->cache[i].pgnum == pgnum)
      {
         *value = mp->cache[i].data;
         return 0;
      }
   }
   return -1;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
   if(mp == NULL)
   {
      return -1;
   }
   for (int i = 0; i < CACHE; i++)
   {
      /* code */
      if(mp->cache[i].valid == 0)
      {
         mp->cache[i].pid = pid;
         mp->cache[i].valid = 1;
         mp->cache[i].data = value;
         mp->cache[i].pgnum = pgnum;
         return 0;
      }
   }
   // flush theo fifo
   for (int i = 0; i < CACHE - 1; i++)
   {
      /* code */
      mp->cache[i] = mp->cache[i + 1];
   }
   mp->cache[CACHE - 1].pid = pid;
   mp->cache[CACHE - 1].pgnum = pgnum;
   mp->cache[CACHE - 1].data = value;
   mp->cache[CACHE - 1].valid = 1;
   
   return -1;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   printf("TLB:\n");
   for (int i = 0; i < mp->maxsz; i++)
   {
      if(mp->cache[i].valid == 1)
      {
         printf("PID: %d\tpage num: %d\tframe num: %d\n", mp->cache[i].pid, mp->cache[i].pgnum, mp->storage[i]);
      }
   }
   
   
   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->storage[max_size - 1] = " ";
   mp->cache = (struct TLBCache*)malloc((max_size / 16)*sizeof(struct TLBCache));

   for (int i = 0; i < (int)(max_size / 16); i++)
   {
      mp->cache[i].valid = 0;
      mp->cache[i].pid = -1;
      mp->cache[i].pgnum = -1;
      mp->cache[i].data = -1;
   }
   
   mp->maxsz = max_size;
   mp->rdmflg = 1;

   return 0;
}

//#endif
