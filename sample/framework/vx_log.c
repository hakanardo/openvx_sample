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

#include <vx_internal.h>

VX_API_ENTRY void VX_API_CALL vxRegisterLogCallback(vx_context context, vx_log_callback_f callback, vx_bool reentrant)
{
    vx_context_t *cntxt = (vx_context_t *)context;
    if (vxIsValidContext(cntxt) == vx_true_e)
    {
        vxSemWait(&cntxt->base.lock);
        if ((cntxt->log_callback == NULL) && (callback != NULL))
        {
            cntxt->log_enabled = vx_true_e;
            if (reentrant == vx_false_e)
            {
                vxCreateSem(&cntxt->log_lock, 1);
            }
            cntxt->log_reentrant = reentrant;
        }
        if ((cntxt->log_callback != NULL) && (callback == NULL))
        {
            if (cntxt->log_reentrant == vx_false_e)
            {
                vxDestroySem(&cntxt->log_lock);
            }
            cntxt->log_enabled = vx_false_e;
        }
        if ((cntxt->log_callback != NULL) && (callback != NULL) && (cntxt->log_callback != callback))
        {
            if (cntxt->log_reentrant == vx_false_e)
            {
                vxDestroySem(&cntxt->log_lock);
            }
            if (reentrant == vx_false_e)
            {
                vxCreateSem(&cntxt->log_lock, 1);
            }
            cntxt->log_reentrant = reentrant;
        }
        cntxt->log_callback = callback;
        vxSemPost(&cntxt->base.lock);
    }
}

VX_API_ENTRY void VX_API_CALL vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    va_list ap;
    vx_context_t *context = NULL;
    vx_char string[VX_MAX_LOG_MESSAGE_LEN];

    if (vxIsValidReference(ref) == vx_false_e)
    {
        if (vxIsValidContext((vx_context_t *)ref) == vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid reference!\n");
            return;
        }
    }

    if (status == VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid status code!\n");
        return;
    }

    if (message == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid message!\n");
        return;
    }

    if (ref->type == VX_TYPE_CONTEXT)
    {
        context = (vx_context_t *)ref;
    }
    else
    {
        context = ref->context;
    }

    if (context->log_callback == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "No callback is registered\n");
        return;
    }

    if (context->log_enabled == vx_false_e)
    {
        return;
    }

    va_start(ap, message);
    vsnprintf(string, VX_MAX_LOG_MESSAGE_LEN, message, ap);
    string[VX_MAX_LOG_MESSAGE_LEN-1] = 0; // for MSVC which is not C99 compliant
    va_end(ap);
    if (context->log_reentrant == vx_false_e)
        vxSemWait(&context->log_lock);
    context->log_callback(context, ref, status, string);
    if (context->log_reentrant == vx_false_e)
        vxSemPost(&context->log_lock);
    return;
}

