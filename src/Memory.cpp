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

#include "Memory.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	namespace internal
	{
		base_memory_pool::base_memory_pool(size_t count, size_t n) :
			m_id(0)
		{
			m_id = osMemoryPoolNew(static_cast<uint32_t>(count), static_cast<uint32_t>(n), NULL);
			if (!m_id)
				throw std::system_error(cmsis::error_code(osError), "osMemoryPoolNew");
		}

		base_memory_pool::base_memory_pool(base_memory_pool&& other) :
			m_id(other.m_id)
		{
			other.m_id = 0;
		}

		base_memory_pool::~base_memory_pool() noexcept(false)
		{
			if (m_id)
			{
				osStatus_t sta = osMemoryPoolDelete(m_id);
				if (sta != osOK)
					throw std::system_error(cmsis::error_code(sta), internal::str_error("osMemoryPoolDelete", m_id));
			}
		}

		base_memory_pool& base_memory_pool::operator=(base_memory_pool&& other)
		{
			if (this != &other)
			{
				if (m_id)
				{
					osStatus_t sta = osMemoryPoolDelete(m_id);
					if (sta != osOK)
						throw std::system_error(cmsis::error_code(sta), internal::str_error("osMemoryPoolDelete", m_id));
					m_id = 0;
				}

				std::swap(m_id, other.m_id);
			}

			return *this;
		}

		void* base_memory_pool::allocate(size_t n)
		{
			if (n != 1)
				throw std::bad_alloc();

			void* p = osMemoryPoolAlloc(m_id, osWaitForever);
			if (!p)
				throw std::bad_alloc();

			return p;
		}

		void base_memory_pool::deallocate(void* p)
		{
			osStatus_t sta = osMemoryPoolFree(m_id, p);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osMemoryPoolFree", m_id));
		}

		size_t base_memory_pool::max_size() const noexcept
		{
			return osMemoryPoolGetCapacity(m_id);
		}

		size_t base_memory_pool::size() const noexcept
		{
			return osMemoryPoolGetCount(m_id);
		}
	}
}
