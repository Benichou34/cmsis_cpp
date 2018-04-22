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

#include <iostream>
#include <string>
#include "OS.h"
#include "OSException.h"
#include "cmsis_os2.h"
#include "rtx_os.h"

extern "C" uint32_t SystemCoreClock;         /**< System Clock Frequency (Core Clock) */
extern "C" void SystemCoreClockUpdate(void); /**< Updates the variable SystemCoreClock */

namespace
{
	std::function<void()> idleHandler;
}

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
				throw std::system_error(sta, os_category(), "osKernelGetInfo");

			return std::string(infobuf);
		}

		uint32_t tick_frequency()
		{
			uint32_t tick = osKernelGetTickFreq();
			if (!tick)
				throw std::system_error(osError, os_category(), "osKernelGetTickFreq");

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
					throw std::system_error(sta, os_category(), "osKernelInitialize");
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
				throw std::system_error(sta, os_category(), "osKernelStart");
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
				throw std::system_error(osError, os_category(), "SystemCoreClock");

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
			throw std::system_error(m_previous_lock_state, os_category(), "osKernelLock");
	}

	void dispatch::unlock()
	{
		if (m_previous_lock_state < 0)
			throw std::system_error(m_previous_lock_state, os_category(), "Bad kernel previous state");

		m_previous_lock_state = osKernelRestoreLock(m_previous_lock_state);
		if (m_previous_lock_state < 0)
			throw std::system_error(m_previous_lock_state, os_category(), "osKernelRestoreLock");
	}

	bool dispatch::locked()
	{
		return (osKernelGetState() == osKernelLocked);
	}
}

// OS Idle Thread
void osRtxIdleThread(void *argument)
{
	(void)argument;
	for (;;)
	{
		if (idleHandler)
			idleHandler();
	}
}

// OS Error Callback function
uint32_t osRtxErrorNotify (uint32_t code, void *object_id)
{
	try
	{
		throw std::system_error(code, cmsis::os_category(), cmsis::internal::str_error("osRtxErrorNotify", object_id));
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		for (;;) {}
	}

	return code;
}
