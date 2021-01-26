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

#include "Timer.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	class cmsis_timer
	{
	public:
		typedef std::function<bool()> callback;

		cmsis_timer(std::chrono::microseconds usec, callback&& function, bool bOnce) :
			m_Callback(std::move(function)),
			m_id(0),
			m_usec(usec)
		{
			if (!m_Callback)
#ifdef __cpp_exceptions
				throw std::system_error(osErrorParameter, os_category(), "timer: missing callback");
#else
				std::terminate();
#endif

			if (m_usec < std::chrono::microseconds::zero())
#ifdef __cpp_exceptions
				throw std::system_error(osErrorParameter, os_category(), "base_timed_mutex: negative timer");
#else
				std::terminate();
#endif

			m_id = osTimerNew(handler, bOnce ? osTimerOnce : osTimerPeriodic, this, NULL);
			if (m_id == 0)
#ifdef __cpp_exceptions
				throw std::system_error(osError, os_category(), "osTimerNew");
#else
				std::terminate();
#endif
		}

		~cmsis_timer() noexcept(false)
		{
			osStatus_t sta = osTimerDelete(m_id);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osTimerDelete", m_id));
#else
				std::terminate();
#endif
		}

		void start()
		{

			uint32_t ticks = static_cast<uint32_t>((m_usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (ticks > std::numeric_limits<uint32_t>::max())
				ticks = osWaitForever;

			osStatus_t sta = osTimerStart(m_id, ticks);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osTimerStart", m_id));
#else
				std::terminate();
#endif
		}

		void stop()
		{
			osStatus_t sta = osTimerStop(m_id);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osTimerStop", m_id));
#else
				std::terminate();
#endif
		}

		bool running() const
		{
			return osTimerIsRunning(m_id) != 0;
		}

		cmsis_timer(const cmsis_timer&) = delete;
		cmsis_timer& operator=(const cmsis_timer&) = delete;

	private:
		static void handler(void* argument)
		{
			cmsis_timer* pTimer = static_cast<cmsis_timer*>(argument);

			if (!pTimer->running())
				return;

			if (!pTimer->m_Callback())
				pTimer->stop();
		}

	private:
		callback m_Callback;
		osTimerId_t m_id;
		std::chrono::microseconds m_usec;
	};

	timer::timer() :
		m_pImplTimer()
	{
	}

	timer::timer(std::chrono::microseconds usec, callback&& function, bool bOnce) :
		m_pImplTimer(std::make_unique<cmsis_timer>(usec, std::move(function), bOnce))
	{
	}

	timer::timer(timer&& t) :
		m_pImplTimer(std::move(t.m_pImplTimer))
	{
	}

	timer& timer::operator=(timer&& t)
	{
		if (&t != this)
			m_pImplTimer = std::move(t.m_pImplTimer);

		return *this;
	}

	timer::~timer()
	{
	}

	void timer::swap(timer& t) noexcept
	{
		std::swap(m_pImplTimer, t.m_pImplTimer);
	}

	void timer::start()
	{
		if (!m_pImplTimer)
#ifdef __cpp_exceptions
			throw std::system_error(osErrorResource, os_category(), "timer::start");
#else
			std::terminate();
#endif

		m_pImplTimer->start();
	}

	void timer::stop()
	{
		if (!m_pImplTimer)
#ifdef __cpp_exceptions
			throw std::system_error(osErrorResource, os_category(), "timer::stop");
#else
			std::terminate();
#endif

		m_pImplTimer->stop();
 	}

	bool timer::running() const
	{
		if (!m_pImplTimer)
			return false;

		return m_pImplTimer->running();
	}
}
