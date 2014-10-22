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

Lat internalLatCreate(int size);

Lat MakeLat(void);

void ClearLat(Lat d);

static void grow(Lat d);

void InsertLatEntry(Lat d, unsigned long *key, unsigned long *value);

unsigned long *SearchLat(Lat d, unsigned long *key);

void DeleteLatEntry(Lat d, unsigned long *key);
