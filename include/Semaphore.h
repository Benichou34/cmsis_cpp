/*
 * Copyright (c) 2017, B. Leforestier
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

namespace cmsis
{
	class semaphore
	{
	public:
		typedef void* native_handle_type;

		enum class status { no_timeout, timeout };

		semaphore(size_t ini_count, size_t max_count);
		~semaphore() noexcept(false);

		void post();
		void wait();

		template<class Rep, class Period>
		status wait_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return wait_for_usec(rel_time);
		}

		template<class Clock, class Duration>
		status wait_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			return wait_for(abs_time - Clock::now());
		}

		native_handle_type native_handle() noexcept { return m_id; }

		semaphore(const semaphore&) = delete;
		semaphore& operator=(const semaphore&) = delete;

	private:
		status wait_for_usec(std::chrono::microseconds usec);

	private:
		native_handle_type m_id;
	};
}

namespace sys
{
	using semaphore = cmsis::semaphore;
}

#endif // CMSIS_SEMAPHORE_H_
