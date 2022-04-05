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

#include <mutex>
#include "Threads.h"
#include "OS.h"
#include "cmsis_os2.h"

namespace cmsis
{
	size_t threads::count() noexcept
	{
		return static_cast<size_t>(osThreadGetCount());
	}

	std::vector<threads::info> threads::enumerate() noexcept
	{
		cmsis::dispatch dptch;
		std::lock_guard<cmsis::dispatch> lg(dptch);

		std::vector<osThreadId_t> thread_array(count());
		uint32_t nb = osThreadEnumerate(thread_array.data(), thread_array.size());

		std::vector<threads::info> infos(nb);
		for(size_t i = 0; i < infos.size(); ++i)
		{
			osThreadId_t tid = thread_array[i];
			infos[i].handle = tid;
			infos[i].name = osThreadGetName(tid);
			infos[i].state = static_cast<size_t>(osThreadGetState(tid));
			infos[i].priority = static_cast<size_t>(osThreadGetPriority(tid));
			infos[i].stack_size = static_cast<size_t>(osThreadGetStackSize(tid));
			infos[i].stack_space = static_cast<size_t>(osThreadGetStackSpace(tid));
		}

		return infos;
	}
}
