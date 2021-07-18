#pragma once

#include <Arduino.h>


namespace x52 {
namespace util {


template <unsigned long LOG_PERIOD_MILLIS>
class RateLogger {
public:
	RateLogger(): m_NumUpdates(0), m_PrevLogTime(0) {}

	void OnUpdate() {
		m_NumUpdates++;
		unsigned long now = millis();
		// using delta to handle the overflows of millis()
		unsigned long elapsed = now - m_PrevLogTime;
		if (elapsed < LOG_PERIOD_MILLIS)
			return;
		Serial.print("Updates per second: ");
		Serial.println(double(m_NumUpdates) / double(elapsed) * 1000.0);
		m_NumUpdates = 0;
		m_PrevLogTime = now;
	}

private:
	unsigned long m_NumUpdates;
	unsigned long m_PrevLogTime;
};


// Rate limiter with a history buffer to be able to deal with some jitter.
// A lower NUM_STORED_TIMESTAMPS value reduces the memory consumption but
// makes the RateLimiter less effective at dealing with jitter.
template <int MAX_UPDATES_PER_SECOND, int NUM_STORED_TIMESTAMPS>
class RateLimiter {
public:
	RateLimiter() {
		m_Index = 0;
		memset(m_UpdateTimes, 0, sizeof(m_UpdateTimes));
	}

	unsigned long MicrosTillNextUpdate() {
		unsigned long now = micros();
		// using delta to handle the overflows of micros()
		unsigned long micros_left = m_UpdateTimes[m_Index] - now;
		if (long(micros_left) > 0)
			return micros_left;
		m_UpdateTimes[m_Index] = now + g_StoredPeriod;
		m_Index = (m_Index + 1) % NUM_STORED_TIMESTAMPS;
		m_UpdateTimes[m_Index] = min(m_UpdateTimes[m_Index], now + g_OneUpdatePeriod);
		return 0;
	}

private:
	int m_Index;
	unsigned long m_UpdateTimes[NUM_STORED_TIMESTAMPS];

	static constexpr unsigned long g_OneUpdatePeriod = (1000000 + MAX_UPDATES_PER_SECOND/2) / MAX_UPDATES_PER_SECOND;
	static constexpr unsigned long g_StoredPeriod = (NUM_STORED_TIMESTAMPS*1000000 + MAX_UPDATES_PER_SECOND/2) / MAX_UPDATES_PER_SECOND;
};


}  // namespace util
}  // namespace x52
