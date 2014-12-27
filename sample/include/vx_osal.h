/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#ifndef _OPENVX_INT_OSAL_H_
#define _OPENVX_INT_OSAL_H_

/*!
 * \file
 * \brief The internal operating system abstraction layer.
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_osal Internal OSAL API
 * \ingroup group_internal
 * \brief The Internal Operating System Abstraction Layer API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a semaphore object and sets to a given count.
 * \ingroup group_int_osal
 */
vx_bool vxCreateSem(vx_sem_t *sem, vx_uint32 count);

/*! \brief Releases a semaphore object.
 * \ingroup group_int_osal
 */
void vxDestroySem(vx_sem_t *sem);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxSemPost(vx_sem_t *sem);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxSemWait(vx_sem_t *sem);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxSemTryWait(vx_sem_t *sem);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxJoinThread(vx_thread_t thread, vx_value_t *value);

/*! \brief
 * \ingroup group_int_osal
 */
vx_thread_t vxCreateThread(vx_thread_f func, void *arg);

/*! \brief
 * \ingroup group_int_osal
 */
void vxSleepThread(vx_uint32 milliseconds);

/*! \brief
 * \ingroup group_int_osal
 */
vx_uint64 vxCaptureTime();

/*! \brief
 * \ingroup group_int_osal
 */
vx_uint64 vxGetClockRate();

/*! \brief
 * \ingroup group_int_osal
 */
void vxStartCapture(vx_perf_t *perf);

/*! \brief
 * \ingroup group_int_osal
 */
void vxStopCapture(vx_perf_t *perf);

/*! \brief
 * \ingroup group_int_osal
 */
void vxInitPerf(vx_perf_t *perf);

/*! \brief Prints the performance information.
 * \ingroup group_int_osal
 */
void vxPrintPerf(vx_perf_t *perf);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxDeinitEvent(vx_event_t *e);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxInitEvent(vx_event_t *e, vx_bool autoreset);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxWaitEvent(vx_event_t *e, vx_uint32 timeout);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxSetEvent(vx_event_t *e);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxResetEvent(vx_event_t *e);

/*! \brief
 * \ingroup group_int_osal
 */
void vxInitQueue(vx_queue_t *q);

/*! \brief
 * \ingroup group_int_osal
 */
vx_queue_t *vxCreateQueue();

/*! \brief
 * \ingroup group_int_osal
 */
void vxDestroyQueue(vx_queue_t **pq);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxWriteQueue(vx_queue_t *q, vx_value_set_t *data);

/*! \brief
 * \ingroup group_int_osal
 */
vx_bool vxReadQueue(vx_queue_t *q, vx_value_set_t **data);

/*! \brief
 * \ingroup group_int_osal
 */
void vxPopQueue(vx_queue_t *q);

/*! \brief
 * \ingroup group_int_osal
 */
void vxDeinitQueue(vx_queue_t *q);

/*! \brief
 * \ingroup group_int_osal
 */
vx_module_handle_t vxLoadModule(vx_char * name);

/*! \brief
 * \ingroup group_int_osal
 */
void vxUnloadModule(vx_module_handle_t mod);

/*! \brief
 * \ingroup group_int_osal
 */
vx_symbol_t vxGetSymbol(vx_module_handle_t mod, vx_char * name);

/*! \brief Converts a vx_uint64 to a float in milliseconds.
 * \ingroup group_int_osal
 */
vx_float32 vxTimeToMS(vx_uint64 c);

void vxDestroyThreadpool(vx_threadpool_t **ppool);

vx_threadpool_t *vxCreateThreadpool(vx_uint32 numThreads,
                                    vx_uint32 numWorkItems,
                                    vx_size sizeWorkItem,
                                    vx_threadpool_f worker,
                                    void *arg);

vx_bool vxIssueThreadpool(vx_threadpool_t *pool, vx_value_set_t workitems[], uint32_t numWorkItems);

vx_bool vxCompleteThreadpool(vx_threadpool_t *pool, vx_bool blocking);

#ifdef __cplusplus
}
#endif

#endif
