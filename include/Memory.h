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

#ifndef CMSIS_MEMORY_H_
#define CMSIS_MEMORY_H_

#include <memory>

namespace cmsis
{
	namespace internal
	{
		class base_memory_pool
		{
		protected:
			typedef void* native_handle_type;

			base_memory_pool(size_t count, size_t n);
			base_memory_pool(base_memory_pool&& other);
			~base_memory_pool() noexcept(false);

			base_memory_pool& operator=(base_memory_pool&& other);

			void* allocate(size_t n);
			void deallocate(void* p);
			size_t max_size() const noexcept;
			size_t size() const noexcept;

			native_handle_type native_handle() noexcept { return m_id; }

			base_memory_pool(const base_memory_pool&) = delete;
			base_memory_pool& operator=(const base_memory_pool&) = delete;

		private:
			native_handle_type m_id;
		};
	} // namespace internal

	template <class T> class memory_pool_delete;

	// This class satisfies allocator completeness requirements.
	template <class T> class memory_pool : private internal::base_memory_pool
	{
	public:
		typedef internal::base_memory_pool Base;
		typedef typename Base::native_handle_type native_handle_type;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef memory_pool_delete<T> deleter_type;

		memory_pool(size_type count) :
			Base(count, sizeof(T))
		{}
		memory_pool(memory_pool&& other) :
			Base(std::move(other))
		{}
		~memory_pool() noexcept(false) = default;

		memory_pool& operator=(memory_pool&& other)
		{
			Base::operator=(std::move(other));
			return *this;
		}

		pointer address(reference x) const noexcept { return std::addressof(x); }
		const_pointer address(const_reference x) const noexcept { return std::addressof(x); }

		pointer allocate(size_type n = 1, const void* = 0) { return static_cast<pointer>(Base::allocate(n)); }
		void deallocate(pointer p, size_type) { Base::deallocate(p); }
		size_type max_size() const noexcept { return Base::max_size(); }
		size_type size() const noexcept { return Base::size(); }

		template <class U, class... _Args> void construct(U* p, _Args&&... args)
		{
			::new ((void*)p) U(std::forward<_Args>(args)...);
		}

		template <class U> void destroy(U* p) { p->~U(); }

		deleter_type get_deleter() noexcept { return deleter_type(*this); }

		template <class... Args> std::unique_ptr<T, deleter_type> make_unique(Args&&... args)
		{
			std::unique_ptr<T, deleter_type> ptr(allocate(), get_deleter());
			construct(ptr.get(), std::forward<Args>(args)...);
			return ptr;
		}

		template <class... Args> std::shared_ptr<T> make_shared(Args&&... args)
		{
			std::shared_ptr<T> ptr(allocate(), get_deleter());
			construct(ptr.get(), std::forward<Args>(args)...);
			return ptr;
		}

		native_handle_type native_handle() noexcept { return Base::native_handle(); }

		memory_pool(const memory_pool&) = delete;
		memory_pool& operator=(const memory_pool&) = delete;
	};

	template <class T> class memory_pool_delete
	{
	public:
		memory_pool_delete(memory_pool<T>& mempool) :
			m_mempool(mempool)
		{}
		memory_pool_delete(const memory_pool_delete& m) :
			m_mempool(m.m_mempool)
		{}
		~memory_pool_delete() = default;

		void operator()(void* p) const
		{
			if (p)
			{
				m_mempool.destroy(static_cast<T*>(p));
				m_mempool.deallocate(static_cast<T*>(p), 1);
			}
		}

		memory_pool_delete& operator=(const memory_pool_delete&) = delete;

	private:
		memory_pool<T>& m_mempool;
	};

	template <class T> bool operator==(const memory_pool<T>& lhs, const memory_pool<T>& rhs)
	{
		return lhs.native_handle() == rhs.native_handle();
	}

	template <class T> bool operator!=(const memory_pool<T>& lhs, const memory_pool<T>& rhs)
	{
		return lhs.native_handle() != rhs.native_handle();
	}

	template <class T1, class T2> bool operator==(const memory_pool<T1>& lhs, const memory_pool<T2>& rhs)
	{
		return lhs.native_handle() == rhs.native_handle();
	}

	template <class T1, class T2> bool operator!=(const memory_pool<T1>& lhs, const memory_pool<T2>& rhs)
	{
		return lhs.native_handle() != rhs.native_handle();
	}
} // namespace cmsis

namespace sys
{
	template <class T> using memory_pool = cmsis::memory_pool<T>;
	template <class T> using memory_pool_delete = cmsis::memory_pool_delete<T>;
} // namespace sys

#endif // CMSIS_MEMORY_H_
