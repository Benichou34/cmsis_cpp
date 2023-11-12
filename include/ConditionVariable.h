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

#ifndef CMSIS_CONDITION_VARIABLE_H_
#define CMSIS_CONDITION_VARIABLE_H_

#include "Mutex.h"
#include "Semaphore.h"
#include <condition_variable>
#include <list>

namespace cmsis
{
	// cv_status
	enum class cv_status
	{
		no_timeout,
		timeout
	};

	// STL like implementation
	class condition_variable
	{
	public:
		typedef void* native_handle_type;
		typedef std::chrono::system_clock clock_t;

		condition_variable() = default;
		~condition_variable() = default;

		void notify_one() noexcept;
		void notify_all() noexcept;
		void wait(std::unique_lock<cmsis::mutex>& lock);

		template <class Predicate> void wait(std::unique_lock<cmsis::mutex>& lock, Predicate pred)
		{
			while (!pred())
				wait(lock);
		}

		template <class Clock, class Duration>
		cv_status
		wait_until(std::unique_lock<cmsis::mutex>& lock, const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			auto rel_time = abs_time - Clock::now();
			if (rel_time < std::chrono::microseconds::zero())
				return cv_status::timeout;

			return wait_for(lock, rel_time);
		}

		template <class Clock, class Duration, class Predicate>
		bool wait_until(
			std::unique_lock<cmsis::mutex>& lock,
			const std::chrono::time_point<Clock, Duration>& abs_time,
			Predicate pred)
		{
			while (!pred())
			{
				if (wait_until(lock, abs_time) == cv_status::timeout)
					return pred();
			}
			return true;
		}

		template <class Rep, class Period>
		cv_status wait_for(std::unique_lock<cmsis::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time)
		{
			return wait_for_usec(lock, std::chrono::duration_cast<std::chrono::microseconds>(rel_time));
		}

		template <class Rep, class Period, class Predicate>
		bool wait_for(
			std::unique_lock<cmsis::mutex>& lock,
			const std::chrono::duration<Rep, Period>& rel_time,
			Predicate pred)
		{
			clock_t::time_point abs_time = clock_t::now() + rel_time;

			while (!pred())
			{
				if (wait_until(lock, abs_time) == cv_status::timeout)
					return pred();
			}
			return true;
		}

		native_handle_type native_handle() noexcept { return nullptr; }

		condition_variable(const condition_variable&) = delete;
		condition_variable& operator=(const condition_variable&) = delete;

	private:
		cv_status wait_for_usec(std::unique_lock<cmsis::mutex>& lock, std::chrono::microseconds usec);

	private:
		cmsis::mutex m_mutex;
		std::list<cmsis::binary_semaphore*> m_wait;
	};
} // namespace cmsis

#if !defined(GLIBCXX_HAS_GTHREADS) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace std
{
	using cv_status = cmsis::cv_status;
	using condition_variable = cmsis::condition_variable;
} // namespace std
#endif

#endif // CMSIS_CONDITION_VARIABLE_H_
