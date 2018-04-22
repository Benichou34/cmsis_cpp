/*
 * Copyright (c) 2018, B. Leforestier
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

#include "Semaphore.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	semaphore::semaphore(size_t ini_count, size_t max_count) :
		m_id(0)
	{
		m_id = osSemaphoreNew(max_count, ini_count, NULL);	
		if (m_id == 0)
			throw std::system_error(osError, os_category(), "osSemaphoreNew");
	}

	semaphore::~semaphore() noexcept(false)
	{
		osStatus_t sta = osSemaphoreDelete(m_id);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreDelete", m_id));
	}

	void semaphore::post()
	{
		osStatus_t sta = osSemaphoreRelease(m_id);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreRelease", m_id));
	}

	void semaphore::wait()
	{
		osStatus_t sta = osSemaphoreAcquire(m_id, osWaitForever);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));
	}

	semaphore::status semaphore::wait_for_usec(std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
			throw std::system_error(osErrorParameter, os_category(), "semaphore: negative timer");

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;
		
		osStatus_t sta = osSemaphoreAcquire(m_id, timeout);
		if (sta != osOK && sta != osErrorTimeout)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));

		return (sta == osErrorTimeout ? status::timeout : status::no_timeout);
	}
}


