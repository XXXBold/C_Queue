#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  #define TEST_QUEUE_THREADS /* Enable to test multithreaded */

#include "booltype.h"
#include "Queue.h"
#ifdef TEST_QUEUE_THREADS
  #include <pthread.h>
  #define PUSH_THREADS 100
  #define POP_THREADS 2
  bool bExit_PopThreads_m;
#endif /* TEST_QUEUE_THREADS */

#define QUEUE_ITEMS_ADD 0xFFFFFF

void vQueue_Test_g(void);
void vQueue_TestThreads_g(void);
void *pvThread_Push_m(void *pvArg);
void *pvThread_Pop_m(void *pvArg);

char pcData[10][10]=
{
  "Anna",
  "Bertha",
  "Caesar",
  "Daedalus",
  "Eugen",
  "Ferdinand",
  "Guido",
  "Hugo",
  "Ida",
  "Jakob"
};


int main(int argc, char *argv[])
{
  (void)argc; (void)argv;
  puts("Starting Test...");
  vQueue_Test_g();
#ifdef TEST_QUEUE_THREADS
  puts("Testing Multithreaded queues...");
  vQueue_TestThreads_g();
#endif
  puts("Test passed!");
  return(0);
}

void vQueue_Test_g(void)
{
  void *pvData;
  int iSize=0;
  int iIndex;
  TagQueue *ptagQueue;
  if(!(ptagQueue=ptagQueue_New_g(0)))
  {
    puts("ptagQueue_New_g() failed");
    exit(-1);
  }
  for(iIndex=0; iIndex<QUEUE_ITEMS_ADD; ++iIndex)
  {
    if(!bQueue_Push_g(ptagQueue,pcData[0],strlen(pcData[0])+1,true))
    {
      puts("push failed");
      exit(-1);
    }
  }
  for(iIndex=0;iIndex<QUEUE_ITEMS_ADD;++iIndex)
  {
    if(iIndex%2)
    {
      if(!bQueue_Pop_g(ptagQueue,&pvData,&iSize,NULL,false))
      {
        puts("bQueue_Pop_g() failed");
        exit(-1);
      }
      free(pvData);
    }
    else
    {
      if(!bQueue_DeleteNextElement_g(ptagQueue,NULL))
      {
        puts("bQueue_DeleteNextElement_g() failed");
        exit(-1);
      }
    }
  }
  printf("Queue Elements left = %d\n",iQueue_GetElementsCount_g(ptagQueue));

  vQueue_Delete_g(ptagQueue);
}

#ifdef TEST_QUEUE_THREADS
void vQueue_TestThreads_g(void)
{
  int iIndex;
  TagQueue *ptagQueue;
  pthread_t threadsPush[PUSH_THREADS];
  pthread_t threadsPop[POP_THREADS];
  if(!(ptagQueue = ptagQueue_New_g(QUEUE_OPTION_SYNCHRONIZED)))
  {
    puts("ptagQueue_New_g() failed");
    exit(-1);
  }
  bExit_PopThreads_m=false;

  for(iIndex=0;iIndex<PUSH_THREADS;++iIndex)
  {
    if(pthread_create(&threadsPush[iIndex],NULL,pvThread_Push_m,ptagQueue))
    {
      printf("pthread_create() failed\n");
      exit(-1);
    }
  }
  for(iIndex=0;iIndex<POP_THREADS;++iIndex)
  {
    if(pthread_create(&threadsPop[iIndex],NULL,pvThread_Pop_m,ptagQueue))
    {
      printf("pthread_create() failed\n");
      exit(-1);
    }
  }
  for(iIndex=0;iIndex<PUSH_THREADS;++iIndex)
  {
    if(pthread_join(threadsPush[iIndex],NULL))
    {
      printf("pthread_join() failed\n");
      exit(-1);
    }
  }
  bExit_PopThreads_m=true;
  for(iIndex=0;iIndex<POP_THREADS;++iIndex)
  {
    if(pthread_join(threadsPop[iIndex],NULL))
    {
      printf("pthread_join() failed\n");
      exit(-1);
    }
  }
  printf("Queue Elements left = %d\n",iQueue_GetElementsCount_g(ptagQueue));

  vQueue_Delete_g(ptagQueue);
}

void *pvThread_Push_m(void *pvArg)
{
  TagQueue *ptagQueue=(TagQueue*)pvArg;
  int iIndex;
  for(iIndex=0;iIndex<100;iIndex++)
  {
//  printf("---> _%s_ %d\n",pcData[iIndex/10],strlen(pcData[iIndex/10])+1);
    if(!bQueue_Push_g(ptagQueue,pcData[iIndex/10],strlen(pcData[iIndex/10])+1,true))
      printf("push failed\n");
  }
  return(NULL);
}

void *pvThread_Pop_m(void *pvArg)
{
  TagQueue *ptagQueue=(TagQueue*)pvArg;
  void *pvData;
  int iBufferSize=0;
  bool bGotData;

  while(((bGotData=bQueue_Pop_g(ptagQueue,&pvData,&iBufferSize,NULL,false))) || (!bExit_PopThreads_m))
  {
    if(!bGotData)
      continue;
//  printf("<--- _%s_\n",(char*)pvData);
    free(pvData);
  }
  return(NULL);
}
#endif
