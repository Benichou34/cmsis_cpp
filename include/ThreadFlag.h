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

#ifndef CPP_CMSIS_THREADFLAG_H_
#define CPP_CMSIS_THREADFLAG_H_

#include "Thread.h"
#include "WaitFlag.h"

namespace cmsis
{
	struct thread_flags
	{
		typedef uint32_t mask_type;

		static mask_type set(thread& t, mask_type mask);
	};

	namespace this_thread
	{
		class flags
		{
		public:
			typedef uint32_t mask_type;

			enum class status
			{
				no_timeout,
				timeout
			};

			static mask_type set(mask_type mask);
			static mask_type get();

			static mask_type clear(mask_type mask = static_cast<mask_type>(0x7FFFFFFF));
			static mask_type wait(mask_type mask, wait_flag flg = wait_flag::any);

			template <class Rep, class Period>
			static status wait_for(
				mask_type mask,
				wait_flag flg,
				const std::chrono::duration<Rep, Period>& rel_time,
				mask_type& flagValue)
			{
				return wait_for_usec(mask, flg, rel_time, flagValue);
			}

			template <class Rep, class Period>
			static status
			wait_for(mask_type mask, const std::chrono::duration<Rep, Period>& rel_time, mask_type& flagValue)
			{
				return wait_for(mask, wait_flag::any, rel_time, flagValue);
			}

			template <class Clock, class Duration>
			static status wait_until(
				mask_type mask,
				wait_flag flg,
				const std::chrono::time_point<Clock, Duration>& abs_time,
				mask_type& flagValue)
			{
				auto rel_time = abs_time - Clock::now();
				if (rel_time < std::chrono::microseconds::zero())
					return status::timeout;

				return wait_for(mask, flg, rel_time, flagValue);
			}

			template <class Clock, class Duration>
			static status
			wait_until(mask_type mask, const std::chrono::time_point<Clock, Duration>& abs_time, mask_type& flagValue)
			{
				return wait_until(mask, wait_flag::any, abs_time, flagValue);
			}

		private:
			static status
			wait_for_usec(mask_type mask, wait_flag flg, std::chrono::microseconds usec, mask_type& flagValue);
		};
	} // namespace this_thread
} // namespace cmsis

namespace sys
{
	using thread_flags = cmsis::thread_flags;
	namespace this_thread = cmsis::this_thread;
} // namespace sys

#endif // CPP_CMSIS_THREADFLAG_H_
