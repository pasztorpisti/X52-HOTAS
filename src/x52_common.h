// Things shared by the pro and std versions.
#pragma once

#include <Arduino.h>
#include <assert.h>


// X52_DEBUG turns serial logging on/off.
// Don't forget to turn it off in your finished "product".
#ifndef X52_DEBUG
	#define X52_DEBUG 0
#endif

#if X52_DEBUG
	#ifndef X52DebugPrint
		#define X52DebugPrint(x) Serial.print(x)
	#endif
	#ifndef X52DebugPrintln
		#define X52DebugPrintln(x) {X52DebugPrint(x); X52DebugPrint("\n");}
	#endif
#else
	#undef X52DebugPrint
	#undef X52DebugPrintln
	#define X52DebugPrint(x)
	#define X52DebugPrintln(x)
#endif

#ifndef X52_BUSY_WAIT
	#define X52_BUSY_WAIT 1
#endif


namespace x52 {


// Direction flag.
// An instance of this type is a bitwise combination of zero or more enum members.
enum Direction {
	NoDirection = 0,
	Right       = 1,
	Down        = 2,
	Left        = 4,
	Up          = 8,
	DownLeft    = Down | Left,
	DownRight   = Down | Right,
	UpLeft      = Up   | Left,
	UpRight     = Up   | Right,
};


// The Mode enum defines the possible states of the rotary mode selector on the joystick.
enum Mode {
	ModeUndefined,  // It's possible to receive ModeUndefined from the joystick!
	Mode1,
	Mode2,
	Mode3,
};


typedef unsigned int uint;
enum Uninitialized { UNINITIALIZED };


// BitField has little endian byte and bit order.
template <int NUM_BITS>
class BitField {
public:
	BitField() {
		memset(m_Bits, 0, sizeof(m_Bits));
	}

	BitField(Uninitialized) {}

	bool Bit(int index) const {
		assert(uint(index) < NUM_BITS);
		return bool(m_Bits[index>>3] & (1 << (index&7)));
	}

	void SetBit(int index, bool bit) {
		if (bit)
			SetBit(index);
		else
			ClearBit(index);
	}

	void SetBit(int index) {
		assert(uint(index) < NUM_BITS);
		m_Bits[index>>3] |= 1 << (index&7);
	}

	void ClearBit(int index) {
		assert(uint(index) < NUM_BITS);
		m_Bits[index>>3] &= ~(1 << (index&7));
	}

	uint UInt(int index, int width) const {
		assert(uint(index) + uint(width) <= NUM_BITS);
		int v = 0;
		for (int i=0; i<width; i++)
			v |= int((1 << i) & -Bit(index+i));
		return v;
	}

	void SetUInt(int index, int width, uint value) {
		assert(uint(index) + uint(width) <= NUM_BITS);
		for (int i=0; i<width; i++)
			SetBit(index+i, bool(value & (1 << i)));
	}

	uint8_t BufByte(int index) const {
		assert(uint(index) < sizeof(m_Bits));
		return m_Bits[index];
	}

	void SetBufByte(int index, uint8_t b) {
		assert(uint(index) < sizeof(m_Bits));
		m_Bits[index] = b;
	}

private:
	uint8_t m_Bits[(NUM_BITS+7)/8];
};


inline bool wait_for_pin_state(
	uint8_t pin,
	int state,
	unsigned long deadline_micros,
	unsigned long poll_period_micros=(X52_BUSY_WAIT?0:5)
) {
	for (;;) {
		if (digitalRead(pin) == state)
			return true;
		// using delta to handle the overflows of micros()
		if (long(micros() - deadline_micros) >= 0)
			return false;
		// On AVR the delay is also busy but it might be able
		// to save some power on other architectures (ARM).
		if (poll_period_micros)
			delayMicroseconds(poll_period_micros);
	}
}


}  // namespace x52
