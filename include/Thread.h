/*
 * Copyright (c) 2017, B. Leforestier
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

#ifndef CMSIS_THREAD_H_
#define CMSIS_THREAD_H_

#include <memory>
#include <thread>

namespace cmsis
{
	class thread_impl;

	// STL like implementation
	class thread
	{
	public:
		typedef void* native_handle_type;

		class id
		{
		public:
			id() noexcept : m_tid(0) { }
			explicit id(native_handle_type __id) : m_tid(__id) { }

		private:
			friend class thread;
			friend class std::hash<thread::id>;

			friend bool	operator==(thread::id __x, thread::id __y) noexcept	{ return __x.m_tid == __y.m_tid; }
			friend bool	operator<(thread::id __x, thread::id __y) noexcept { return __x.m_tid < __y.m_tid; }

			template<class _CharT, class _Traits>
			friend std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __out, thread::id __id);

		private:
			native_handle_type m_tid;
		};

		thread() noexcept = default;
		thread(thread&& __t) noexcept;

		template<typename _Callable, typename... _Args>
		explicit thread(_Callable&& __f, _Args&&... __args) : thread(make_routine(std::bind(std::forward<_Callable>(__f), std::forward<_Args>(__args)...))) { }

		virtual ~thread();

		thread& operator=(const thread&) = delete;
		thread& operator=(thread&& __t);

		void join();

		void detach();

		bool joinable() const; // checks whether the thread is joinable

		id get_id() const;

		native_handle_type native_handle() const;

		void swap(thread& __t);

		// Returns a value that hints at the number of hardware thread contexts.
		static unsigned int	hardware_concurrency() noexcept;

		// Simple base type that the templatized, derived class containing an arbitrary functor can be converted to and called.
		struct CallableBase
		{
			virtual ~CallableBase() = default;
			virtual void run() = 0;
		};

	private:
		template<typename _Callable>
		struct CallableImpl : public CallableBase
		{
			CallableImpl(_Callable&& __f) : m_func(std::forward<_Callable>(__f)) { }
			virtual void run() override { m_func(); }
			_Callable m_func;
		};

	    template<typename _Callable>
	    std::unique_ptr<CallableBase> make_routine(_Callable&& __f)
		{
	    	// Create and allocate full data structure, not base.
	    	return std::unique_ptr<CallableBase>(new CallableImpl<_Callable>(std::forward<_Callable>(__f)));
		}

	    thread(std::unique_ptr<CallableBase> base);

	private:
		std::unique_ptr<thread_impl> m_pThread;
	};

	inline void	swap(thread& __x, thread& __y) noexcept	{ __x.swap(__y); }

	inline bool	operator!=(thread::id __x, thread::id __y) noexcept	{ return !(__x == __y); }

	inline bool	operator<=(thread::id __x, thread::id __y) noexcept	{ return !(__y < __x); }

	inline bool	operator>(thread::id __x, thread::id __y) noexcept	{ return __y < __x; }

	inline bool	operator>=(thread::id __x, thread::id __y) noexcept	{ return !(__x < __y); }

	template<class _CharT, class _Traits>
	inline std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __out, thread::id __id)
	{
		if (__id == thread::id())
			return __out << "task::id of a non-executing thread";
		else
			return __out << __id.m_tid;
	}

	namespace this_thread
	{
		namespace internal
		{
			void sleep_for_usec(std::chrono::microseconds usec);
		}

		// Provides a hint to the implementation to reschedule the execution of threads, allowing other threads to run.
		void yield();

		 // Returns the id of the current thread.
		thread::id get_id();

		template<class Rep, class Period>
		void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration)
		{
			internal::sleep_for_usec(sleep_duration);
		}

		template<class Clock, class Duration>
		void sleep_until(const std::chrono::time_point<Clock, Duration>& sleep_time)
		{
			sleep_for(sleep_time - Clock::now());
		}
	}
}

namespace std
{
	/// std::hash specialization for thread::id.
	template<>
	struct hash<cmsis::thread::id> : public __hash_base<size_t, cmsis::thread::id>
	{
		size_t operator()(const cmsis::thread::id& __id) const noexcept
		{ return std::hash<cmsis::thread::native_handle_type>()(__id.m_tid); }
	};

#if !defined(GLIBCXX_HAS_GTHREADS) && !defined(_GLIBCXX_HAS_GTHREADS)
	using thread = cmsis::thread;
	namespace this_thread = cmsis::this_thread;
#endif
}

#endif // CMSIS_THREAD_H_
