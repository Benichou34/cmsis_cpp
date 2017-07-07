#include <MessageQueue.h>
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis { namespace internal
{
	message_queue_impl::message_queue_impl(size_t max_len, size_t ele_len)
		: m_id(0)
	{
		m_id = osMessageQueueNew(max_len, ele_len, NULL);
		if (m_id == 0)
			throw std::system_error(cmsis::error_code(osError), "osMessageQueueNew");
	}

	message_queue_impl::~message_queue_impl() noexcept(false)
	{
		osStatus_t sta = osMessageQueueDelete(m_id);
		if (sta != osOK)
			throw std::system_error(cmsis::error_code(sta), internal::str_error("osMessageQueueDelete", m_id));
	}

	void message_queue_impl::send(const void* data)
	{
		osStatus_t sta = osMessageQueuePut(m_id, data, 0, osWaitForever);
		if (sta != osOK)
			throw std::system_error(cmsis::error_code(sta), internal::str_error("osMessageQueuePut", m_id));
	}

	bool message_queue_impl::send(const void* data, std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
			throw std::system_error(cmsis::error_code(osErrorParameter), "Data queue: negative timer");

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		osStatus_t sta = osMessageQueuePut(m_id, data, 0, timeout);
		if (sta != osOK && sta != osErrorTimeout)
			throw std::system_error(cmsis::error_code(sta), internal::str_error("osMessageQueuePut", m_id));

		return (sta != osErrorTimeout);
	}

	void* message_queue_impl::receive()
	{
		void* data = nullptr;
		osStatus_t sta = osMessageQueueGet(m_id, &data, 0, osWaitForever);     // wait for message
		if (sta != osOK)
			throw std::system_error(cmsis::error_code(sta), internal::str_error("osMessageQueueGet", m_id));

		return data;
	}

	bool message_queue_impl::receive(void*& data, std::chrono::microseconds usec)
	{
		if (usec < std::chrono::microseconds::zero())
			throw std::system_error(cmsis::error_code(osErrorParameter), "Data queue: negative timer");

		uint32_t timeout = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
		if (timeout > std::numeric_limits<uint32_t>::max())
			timeout = osWaitForever;

		osStatus_t sta = osMessageQueueGet(m_id, &data, 0, timeout);     // wait for message
		if (sta != osOK && sta != osErrorTimeout)
			throw std::system_error(cmsis::error_code(sta), internal::str_error("osMessageQueueGet", m_id));

		return (sta != osErrorTimeout);
	}

	size_t message_queue_impl::size() const
	{
		return osMessageQueueGetCount(m_id);
	}

	size_t message_queue_impl::capacity() const
	{
		return osMessageQueueGetCapacity(m_id);
	}
}}
