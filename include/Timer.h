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

#ifndef CPP_CMSIS_TIMER_H_INCLUDED
#define CPP_CMSIS_TIMER_H_INCLUDED

#include <chrono>
#include <functional>
#include <memory>

namespace cmsis
{
	class cmsis_timer;

	class timer
	{
	public:
		typedef std::function<bool()> callback_t; // If callback return false, the timer is stopped
		enum class timer_type_t
		{
			once,
			periodic
		};

		timer();
		timer(std::chrono::microseconds usec, callback_t&& function, timer_type_t type = timer_type_t::periodic);
		timer(const timer&) = delete;
		timer(timer&& t);
		~timer();

		void swap(timer& t) noexcept;

		timer& operator=(const timer&) = delete;
		timer& operator=(timer&& t);

		void start();
		void stop();
		bool running() const;

	private:
		std::unique_ptr<cmsis_timer> m_pImplTimer;
	};

	inline void swap(timer& __x, timer& __y) noexcept
	{
		__x.swap(__y);
	}
} // namespace cmsis

namespace sys
{
	using timer = cmsis::timer;
}

#endif // CPP_CMSIS_TIMER_H_INCLUDED
