#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "booltype.h"
#include "Queue.h"
#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #define __STDC_FORMAT_MACROS
  #include <inttypes.h>
  #ifdef _WIN32
    #ifdef _WIN64
      #define PRI_SIZET PRIu64
    #else
      #define PRI_SIZET PRIu32
    #endif
  #else
    #define PRI_SIZET "zu"
  #endif
#else
  #define PRI_SIZET "%lu"
#endif /* __STDC_VERSION__ >= C99 */
#ifdef QUEUE_INCLUDE_THREADSAFETY
  #include <pthread.h>
#endif /* QUEUE_INCLUDE_THREADSAFETY */

#ifdef QUEUE_INCLUDE_THREADSAFETY
  #define QUEUE_MUTEX_INIT(queue)    if(queue->uiQueueOptions&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_init(&queue->tagQueueMutex,NULL)
  #define QUEUE_MUTEX_DESTROY(queue) if(queue->uiQueueOptions&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_destroy(&queue->tagQueueMutex)
  #define QUEUE_MUTEX_LOCK(queue)    if(queue->uiQueueOptions&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_lock(&queue->tagQueueMutex)
  #define QUEUE_MUTEX_UNLOCK(queue)  if(queue->uiQueueOptions&QUEUE_OPTION_SYNCHRONIZED) pthread_mutex_unlock(&queue->tagQueueMutex)
#else /* !QUEUE_INCLUDE_THREADSAFETY */
  #define QUEUE_MUTEX_INIT(queue)
  #define QUEUE_MUTEX_DESTROY(queue)
  #define QUEUE_MUTEX_LOCK(queue)
  #define QUEUE_MUTEX_UNLOCK(queue)
#endif /* QUEUE_INCLUDE_THREADSAFETY */

#define QUEUE_FORCE_INLINE

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #ifdef QUEUE_FORCE_INLINE
    #define INLINE_FCT_LOCAL __attribute__((always_inline)) inline
    #define INLINE_PROT_LOCAL __attribute__((always_inline)) static inline
    #define INLINE_FCT_SHARED  inline __attribute__((always_inline))
    #define INLINE_PROT_SHARED  extern
  #else /* !QUEUE_FORCE_INLINE */
    #define INLINE_FCT_LOCAL inline
    #define INLINE_PROT_LOCAL static inline
    #define INLINE_FCT_SHARED  inline
    #define INLINE_PROT_SHARED extern
  #endif /* QUEUE_FORCE_INLINE */
  #else /* No inline available from C Standard */
    #define INLINE_FCT_LOCAL
    #define INLINE_PROT_LOCAL
    #define INLINE_FCT_SHARED  inline
    #define INLINE_PROT_SHARED extern
#endif /* __STDC_VERSION__ >= C99 */


struct TagQueueElement_t
{
  struct TagQueueElement_t *ptagNext;
  void *pvData;
  size_t sDataSize;
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
  QUEUE_MUTEX_INIT(ptagNewQueue);
  return(ptagNewQueue);
}

void *pvQueue_AllocateElement_g(size_t sSize)
{
  return(malloc(sSize+sizeof(TagQueueElement)));
}

bool bQueue_Push_g(TagQueue *ptagQueue,
                   void *pvData,
                   size_t sDataSize,
                   EQueue_DataInfo eDataInfo)
{
  void *pvDataToAdd=pvData;
  TagQueueElement *ptagNewElement;

  if(eDataInfo==eQueue_DataUseCopy)
  {
    if(!(pvDataToAdd=malloc(sDataSize+sizeof(TagQueueElement))))
      return(false);
    memcpy(pvDataToAdd,pvData,sDataSize);
  }
  ptagNewElement=(TagQueueElement*)(((char*)pvDataToAdd)+sDataSize);
  ptagNewElement->sDataSize=sDataSize;
  ptagNewElement->pvData=pvDataToAdd;
  ptagNewElement->ptagNext=NULL;
  printf("***QUEUE 0x%p: Adding Msg data 0x%p,to datap ->0x%p, size: %" PRI_SIZET "\n",ptagQueue,pvData,pvDataToAdd,sDataSize);   // TODO: remove this

  QUEUE_MUTEX_LOCK(ptagQueue);
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
  QUEUE_MUTEX_UNLOCK(ptagQueue);
  return(true);
}


int iQueue_GetElementsCount_g(TagQueue *ptagQueue)
{
  return(ptagQueue->iItemCount);
}


size_t sQueue_GetNextElementSize_g(TagQueue *ptagQueue)
{
  if(!ptagQueue->ptagFirstElement)
    return(0);
  return(ptagQueue->ptagFirstElement->sDataSize);
}

bool bQueue_DeleteNextElement_g(TagQueue *ptagQueue,
                                int *piItemsRemaining)
{
  TagQueueElement *ptagNewFirstElement;

  QUEUE_MUTEX_LOCK(ptagQueue);
  if(!ptagQueue->ptagFirstElement)
  {
    QUEUE_MUTEX_UNLOCK(ptagQueue);
    return(false);
  }

  ptagNewFirstElement=ptagQueue->ptagFirstElement->ptagNext;
  free(ptagQueue->ptagFirstElement->pvData);
  ptagQueue->ptagFirstElement=ptagNewFirstElement;
  --ptagQueue->iItemCount;
  if(piItemsRemaining)
    *piItemsRemaining=ptagQueue->iItemCount;
  QUEUE_MUTEX_UNLOCK(ptagQueue);
  return(true);
}

bool bQueue_Pop_g(TagQueue *ptagQueue,
                  void **ppvData,
                  size_t *psDataSize,
                  int *piItemsRemaining)
{
  TagQueueElement *ptagNewFirstElement;

  /* Check if Queue is empty */
  QUEUE_MUTEX_LOCK(ptagQueue);
  if(!ptagQueue->ptagFirstElement)
  {
    QUEUE_MUTEX_UNLOCK(ptagQueue);
    return(false);
  }

  printf("QUEUE 0x%p: ***getting data 0x%p, size: %" PRI_SIZET "\n",ptagQueue,ptagQueue->ptagFirstElement->pvData,ptagQueue->ptagFirstElement->sDataSize);   // TODO: remove this
  ptagNewFirstElement=ptagQueue->ptagFirstElement->ptagNext;
  *ppvData=ptagQueue->ptagFirstElement->pvData;
  *psDataSize=ptagQueue->ptagFirstElement->sDataSize;
  ptagQueue->ptagFirstElement=ptagNewFirstElement;
  --ptagQueue->iItemCount;
  if(piItemsRemaining)
    *piItemsRemaining=ptagQueue->iItemCount;
  QUEUE_MUTEX_UNLOCK(ptagQueue);
  return(true);
}

bool bQueue_Peek_g(TagQueue *ptagQueue,
                   void **ppvData,
                   size_t *psDataSize)
{
  QUEUE_MUTEX_LOCK(ptagQueue);
  if(!ptagQueue->ptagFirstElement)
  {
    QUEUE_MUTEX_UNLOCK(ptagQueue);
    return(false);
  }

  *ppvData=ptagQueue->ptagFirstElement->pvData;
  *psDataSize=ptagQueue->ptagFirstElement->sDataSize;
  QUEUE_MUTEX_UNLOCK(ptagQueue);
  return(true);
}

INLINE_FCT_LOCAL void vQueue_ElementsDelete_m(TagQueue *ptagQueue)
{
  TagQueueElement *ptagCurrElement;
  TagQueueElement *ptagNextElement;
  ptagCurrElement=ptagQueue->ptagFirstElement;
  while(ptagCurrElement!=NULL)
  {
    ptagNextElement=ptagCurrElement->ptagNext;
    free(ptagCurrElement->pvData);
    ptagCurrElement=ptagNextElement;
  }
}

void vQueue_Clear_g(TagQueue *ptagQueue)
{
  QUEUE_MUTEX_LOCK(ptagQueue);
  vQueue_ElementsDelete_m(ptagQueue);
  ptagQueue->iItemCount=0;
  ptagQueue->ptagFirstElement=NULL;
  ptagQueue->ptagLastElement=NULL;
  QUEUE_MUTEX_UNLOCK(ptagQueue);
}

void vQueue_Delete_g(TagQueue *ptagQueue)
{
  QUEUE_MUTEX_LOCK(ptagQueue);
  vQueue_ElementsDelete_m(ptagQueue);
  QUEUE_MUTEX_UNLOCK(ptagQueue);
  QUEUE_MUTEX_DESTROY(ptagQueue);
  free(ptagQueue);
}

void vQueue_DumpElements_g(TagQueue *ptagQueue)
{
  TagQueueElement *ptagElement;
  char caBuffer[256];
  QUEUE_MUTEX_LOCK(ptagQueue);

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
  QUEUE_MUTEX_UNLOCK(ptagQueue);
}
