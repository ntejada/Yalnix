#include "./map.h"

int
hashInit(MapData **hash_table)
{
  int i;
  for (i = 0; i < HASH_SIZE; i++)
    hash_table[i] = NULL;

  return SUCCESS;
}



// Modeled from Knuth's multiplicative method;
int 
hash(int value) 
{
  return value*2654435761 % 2^32;
}


MapData *
accessMap(int pid) 
{

  int hashval = hash(pid);
  int index = hashval % HASH_SIZE;

  MapData *md = hash_table[index];

  if (NULL == md) {
    TracePrintf(1, "MapData Node not found in accessNode\n");
    return md;
  }

  while (md != NULL && md->pid != pid)
    md = md->next;

  if (NULL == md)
    TracePrintf(1, "MapData Node not found in accessNode\n");
  return md;
}


int 
insertMap(MapData *md) 
{

  int hashval = hash(md->pid);
  int index = hashval % HASH_SIZE;

  MapData *temp_md = hash_table[index];

  if (NULL == temp_md)
    hash_table[index] = md;
  else {
    while(temp_md->next != NULL)
      temp_md = temp_md->next;
    temp_md->next = md; 
  }

  return SUCCESS;
}


int 
removeFromMap(int pid)
{

  int hashval = hash(pid);
  int index = hashval % HASH_SIZE;

  MapData *md = hash_table[index];
  if (NULL == md) 
    return ERROR;
  else if (md->pid == pid)
    hash_table[index] = md->next;
  else {
    while (md->next != NULL && md->next->pid != pid)
      md = md->next;
    
    if (NULL == md->next)
      return ERROR;
    
    MapData *temp = md->next;
    md = md->next->next;
    free(temp);
  }
  return SUCCESS;
}

MapData *
createMapData(int pid, void *kernelsp_1, void *kernelsp_2) 
{

  MapData *mp = malloc(sizeof(MapData));
  if (NULL == mp) {
    TracePrintf(1, "Malloc error, createMapData\n");
    return NULL;
  }

  mp->kernelsp_1 = kernelsp_1;
  mp->kernelsp_2 = kernelsp_2;

  mp->pid = pid;

  return mp;

}

int
addToMap(int pid, void *kernelsp_1, void *kernelsp_2)
{
	MapData* newPid = createMapData(pid, kernelsp_1, kernelsp_2);
	if(newPid == NULL) {
		TracePrintf(1, "Malloc error\n");
		return ERROR;
	}
	insertMap(newPid);
	return SUCCESS;
}



