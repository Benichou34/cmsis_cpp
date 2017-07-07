#include "Thread.h"
#include "OSException.h"
#include "cmsis_os2.h"
#include <atomic>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace cmsis
{
	class thread_impl
	{
	public:
		thread_impl(std::unique_ptr<thread::CallableBase> targetfunc) :
			m_id(0),
			m_attr(),
			m_function(std::move(targetfunc)),
			m_detached(false)
		{
			m_attr.attr_bits = osThreadJoinable;

			m_id = osThreadNew(runnableMethodStatic, this, &m_attr);
			if (m_id == 0)
				throw std::system_error(cmsis::error_code(osError), "osThreadNew");
		}

		thread_impl(const thread_impl&) = delete;
		thread_impl& operator=(const thread_impl&) = delete;

		~thread_impl() noexcept(false)
		{
			osStatus_t sta = osThreadTerminate(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osThreadTerminate", m_id));
		}

		void join()
		{
			osStatus_t sta = osThreadJoin(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osThreadJoin", m_id));

			m_detached.store(true);
		}

		void detach()
		{
			osStatus_t sta = osThreadDetach(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osThreadDetach", m_id));

			m_detached.store(true);
		}

		bool joinable() const
		{
			return !m_detached.load();
		}

		osThreadId_t get_id() const noexcept { return m_id; }

	private:
		static void runnableMethodStatic(void* pVThread)
		{
			try
			{
				thread_impl* pThreadImpl = reinterpret_cast<thread_impl*>(pVThread);
				pThreadImpl->m_function->run();
			}
#ifdef __GNUC__
			catch(const abi::__forced_unwind&)
			{
				throw;
			}
#endif // __GNUC__
			catch(...)
			{
				std::terminate();
			}

			osThreadExit();
		}

	private:
		osThreadId_t m_id; // task identifier
		osThreadAttr_t m_attr;
		std::unique_ptr<thread::CallableBase> m_function;
		std::atomic_bool m_detached;
	};

	thread::thread(std::unique_ptr<thread::CallableBase> base) :
		m_pThread(std::make_unique<thread_impl>(std::move(base)))
	{
	}

	thread::thread(thread&& __t) noexcept : m_pThread(std::move(__t.m_pThread))
	{
	}

	thread::~thread()
	{
		if (joinable())
			std::terminate();
	}

	thread& thread::operator=(thread&& __t)
	{
		if (joinable())
			std::terminate();

		swap(__t);
		return *this;
	}

	void thread::join()
	{
		if (!joinable())
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "thread::join"); // task is detached (aka auto-delete)

		if (thread::id(m_pThread->get_id()) == cmsis::this_thread::get_id())
			throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur), "thread::join");

		m_pThread->join();
	}

	bool thread::joinable() const
	{
		return m_pThread->joinable();
	}

	void thread::detach()
	{
		if (!joinable())
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "thread::detach"); // task is detached (aka auto-delete)

		m_pThread->detach();
	}

	thread::id thread::get_id() const
	{
		return thread::id(m_pThread->get_id());
	}

	thread::native_handle_type thread::native_handle() const
	{
		return get_id().m_tid;
	}

	unsigned int thread::hardware_concurrency() noexcept
	{
		return 255;
	}

	void thread::swap(thread& __t)
	{
		m_pThread.swap(__t.m_pThread);
	}

	namespace this_thread
	{
		void yield()
		{
			osStatus_t sta = osThreadYield();
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), "osThreadYield");
		}

		thread::id get_id()
		{
			osThreadId_t tid = osThreadGetId();
			if (tid == NULL)
				throw std::system_error(cmsis::error_code(osErrorResource), "osThreadGetId");

			return thread::id(tid);
		}

		namespace internal
		{
			void sleep_for_usec (std::chrono::microseconds usec)
			{
				if (usec > std::chrono::microseconds::zero())
				{
					uint32_t ticks = static_cast<uint32_t>((usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
					if (ticks > std::numeric_limits<uint32_t>::max())
						ticks = osWaitForever;

					osStatus_t sta = osDelay(ticks);
					if (sta != osOK)
						throw std::system_error(cmsis::error_code(sta), "osDelay");
				}
			}
		}
	}
}
