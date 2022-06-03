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

#include "Mutex.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	namespace internal
	{
		base_timed_mutex::base_timed_mutex(const char* name, bool recursive) :
			m_id(0)
		{
			osMutexAttr_t Mutex_attr = { name, osMutexPrioInherit, NULL, 0 };
			if (recursive)
				Mutex_attr.attr_bits |= osMutexRecursive;
			
			m_id = osMutexNew(&Mutex_attr);	
			if (m_id == 0)
#ifdef __cpp_exceptions
				throw std::system_error(osError, os_category(), "osMutexNew");
#else
				std::terminate();
#endif
		}

		base_timed_mutex::~base_timed_mutex() noexcept(false)
		{
			osStatus_t sta = osMutexDelete(m_id);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osMutexDelete", m_id));
#else
				std::terminate();
#endif
		}

		void base_timed_mutex::lock()
		{
			osStatus_t sta = osMutexAcquire(m_id, osWaitForever);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osMutexAcquire", m_id));
#else
				std::terminate();
#endif
		}

		void base_timed_mutex::unlock()
		{
			osStatus_t sta = osMutexRelease(m_id);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osMutexRelease", m_id));
#else
				std::terminate();
#endif
		}

		bool base_timed_mutex::try_lock()
		{
			osStatus_t sta = osMutexAcquire(m_id, 0);
			if (sta != osOK && sta != osErrorTimeout)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osMutexAcquire", m_id));
#else
				std::terminate();
#endif

			return (sta != osErrorTimeout);
		}
		
		bool base_timed_mutex::try_lock_for_usec(std::chrono::microseconds usec)
		{
			if (usec < std::chrono::microseconds::zero())
#ifdef __cpp_exceptions
				throw std::system_error(osErrorParameter, os_category(), "base_timed_mutex: negative timer");
#else
				std::terminate();
#endif

			uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (timeout > std::numeric_limits<uint32_t>::max())
				timeout = osWaitForever;
			
			osStatus_t sta = osMutexAcquire(m_id, timeout);
			if (timeout == 0 && sta == osErrorResource)
				return false;

			if (sta != osOK && sta != osErrorTimeout)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osMutexAcquire", m_id));
#else
				std::terminate();
#endif

			return (sta != osErrorTimeout);
		}
	}
}

