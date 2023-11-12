/*
 * Copyright (c) 2023, B. Leforestier
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

#ifndef CPP_CMSIS_SEMAPHORE_H_
#define CPP_CMSIS_SEMAPHORE_H_

#include <chrono>

namespace cmsis
{
	namespace internal
	{
		class base_semaphore
		{
		public:
			typedef void* native_handle_type;

			base_semaphore(std::ptrdiff_t max, std::ptrdiff_t desired);
			~base_semaphore() noexcept(false);

			void release(std::ptrdiff_t update = 1);
			void acquire();
			bool try_acquire() noexcept;

			template <class Rep, class Period> bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
			{
				return try_acquire_for_usec(std::chrono::duration_cast<std::chrono::microseconds>(rel_time));
			}

			template <class Clock, class Duration>
			bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
			{
				auto rel_time = abs_time - Clock::now();
				if (rel_time < std::chrono::microseconds::zero())
					return false;

				return try_acquire_for(rel_time);
			}

			native_handle_type native_handle() noexcept { return m_id; }

			base_semaphore(const base_semaphore&) = delete;
			base_semaphore& operator=(const base_semaphore&) = delete;

		private:
			bool try_acquire_for_usec(std::chrono::microseconds usec);

		private:
			native_handle_type m_id; ///< sempahore identifier
		};
	} // namespace internal

	template <std::ptrdiff_t LeastMaxValue = 0xFFFFFFFF> class counting_semaphore : private internal::base_semaphore
	{
	public:
		typedef internal::base_semaphore::native_handle_type native_handle_type;

		constexpr explicit counting_semaphore(std::ptrdiff_t desired) :
			internal::base_semaphore(max(), desired)
		{}
		~counting_semaphore() = default;

		void release(std::ptrdiff_t update = 1) { internal::base_semaphore::release(update); }
		void acquire() { internal::base_semaphore::acquire(); }
		bool try_acquire() noexcept { return internal::base_semaphore::try_acquire(); }

		template <class Rep, class Period> bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			return internal::base_semaphore::try_acquire_for(rel_time);
		}

		template <class Clock, class Duration>
		bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			return internal::base_semaphore::try_acquire_until(abs_time);
		}

		static constexpr std::ptrdiff_t max() noexcept { return LeastMaxValue; }

		native_handle_type native_handle() noexcept { return internal::base_semaphore::native_handle(); }

		counting_semaphore(const counting_semaphore&) = delete;
		counting_semaphore& operator=(const counting_semaphore&) = delete;
	};

	using binary_semaphore = counting_semaphore<1>;
} // namespace cmsis

#if !defined(GLIBCXX_HAS_GTHREADS) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace std
{
	template <std::ptrdiff_t LeastMaxValue = 0xFFFFFFFF>
	using counting_semaphore = cmsis::counting_semaphore<LeastMaxValue>;
	using binary_semaphore = cmsis::binary_semaphore;
} // namespace std
#endif

#endif // CPP_CMSIS_SEMAPHORE_H_
