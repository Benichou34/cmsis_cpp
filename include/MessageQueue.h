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

#ifndef CPP_CMSIS_MESSAGE_QUEUE_H_INCLUDED
#define CPP_CMSIS_MESSAGE_QUEUE_H_INCLUDED

#include <chrono>
#include <memory>
#include <type_traits>

namespace cmsis
{
	// mq_status
	enum class mq_status
	{
		no_timeout,
		timeout,
		full,
		empty
	};

	namespace internal
	{
		class message_queue_impl
		{
		public:
			message_queue_impl(size_t max_len, size_t ele_len);
			message_queue_impl(const message_queue_impl&) = delete;
			message_queue_impl(message_queue_impl&& t);
			~message_queue_impl() noexcept(false);

			void swap(message_queue_impl& t);

			message_queue_impl& operator=(const message_queue_impl&) = delete;
			message_queue_impl& operator=(message_queue_impl&& t);

			void put(const void* data, uint8_t priority);
			mq_status put(const void* data, uint8_t priority, std::chrono::microseconds usec);

			void get(void* data);
			mq_status get(void* data, std::chrono::microseconds usec);

			size_t size() const;
			size_t capacity() const;

			void clear();

		private:
			void* m_id;
		};
	} // namespace internal

	template <class T> // Default implementation
	class message_queue : private internal::message_queue_impl
	{
		static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value, "Only support POD type");

	public:
		typedef T element_type;

		message_queue(size_t max_len) :
			internal::message_queue_impl(max_len, sizeof(T))
		{}
		message_queue(const message_queue&) = delete;
		message_queue(message_queue&& t) :
			internal::message_queue_impl(std::move(t))
		{}
		~message_queue() = default;

		void swap(message_queue& t) noexcept { internal::message_queue_impl::swap(t); }

		message_queue& operator=(const message_queue&) = delete;
		message_queue& operator=(message_queue&& t)
		{
			internal::message_queue_impl::operator=(std::move(t));
			return *this;
		}

		void put(const element_type& data, uint8_t priority = 0) { internal::message_queue_impl::put(&data, priority); }

		template <class Rep, class Period>
		mq_status put(const element_type& data, uint8_t priority, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return internal::message_queue_impl::put(&data, priority, wait_time);
		}

		template <class Rep, class Period>
		mq_status put(const element_type& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return internal::message_queue_impl::put(&data, 0, wait_time);
		}

		element_type get()
		{
			element_type data;
			internal::message_queue_impl::get(&data);
			return data;
		}

		void get(element_type& data) { internal::message_queue_impl::get(&data); }

		template <class Rep, class Period>
		mq_status get(element_type& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return internal::message_queue_impl::get(&data, wait_time);
		}

		bool empty() const { return size() == 0; }
		size_t size() const { return internal::message_queue_impl::size(); }
		size_t capacity() const { return internal::message_queue_impl::capacity(); }

		void clear() { internal::message_queue_impl::clear(); }
	};

	template <class T> // Specialization for unique_pointer
	class message_queue<std::unique_ptr<T>> : private internal::message_queue_impl
	{
	public:
		typedef typename std::unique_ptr<T>::element_type element_type;
		typedef typename std::unique_ptr<T>::pointer pointer;

		message_queue(size_t max_len) :
			internal::message_queue_impl(max_len, sizeof(pointer))
		{}
		message_queue(const message_queue&) = delete;
		message_queue(message_queue&& t) :
			internal::message_queue_impl(std::move(t))
		{}
		~message_queue() = default;

		void swap(message_queue& t) noexcept { internal::message_queue_impl::swap(t); }

		message_queue& operator=(const message_queue&) = delete;
		message_queue& operator=(message_queue&& t)
		{
			internal::message_queue_impl::operator=(std::move(t));
			return *this;
		}

		void put(std::unique_ptr<T>&& data, uint8_t priority = 0)
		{
			pointer ptr = data.release();
			internal::message_queue_impl::put(&ptr, priority);
		}

		template <class Rep, class Period>
		mq_status put(std::unique_ptr<T>&& data, uint8_t priority, const std::chrono::duration<Rep, Period>& wait_time)
		{
			pointer ptr = data.release();
			return internal::message_queue_impl::put(&ptr, priority, wait_time);
		}

		template <class Rep, class Period>
		mq_status put(std::unique_ptr<T>&& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			pointer ptr = data.release();
			return internal::message_queue_impl::put(&ptr, 0, wait_time);
		}

		std::unique_ptr<T> get()
		{
			void* ptr = nullptr;
			internal::message_queue_impl::get(&ptr);
			return std::unique_ptr<T>(static_cast<pointer>(ptr));
		}

		void get(std::unique_ptr<T>& data)
		{
			void* ptr = nullptr;
			bool ret = internal::message_queue_impl::get(&ptr);
			data.reset(static_cast<pointer>(ptr));
		}

		template <class Rep, class Period>
		mq_status get(std::unique_ptr<T>& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			void* ptr = nullptr;
			mq_status ret = internal::message_queue_impl::get(&ptr, wait_time);
			data.reset(static_cast<pointer>(ptr));
			return ret;
		}

		bool empty() const { return size() == 0; }
		size_t size() const { return internal::message_queue_impl::size(); }
		size_t capacity() const { return internal::message_queue_impl::capacity(); }

		void clear()
		{
			// Avoid memory leak
			while (!empty())
				get();

			internal::message_queue_impl::clear();
		}
	};

	template <class T> // Specialization for pointer
	class message_queue<T*> : private internal::message_queue_impl
	{
	public:
		typedef T element_type;
		typedef typename std::add_pointer<element_type>::type pointer;

		message_queue(size_t max_len) :
			internal::message_queue_impl(max_len, sizeof(pointer))
		{}
		message_queue(const message_queue&) = delete;
		message_queue(message_queue&& t) :
			internal::message_queue_impl(std::move(t))
		{}
		~message_queue() = default;

		void swap(message_queue& t) noexcept { internal::message_queue_impl::swap(t); }

		message_queue& operator=(const message_queue&) = delete;
		message_queue& operator=(message_queue&& t)
		{
			internal::message_queue_impl::operator=(std::move(t));
			return *this;
		}

		void put(const pointer& ptr, uint8_t priority = 0) { internal::message_queue_impl::put(&ptr, priority); }

		template <class Rep, class Period>
		mq_status put(const pointer& ptr, uint8_t priority, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return internal::message_queue_impl::put(&ptr, priority, wait_time);
		}

		template <class Rep, class Period>
		mq_status put(const pointer& ptr, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return internal::message_queue_impl::put(&ptr, 0, wait_time);
		}

		pointer get()
		{
			void* ptr = nullptr;
			internal::message_queue_impl::get(&ptr);
			return static_cast<pointer>(ptr);
		}

		void get(pointer& data)
		{
			void* ptr = nullptr;
			bool ret = internal::message_queue_impl::get(&ptr);
			data = static_cast<pointer>(ptr);
		}

		template <class Rep, class Period>
		mq_status get(pointer& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			void* ptr = nullptr;
			mq_status ret = internal::message_queue_impl::get(&ptr, wait_time);
			data = static_cast<pointer>(ptr);
			return ret;
		}

		bool empty() const { return size() == 0; }
		size_t size() const { return internal::message_queue_impl::size(); }
		size_t capacity() const { return internal::message_queue_impl::capacity(); }

		void clear() { internal::message_queue_impl::clear(); }
	};

	template <class T> inline void swap(message_queue<T>& __x, message_queue<T>& __y) noexcept
	{
		__x.swap(__y);
	}
} // namespace cmsis

namespace sys
{
	using mq_status = cmsis::mq_status;
	template <class T> using message_queue = cmsis::message_queue<T>;
} // namespace sys

#endif // CPP_CMSIS_MESSAGE_QUEUE_H_INCLUDED
