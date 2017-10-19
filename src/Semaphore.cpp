#include "Semaphore.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	semaphore::semaphore(size_t ini_count, size_t max_count) :
		m_id(0)
	{
		m_id = osSemaphoreNew(max_count, ini_count, NULL);	
		if (m_id == 0)
			throw std::system_error(osError, os_category(), "osSemaphoreNew");
	}

	semaphore::~semaphore() noexcept(false)
	{
		osStatus_t sta = osSemaphoreDelete(m_id);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreDelete", m_id));
	}

	void semaphore::post()
	{
		osStatus_t sta = osSemaphoreRelease(m_id);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreRelease", m_id));
	}

	void semaphore::wait()
	{
		osStatus_t sta = osSemaphoreAcquire(m_id, osWaitForever);
		if (sta != osOK)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));
	}

	semaphore::status semaphore::wait_for_usec(std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
			throw std::system_error(osErrorParameter, os_category(), "semaphore: negative timer");

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;
		
		osStatus_t sta = osSemaphoreAcquire(m_id, timeout);
		if (sta != osOK && sta != osErrorTimeout)
			throw std::system_error(sta, os_category(), internal::str_error("osSemaphoreAcquire", m_id));

		return (sta == osErrorTimeout ? status::timeout : status::no_timeout);
	}
}


