/*
 * A place to temporarily store code/pseudocode for ideas
 * then reallocating to appropriate source files
 *
 */
PAGES_PER_REGION=VMEM_REGION_SIZE/PAGESIZE;
unsigned long[] init_page_table(void)
{
  unsigned long vaddr, end;
  pgd_t *pgd, *pgd_base;
  int num_pages,i, j, k;
  unsigned long * paddr
  pmd_t *pmd;
  pte_t *pte, *pte_base;
  
  unsigned long page_table[PAGES_PER_REGION];
  
  paddr
  for (i=0; i<PTRS_PER_PGD; i++)
    set_pgd(pgd_base + i; __pgd(1 + __pa(empty_zero_page)));
  i = _pgd_offset(PAGE_OFFSET);
  pgd = pgd_base+i;
  return page_table
    }
}
struct entry{
  struct entry *next;
  unsigned long *key;
  unsigned long *value;
}
struct dict {
  int size;
  int n;
  struct entry **table;
}

Dict internalDictCreate(int size)
{
  Dict d;
  int i;
  d = malloc(sizeof(*d));
  assert(d!=0);
  
  d->size = size;
  d->n = 0;
  d->table = malloc(sizeof(struct entry *) * d->size);
  assert(d->table !=0);
  for(i=0; i<d->size; i++)
    d->table[i]=0;
  return d;
}
Dict DictCreate(void)
{
  return internalDictCreate(INITIAL_SIZE);
}
void DictDestroy(Dict d)
{
  int i;
  struct entry *e;
  struct entry *next;
  for(i=0; i< d->size; i++){
    for(e=d->table[i]; e!=0; e=next){
      next = e->next;
      free(e->key);
      free(e->value);
      free(e);
    }
  }
  free(d->table);
  free(d);
}
static void grow(Dict D)
{
  Dict d2;
  struct dict swap;
  int i;
  struct entry *e;
  d2 = internalDictCreate(d->size*GROWTH_FACTOR);
  for(i=0; i<d->size; i++){
    for(e=d->table[i];e!=0; e=e->next){
      DictInsert(d2,e->key,e->value);
    }
  }
  swap = *d;
  *d = *d2;
  *d2 = swap;
  DictDestroy(d2);
}
void DictInsert(Dict d, unsigned long *key, const char*value){
  
}
unsigned long *DictSearch(Dict d, unsigned long *key){
  struct entry *e;
  for(e=d->table[hash_function(key) % d->size]; e != 0; e=e->next){
    if(e->key==key){
      return e->value;
    }
  }
  return 0;
}
void DictDelete(Dict d, unsigned long *key){
  struct entry **prev;
  struct entry *e;
  
  for(prev =&(d->table[hash_function(key)%d->size]); *prev !=0; prev = &((*prev)->next)){
    if((*prev)->key==key){
      e=*prev;
      *prev = e->next;
      free(e->key);
      free(e->value);
      free(e);
      return;
    }
}


#define INITIAL_SIZE (1024)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)

unsigned long * MMU(unsigned long * vaddr){
  dictGet(TLB,vaddr)
}
/*
static void  MakeStartupTables(unsigned long page_table[])
{
long result;
for (i=0; i<PAGES_PER_REGION; i++){
  pfn=PAGESIZE*i+VMEM_BASE+1;
  result = 0+prot+pfn;
  page_table[i]=result;
    }
for (i=PAGES_PER_REGION; i<2*PAGES_PER_REGION; i++){
  pfn=PAGESIZE*i+VMEM_BASE+1;
  result = 1+prot+pfn;
  page_table[i]=result;
    }
} 
*/ 

unsigned long TLB(unsigned long page_table[], unsigned long vaddr){
  page=(vaddr-VMEM_BASE)%PAGESIZE
  paddr=page_table[page]
  return paddr;
}
unsigned long MMU(unsigned long page_table[], unsigned long vaddr){
  page=(vaddr-VMEM_BASE)%PAGESIZE
  paddr=page_table[page]
  return paddr;
}
