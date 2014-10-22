#define INITIAL_SIZE (1024)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)
struct entry{
  struct entry *next;
  unsigned long *key;
  unsigned long *value;
}
/* look aside table*/
struct lat{
  int size;
  int n;
  struct entry **table;
}

Lat MakeLat(int size);
/*
  Lat a;
  int i;
  a = malloc(sizeof(*a))

  assert a is not empty
  
  a's size= size
  a's n=0;
  a's table = malloc(sizeof(entry) * size of a)

  assert table is not empty
  
  for(i=0; i<size; i++)
  a's table[i]=0;
  return a
*/

void ClearLat(Lat a);
/*
  int i;
  pointer to an entry=e;
  pointer to the next entry=next;
  for(i=0;i<a's size; i++){
  for(e=a's table[i]; e!=0; e=next){
  next=e's next, free e's values and e
  }}
  free a's table
  free a
 */

static void grow(Lat a);
/*
  Dict b;
  struct lat swap;
  int i;
  pointer to entry e;
  b=MakeLat(a's size*multiplier)
  for(i=0; i<a's size; i++){
  for(e=a's table[i]; e!=0; e=e's next)
  InsertLatEntry(b,e's key, e's value); 
  }}
  swap=a;
  a=b;
  b=swap
  Clear(b)

 */

void InsertLatEntry(Lat a, unsigned long *key, unsigned long *value);
/*
  e= new entry;
  e's key=key;
  e's value=value;
  if SearchLat(key)!=0;
{
  DeleteLatEntry(a,SearchLat(key))
}
  a's table[a's size]=curr_entry;
  next_entry=curr_entry's next;
  next_entry's previous -> e;
  curr_entry's next -> e;

  }}
 */
unsigned long *SearchLat(Lat a, unsigned long *key);
/*
  for(e=a's table[key%497%a's size]; e!=0; e=e's next){
  if(e's key==key)
  return e's value
  }
  return 0 if not found
 */
void DeleteLatEntry(Lat a, unsigned long *key);
/*
  for(prev =a's table[key%497%a's size]; prev!=0; prev=prev's next){
  if(prev's key==key){
  e=prev;
  prev=e's next;
  free e's values and e
  return
  }
 */
