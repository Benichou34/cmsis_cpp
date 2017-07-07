#include "Mutex.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	namespace internal
	{
		base_timed_mutex::base_timed_mutex(const char* name, bool recursive) :
			m_id(0)
		{
			osMutexAttr_t Mutex_attr = { name, osMutexPrioInherit, NULL, 0 };
			if (recursive)
				Mutex_attr.attr_bits |= osMutexRecursive;
			
			m_id = osMutexNew(&Mutex_attr);	
			if (m_id == 0)
				throw std::system_error(cmsis::error_code(osError), "osMutexNew");
		}

		base_timed_mutex::~base_timed_mutex() noexcept(false)
		{
			osStatus_t sta = osMutexDelete(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMutexDelete", m_id));
		}

		void base_timed_mutex::lock()
		{
			osStatus_t sta = osMutexAcquire(m_id, osWaitForever);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMutexAcquire", m_id));
		}

		void base_timed_mutex::unlock()
		{
			osStatus_t sta = osMutexRelease(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMutexRelease", m_id));
		}

		bool base_timed_mutex::try_lock()
		{
			osStatus_t sta = osMutexAcquire(m_id, 0);
			if (sta != osOK && sta != osErrorTimeout)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMutexAcquire", m_id));

			return (sta != osErrorTimeout);
		}
		
		bool base_timed_mutex::try_lock_for_usec(std::chrono::microseconds usec)
		{
			if (usec < std::chrono::microseconds::zero())
				throw std::system_error(cmsis::error_code(osErrorParameter), "base_timed_mutex: negative timer");

			uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (timeout > std::numeric_limits<uint32_t>::max())
				timeout = osWaitForever;
			
			osStatus_t sta = osMutexAcquire(m_id, timeout);
			if (sta != osOK && sta != osErrorTimeout)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMutexAcquire", m_id));

			return (sta != osErrorTimeout);
		}
	}
}

