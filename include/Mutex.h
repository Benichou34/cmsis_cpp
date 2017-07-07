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

#ifndef CMSIS_MUTEX_H_
#define CMSIS_MUTEX_H_

#include <mutex>

namespace cmsis
{
	namespace internal
	{
		class base_timed_mutex
		{
		protected:
			typedef void* native_handle_type;

			base_timed_mutex(const char* name, bool recursive);
			~base_timed_mutex() noexcept(false);

			void lock();
			void unlock();
			bool try_lock();
			
			template<class Rep, class Period>
			bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
			{
				return try_lock_for_usec(rel_time);
			}
			
			template<class Clock, class Duration>
			bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
			{
				return try_lock_for(abs_time - Clock::now());
			}

			native_handle_type native_handle() noexcept { return m_id; }

			base_timed_mutex(const base_timed_mutex&) = delete;
			base_timed_mutex& operator=(const base_timed_mutex&) = delete;

		private:
			bool try_lock_for_usec(std::chrono::microseconds usec);

		private:
			native_handle_type m_id;                    ///< mutex identifier
		};
	}
	
	// STL like implementation
	class mutex : private internal::base_timed_mutex
	{
	public:
		typedef internal::base_timed_mutex::native_handle_type native_handle_type;

		mutex() : internal::base_timed_mutex("mutex", false) {}
		~mutex() = default;

		void lock() { internal::base_timed_mutex::lock(); }
		void unlock() { internal::base_timed_mutex::unlock(); }
		bool try_lock() { return internal::base_timed_mutex::try_lock(); }

		native_handle_type native_handle() noexcept { return internal::base_timed_mutex::native_handle(); }

		mutex(const mutex&) = delete;
		mutex& operator=(const mutex&) = delete;
	};

	class recursive_mutex : private internal::base_timed_mutex
	{
	public:
		typedef internal::base_timed_mutex::native_handle_type native_handle_type;

		recursive_mutex() : internal::base_timed_mutex("recursive_mutex", true) {}
		~recursive_mutex() = default;

		void lock() { internal::base_timed_mutex::lock(); }
		void unlock() { internal::base_timed_mutex::unlock(); }
		bool try_lock() { return internal::base_timed_mutex::try_lock(); }

		native_handle_type native_handle() noexcept { return internal::base_timed_mutex::native_handle(); }

		recursive_mutex(const recursive_mutex&) = delete;
		recursive_mutex& operator=(const recursive_mutex&) = delete;
	};

	class timed_mutex : private internal::base_timed_mutex
	{
	public:
		typedef internal::base_timed_mutex::native_handle_type native_handle_type;

		timed_mutex() : internal::base_timed_mutex("timed_mutex", false) {}
		~timed_mutex() = default;

		void lock() { internal::base_timed_mutex::lock(); }
		void unlock() { internal::base_timed_mutex::unlock(); }
		bool try_lock() { return internal::base_timed_mutex::try_lock(); }

		template<class Rep, class Period>
		bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return internal::base_timed_mutex::try_lock_for(rel_time);
		}
		
		template<class Clock, class Duration>
		bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			return internal::base_timed_mutex::try_lock_until(abs_time);
		}

		native_handle_type native_handle() noexcept { return internal::base_timed_mutex::native_handle(); }

		timed_mutex(const timed_mutex&) = delete;
		timed_mutex& operator=(const timed_mutex&) = delete;
	};

	class recursive_timed_mutex : private internal::base_timed_mutex
	{
	public:
		typedef internal::base_timed_mutex::native_handle_type native_handle_type;

		recursive_timed_mutex() : internal::base_timed_mutex("recursive_timed_mutex", true) {}
		~recursive_timed_mutex() = default;

		void lock() { internal::base_timed_mutex::lock(); }
		void unlock() { internal::base_timed_mutex::unlock(); }
		bool try_lock() { return internal::base_timed_mutex::try_lock(); }

		template<class Rep, class Period>
		bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return internal::base_timed_mutex::try_lock_for(rel_time);
		}
		
		template<class Clock, class Duration>
		bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			return internal::base_timed_mutex::try_lock_until(abs_time);
		}

		native_handle_type native_handle() noexcept { return internal::base_timed_mutex::native_handle(); }

		recursive_timed_mutex(const recursive_timed_mutex&) = delete;
		recursive_timed_mutex& operator=(const recursive_timed_mutex&) = delete;
	};
}

#if !defined(GLIBCXX_HAS_GTHREADS) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace std
{
	using mutex = cmsis::mutex;
	using recursive_mutex = cmsis::recursive_mutex;
	using timed_mutex = cmsis::timed_mutex;
	using recursive_timed_mutex = cmsis::recursive_timed_mutex;
}
#endif

#endif // CMSIS_MUTEX_H_
