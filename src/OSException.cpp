/*
 * Copyright (c) 2022, B. Leforestier
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

#include "OSException.h"
#include "cmsis_os2.h"

#ifdef RTE_CMSIS_RTOS2_RTX5
#include "rtx_os.h"
#endif

namespace
{
	class cmsis_os_error_category : public std::error_category
	{
	public:
		cmsis_os_error_category(const cmsis_os_error_category&) = delete;
		constexpr cmsis_os_error_category() = default;
		virtual ~cmsis_os_error_category() = default;

		virtual const char* name() const noexcept override { return "cmsis os"; }

		virtual std::string message(int condition) const noexcept override
		{
			switch (condition)
			{
			case osOK            : return "Operation completed successfully";
			case osError         : return "Unspecified RTOS error: run-time error but no other error message fits";
			case osErrorTimeout  : return "Operation not completed within the timeout period";
			case osErrorResource : return "Resource not available";
			case osErrorParameter: return "Parameter error";
			case osErrorNoMemory : return "System is out of memory: it was impossible to allocate or reserve memory for the operation";
			case osErrorISR      : return "Not allowed in ISR context: the function cannot be called from interrupt service routines";
			case osStatusReserved: return "Prevents enum down-size compiler optimization";
#ifdef RTE_CMSIS_RTOS2_RTX5
			case osRtxErrorStackUnderflow: return "Stack underflow detected for thread";
			case osRtxErrorISRQueueOverflow: return "ISR Queue overflow detected when inserting object";
			case osRtxErrorTimerQueueOverflow: return "User Timer Callback Queue overflow detected for timer";
			case osRtxErrorClibSpace: return "Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM";
			case osRtxErrorClibMutex: return "Standard C/C++ library mutex initialization failed";
#endif

			default : return "Unknown error";
			}
		}
	};

	class cmsis_flag_error_category : public std::error_category
	{
	public:
		cmsis_flag_error_category(const cmsis_flag_error_category&) = delete;
		constexpr cmsis_flag_error_category() = default;
		virtual ~cmsis_flag_error_category() = default;

		virtual const char* name() const noexcept override { return "cmsis flag"; }

		virtual std::string message(int condition) const noexcept override
		{
			switch (condition)
			{
			case static_cast<int>(osFlagsErrorUnknown)  : return "Generic error";
			case static_cast<int>(osFlagsErrorTimeout)  : return "A timeout was specified and the specified flags were not set, when the timeout occurred";
			case static_cast<int>(osFlagsErrorResource) : return "Try to get a flag that was not set and timeout 0 was specified, or the specified object identifier is corrupt or invalid";
			case static_cast<int>(osFlagsErrorParameter): return "A given parameter is wrong";
			case static_cast<int>(osFlagsErrorISR)      : return "Not allowed in ISR context: the function cannot be called from interrupt service routines";

			default : return "Unknown error";
			}
		}
	};
}

namespace cmsis
{
	const std::error_category& os_category()
	{
		static cmsis_os_error_category osErrorCategory;
		return osErrorCategory;
	}

	const std::error_category& flags_category()
	{
		static cmsis_flag_error_category osErrorCategory;
		return osErrorCategory;
	}

	namespace internal
	{
		std::string str_error(const std::string& func, const void* id)
		{
			std::string str = func + '(' + std::to_string(reinterpret_cast<size_t>(id)) + ')';
			return str;
		}
	}
}
