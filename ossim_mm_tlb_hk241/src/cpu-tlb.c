/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
#include "mm.h"
#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
#include <stdlib.h>
#include <stdio.h>
#define CACHE 4096

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}


int tlb_flush_tlb_of(struct memphy_struct *mp) 
{

  return 0;
}
/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;
  val = __alloc(proc, 0, reg_index, size, &addr); // cấp phân vùng có kích thước là size, trả về địa chỉ của phân vùng vừa cấp
  BYTE fpn = -1;
  int rg_start = proc->mm->symrgtbl[reg_index].rg_start; // lấy địa chỉ bắt đầu của phân vùng ảo thứ reg_index
  int rg_end = proc->mm->symrgtbl[reg_index].rg_end;

  int limit = rg_end - rg_start; // giới hạn của phân vùng
  int size_align = PAGING_PAGE_ALIGNSZ(limit); // lấy size cho số khung trang cần thiết
  int num_pages = size_align / PAGING_PAGESZ; // lấy số lượng khung trang

  for(int i = 0; i < num_pages; i++)
  {
    int pgn = PAGING_PGN(rg_start + PAGING_PAGESZ * i); // số trang
    pg_getpage(proc->mm, pgn, &fpn, proc); // lấy đia chỉ khung trang thông qua số trang
    for (int j = 0; j < proc->tlb->maxsz * i / 10; j++)
    {
      tlb_cache_write(proc->tlb, proc->pid, pgn, fpn); // lưu chỉ số khung trang vào cache
      proc->tlb->cache[i]->valid = 0;
    }
  }
  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index) 
{
  __free(proc, 0, reg_index); // đẩy phân vùng thứ reg_index vào list free

  int rg_start = proc->mm->symrgtbl[reg_index].rg_start;
  int rg_end = proc->mm->symrgtbl[reg_index].rg_end;

  int limit = rg_end - rg_start;
  
  int size_align = PAGING_PAGE_ALIGNSZ(limit);
  int num_pages = size_align / PAGING_PAGESZ;
  for(int i = 0; i < num_pages; i++){
    int pgn = PAGING_PGN(rg_start + PAGING_PAGESZ * i); // xóa các trang thuộc phân vùng đã bị xóa ra khỏi cache
    for (int i = 0; i < proc->tlb->maxsz; i++)
    {
      /* code */
      if(proc->pid == proc->tlb->cache[i]->pid && pgn == proc->tlb->cache[i]->pgnum)
      {
        proc->tlb->cache[i]->valid = 0;
        proc->tlb->cache[i]->data = -1;
        proc->tlb->cache[i]->pid = -1;
        proc->tlb->cache[i]->pgnum = -1;
      }
    }
  }
  return 0;
}

/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{	
  BYTE data, frmnum = -1;
  int rg_start = proc->mm->symrgtbl[source].rg_start + offset; // 
  int pgn = PAGING_PGN(rg_start);
  BYTE fpn = -1;
  frmnum = tlb_cache_read(proc->tlb, proc->pid, pgn, &fpn);
	
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
#ifdef MM_PAGING
  MEMPHY_dump(proc->mram);
  // TLBMEMPHY_dump(proc->tlb);
#endif
#endif
int val;
  if(frmnum >= 0)
  {
    int phyaddress = (frmnum << PAGING_ADDR_FPN_LOBIT + offset);
    TLBMEMPHY_read(proc->mram, phyaddress, &data);
  }
  else
  {
  val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t) data;
  pg_getpage(proc->mm, pgn, &fpn, proc);
  tlb_cache_write(proc->tlb, proc->pid, pgn, fpn);
  }
  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  BYTE frmnum = -1;
  int rg_start = proc->mm->symrgtbl[destination].rg_start + offset;
  int pgn = PAGING_PGN(rg_start);
  BYTE fpn = 0; 
  pg_getpage(proc->mm, pgn, &fpn, proc);
  frmnum = tlb_cache_write(proc->tlb, proc->pid, pgn, fpn);

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
  // TLBMEMPHY_dump(proc->tlb);
#endif

if(frmnum >= 0)
{
  int phyaddress = (frmnum << PAGING_ADDR_FPN_LOBIT + offset);
  TLBMEMPHY_write(proc->mram, phyaddress, data);
}
else{
  val = __write(proc, 0, destination, offset, data);
  tlb_cache_write(proc->tlb, proc->pid, pgn, fpn);
  destination = (uint32_t)data;
}
  return val;
}

#endif
