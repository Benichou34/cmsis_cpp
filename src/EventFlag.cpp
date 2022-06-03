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

#include "EventFlag.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	/**
	 * Event flag constructor
	 * @throw std::system_error if an error occurs
	 */
	event::event(mask_type mask)
		: m_id(0)
	{
		m_id = osEventFlagsNew(NULL);	
		if (m_id == 0)
#ifdef __cpp_exceptions
			throw std::system_error(osError, os_category(), "osEventFlagsNew");
#else
			std::terminate();
#endif

		if (mask != 0)
			set(mask);
	}

	event::event(event&& evt) noexcept
		: m_id(0)
	{
		swap(evt);
	}

	/**
	 * Event flag destructor
	 */
	event::~event() noexcept(false)
	{
		if (m_id)
		{
			osStatus_t sta = osEventFlagsDelete(m_id);
			if (sta != osOK)
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osEventFlagsDelete", m_id));
#else
				std::terminate();
#endif
		}
	}

	void event::swap(event& evt) noexcept
	{
		std::swap(m_id, evt.m_id);
	}

	event& event::operator=(event&& evt) noexcept
	{
		swap(evt);
		return *this;
	}

	/**
	 * Get current event flag pattern.
	 * @throw std::system_error if an error occurs
	 */
	event::mask_type event::get() const
	{
		int32_t flags = osEventFlagsGet(m_id);
		if (flags < 0)
#ifdef __cpp_exceptions
			throw std::system_error(flags, os_category(), internal::str_error("osEventFlagsGet", m_id));
#else
			std::terminate();
#endif

		return flags;	
	}

	/**
	 * Sets an event flag.
	 * @throw std::system_error if an error occurs
	 */
	event::mask_type event::set(mask_type mask)
	{
		int32_t flags = osEventFlagsSet(m_id, mask);
		if (flags < 0)
#ifdef __cpp_exceptions
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsSet", m_id));
#else
			std::terminate();
#endif

		return flags;
	}

	/**
	 * Clears an event flag.
	 * @throw std::system_error if an error occurs
	 */
	event::mask_type event::clear(mask_type mask)
	{
		int32_t flags = osEventFlagsClear(m_id, mask);
		if (flags < 0)
#ifdef __cpp_exceptions
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsClear", m_id));
#else
			std::terminate();
#endif

		return flags;
	}

	/**
	 * Wait until an event flag is set
	 * @param mask
	 * @return the event flag value
	 * @throw std::system_error if an error occurs
	 */
	event::mask_type event::wait(mask_type mask, wait_flag flg)
	{
		uint32_t option = (static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::any)) ? osFlagsWaitAny : osFlagsWaitAll;
		if ((static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::clear)) == 0)
			option |= osFlagsNoClear;

		int32_t flags = osEventFlagsWait(m_id, mask, option, osWaitForever);
		if (flags < 0)
#ifdef __cpp_exceptions
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsWait", m_id));
#else
			std::terminate();
#endif

		return flags;
	}

	/**
	 * Wait until an event flag is set or a timeout occurs
	 * @param mask
	 * @return the event flag value
	 * @throw std::system_error if an error occurs
	 */
	event::status event::wait_for_usec(mask_type mask, wait_flag flg, std::chrono::microseconds usec, mask_type& flagValue)
	{
		if (usec < std::chrono::microseconds::zero())
#ifdef __cpp_exceptions
			throw std::system_error(osErrorParameter, os_category(), "event: negative timer");
#else
			std::terminate();
#endif

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		uint32_t option = (static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::any)) ? osFlagsWaitAny : osFlagsWaitAll;
		if ((static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::clear)) == 0)
			option |= osFlagsNoClear;

		flagValue = osEventFlagsWait(m_id, mask, option, timeout);
		if (timeout == 0 && flagValue == osFlagsErrorResource)
			return status::timeout;

		if ((flagValue & osFlagsError) && flagValue != osFlagsErrorTimeout)
#ifdef __cpp_exceptions
			throw std::system_error(flagValue, flags_category(), internal::str_error("osEventFlagsWait", m_id));
#else
			std::terminate();
#endif

		return (flagValue == osFlagsErrorTimeout ? status::timeout : status::no_timeout);
	}
}
