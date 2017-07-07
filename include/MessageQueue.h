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

#ifndef CMSIS_MESSAGE_QUEUE_H_INCLUDED
#define CMSIS_MESSAGE_QUEUE_H_INCLUDED

#include <chrono>
#include <memory>
#include <type_traits>

namespace cmsis
{
	namespace internal
	{
		class message_queue_impl
		{
		public:
			message_queue_impl(size_t max_len, size_t ele_len);
			message_queue_impl(const message_queue_impl&) = delete;
			~message_queue_impl() noexcept(false);

			message_queue_impl& operator=(const message_queue_impl&) = delete;

			void send(const void* data);
			bool send(const void* data, std::chrono::microseconds usec);

			void* receive();
			bool receive(void*& data, std::chrono::microseconds usec);

			size_t size() const;
			size_t capacity() const;

		private:
			void* m_id;
		};
	}

	template <class T, typename Enable = void> // Default implementation
	class message_queue
	{
	public:
		typedef T element_type;
		enum class status { no_timeout, timeout };

		message_queue(size_t max_len) : m_pImpl(std::make_unique<internal::message_queue_impl>(max_len, sizeof(T*))) {}
		message_queue(const message_queue&) = delete;
		message_queue(message_queue&& t) : m_pImpl(std::move(t.m_pImpl)) {}
		~message_queue() = default;

		void swap(message_queue& t) noexcept { std::swap(m_pImpl, t.m_pImpl); }

		message_queue& operator=(const message_queue&) = delete;
		message_queue& operator=(message_queue&& t)
		{
			if (&t != this)
				m_pImpl = std::move(t.m_pImpl);

			return *this;
		}

		void send(std::unique_ptr<element_type>&& data)
		{
			m_pImpl->send(&data.release());
		}

		template<class Rep, class Period>
		status send(std::unique_ptr<element_type>&& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return m_pImpl->send(&data.release(), wait_time) ? status::no_timeout : status::timeout;
		}

		std::unique_ptr<element_type> receive()
		{
			return std::unique_ptr<element_type>(static_cast<element_type*>(m_pImpl->receive()));
		}

		template<class Rep, class Period>
		status receive(std::unique_ptr<element_type>& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			void* ptr = nullptr;
			bool ret = m_pImpl->receive(ptr, wait_time);
			data.reset(static_cast<element_type*>(ptr));
			return ret ? status::no_timeout : status::timeout;
		}

		bool empty() const { return size() == 0; }
		size_t size() const { return m_pImpl->size(); }
		size_t capacity() const { return m_pImpl->capacity(); }

	private:
		std::unique_ptr<internal::message_queue_impl> m_pImpl;
	};

	template <class T> // Partial specialization for pointer
	class message_queue<T, typename std::enable_if<std::is_pointer<T>::value>::type>
	{
	public:
		typedef T element_type;
		enum class status { no_timeout, timeout };

		message_queue(size_t max_len) : m_pImpl(std::make_unique<internal::message_queue_impl>(max_len, sizeof(T))) {}
		message_queue(const message_queue&) = delete;
		message_queue(message_queue&& t) : m_pImpl(std::move(t.m_pImpl)) {}
		~message_queue() = default;

		void swap(message_queue& t) noexcept { std::swap(m_pImpl, t.m_pImpl); }

		message_queue& operator=(const message_queue&) = delete;
		message_queue& operator=(message_queue&& t)
		{
			if (&t != this)
				m_pImpl = std::move(t.m_pImpl);

			return *this;
		}

		void send(element_type ptr)
		{
			m_pImpl->send(&ptr);
		}

		template<class Rep, class Period>
		status send(element_type ptr, const std::chrono::duration<Rep, Period>& wait_time)
		{
			return m_pImpl->send(&ptr, wait_time) ? status::no_timeout : status::timeout;
		}

		element_type receive()
		{
			return static_cast<element_type>(m_pImpl->receive());
		}

		template<class Rep, class Period>
		status receive(element_type& data, const std::chrono::duration<Rep, Period>& wait_time)
		{
			void* ptr = nullptr;
			bool ret = m_pImpl->receive(ptr, wait_time);
			data = static_cast<element_type>(ptr);
			return ret ? status::no_timeout : status::timeout;
		}

		bool empty() const { return size() == 0; }
		size_t size() const { return m_pImpl->size(); }
		size_t capacity() const { return m_pImpl->capacity(); }

	private:
		std::unique_ptr<internal::message_queue_impl> m_pImpl;
	};

	template <class T>
	inline void	swap(message_queue<T>& __x, message_queue<T>& __y) noexcept { __x.swap(__y); }
}

namespace sys
{
	template<class T>
	using message_queue = cmsis::message_queue<T>;
}

#endif // CMSIS_MESSAGE_QUEUE_H_INCLUDED
