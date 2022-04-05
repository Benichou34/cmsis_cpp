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

#include <string>
#include "OS.h"
#include "OSException.h"
#include "cmsis_os2.h"

extern "C" uint32_t SystemCoreClock;         /**< System Clock Frequency (Core Clock) */
extern "C" void SystemCoreClockUpdate(void); /**< Updates the variable SystemCoreClock */

std::function<void()> idleHandler;

namespace cmsis
{
	namespace kernel
	{
		std::string version()
		{
			char infobuf[100];
			osVersion_t osv;

			osStatus_t sta = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
			if(sta != osOK)
			{
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), "osKernelGetInfo");
#else
				std::terminate();
#endif
			}

			return std::string(infobuf);
		}

		uint32_t tick_frequency()
		{
			uint32_t tick = osKernelGetTickFreq();
			if (!tick)
			{
#ifdef __cpp_exceptions
				throw std::system_error(osError, os_category(), "osKernelGetTickFreq");
#else
				std::terminate();
#endif
			}

			return tick;
		}

		/**
		 * Initialize the RTOS Kernel.
		 * @throw std::system_error if an error occurs
		 */
		void initialize()
		{
			if (osKernelGetState() == osKernelInactive)
			{
				osStatus_t sta = osKernelInitialize();
				if (sta != osOK)
				{
#ifdef __cpp_exceptions
					throw std::system_error(sta, os_category(), "osKernelInitialize");
#else
					std::terminate();
#endif
				}
			}
		}

		/**
		 * Start the RTOS Kernel scheduler. In case of success, this function never returns.
		 * @throw std::system_error if an error occurs
		 */
		void start()
		{
			osStatus_t sta = osKernelStart();
			if (sta != osOK)
			{
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), "osKernelStart");
#else
				std::terminate();
#endif
			}
		}

		/**
		 * Suspends the RTOS kernel scheduler and thus enables sleep modes.
		 * Returns time in ticks, for how long the system can sleep or power-down.
		 */
		uint32_t suspend() noexcept
		{
			return osKernelSuspend();
		}

		/**
		 * Enables the RTOS kernel scheduler and thus wakes up the system from sleep mode.
		 * sleep_ticks: time in ticks for how long the system was in sleep or power-down mode.
		 */
		void resume(uint32_t sleep_ticks) noexcept
		{
			return osKernelResume(sleep_ticks);
		}

		/**
		 * Start the idle handler called by the idle thread.
		 * @throw std::system_error if an error occurs
		 */
		void set_idle_handler(std::function<void()>&& handler)
		{
			idleHandler = std::move(handler);
		}
	}

	namespace core
	{
		uint32_t clock_frequency()
		{
			SystemCoreClockUpdate();
			if (!SystemCoreClock)
			{
#ifdef __cpp_exceptions
				throw std::system_error(osError, os_category(), "SystemCoreClock");
#else
				std::terminate();
#endif
			}

			return SystemCoreClock;
		}
	}

	/**
	 * Lock / unlock Dispatching.
	 * @throw std::system_error if an error occurs
	 */
	dispatch::dispatch() : m_previous_lock_state(osError)
	{
	}

	void dispatch::lock()
	{
		m_previous_lock_state = osKernelLock();
		if (m_previous_lock_state < 0)
		{
#ifdef __cpp_exceptions
			throw std::system_error(m_previous_lock_state, os_category(), "osKernelLock");
#else
			std::terminate();
#endif
		}
	}

	void dispatch::unlock()
	{
		if (m_previous_lock_state < 0)
		{
#ifdef __cpp_exceptions
			throw std::system_error(m_previous_lock_state, os_category(), "Bad kernel previous state");
#else
			std::terminate();
#endif
		}

		m_previous_lock_state = osKernelRestoreLock(m_previous_lock_state);
		if (m_previous_lock_state < 0)
		{
#ifdef __cpp_exceptions
			throw std::system_error(m_previous_lock_state, os_category(), "osKernelRestoreLock");
#else
			std::terminate();
#endif
		}
	}

	bool dispatch::locked()
	{
		return (osKernelGetState() == osKernelLocked);
	}
}
