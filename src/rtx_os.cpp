/*
 * Copyright (c) 2022, B. Leforestier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// This following part is specific to CMSIS-RTOS RTX implementation
#include <new>
#include <functional>
#include "rtx_os.h"

extern "C" void*    osRtxMemoryAlloc(void* mem, uint32_t size, uint32_t type);
extern "C" uint32_t osRtxMemoryFree (void* mem, void* block);

extern std::function<void()> idleHandler;

void* operator new(std::size_t count)
{
	void* ptr = osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
#ifdef __cpp_exceptions
	if (!ptr)
		throw std::bad_alloc();
#endif

	return ptr;
}

void* operator new[](std::size_t count)
{
	void* ptr = osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
#ifdef __cpp_exceptions
	if (!ptr)
		throw std::bad_alloc();
#endif

	return ptr;
}

void* operator new( std::size_t count, const std::nothrow_t&) noexcept
{
	return osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
}

void* operator new[]( std::size_t count, const std::nothrow_t&) noexcept
{
	return osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
}

void operator delete(void* ptr) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

void operator delete[](void* ptr) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

void operator delete( void* ptr, const std::nothrow_t&) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

void operator delete[]( void* ptr, const std::nothrow_t&) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

#if __cplusplus > 201103L
void operator delete(void* ptr, std::size_t) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept
{
	osRtxMemoryFree(osRtxInfo.mem.common, ptr);
}

// OS Idle Thread
extern "C" __attribute__((weak)) void osRtxIdleThread(void *argument)
{
	(void)argument;
	for (;;)
	{
		if (idleHandler)
			idleHandler();
	}
}

// OS Error Callback function
extern "C" __attribute__((weak))  uint32_t osRtxErrorNotify (uint32_t code, void *object_id)
{
#ifdef __cpp_exceptions
	try
	{
		throw std::system_error(code, cmsis::os_category(), cmsis::internal::str_error("osRtxErrorNotify", object_id));
	}
	catch(std::exception& e)
	{
		for (;;) {}
	}
#else
	(void)object_id;
	switch (code)
	{
	case osRtxErrorStackOverflow:
	  // Stack overflow detected for thread (thread_id=object_id)
	  break;
	case osRtxErrorISRQueueOverflow:
	  // ISR Queue overflow detected when inserting object (object_id)
	  break;
	case osRtxErrorTimerQueueOverflow:
	  // User Timer Callback Queue overflow detected for timer (timer_id=object_id)
	  break;
	case osRtxErrorClibSpace:
	  // Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM
	  break;
	case osRtxErrorClibMutex:
	  // Standard C/C++ library mutex initialization failed
	  break;
	default:
	  // Reserved
	  break;
	}

	for (;;) {}
#endif
	return code;
}
#endif
