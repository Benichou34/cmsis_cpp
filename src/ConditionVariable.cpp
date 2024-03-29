/*
 * Copyright (c) 2023, B. Leforestier
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

#include "ConditionVariable.h"
#include "OSException.h"
#include "Semaphore.h"
#include "cmsis_os2.h"

namespace cmsis
{
	void condition_variable::notify_one() noexcept
	{
		std::lock_guard<cmsis::mutex> lg(m_mutex);
		if (!m_wait.empty())
		{
			m_wait.front()->release();
			m_wait.pop_front();
		}
	}

	void condition_variable::notify_all() noexcept
	{
		std::lock_guard<cmsis::mutex> lg(m_mutex);
		for (auto psema : m_wait)
			psema->release();

		m_wait.clear();
	}

	void condition_variable::wait(std::unique_lock<cmsis::mutex>& lock)
	{
		wait_for(lock, std::chrono::microseconds::max());
	}

	cmsis::cv_status
	condition_variable::wait_for_usec(std::unique_lock<cmsis::mutex>& lock, std::chrono::microseconds usec)
	{
		if (!lock.owns_lock())
			std::terminate();

		cmsis::binary_semaphore sema(0);
		std::list<cmsis::binary_semaphore*>::iterator it;
		{
			std::lock_guard<std::mutex> lg(m_mutex);
			it = m_wait.insert(m_wait.end(), &sema);
		}

		lock.unlock();
		bool st = sema.try_acquire_for(usec);
		lock.lock();

		if (!st)
		{
			std::lock_guard<std::mutex> lg(m_mutex);
			m_wait.erase(it);
			return cmsis::cv_status::timeout;
		}

		return cmsis::cv_status::no_timeout;
	}
} // namespace cmsis
