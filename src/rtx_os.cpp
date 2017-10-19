// This following part is specific to CMSIS-RTOS RTX implementation
#include <iostream>
#include "OSException.h"
#include "rtx_os.h"

// OS Idle Thread
void osRtxIdleThread (void *argument)
{
	(void)argument;
	for (;;) {}
}

// OS Error Callback function
uint32_t osRtxErrorNotify (uint32_t code, void *object_id)
{
	try
	{
		throw std::system_error(code, cmsis::os_category(), cmsis::internal::str_error("osRtxErrorNotify", object_id));
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	for (;;) {}
	return code;
}

extern "C" void    *osRtxMemoryAlloc(void *mem, uint32_t size, uint32_t type);
extern "C" uint32_t osRtxMemoryFree (void *mem, void *block);

void* operator new(std::size_t count)
{
	void* ptr = osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
	if (!ptr)
		throw std::bad_alloc();

	return ptr;
}

void* operator new[](std::size_t count)
{
	void* ptr = osRtxMemoryAlloc(osRtxInfo.mem.common, count, 0U);
	if (!ptr)
		throw std::bad_alloc();

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
#endif
