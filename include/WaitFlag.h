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

#ifndef CMSIS_WAITFLAG_H_
#define CMSIS_WAITFLAG_H_

#include <cstdint>

namespace cmsis
{
	enum class wait_flag : uint32_t
	{
		any = 0,     // Wait for any flag
		all = 1,     // Wait for all flags
		no_clear = 2 // Do not clear flags which have been specified to wait for
	};
}

inline cmsis::wait_flag operator|(cmsis::wait_flag left, cmsis::wait_flag right)
{
	return static_cast<cmsis::wait_flag>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
}

inline cmsis::wait_flag operator|=(cmsis::wait_flag left, cmsis::wait_flag right)
{
	left = left | right;
	return left;
}

inline cmsis::wait_flag operator&(cmsis::wait_flag left, cmsis::wait_flag right)
{
	return static_cast<cmsis::wait_flag>(static_cast<uint32_t>(left) & static_cast<uint32_t>(right));
}

inline cmsis::wait_flag operator&=(cmsis::wait_flag left, cmsis::wait_flag right)
{
	left = left & right;
	return left;
}

namespace sys
{
	using wait_flag = cmsis::wait_flag;
}

#endif // CMSIS_WAITFLAG_H_
