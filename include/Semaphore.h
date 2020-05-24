/*
 * Copyright (c) 2020, B. Leforestier
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

#ifndef CMSIS_SEMAPHORE_H_
#define CMSIS_SEMAPHORE_H_

#include <chrono>
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	template<std::ptrdiff_t LeastMaxValue = 0xFFFFFFFF>
	class counting_semaphore
	{
	public:
		typedef void* native_handle_type;

		constexpr explicit counting_semaphore(std::ptrdiff_t desired) :
			m_id(0)
		{
			m_id = osSemaphoreNew(max(), desired, NULL);	
			if (m_id == 0)
				throw std::system_error(osError, os_category(), "osSemaphoreNew");
		}

		~counting_semaphore() noexcept(false)
		{
			osStatus_t sta = osSemaphoreDelete(m_id);
			if (sta != osOK)
				throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreDelete", m_id));
		}

		void release(std::ptrdiff_t update = 1)
		{
			while (update--)
			{
				osStatus_t sta = osSemaphoreRelease(m_id);
				if (sta != osOK)
					throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreRelease", m_id));
			}
		}

		void acquire()
		{
			osStatus_t sta = osSemaphoreAcquire(m_id, osWaitForever);
			if (sta != osOK)
				throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));
		}

		bool try_acquire() noexcept
		{
			return (osSemaphoreAcquire(m_id, 0) == osOK);
		}

		template<class Rep, class Period>
		bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return try_acquire_for_usec(std::chrono::duration_cast<std::chrono::microseconds>(rel_time));
		}

		template<class Clock, class Duration>
		bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			return try_acquire_for(abs_time - Clock::now());
		}

		constexpr std::ptrdiff_t max() noexcept { return LeastMaxValue; }

		native_handle_type native_handle() noexcept { return m_id; }

		counting_semaphore(const counting_semaphore&) = delete;
		counting_semaphore& operator=(const counting_semaphore&) = delete;

	private:
		bool try_acquire_for_usec(std::chrono::microseconds usec)
		{
			if (usec < std::chrono::microseconds::zero())
				throw std::system_error(osErrorParameter, os_category(), "semaphore: negative timer");

			uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (timeout > std::numeric_limits<uint32_t>::max())
				timeout = osWaitForever;
			
			osStatus_t sta = osSemaphoreAcquire(m_id, timeout);
			if (sta != osOK && sta != osErrorTimeout)
				throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));

			return (sta == osOK);
		}

	private:
		native_handle_type m_id;
	};

	using binary_semaphore = counting_semaphore<1>;
}

#if !defined(GLIBCXX_HAS_GTHREADS) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace std
{
	template<std::ptrdiff_t LeastMaxValue = 0xFFFFFFFF>
	using counting_semaphore = cmsis::counting_semaphore<LeastMaxValue>;
	using binary_semaphore = cmsis::binary_semaphore;
}
#endif

#endif // CMSIS_SEMAPHORE_H_
