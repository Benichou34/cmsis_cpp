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

#include "ThreadFlag.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	/**
	 * Sets a thread flag.
	 * @throw std::system_error if an error occurs
	 */
	thread_flags::mask_type thread_flags::set(thread& t, mask_type mask)
	{
		int32_t flags = osThreadFlagsSet(t.native_handle(), mask);
		if (flags < 0)
#ifdef __cpp_exceptions
			throw std::system_error(flags, os_category(), internal::str_error("osThreadFlagsSet", t.native_handle()));
#else
			std::terminate();
#endif

		return flags;
	}

	namespace this_thread
	{
		flags::mask_type flags::set(mask_type mask)
		{
			osThreadId_t tid = osThreadGetId();
			if (tid == NULL)
#ifdef __cpp_exceptions
				throw std::system_error(osErrorResource, os_category(), "osThreadGetId");
#else
				std::terminate();
#endif

			int32_t flags = osThreadFlagsSet(tid, mask);
			if (flags < 0)
#ifdef __cpp_exceptions
				throw std::system_error(flags, os_category(), cmsis::internal::str_error("osThreadFlagsSet", tid));
#else
				std::terminate();
#endif

			return flags;
		}

		flags::mask_type flags::get()
		{
			int32_t flags = osThreadFlagsGet();
			if (flags < 0)
#ifdef __cpp_exceptions
				throw std::system_error(flags, flags_category(), "osThreadFlagsGet");
#else
				std::terminate();
#endif

			return flags;
		}

		/**
		 * Clears a thread flag.
		 * @param mask
		 * @return the thread flag value
		 * @throw std::system_error if an error occurs
		 */
		flags::mask_type flags::clear(mask_type mask)
		{
			int32_t flags = osThreadFlagsClear(mask);
			if (flags < 0)
#ifdef __cpp_exceptions
				throw std::system_error(flags, flags_category(), "osThreadFlagsClear");
#else
				std::terminate();
#endif

			return flags;
		}

		/**
		 * Wait until a thread flag is set
		 * @param mask
		 * @return the thread flag value
		 * @throw std::system_error if an error occurs
		 */
		flags::mask_type flags::wait(mask_type mask, wait_flag flg)
		{
			uint32_t option = ((flg & wait_flag::all) == wait_flag::all) ? osFlagsWaitAll : osFlagsWaitAny;
			if ((flg & wait_flag::no_clear) == wait_flag::no_clear)
				option |= osFlagsNoClear;

			int32_t flags = osThreadFlagsWait(mask, option, osWaitForever);
			if (flags < 0)
#ifdef __cpp_exceptions
				throw std::system_error(flags, flags_category(), "osThreadFlagsWait");
#else
				std::terminate();
#endif

			return flags;
		}

		/**
		 * Wait until a thread flag is set or a timeout occurs
		 * @param mask
		 * @return the thread flag value
		 * @throw std::system_error if an error occurs
		 */
		flags::status
		flags::wait_for_usec(mask_type mask, wait_flag flg, std::chrono::microseconds usec, mask_type& flagValue)
		{
			if (usec < std::chrono::microseconds::zero())
#ifdef __cpp_exceptions
				throw std::system_error(osErrorParameter, os_category(), "thread_flag: negative timer");
#else
				std::terminate();
#endif

			uint32_t timeout = static_cast<uint32_t>(
				(usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) /
				std::chrono::microseconds::period::den);
			if (timeout > std::numeric_limits<uint32_t>::max())
				timeout = osWaitForever;

			uint32_t option = ((flg & wait_flag::all) == wait_flag::all) ? osFlagsWaitAll : osFlagsWaitAny;
			if ((flg & wait_flag::no_clear) == wait_flag::no_clear)
				option |= osFlagsNoClear;

			flagValue = osThreadFlagsWait(mask, option, timeout);
			if (timeout == 0 && flagValue == osFlagsErrorResource)
				return status::timeout;

			if ((flagValue & osFlagsError) && flagValue != osFlagsErrorTimeout)
#ifdef __cpp_exceptions
				throw std::system_error(flagValue, flags_category(), "osThreadFlagsWait");
#else
				std::terminate();
#endif

			return (flagValue == osFlagsErrorTimeout ? status::timeout : status::no_timeout);
		}
	} // namespace this_thread
} // namespace cmsis
