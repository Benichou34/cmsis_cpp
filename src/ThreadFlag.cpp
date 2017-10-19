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
			throw std::system_error(flags, os_category(), internal::str_error("osThreadFlagsSet", t.native_handle()));

		return flags;
	}

	namespace this_thread
	{
		flags::mask_type flags::set(mask_type mask)
		{
			osThreadId_t tid = osThreadGetId();
			if (tid == NULL)
				throw std::system_error(osErrorResource, os_category(), "osThreadGetId");

			int32_t flags = osThreadFlagsSet(tid, mask);
			if (flags < 0)
				throw std::system_error(flags, os_category(), cmsis::internal::str_error("osThreadFlagsSet", tid));

			return flags;
		}

		flags::mask_type flags::get()
		{
			int32_t flags = osThreadFlagsGet();
			if (flags < 0)
				throw std::system_error(flags, flags_category(), "osThreadFlagsGet");

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
				throw std::system_error(flags, flags_category(), "osThreadFlagsClear");

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
			uint32_t option = (static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::any)) ? osFlagsWaitAny : osFlagsWaitAll;
			if ((static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::clear)) == 0)
				option |= osFlagsNoClear;

			int32_t flags = osThreadFlagsWait(mask, option, osWaitForever);
			if (flags < 0)
				throw std::system_error(flags, flags_category(), "osThreadFlagsWait");

			return flags;
		}

		/**
		 * Wait until a thread flag is set or a timeout occurs
		 * @param mask
		 * @return the thread flag value
		 * @throw std::system_error if an error occurs
		 */
		flags::status flags::wait_for_usec(mask_type mask, wait_flag flg, std::chrono::microseconds usec, mask_type& flagValue)
		{
			if (usec < std::chrono::microseconds::zero())
				throw std::system_error(osErrorParameter, os_category(), "thread_flag: negative timer");

			uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (timeout > std::numeric_limits<uint32_t>::max())
				timeout = osWaitForever;

			uint32_t option = (static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::any)) ? osFlagsWaitAny : osFlagsWaitAll;
			if ((static_cast<unsigned int>(flg) & static_cast<unsigned int>(wait_flag::clear)) == 0)
				option |= osFlagsNoClear;

			flagValue = osThreadFlagsWait(mask, option, timeout);
			if (flagValue < 0 && flagValue != osErrorTimeout)
				throw std::system_error(flagValue, flags_category(), "osThreadFlagsWait");

			return (flagValue == osErrorTimeout ? status::timeout : status::no_timeout);
		}
	}
}
