#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "booltype.h"
#include "Queue.h"
#ifdef QUEUE_INCLUDE_THREADSAFETY
  #include <pthread.h>
#endif /* QUEUE_INCLUDE_THREADSAFETY */

/**
 * Datasize is signed int, so the highest byte is not used to indicate the size.
 * It's used internally to indicate whenever there's just one block allocated for Element+data.
 */
#define QUEUE_USE_ONE_MEMORY_BLOCK_IS_SET(var)  (var&(1<<((sizeof(int)*CHAR_BIT)-1)))
#define QUEUE_USE_ONE_MEMORY_BLOCK_SET(var)     (var|(1<<((sizeof(int)*CHAR_BIT)-1)))
#define QUEUE_USE_ONE_MEMORY_BLOCK_UNSET(var)   (var&(~(1<<((sizeof(int)*CHAR_BIT)-1))))

#ifdef QUEUE_INCLUDE_THREADSAFETY
  #define QUEUE_MUTEX_INIT(opt,m)    if(opt&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_init(m,NULL)
  #define QUEUE_MUTEX_DESTROY(opt,m) if(opt&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_destroy(m)
  #define QUEUE_MUTEX_LOCK(opt,m)    if(opt&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_lock(m)
  #define QUEUE_MUTEX_UNLOCK(opt,m)  if(opt&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_unlock(m)
#else /* !QUEUE_INCLUDE_THREADSAFETY */
  #define QUEUE_MUTEX_INIT(opt,m)
  #define QUEUE_MUTEX_DESTROY(opt,m)
  #define QUEUE_MUTEX_LOCK(opt,m)
  #define QUEUE_MUTEX_UNLOCK(opt,m)
#endif /* QUEUE_INCLUDE_THREADSAFETY */

#define QUEUE_FORCE_INLINE

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #ifdef QUEUE_FORCE_INLINE
    #define INLINE_FCT_LOCAL __attribute__((always_inline)) inline
    #define INLINE_PROT_LOCAL __attribute__((always_inline)) static inline
  #else /* !QUEUE_FORCE_INLINE */
    #define INLINE_FCT_LOCAL inline
    #define INLINE_PROT_LOCAL static inline
  #endif /* QUEUE_FORCE_INLINE */
  #else /* No inline available from C Standard */
    #define INLINE_FCT_LOCAL
    #define INLINE_PROT_LOCAL
#endif /* __STDC_VERSION__ >= C99 */

struct TagQueueElement_t
{
  struct TagQueueElement_t *ptagNext;
  void *pvData;
  int iDataSize;
};

typedef struct TagQueueElement_t TagQueueElement;

struct TagQueue_t
{
  int iItemCount;
  unsigned int uiQueueOptions;
  struct TagQueueElement_t *ptagFirstElement;
  struct TagQueueElement_t *ptagLastElement;
#ifdef QUEUE_INCLUDE_THREADSAFETY
  pthread_mutex_t tagQueueMutex;
#endif /* QUEUE_INCLUDE_THREADSAFETY */
};

INLINE_PROT_LOCAL void vQueue_ElementsDelete_m(TagQueue *ptagQueue);

TagQueue *ptagQueue_New_g(unsigned int uiOptions) // TODO: Implement more otions
{
  TagQueue *ptagNewQueue;
#ifndef QUEUE_INCLUDE_THREADSAFETY
  /* If Threadsafe stuff is not included, option synchronized is not allowed */
  if(uiOptions & QUEUE_OPTION_SYNCHRONIZED)
    return(NULL);
#endif /* !QUEUE_INCLUDE_THREADSAFETY */

  if(!(ptagNewQueue=malloc(sizeof(TagQueue))))
    return(NULL);

  ptagNewQueue->iItemCount=0;
  ptagNewQueue->ptagFirstElement=NULL;
  ptagNewQueue->ptagLastElement=NULL;
  ptagNewQueue->uiQueueOptions=uiOptions;
  QUEUE_MUTEX_INIT(uiOptions,&ptagNewQueue->tagQueueMutex);
  return(ptagNewQueue);
}


bool bQueue_Push_g(TagQueue *ptagQueue,
                   void *pvData,
                   int iDataSize,
                   bool bCopyDataToNewBuffer)
{
  TagQueueElement *ptagNewElement;
  unsigned char *pucTmp;

  if(iDataSize<1)
    return(false);

  if(bCopyDataToNewBuffer)
  {
    if(!(pucTmp=malloc(sizeof(TagQueueElement)+iDataSize)))
      return(false);
    ptagNewElement=(TagQueueElement*)&pucTmp[iDataSize];
    ptagNewElement->pvData=pucTmp;
    memcpy(ptagNewElement->pvData,pvData,iDataSize);
    iDataSize=QUEUE_USE_ONE_MEMORY_BLOCK_SET(iDataSize);
  }
  else
  {
    if(!(ptagNewElement=malloc(sizeof(TagQueueElement))))
      return(false);
    ptagNewElement->pvData=pvData;
  }
  ptagNewElement->iDataSize=iDataSize;
  ptagNewElement->ptagNext=NULL;

  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  /* Check if first element */
  if(!ptagQueue->ptagFirstElement)
  {
    ptagQueue->ptagFirstElement=ptagNewElement;
    ptagQueue->ptagLastElement=ptagNewElement;
  }
  else
  {
    ptagQueue->ptagLastElement->ptagNext=ptagNewElement;
    ptagQueue->ptagLastElement=ptagNewElement;
  }
  ++ptagQueue->iItemCount;
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  return(true);
}


int iQueue_GetElementsCount_g(TagQueue *ptagQueue)
{
  return(ptagQueue->iItemCount);
}


int iQueue_GetNextElementSize_g(TagQueue *ptagQueue)
{
  if(!ptagQueue->ptagFirstElement)
    return(-1);
  return(QUEUE_USE_ONE_MEMORY_BLOCK_UNSET(ptagQueue->ptagFirstElement->iDataSize));
}

bool bQueue_DeleteNextElement_g(TagQueue *ptagQueue,
                                int *piItemsRemaining)
{
  TagQueueElement *ptagNewFirstElement;

  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  if(!ptagQueue->ptagFirstElement)
  {
    QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
    return(false);
  }

  ptagNewFirstElement=ptagQueue->ptagFirstElement->ptagNext;
  if(QUEUE_USE_ONE_MEMORY_BLOCK_IS_SET(ptagQueue->ptagFirstElement->iDataSize))
  {
    free(ptagQueue->ptagFirstElement->pvData);
  }
  else
  {
    free(ptagQueue->ptagFirstElement->pvData);
    free(ptagQueue->ptagFirstElement);
  }
  ptagQueue->ptagFirstElement=ptagNewFirstElement;
  --ptagQueue->iItemCount;
  if(piItemsRemaining)
    *piItemsRemaining=ptagQueue->iItemCount;
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  return(true);
}

bool bQueue_Pop_g(TagQueue *ptagQueue,
                  void **ppvData,
                  int *piDataSize,
                  int *piItemsRemaining,
                  bool bCopyData)
{
  TagQueueElement *ptagNewFirstElement;

  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  /* Check if Queue is empty */
  if(!ptagQueue->ptagFirstElement)
  {
    QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
    return(false);
  }
  ptagNewFirstElement=ptagQueue->ptagFirstElement->ptagNext;
  if(bCopyData)
  {
    if(*piDataSize < QUEUE_USE_ONE_MEMORY_BLOCK_UNSET(ptagQueue->ptagFirstElement->iDataSize))
    {
      QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
      return(false);
    }
    *piDataSize=QUEUE_USE_ONE_MEMORY_BLOCK_UNSET(ptagQueue->ptagFirstElement->iDataSize);
    memcpy(*ppvData,ptagQueue->ptagFirstElement->pvData,*piDataSize);
    if(QUEUE_USE_ONE_MEMORY_BLOCK_IS_SET(ptagQueue->ptagFirstElement->iDataSize))
    {
      free(ptagQueue->ptagFirstElement->pvData);
    }
    else
    {
      free(ptagQueue->ptagFirstElement->pvData);
      free(ptagQueue->ptagFirstElement);
    }
  }
  else
  {
    *ppvData=ptagQueue->ptagFirstElement->pvData;
    *piDataSize=QUEUE_USE_ONE_MEMORY_BLOCK_UNSET(ptagQueue->ptagFirstElement->iDataSize);
    if(!QUEUE_USE_ONE_MEMORY_BLOCK_IS_SET(ptagQueue->ptagFirstElement->iDataSize))
    {
      free(ptagQueue->ptagFirstElement);
    }
  }
  ptagQueue->ptagFirstElement=ptagNewFirstElement;
  --ptagQueue->iItemCount;
  if(piItemsRemaining)
    *piItemsRemaining=ptagQueue->iItemCount;
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  return(true);
}

bool bQueue_Peek_g(TagQueue *ptagQueue,
                   void **ppvData,
                   int *piDataSize,
                   bool bCopyData)
{
  if(!ptagQueue->ptagFirstElement)
    return(false);

  if(!bCopyData)
  {
    *ppvData=ptagQueue->ptagFirstElement->pvData;
    *piDataSize=ptagQueue->ptagFirstElement->iDataSize;
    return(true);
  }
  if(*piDataSize < ptagQueue->iItemCount)
    return(false);

  memcpy(*ppvData,ptagQueue->ptagFirstElement->pvData,ptagQueue->ptagFirstElement->iDataSize);
  *piDataSize=ptagQueue->ptagFirstElement->iDataSize;
  return(true);
}

INLINE_FCT_LOCAL void vQueue_ElementsDelete_m(TagQueue *ptagQueue)
{
  TagQueueElement *ptagCurrElement;
  TagQueueElement *ptagNextElement;
  ptagCurrElement=ptagQueue->ptagFirstElement;
  while(ptagCurrElement!=NULL)
  {
    if(QUEUE_USE_ONE_MEMORY_BLOCK_IS_SET(ptagCurrElement->iDataSize))
    {
      ptagNextElement=ptagCurrElement->ptagNext;
      free(ptagCurrElement->pvData);
    }
    else
    {
      ptagNextElement=ptagCurrElement->ptagNext;
      free(ptagCurrElement->pvData);
      free(ptagCurrElement);
    }
    ptagCurrElement=ptagNextElement;
  }
}

void vQueue_Clear_g(TagQueue *ptagQueue)
{
  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  vQueue_ElementsDelete_m(ptagQueue);
  ptagQueue->iItemCount=0;
  ptagQueue->ptagFirstElement=NULL;
  ptagQueue->ptagLastElement=NULL;
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
}

void vQueue_Delete_g(TagQueue *ptagQueue)
{
  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  vQueue_ElementsDelete_m(ptagQueue);
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  QUEUE_MUTEX_DESTROY(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
  free(ptagQueue);
}


void vQueue_DumpElements_g(TagQueue *ptagQueue)
{
  TagQueueElement *ptagElement;
  char caBuffer[256];
  QUEUE_MUTEX_LOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);

  sprintf(caBuffer,
          "****DUMPING_QUEUE****\n"
          "Queue 0x%p with %d elements, first element: 0x%p\n",
          ptagQueue,
          ptagQueue->iItemCount,
          ptagQueue->ptagFirstElement);

  puts(caBuffer);

  for(ptagElement=ptagQueue->ptagFirstElement;
      ptagElement!=NULL;
      ptagElement=ptagElement->ptagNext)
  {

    sprintf(caBuffer,
            "***********\n"
            "0x%p->0x%p\n"
            "***********\n"
            "0x%p\n"
            "*****v*****\n",
            ptagElement,
            ptagElement->pvData,
            ptagElement->ptagNext);

    puts(caBuffer);
  }
  QUEUE_MUTEX_UNLOCK(ptagQueue->uiQueueOptions,&ptagQueue->tagQueueMutex);
}
