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

#include <MessageQueue.h>
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis { namespace internal
{
	message_queue_impl::message_queue_impl(size_t max_len, size_t ele_len)
		: m_id(0)
	{
		m_id = osMessageQueueNew(max_len, ele_len, NULL);
		if (m_id == 0)
		{
#ifdef __cpp_exceptions
			throw std::system_error(osError, os_category(), "osMessageQueueNew");
#else
			std::terminate();
#endif
		}
	}

	message_queue_impl::message_queue_impl(message_queue_impl&& t)
		: m_id(t.m_id)
	{
		t.m_id = 0;
	}

	message_queue_impl::~message_queue_impl() noexcept(false)
	{
		osStatus_t sta = osMessageQueueDelete(m_id);
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueueDelete", m_id));
#else
			std::terminate();
#endif
		}
	}

	void message_queue_impl::swap(message_queue_impl& t)
	{
		std::swap(m_id, t.m_id);
	}

	message_queue_impl& message_queue_impl::operator=(message_queue_impl&& t)
	{
		if (&t != this)
			std::swap(m_id, t.m_id);

		return *this;
	}

	void message_queue_impl::put(const void* data)
	{
		osStatus_t sta = osMessageQueuePut(m_id, data, 0, osWaitForever);
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueuePut", m_id));
#else
			std::terminate();
#endif
		}
	}

	bool message_queue_impl::put(const void* data, std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
		{
#ifdef __cpp_exceptions
			throw std::system_error(osErrorParameter, os_category(), "Data queue: negative timer");
#else
			std::terminate();
#endif
		}

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		osStatus_t sta = osMessageQueuePut(m_id, data, 0, timeout);
		if (sta != osOK && sta != osErrorTimeout)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueuePut", m_id));
#else
			std::terminate();
#endif
		}

		return (sta != osErrorTimeout);
	}

	void message_queue_impl::get(void* data)
	{
		osStatus_t sta = osMessageQueueGet(m_id, data, 0, osWaitForever);     // wait for message
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueueGet", m_id));
#else
			std::terminate();
#endif
		}
	}

	bool message_queue_impl::get(void* data, std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
		{
#ifdef __cpp_exceptions
			throw std::system_error(osErrorParameter, os_category(), "Data queue: negative timer");
#else
			std::terminate();
#endif
		}

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		osStatus_t sta = osMessageQueueGet(m_id, data, 0, timeout);     // wait for message
		if (sta != osOK && sta != osErrorTimeout)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueueGet", m_id));
#else
			std::terminate();
#endif
		}

		return (sta != osErrorTimeout);
	}

	size_t message_queue_impl::size() const
	{
		return osMessageQueueGetCount(m_id);
	}

	size_t message_queue_impl::capacity() const
	{
		return osMessageQueueGetCapacity(m_id);
	}

	void message_queue_impl::reset()
	{
		osStatus_t sta = osMessageQueueReset(m_id);
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osMessageQueueReset", m_id));
#else
			std::terminate();
#endif
		}
	}
}}
