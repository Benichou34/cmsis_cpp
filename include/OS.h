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

#ifndef CMSIS_OS_H_
#define CMSIS_OS_H_

#include <string>

namespace cmsis
{
	/// Kernel Information and Control.
	namespace os
	{
		/// Get RTOS Kernel version.
		/// \return string that contains version information.
		/// \exception In case of failure, throws a std::system_error exception.
		std::string version();

		/// Get RTOS Kernel tick frequency in Hz.
		/// \return the frequency of the current RTOS kernel tick.
		/// \exception In case of failure, throws a std::system_error exception.
		uint32_t tick_frequency();

		/// Initialize the RTOS Kernel.
		/// \exception In case of failure, throws a std::system_error exception.
		void initialize();

		/// Start the RTOS Kernel scheduler.
		/// In case of success, this function will never returns.
		/// \exception In case of failure, throws a std::system_error exception.
		void start();
	}

	namespace core
	{
		/// Get system core clock frequency in Hz.
		/// \return the frequency of the current system core clock.
		/// \exception In case of failure, throws a std::system_error exception.
		uint32_t clock_frequency();
	}

	/// Management of the RTOS Kernel scheduler.
	/// This class is conform to the C++ BasicLockable concept, and can be used by std::lock_guard.
	class dispatch
	{
	public:
		dispatch();
		~dispatch() = default;

		dispatch(const dispatch&) = delete;
		dispatch& operator=(const dispatch&) = delete;

		void lock();
		void unlock();

		static bool locked();

	private:
		int32_t m_previous_lock_state;
	};
}

namespace sys
{
	namespace os = cmsis::os;
	namespace core = cmsis::core;
	using dispatch = cmsis::dispatch;
}

#endif // CMSIS_OS_H_
