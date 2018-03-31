#ifndef QUEUE_H_INCLUDED
  #define QUEUE_H_INCLUDED

  #include <stdlib.h>
  #include "booltype.h"

  #define QUEUE_INCLUDE_THREADSAFETY /* Define to make this library Thread-Safe */

typedef struct TagQueue_t TagQueue;

/* Queue Options */
#define QUEUE_OPTION_SYNCHRONIZED 0x1

/**
 * Used if you add new Data to a Queue.
 */
typedef enum
{
  /**
   * Will use the passed pointer directly and add it to the Queue.
   * In this case, it must be allocated using pvQueue_AllocateElement_g.
   */
  eQueue_DataUseDirectly,
  /**
   * Will allocate a new Buffer for pvData, and copy the data there.
   * Use this for all Data not allocated by pvQueue_AllocateElement_g.
   */
  eQueue_DataUseCopy
}EQueue_DataInfo;

/**
 * Creates a New Queue
 *
 * @param uiOptions _IN_ Options for the Queue, 0 for default.
 *
 * @return New Queue or NULL if failed
 */
TagQueue *ptagQueue_New_g(unsigned int uiOptions);

void *pvQueue_AllocateElement_g(size_t sSize);

/**
 * Pushes an Item to the Queue
 *
 * @param ptagQueue _IN_ Queue to push the item to
 * @param pvData    _IN_ The item to push
 * @param sDataSize _IN_ Size of the item, in bytes
 * @param eDataInfo Info how to handle pvData, see enum.
 *
 * @return true on success, else false
 */
bool bQueue_Push_g(TagQueue *ptagQueue,
                   void *pvData,
                   size_t sDataSize,
                   EQueue_DataInfo eDataInfo);

/**
 * Returns the count of elements in the Queue
 *
 * @param ptagQueue _IN_ The Queue
 *
 * @return Elements count or < 0 if failed
 */
int iQueue_GetElementsCount_g(TagQueue *ptagQueue);

/**
 * Returns the size of the next Element in the Queue, in bytes.
 *
 * @param ptagQueue _IN_ The Queue
 *
 * @return The Size of the next Element in Bytes or 0 if no Element present
 */
size_t sQueue_GetNextElementSize_g(TagQueue *ptagQueue);


/**
 * Deletes the next element in the Queue
 *
 * @param ptagQueue _IN_ The Queue
 * @param piItemsRemaining
 *                  _OUT_ Items remaining in Queue, pass NULL if not needed.
 *
 * @return true on success, else false.
 */
bool bQueue_DeleteNextElement_g(TagQueue *ptagQueue,
                                int *piItemsRemaining);

/**
 * Pops the next item from the Queue and stores it in ppvData.
 *
 * @param ptagQueue  _IN_ The Queue
 * @param ppvData    _OUT_ Pointer to Pointer to void, will be set to data. Free after use!
 * @param psDataSize _IN_OUT_ Size of ppvData, will be set to the size of ppvData.
 * @param piItemsRemaining
 *                   _OUT_ Remaining Items in Queue, pass NULL if not needed.
 *
 * @return true on success, else false
 */
bool bQueue_Pop_g(TagQueue *ptagQueue,
                  void **ppvData,
                  size_t *psDataSize,
                  int *piItemsRemaining);

/**
 * Allows to Peek the next element's data in the Queue, without removing it.
 * Do not modify or free ppvData!
 *
 * @param ptagQueue  _IN_ The Queue
 * @param ppvData    _OUT_ Pointer will be set to data.
 * @param psDataSize _OUT_ Will be set to the size of ppvData.
 *
 * @return true on success, else false.
 */
bool bQueue_Peek_g(TagQueue *ptagQueue,
                   void **ppvData,
                   size_t *psDataSize);


/**
 * Deletes all Elements in the Queue.
 *
 * @param ptagQueue _IN_ The Queue to clear
 */
void vQueue_Clear_g(TagQueue *ptagQueue);

/**
 * Deletes a Queue and all Elements it contains
 *
 * @param ptagQueue _IN_ Queue to delete
 */
void vQueue_Delete_g(TagQueue *ptagQueue);

/**
 * Prints out all Elements on a Queue,
 * for testing purposes
 *
 * @param ptagQueue _IN_ Queue to dump
 */
void vQueue_DumpElements_g(TagQueue *ptagQueue);

#endif /* !QUEUE_H_INCLUDED */

