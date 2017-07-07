#include "Timer.h"
#include "OSException.h"
#include "cmsis_os2.h"

namespace cmsis
{
	class cmsis_timer
	{
	public:
		typedef std::function<bool()> callback;

		cmsis_timer(std::chrono::microseconds usec, callback&& function, bool bOnce) :
			m_Callback(std::move(function)),
			m_id(0),
			m_usec(usec)
		{
			if (!m_Callback)
				throw std::system_error(cmsis::error_code(osErrorParameter), "timer: missing callback");

			if (m_usec < std::chrono::microseconds::zero())
				throw std::system_error(cmsis::error_code(osErrorParameter), "base_timed_mutex: negative timer");

			m_id = osTimerNew(handler, bOnce ? osTimerOnce : osTimerPeriodic, this, NULL);
			if (m_id == 0)
				throw std::system_error(cmsis::error_code(osError), "osTimerNew");
		}

		~cmsis_timer() noexcept(false)
		{
			osStatus_t sta = osTimerDelete(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osTimerDelete", m_id));
		}

		void start()
		{

			uint32_t ticks = static_cast<uint32_t>((m_usec.count() * osKernelGetTickFreq() * std::chrono::microseconds::period::num) / std::chrono::microseconds::period::den);
			if (ticks > std::numeric_limits<uint32_t>::max())
				ticks = osWaitForever;

			osStatus_t sta = osTimerStart(m_id, ticks);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osTimerStart", m_id));
		}

		void stop()
		{
			osStatus_t sta = osTimerStop(m_id);
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), internal::str_error("osTimerStop", m_id));
		}

		bool running() const
		{
			return osTimerIsRunning(m_id) != 0;
		}

		cmsis_timer(const cmsis_timer&) = delete;
		cmsis_timer& operator=(const cmsis_timer&) = delete;

	private:
		static void handler(void* argument)
		{
			cmsis_timer* pTimer = static_cast<cmsis_timer*>(argument);

			if (!pTimer->running())
				return;

			if (!pTimer->m_Callback())
				pTimer->stop();
		}

	private:
		callback m_Callback;
		osTimerId_t m_id;
		std::chrono::microseconds m_usec;
	};

	timer::timer() :
		m_pImplTimer()
	{
	}

	timer::timer(std::chrono::microseconds usec, callback&& function, bool bOnce) :
		m_pImplTimer(std::make_unique<cmsis_timer>(usec, std::move(function), bOnce))
	{
	}

	timer::timer(timer&& t) :
		m_pImplTimer(std::move(t.m_pImplTimer))
	{
	}

	timer& timer::operator=(timer&& t)
	{
		if (&t != this)
			m_pImplTimer = std::move(t.m_pImplTimer);

		return *this;
	}

	timer::~timer()
	{
	}

	void timer::swap(timer& t) noexcept
	{
		std::swap(m_pImplTimer, t.m_pImplTimer);
	}

	void timer::start()
	{
		if (!m_pImplTimer)
			throw std::system_error(cmsis::error_code(osErrorResource), "timer::start");

		m_pImplTimer->start();
	}

	void timer::stop()
	{
		if (!m_pImplTimer)
			throw std::system_error(cmsis::error_code(osErrorResource), "timer::stop");

		m_pImplTimer->stop();
 	}

	bool timer::running() const
	{
		if (!m_pImplTimer)
			return false;

		return m_pImplTimer->running();
	}
}
