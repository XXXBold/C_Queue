#ifndef QUEUE_H_INCLUDED
  #define QUEUE_H_INCLUDED

  #include "booltype.h"

  #define QUEUE_INCLUDE_THREADSAFETY /* Define to make this library Thread-Safe */

typedef struct TagQueue_t TagQueue;

/* Queue Options */
#define QUEUE_OPTION_SYNCHRONIZED 0x1

/**
 * Creates a New Queue
 *
 * @param uiOptions _IN_ Options for the Queue, 0 for default.
 *
 * @return New Queue or NULL if failed
 */
TagQueue *ptagQueue_New_g(unsigned int uiOptions);

/**
 * Pushes an Item to the Queue
 *
 * @param ptagQueue _IN_ Queue to push the item to
 * @param pvData    _IN_ The item to push
 * @param iDataSize _IN_ Size of the item, in bytes
 * @param bCopyDataToNewBuffer
 *                  _IN_ true to allocate a new buffer for the data, <br>
 *                  false to store the Pointer of pvData directly.
 *                  In this case, pvData MUST be dynamically allocated.
 *
 * @return true on success, else false
 */
bool bQueue_Push_g(TagQueue *ptagQueue,
                   void *pvData,
                   int iDataSize,
                   bool bCopyDataToNewBuffer);

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
 * @return The Size of the next Element in Bytes or < 0 if failed
 */
int iQueue_GetNextElementSize_g(TagQueue *ptagQueue);


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
 * Pops the next item from the Queue and stores it in pvData.
 * if pvData is NULL, the data is just popped out and freed.
 *
 * @param ptagQueue  _IN_ The Queue
 * @param ppvData    _IN_OUT_ Pointer to Pointer to void, will be filled with next available data.
 * @param piDataSize _IN_OUT_ Size of ppvData, will be set to the size of ppvData.
 * @param piItemsRemaining
 *                   _OUT_ Remaining Items in Queue, can be NULL if not needed.
 * @param bCopyData  _IN_ If true, data will be copied to ppvData, must be preallocated then.
 *                   Else ppvData will be set to the original pointer of the data, must be freed after use.
 *
 * @return true on success, else false
 */
bool bQueue_Pop_g(TagQueue *ptagQueue,
                  void **ppvData,
                  int *piDataSize,
                  int *piItemsRemaining,
                  bool bCopyData);

/**
 * Allows to Peek the next element's data in the Queue, without removing it.
 *
 * @param ptagQueue  _IN_ The Queue
 * @param ppvData     _OUT_ Target pointer for the data, see bCopyData.
 * @param piDataSize _IN_OUT_ Pass size of pvData, will be set to the size of next element's data.
 * @param bCopyData  _IN_ true copies data to ppvData, if it's large enough, false will set it to the original Pointer.
 *                   Don't Modify then!
 *
 * @return true on success, else false.
 */
bool bQueue_Peek_g(TagQueue *ptagQueue,
                   void **pvData,
                   int *piDataSize,
                   bool bCopyData);


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

