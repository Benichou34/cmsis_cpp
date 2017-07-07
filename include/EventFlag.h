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

#ifndef CMSIS_EVENTFLAG_H_
#define CMSIS_EVENTFLAG_H_

#include <memory>
#include <chrono>

namespace cmsis
{
	class event
	{
	public:
		typedef void* native_handle_type;
		typedef int mask_type;

		enum class status { no_timeout, timeout };
		enum class wait_flag : unsigned int
		{
			any = 0,
			all = 1,
			clear = 2
		};

		event(mask_type mask = 0);
		event(const event&) = delete;
		event(event&& evt) noexcept;
		~event() noexcept(false);

		event& operator=(const event&) = delete;
		event& operator=(event&& evt) noexcept;

		void swap(event& evt) noexcept;

		mask_type get() const;

		void set(mask_type mask);
		void clear(mask_type mask = static_cast<mask_type>(-1));
		mask_type wait(mask_type mask, wait_flag flg = wait_flag::any);

		template<class Rep, class Period>
		status wait_for(mask_type mask, wait_flag flg, const std::chrono::duration<Rep, Period>& rel_time, mask_type& flagValue)
		{
			return wait_for_usec(mask, flg, rel_time, flagValue);
		}

		template<class Rep, class Period>
		status wait_for(mask_type mask, const std::chrono::duration<Rep, Period>& rel_time, mask_type& flagValue)
		{
			return wait_for(mask, wait_flag::any, rel_time, flagValue);
		}

		template<class Clock, class Duration>
		status wait_until(mask_type mask, wait_flag flg, const std::chrono::time_point<Clock, Duration>& abs_time, mask_type& flagValue)
		{
			return wait_for(mask, flg, abs_time - Clock::now(), flagValue);
		}

		template<class Clock, class Duration>
		status wait_until(mask_type mask, const std::chrono::time_point<Clock, Duration>& abs_time, mask_type& flagValue)
		{
			return wait_until(mask, wait_flag::any, abs_time, flagValue);
		}

		native_handle_type native_handle() noexcept { return m_id; }

	private:
		status wait_for_usec(mask_type mask, wait_flag flg, std::chrono::microseconds usec, mask_type& flagValue);

	private:
		native_handle_type m_id; // event flag identifier
	};

	inline void	swap(event& __x, event& __y) noexcept { __x.swap(__y); }
}

namespace sys
{
	using event = cmsis::event;
}

#endif // CMSIS_EVENTFLAG_H_
