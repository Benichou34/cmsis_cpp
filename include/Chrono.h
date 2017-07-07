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

#ifndef CMSIS_CHRONO_H_
#define CMSIS_CHRONO_H_

#include <chrono>

namespace cmsis
{
	namespace chrono
	{
		struct system_clock
		{
			typedef std::chrono::microseconds duration;
			typedef duration::rep rep;
			typedef duration::period period;
			typedef std::chrono::time_point<system_clock, duration> time_point;
			static constexpr bool is_steady = true;

			static_assert(system_clock::duration::min()
				< system_clock::duration::zero(),
				"a clock's minimum duration cannot be less than its epoch");

			static time_point now() noexcept;
			static std::time_t to_time_t(const time_point& t);
			static time_point from_time_t(std::time_t t);
		};

		struct high_resolution_clock
		{
			typedef std::chrono::nanoseconds duration;
			typedef duration::rep rep;
			typedef duration::period period;
			typedef std::chrono::time_point<high_resolution_clock, duration> time_point;
			static constexpr bool is_steady = true;

			static time_point now() noexcept;
		};

		using steady_clock = high_resolution_clock;
	}
}

namespace sys
{
	namespace chrono = cmsis::chrono;
}

#endif // CMSIS_CHRONO_H_
