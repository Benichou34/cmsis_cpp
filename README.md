# CMSIS C++
A C++11/C++14 interface for CMSIS-RTOS API Version 2.

This source code implements some classes of the STL, based on [CMSIS-RTOS API Version 2](https://arm-software.github.io/CMSIS_5/RTOS2/html/index.html) interface.
You must use a C++ compiler that supports the C++11/C++14 standard, like [GNU ARM Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm).

## C++ Standard
### Thread
Defined in header "Thread.h"

This header is part of the [thread support](http://en.cppreference.com/w/cpp/thread) library. It provides a full implementation of STL [<thread>](http://en.cppreference.com/w/cpp/header/thread) interfaces.
You can directly use [std::thread](http://en.cppreference.com/w/cpp/thread/thread) and [std::thread::id](http://en.cppreference.com/w/cpp/thread/thread/id) classes, and std::this_thread namespace.

Threads are created in a join-able state, with default thread priority (osPriorityNormal) and default stack size from the [Global Memory Pool](https://arm-software.github.io/CMSIS_5/RTOS2/html/theory_of_operation.html#GlobalMemoryPool). See [Thread Management](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html) for more details.

### Mutex
Defined in header "Mutex.h"

This header is part of the [thread support](http://en.cppreference.com/w/cpp/thread) library. It provides a partial implementation of STL [<mutex>](http://en.cppreference.com/w/cpp/header/mutex) interfaces.
You can directly use [std::mutex](http://en.cppreference.com/w/cpp/thread/thread), [std::timed_mutex](http://en.cppreference.com/w/cpp/thread/timed_mutex), [std::recursive_mutex](http://en.cppreference.com/w/cpp/thread/recursive_mutex) and [std::recursive_timed_mutex](http://en.cppreference.com/w/cpp/thread/recursive_timed_mutex) classes.
[once_flag](http://en.cppreference.com/w/cpp/thread/once_flag) and [call_once](http://en.cppreference.com/w/cpp/thread/call_once) are not implemented yet.

Mutexes are created with Priority inheritance protocol (osMutexPrioInherit flag). While a thread owns this mutex it cannot be preempted by a higher priority thread to avoid starvation. See [Mutex Management](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html) for more details.

Mutex management functions cannot be called from [Interrupt Service Routines](https://arm-software.github.io/CMSIS_5/RTOS2/html/theory_of_operation.html#CMSIS_RTOS_ISR_Calls) (ISR), unlike a binary semaphore that can be released from an ISR.

### Chrono
Defined in header "Chrono.h"

This header is part of the [date and time](http://en.cppreference.com/w/cpp/chrono) library. It provides a full implementation of STL [<chrono>](http://en.cppreference.com/w/cpp/header/chrono) interfaces. [std::chrono::system_clock](http://en.cppreference.com/w/cpp/chrono/system_clock) and [std::chrono::high_resolution_clock](http://en.cppreference.com/w/cpp/chrono/high_resolution_clock) are implemented using the osKernelGetTickCount() function.
If you need more precision, you can use sys::chrono::high\_resolution\_clock that is implemented with  osKernelGetSysTimerCount() function.

### Dynamic memory managment (new, delete)
Globals operators [new](http://en.cppreference.com/w/cpp/memory/new/operator_new) and [delete](http://en.cppreference.com/w/cpp/memory/new/operator_delete) are overridden for using the [Global Memory Pool](https://arm-software.github.io/CMSIS_5/RTOS2/html/theory_of_operation.html#GlobalMemoryPool). For now, this part is specific to RTX5 implementation, and need to be ported for other RTOS (like [FreeRTOS](http://www.freertos.org)).

### Exceptions
Defined in header "OSException.h"

In case of failure, methods throws a [std::system_error](http://en.cppreference.com/w/cpp/error/system_error) exception. [std::error_code::value](http://en.cppreference.com/w/cpp/error/error_code/value) contains the [CMSIS error code](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__Definitions.html#ga6c0dbe6069e4e7f47bb4cd32ae2b813e).

## CMSIS Specific
This part is a group of some CMSIS specific classes, but as close as possible of the "STL spirit".

### Kernel Information and Control
Defined in header "OS.h"

#### std::string sys::os::version()
Get RTOS Kernel version. Returns a string that contains version information. In case of failure, throws a std::system_error exception.

#### uint32_t sys::os::tick_frequency();
Get RTOS Kernel tick frequency in Hz. Returns the frequency of the current RTOS kernel tick. In case of failure, throws a std::system_error exception.

#### void sys::os::initialize()
Initialize the RTOS Kernel. In case of failure, throws a std::system_error exception.

#### void sys::os::start()
Start the RTOS Kernel scheduler. In case of success, this function will never returns. In case of failure, throws a std::system_error exception.

#### uint32_t sys::core::clock_frequency();
Get system core clock frequency in Hz. Returns the frequency of the current system core clock. In case of failure, throws a std::system_error exception.

### Semaphore
Defined in header "Semaphore.h"

[Semaphores](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html) are used to manage and protect access to shared resources.

Class sys::semaphore.

### Event Flags
Defined in header "EventFlag.h"

The [event flags management](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__EventFlags.html) allow you to control or wait for event flags. Each signal has up to 31 event flags.

Class sys::event.

### Timer
Defined in header "Timer.h"

Timer Management, class sys::timer.

### Message Queue
Defined in header "MessageQueue.h"

[Message Queue](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__Message.html) exchange messages between threads in a FIFO-like operation.

class sys::message_queue.

### Memory Pool
Defined in header "Memory.h"

[Memory Pools](https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html) are fixed-size blocks of memory that are thread-safe.


class sys::memory\_pool\_delete is a Deleter (like [std::default_delete](http://en.cppreference.com/w/cpp/memory/default_delete)) associated to an memory pool.
 
Be carreful with memory pools and smart pointers. Don't delete a memory pool with living associated smart pointers.

## Exemple
```
#include <iostream>
#include "OS.h"
#include "Thread.h"
#include "Timer.h"
#include "Chrono.h"

int main()
{
	cmsis::os::initialize();

	std::thread main_thread([]
	{
		std::cout << cmsis::os::version() << std::endl;
		std::cout << "Core Clock: " << sys::core::clock_frequency() << "Hz" << std::endl;

		try
		{
			std::chrono::system_clock::time_point tp0 = std::chrono::system_clock::now();

			sys::timer ledTimer(std::chrono::seconds(1), [&]
			{
				std::cout << "now: " << (std::chrono::system_clock::now() - tp0).count() << std::endl;
				return true;
			});
			ledTimer.start();

			sys::chrono::high_resolution_clock::time_point tp1 = sys::chrono::high_resolution_clock::now();

			for (;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(900));

				std::cout << "hrc: " << (sys::chrono::high_resolution_clock::now() - tp1).count() << std::endl;
			}
		}
		catch(std::exception& e)
		{
			std::cout << "ERROR: " << e.what() << std::endl;
		}
	});

	cmsis::os::start();
	return 0;
}
```
