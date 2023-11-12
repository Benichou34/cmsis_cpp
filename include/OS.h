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

#ifndef CPP_CMSIS_OS_H_
#define CPP_CMSIS_OS_H_

#include <cstdint>
#include <functional>

namespace cmsis
{
	/// Kernel Information and Control.
	namespace kernel
	{
		/// Get RTOS Kernel version.
		/// \return string that contains version information.
		/// \exception In case of failure, throws a std::system_error exception.
		const char* version();

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

		/// Suspends the RTOS kernel scheduler and thus enables sleep modes.
		uint32_t suspend() noexcept;

		/// Enables the RTOS kernel scheduler and thus wakes up the system from sleep mode.
		void resume(uint32_t sleep_ticks) noexcept;

		/// Start the idle handler called by the idle thread.
		/// \exception In case of failure, throws a std::system_error exception.
		void set_idle_handler(std::function<void()>&& handler);
	} // namespace kernel

	namespace core
	{
		/// Get system core clock frequency in Hz.
		/// \return the frequency of the current system core clock.
		/// \exception In case of failure, throws a std::system_error exception.
		uint32_t clock_frequency();
	} // namespace core

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
} // namespace cmsis

namespace sys
{
	namespace kernel = cmsis::kernel;
	namespace core = cmsis::core;
	using dispatch = cmsis::dispatch;
} // namespace sys

#endif // CPP_CMSIS_OS_H_
