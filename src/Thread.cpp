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
		thread_impl(const thread::attributes& attr, std::unique_ptr<thread::CallableBase> targetfunc) :
			m_id(0),
			m_function(std::move(targetfunc)),
			m_detached(false)
		{
			osThreadAttr_t osAttr = {};
			osAttr.attr_bits = osThreadJoinable;
			osAttr.name = attr.name;
			osAttr.stack_mem = attr.stack_mem;
			osAttr.stack_size = attr.stack_size;
			osAttr.priority = static_cast<osPriority_t>(attr.priority);

			if (osAttr.priority == osPriorityNone)
				osAttr.priority = osPriorityNormal;

			m_id = osThreadNew(runnableMethodStatic, this, &osAttr);
			if (m_id == 0)
			{
#ifdef __cpp_exceptions
				throw std::system_error(osError, os_category(), "osThreadNew");
#else
				std::terminate();
#endif
			}
		}

		thread_impl(const thread_impl&) = delete;
		thread_impl& operator=(const thread_impl&) = delete;

		~thread_impl()
		{
			osThreadTerminate(m_id);
		}

		void join()
		{
			osStatus_t sta = osThreadJoin(m_id);
			if (sta != osOK)
			{
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osThreadJoin", m_id));
#else
				std::terminate();
#endif
			}

			m_detached.store(true);
		}

		void detach()
		{
			osStatus_t sta = osThreadDetach(m_id);
			if (sta != osOK)
			{
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), internal::str_error("osThreadDetach", m_id));
#else
				std::terminate();
#endif
			}

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
#ifdef __cpp_exceptions
			try
			{
#endif // __cpp_exceptions
				thread_impl* pThreadImpl = reinterpret_cast<thread_impl*>(pVThread);
				pThreadImpl->m_function->run();
#ifdef __cpp_exceptions
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
#endif // __cpp_exceptions

			osThreadExit();
		}

	private:
		osThreadId_t m_id; // task identifier
		std::unique_ptr<thread::CallableBase> m_function;
		std::atomic_bool m_detached;
	};

	thread::thread(const attributes& attr, std::unique_ptr<thread::CallableBase> base) :
		m_pThread(std::make_unique<thread_impl>(attr, std::move(base)))
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
		{
#ifdef __cpp_exceptions
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "thread::join"); // task is detached (aka auto-delete)
#else
			std::terminate();
#endif
		}

		if (thread::id(m_pThread->get_id()) == cmsis::this_thread::get_id())
		{
#ifdef __cpp_exceptions
			throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur), "thread::join");
#else
			std::terminate();
#endif
		}

		m_pThread->join();
	}

	bool thread::joinable() const
	{
		return m_pThread && m_pThread->joinable();
	}

	void thread::detach()
	{
		if (!joinable())
		{
#ifdef __cpp_exceptions
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "thread::detach"); // task is detached (aka auto-delete)
#else
			std::terminate();
#endif
		}

		m_pThread->detach();
	}

	thread::id thread::get_id() const
	{
		return m_pThread ? thread::id(m_pThread->get_id()) : thread::id();
	}

	thread::native_handle_type thread::native_handle()
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

	void thread::suspend()
	{
		osStatus_t sta = osThreadSuspend(get_id().m_tid);
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osThreadSuspend", get_id().m_tid));
#else
			std::terminate();
#endif
		}
	}

	void thread::resume()
	{
		osStatus_t sta = osThreadResume(get_id().m_tid);
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osThreadResume", get_id().m_tid));
#else
			std::terminate();
#endif
		}
	}

	void thread::priority(size_t prio)
	{
		osStatus_t sta = osThreadSetPriority(get_id().m_tid, static_cast<osPriority_t>(prio));
		if (sta != osOK)
		{
#ifdef __cpp_exceptions
			throw std::system_error(sta, os_category(), internal::str_error("osThreadSetPriority", get_id().m_tid));
#else
			std::terminate();
#endif
		}
	}

	size_t thread::priority() const
	{
		osPriority_t prio = osThreadGetPriority(get_id().m_tid);
		if (prio == osPriorityError)
		{
#ifdef __cpp_exceptions
			throw std::system_error(osError, os_category(), internal::str_error("osThreadGetPriority", get_id().m_tid));
#else
			std::terminate();
#endif
		}

		return  static_cast<size_t>(prio);
	}

	const char* thread::name() const noexcept
	{
		return osThreadGetName(get_id().m_tid);
	}


	bool thread::is_blocked() const
	{
		osThreadState_t state = osThreadGetState(get_id().m_tid);
		if (state == osThreadError)
		{
#ifdef __cpp_exceptions
			throw std::system_error(osError, os_category(), internal::str_error("osThreadGetState", get_id().m_tid));
#else
			std::terminate();
#endif
		}

		return (state == osThreadBlocked);
	}

	size_t thread::stack_size() const noexcept
	{
		return static_cast<size_t>(osThreadGetStackSize(get_id().m_tid));
	}

	size_t thread::stack_space() const noexcept
	{
		return static_cast<size_t>(osThreadGetStackSpace(get_id().m_tid));
	}

	namespace this_thread
	{
		void yield()
		{
			osStatus_t sta = osThreadYield();
			if (sta != osOK)
			{
#ifdef __cpp_exceptions
				throw std::system_error(sta, os_category(), "osThreadYield");
#else
				std::terminate();
#endif
			}
		}

		thread::id get_id()
		{
			osThreadId_t tid = osThreadGetId();
#ifdef __cpp_exceptions
			if (tid == NULL)
				throw std::system_error(osErrorResource, os_category(), "osThreadGetId");
#endif
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
					{
#ifdef __cpp_exceptions
						throw std::system_error(sta, os_category(), "osDelay");
#else
						std::terminate();
#endif
					}
				}
			}
		}
	}
}

#if !defined(OS_USE_SEMIHOSTING)

extern "C" int _getpid(void)
{
	return reinterpret_cast<int>(osThreadGetId());
}

#endif // !OS_USE_SEMIHOSTING
