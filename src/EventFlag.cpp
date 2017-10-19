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
			throw std::system_error(osError, os_category(), "osEventFlagsNew");

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
				throw std::system_error(sta, os_category(), internal::str_error("osEventFlagsDelete", m_id));
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
			throw std::system_error(flags, os_category(), internal::str_error("osEventFlagsGet", m_id));

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
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsSet", m_id));

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
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsClear", m_id));

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
			throw std::system_error(flags, flags_category(), internal::str_error("osEventFlagsWait", m_id));

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
			throw std::system_error(osErrorParameter, os_category(), "event: negative timer");

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		uint32_t option = (static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::any)) ? osFlagsWaitAny : osFlagsWaitAll;
		if ((static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::clear)) == 0)
			option |= osFlagsNoClear;

		flagValue = osEventFlagsWait(m_id, mask, option, timeout);
		if (flagValue < 0 && flagValue != osErrorTimeout)
			throw std::system_error(flagValue, flags_category(), internal::str_error("osEventFlagsWait", m_id));

		return (flagValue == osErrorTimeout ? status::timeout : status::no_timeout);
	}
}
