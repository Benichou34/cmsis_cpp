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

#include "Chrono.h"
#include "cmsis_os2.h"
#include <type_traits>

namespace
{
	template <class D> D convertDuration(uint64_t count, uint32_t freq)
	{
		return D((count * D::period::den) / (freq * D::period::num));
	}
} // namespace

namespace cmsis
{
	namespace chrono
	{
		system_clock::time_point system_clock::now() noexcept
		{
			return time_point(convertDuration<system_clock::duration>(osKernelGetTickCount(), osKernelGetTickFreq()));
		}

		std::time_t system_clock::to_time_t(const time_point& __t)
		{
			return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(__t.time_since_epoch()).count());
		}

		system_clock::time_point system_clock::from_time_t(std::time_t __t)
		{
			typedef std::chrono::time_point<system_clock, std::chrono::seconds> __from;
			return std::chrono::time_point_cast<system_clock::duration>(__from(std::chrono::seconds(__t)));
		}

		high_resolution_clock::time_point high_resolution_clock::now() noexcept
		{
			return time_point(convertDuration<high_resolution_clock::duration>(
				osKernelGetSysTimerCount(),
				osKernelGetSysTimerFreq()));
		}
	} // namespace chrono
} // namespace cmsis

#if !defined(OS_USE_SEMIHOSTING)
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/time.h>

	int _gettimeofday(struct timeval*, void*);

	int _gettimeofday(struct timeval* tp, void* tzvp)
	{
		if (tp != NULL)
		{
			uint64_t now =
				convertDuration<std::chrono::microseconds>(osKernelGetTickCount(), osKernelGetTickFreq()).count();
			tp->tv_sec = static_cast<time_t>(now / 1000000);
			tp->tv_usec = static_cast<long>(now % 1000000);
		}

		if (tzvp != NULL)
		{
			struct timezone* tzp = static_cast<struct timezone*>(tzvp);
			tzp->tz_minuteswest = 0;
			tzp->tz_dsttime = 0;
		}

		return 0;
	}
#ifdef __cplusplus
}
#endif
#endif // !OS_USE_SEMIHOSTING
