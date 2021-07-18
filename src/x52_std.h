#pragma once

#include "x52_common.h"


// I don't have a non-Pro throttle to measure its timeout values.
// Fortunately there is no need for an accurate value. It has to be only an OK
// one that makes it possible to recover from clock desyncs that happen rarely.
#ifndef X52_THROTTLE_TIMEOUT_MICROS
	#define X52_THROTTLE_TIMEOUT_MICROS 15000
#endif

// This value is hardcoded into the firmware of my original X52 joystick.
#ifndef X52_JOYSTICK_TIMEOUT_MICROS
	#define X52_JOYSTICK_TIMEOUT_MICROS 40000
#endif

#ifndef X52_THROTTLE_UNRESPONSIVE_MICROS
	#define X52_THROTTLE_UNRESPONSIVE_MICROS 3000
#endif

#ifndef X52_JOYSTICK_UNRESPONSIVE_MICROS
	#define X52_JOYSTICK_UNRESPONSIVE_MICROS 1000
#endif

#ifndef X52_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS
	#define X52_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS 100000
#endif

#ifndef X52_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS
	#define X52_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS 100000
#endif

#ifndef X52_FIRST_C04_PULSE_MICROS
	#define X52_FIRST_C04_PULSE_MICROS 15
#endif

#ifndef X52_SECOND_C04_PULSE_MICROS
	#define X52_SECOND_C04_PULSE_MICROS 50
#endif



namespace x52 {
namespace std {


// JoystickState is the data sent by the joystick through the PS/2 cable.
struct JoystickState {
	static constexpr uint16_t MAX_X = 2047;
	static constexpr uint16_t MAX_Y = 2047;
	static constexpr uint16_t MAX_Z = 1023;

	static constexpr uint16_t CENTER_X = 1024;
	static constexpr uint16_t CENTER_Y = 1024;
	static constexpr uint16_t CENTER_Z = 512;

	uint16_t x;  // valid_range: 0..2047(MAX_X)  center: 1024(CENTER_X)
	uint16_t y;  // valid_range: 0..2047(MAX_Y)  center: 1024(CENTER_Y)
	uint16_t z;  // valid_range: 0..1023(MAX_Z)  center: 512(CENTER_Z)

	Direction pov_1;
	Direction pov_2;
	Mode mode;

	bool trigger_stage_1: 1;
	bool trigger_stage_2: 1;
	bool pinkie_switch: 1;
	bool button_fire: 1;
	bool button_a: 1;
	bool button_b: 1;
	bool button_c: 1;
	bool button_t1: 1;
	bool button_t2: 1;
	bool button_t3: 1;
	bool button_t4: 1;
	bool button_t5: 1;
	bool button_t6: 1;

	JoystickState() {
		memset(this, 0, sizeof(*this));
	}

	// Constructor for the pros who believe they know what they are doing.
	JoystickState(Uninitialized) {}

	static constexpr int NUM_BITS = 64;
	typedef BitField<NUM_BITS> Binary;

	bool SetFromBinary(const Binary&);
	void ToBinary(Binary&) const;

	static uint8_t Checksum(const Binary&);
};


// JoystickConfig is the data sent by the throttle through the PS/2 cable.
struct JoystickConfig {
	static constexpr uint8_t MAX_LED_BRIGHTNESS = 127;

	uint8_t led_brightness;   // valid range: 0..127 (MAX_LED_BRIGHTNESS)
	bool pov_1_led_blinking;  // blinks about 25 times per second

	JoystickConfig() {
		led_brightness = MAX_LED_BRIGHTNESS;
		pov_1_led_blinking = false;
	}

	// Constructor for the pros who believe they know what they are doing.
	JoystickConfig(Uninitialized) {}

	static constexpr int NUM_BITS = 8;
	typedef BitField<NUM_BITS> Binary;

	void SetFromBinary(const Binary&);
	void ToBinary(Binary&) const;
};


inline void JoystickConfig::SetFromBinary(const Binary& b) {
	led_brightness = uint8_t(b.UInt(0, 7));
	pov_1_led_blinking = b.Bit(7);
}


inline void JoystickConfig::ToBinary(Binary& b) const {
	b.SetUInt(0, 7, led_brightness);
	b.SetBit(7, pov_1_led_blinking);
}


inline bool JoystickState::SetFromBinary(const Binary& b) {
	if (Checksum(b) != b.BufByte(7))
		return false;

	x = uint16_t(b.UInt(0, 8) | (b.UInt(16, 3) << 8));
	y = uint16_t(b.UInt(8, 8) | (b.UInt(19, 3) << 8));
	z = uint16_t(b.UInt(24, 8) | (b.UInt(22, 2) << 8));

	switch (b.UInt(32, 4)) {
		case 1: pov_1 = Up; break;
		case 2: pov_1 = UpRight; break;
		case 3: pov_1 = Right; break;
		case 4: pov_1 = DownRight; break;
		case 5: pov_1 = Down; break;
		case 6: pov_1 = DownLeft; break;
		case 7: pov_1 = Left; break;
		case 8: pov_1 = UpLeft; break;
		default: pov_1 = NoDirection; break;
	}

	pov_2 = Direction(
		(-b.Bit(36) & Right) |
		(-b.Bit(37) & Down) |
		(-b.Bit(38) & Left) |
		(-b.Bit(39) & Up)
	);

	switch (b.UInt(54, 2)) {
		case 0: mode = b.Bit(47) ? Mode1 : ModeUndefined; break;
		case 1: mode = Mode2; break;
		case 2: mode = Mode3; break;
		default: mode = ModeUndefined; break;
	}

	trigger_stage_1 = b.Bit(40);
	trigger_stage_2 = b.Bit(41);
	button_fire = b.Bit(42);
	button_a = b.Bit(43);
	button_b = b.Bit(44);
	button_c = b.Bit(45);
	pinkie_switch = b.Bit(46);
	button_t1 = b.Bit(48);
	button_t2 = b.Bit(49);
	button_t3 = b.Bit(50);
	button_t4 = b.Bit(51);
	button_t5 = b.Bit(52);
	button_t6 = b.Bit(53);

	return true;
}


inline void JoystickState::ToBinary(Binary& b) const {
#if X52_DEBUG
	if (x > MAX_X) {
		X52DebugPrint("x52::std::JoystickState.x exceeds the maximum value. x=");
		X52DebugPrintln(x);
	}
	if (y > MAX_Y) {
		X52DebugPrint("x52::std::JoystickState.y exceeds the maximum value. y=");
		X52DebugPrintln(y);
	}
	if (z > MAX_Z) {
		X52DebugPrint("x52::std::JoystickState.z exceeds the maximum value. z=");
		X52DebugPrintln(z);
	}
#endif

	b.SetUInt(0, 8, x);
	b.SetUInt(8, 8, y);
	b.SetUInt(24, 8, z);

	b.SetUInt(16, 3, x >> 8);
	b.SetUInt(19, 3, y >> 8);
	b.SetUInt(22, 2, z >> 8);

	uint8_t pov;
	switch (pov_1) {
		case Up: pov = 1; break;
		case UpRight: pov = 2; break;
		case Right: pov = 3; break;
		case DownRight: pov = 4; break;
		case Down: pov = 5; break;
		case DownLeft: pov = 6; break;
		case Left: pov = 7; break;
		case UpLeft: pov = 8; break;
		default: pov = 0; break;
	}
	b.SetUInt(32, 4, pov);

	b.SetBit(36, bool(pov_2 & Right));
	b.SetBit(37, bool(pov_2 & Down));
	b.SetBit(38, bool(pov_2 & Left));
	b.SetBit(39, bool(pov_2 & Up));

	b.SetBit(47, mode == Mode1);
	switch (mode) {
		case Mode2: b.SetUInt(54, 2, 1); break;
		case Mode3: b.SetUInt(54, 2, 2); break;
		default: b.SetUInt(54, 2, 0); break;
	}

	b.SetBit(40, trigger_stage_1);
	b.SetBit(41, trigger_stage_2);
	b.SetBit(42, button_fire);
	b.SetBit(43, button_a);
	b.SetBit(44, button_b);
	b.SetBit(45, button_c);
	b.SetBit(46, pinkie_switch);
	b.SetBit(48, button_t1);
	b.SetBit(49, button_t2);
	b.SetBit(50, button_t3);
	b.SetBit(51, button_t4);
	b.SetBit(52, button_t5);
	b.SetBit(53, button_t6);

	b.SetBufByte(7, Checksum(b));
}

inline uint8_t JoystickState::Checksum(const Binary& b) {
	uint8_t chksum = b.BufByte(0);
	for (int i=1; i<7; i++)
		chksum ^= b.BufByte(i);
	return chksum;
}


enum PulseWaitResult {
	PulseFinished,
	PulseStarted,
	PulseNotStarted,
	TooManyPulses,
};


// The BitBangPulseWaiter is the naive implementation that can miss a pulse
// especially on a slower MCU. On an Arduino Micro (ATMega32U4 16MHz) this
// implementation can miss pulses every few minutes. That results in desync
// issues for a short period so the joystick is unresponsive for 500-1000ms
// and the LEDs turn off for a fraction of a second.
//
// This bit-banging worked well on my 96MHz teensy 3.2. However, it's better
// to be safe than sorry and use the InterruptPulseWaiter whenever possible.
// That works well on slower MCUs too but it requires a pin that can trigger
// interrupts on falling edges. That isn't a huge requirement. I'd use this
// BitBangPulseWaiter only if I had no interrupt pins available (basically "never").
// This naive implementation is still useful as a form of documentation because
// the code explains very clearly what we want to achieve with the more
// complicated interrupt based solution.
template <int PIN_C04>
class BitBangPulseWaiter {
public:
	void Setup() {}

	template <typename PulseTriggerFunc>
	PulseWaitResult WaitForPulse(unsigned long deadline, PulseTriggerFunc trigger) {
		trigger();
		if (!wait_for_pin_state(PIN_C04, HIGH, deadline, 0))
			return PulseNotStarted;
		if (!wait_for_pin_state(PIN_C04, LOW, deadline, 0))
			return PulseStarted;
		return PulseFinished;
	}
};


// The InterruptPulseWaiter attaches an interrupt handler to the C04 pin to be
// able to reliably detect pulses. This whole "waiting for a pulse" problem
// affects only the non-Pro joystick. My X52 Pro uses a different protocol that
// is completely bitbang-friendly even on slower MCUs (as long as they are fast
// enough to transmit the whole frame within 17ms).
template <int PIN_C04>
class InterruptPulseWaiter {
public:
	void Setup() {
		attachInterrupt(digitalPinToInterrupt(PIN_C04), InterruptHandler, FALLING);
	}

	template <typename PulseTriggerFunc>
	PulseWaitResult WaitForPulse(unsigned long deadline, PulseTriggerFunc trigger) {
		int c0 = Counter();
		trigger();
		for (;;) {
			int c = Counter();
			if (c != c0)
				return (c == c0 + 1) ? PulseFinished : TooManyPulses;
			// using delta to handle the overflows of micros()
			unsigned long micros_left = deadline - micros();
			if (long(micros_left) <= 0)
				return digitalRead(PIN_C04) ? PulseStarted : PulseNotStarted;
#if !X52_BUSY_WAIT
			delayMicroseconds(min(10, micros_left));
#endif
		}
	}

private:
	static int Counter() {
		noInterrupts();
		int c = m_Counter;
		interrupts();
		return c;
	}

	static void InterruptHandler() {
		m_Counter++;
	}

	static volatile int m_Counter;
};

template <int PIN_C04>
volatile int InterruptPulseWaiter<PIN_C04>::m_Counter = 0;


// JoystickClient makes it possible to use some of your Arduino pins as a
// connection to the PS/2 socket of an X52 (non-Pro) Joystick.
//
// These pin names (C01..C04) were printed on the PCB of my X52 joystick.
// The pinout of the PS/2 connector is the same as that of the X52 Pro
// but the protocol is different.
//
// Standard PS/2 (6-pin mini-DIN) female socket pin numbering:
// https://en.wikipedia.org/wiki/Mini-DIN_connector#/media/File:MiniDIN-6_Connector_Pinout.svg
//
// PIN_C01: data output of the throttle   (pin #4 of the PS/2 female socket)
// PIN_C02: clock output of the throttle  (pin #6 of the PS/2 female socket)
// PIN_C03: data output of the joystick   (pin #2 of the PS/2 female socket)
// PIN_C04: clock output of the joystick  (pin #1 of the PS/2 female socket)
//
// Pin #3 of the PS/2 female socket is GND.
// Pin #5 of the PS/2 female socket is VCC.
//
// My joystick claims to be 5V 500mW but works with 3.3V too.
template <int PIN_C01, int PIN_C02, int PIN_C03, int PIN_C04, typename PulseWaiter=InterruptPulseWaiter<PIN_C04>>
class JoystickClient {
public:
	// Call Setup from the setup function of your Arduino project to initialize
	// a JoystickClient instance.
	void Setup() {
		pinMode(PIN_C01, OUTPUT);
		// The value of C01 is allowed to be anything between frames (undefined).
		// digitalWrite(PIN_C01, LOW);
		pinMode(PIN_C02, OUTPUT);
		// On the teensy the digitalWrite seems to work only after pinMode.
		digitalWrite(PIN_C02, LOW);
		pinMode(PIN_C03, INPUT);
		pinMode(PIN_C04, INPUT);
		m_PulseWaiter.Setup();
	}

	// PollJoystickState polls the joystick for its state. It creates a frame
	// transmission request, waits for the joystick to respond and handles the
	// bidirectional data transfer: sends the JoystickConfig and receives the
	// JoystickState. Returns zero on success.
	//
	// A nonzero return value means error and gives the recommended number
	// of microseconds to wait before calling PollJoystickState again.
	// In that situation the value of the JoystickState is undefined.
	//
	// My X52 joystick is willing to send its state at most ~50 times per second.
	unsigned long PollJoystickState(JoystickState& state, const JoystickConfig& cfg, unsigned long wait_micros=X52_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS) {
		// Note: PIN_C02 has to be LOW when this function returns.

		{
			// deadline for the initial response
			unsigned long wait_deadline = micros()+wait_micros;

			if (!wait_for_pin_state(PIN_C04, LOW, wait_deadline))
				return 1;

			// The original joystick's C04 pulse seems to be at least 15us long.

			auto trigger = [](){
				digitalWrite(PIN_C02, HIGH);
			};
			auto wait_res = m_PulseWaiter.WaitForPulse(wait_deadline, trigger);
			if (wait_res != PulseFinished) {
				X52DebugPrintln("Timed out while waiting for the C04 pulse before receiving the joystick state.");
				digitalWrite(PIN_C02, LOW);
				return (wait_res == PulseNotStarted) ? 1 : X52_THROTTLE_UNRESPONSIVE_MICROS;
			}
		}

		// deadline for the whole frame transmission
		unsigned long deadline = micros() + X52_THROTTLE_TIMEOUT_MICROS;

		JoystickState::Binary recv_buf;

		for (int i=0; i<(JoystickState::NUM_BITS-1); i++) {

			// I don't have an X52 throttle to test this but the throttle must
			// be sampling C03 between falling-C04 and falling-C02.
			// The other sensible option (between rising-C04 and rising-C02)
			// wouldn't work because the joystick often removes the data bit
			// from C03 before the rising-C02 edge.
			recv_buf.SetBit(i, bool(digitalRead(PIN_C03)));

			digitalWrite(PIN_C02, LOW);

			if (!wait_for_pin_state(PIN_C04, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C04=1 while receiving the joystick state. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C02, LOW);
				return X52_THROTTLE_UNRESPONSIVE_MICROS;
			}

			digitalWrite(PIN_C02, HIGH);

			if (!wait_for_pin_state(PIN_C04, LOW, deadline)) {
				X52DebugPrint("Error waiting for C04=0 while receiving the joystick state. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C02, LOW);
				return X52_THROTTLE_UNRESPONSIVE_MICROS;
			}
		}
		recv_buf.SetBit(JoystickState::NUM_BITS-1, bool(digitalRead(PIN_C03)));

		// The original joystick's C04 pulse seems to be at least 50us long.

		auto trigger = [](){
			digitalWrite(PIN_C02, LOW);
		};
		auto wait_res = m_PulseWaiter.WaitForPulse(deadline, trigger);
		if (wait_res != PulseFinished) {
			X52DebugPrintln("Timed out while waiting for the C04 pulse before sending the joystick config.");
			return X52_THROTTLE_UNRESPONSIVE_MICROS;
		}

		JoystickConfig::Binary send_buf;
		cfg.ToBinary(send_buf);

		for (int i=0; i<JoystickConfig::NUM_BITS; i++) {
			// The joystick samples C01 between rising-C02 and rising-C04 (last 8 rising edges of C02)
			digitalWrite(PIN_C01, send_buf.Bit(i));
			digitalWrite(PIN_C02, HIGH);
			if (!wait_for_pin_state(PIN_C04, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C04=1 while sending the joystick config. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C02, LOW);
				return X52_THROTTLE_UNRESPONSIVE_MICROS;
			}
			digitalWrite(PIN_C02, LOW);
			if (!wait_for_pin_state(PIN_C04, LOW, deadline)) {
				X52DebugPrint("Error waiting for C04=0 while sending the joystick config. Clock cycle: ");
				X52DebugPrintln(i);
				return X52_THROTTLE_UNRESPONSIVE_MICROS;
			}
		}
		// The value of C01 is allowed to be anything between frames (undefined).
		// digitalWrite(PIN_C01, LOW);

		// SetFromBinary verifies the checksum and returns false on error
		return state.SetFromBinary(recv_buf) ? 0 : X52_THROTTLE_UNRESPONSIVE_MICROS;
	}

private:
	PulseWaiter m_PulseWaiter;
};


// ThrottleClient makes it possible to use some of your Arduino pins as a
// connection to the PS/2 socket of an X52 (non-Pro) Throttle.
//
// The pin config is the same as that of the JoystickClient.
template <int PIN_C01, int PIN_C02, int PIN_C03, int PIN_C04>
class ThrottleClient {
public:
	// Call Setup from the setup function of your Arduino project to initialize
	// a ThrottleClient instance.
	void Setup() {
		pinMode(PIN_C01, INPUT);
		pinMode(PIN_C02, INPUT);
		pinMode(PIN_C03, OUTPUT);
		pinMode(PIN_C04, OUTPUT);
		// On the teensy the digitalWrite seems to work only after pinMode.
		digitalWrite(PIN_C04, LOW);
	}

	// SendJoystickState sends the JoystickState to the throttle and receives
	// the JoystickConfig. On the other side there must be a throttle polling/waiting
	// for the JoystickState. Returns zero on success.
	//
	// A nonzero return value means error and gives the number of microseconds
	// to wait before calling SendJoystickState again. In that situation the
	// value of the JoystickConfig is undefined.
	unsigned long SendJoystickState(const JoystickState& state, JoystickConfig& cfg, unsigned long wait_micros=X52_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS) {
		if (!wait_for_pin_state(PIN_C02, HIGH, micros()+wait_micros))
			return 1;

		JoystickState::Binary send_buf;
		state.ToBinary(send_buf);

		auto deadline = micros() + X52_JOYSTICK_TIMEOUT_MICROS;

		// The first data bit has to be on C03 before the falling edge of C04
		digitalWrite(PIN_C03, send_buf.Bit(0));

		// The first C04 pulse that doesn't require an ACK from the throttle
		digitalWrite(PIN_C04, HIGH);
		// The original joystick uses a >=15us pulse and I don't have a throttle to test shorter pulses.
		delayMicroseconds(X52_FIRST_C04_PULSE_MICROS);
		digitalWrite(PIN_C04, LOW);

		// The throttle samples C03 for the first data bit here between falling-C04 and falling-C02.

		if (!wait_for_pin_state(PIN_C02, LOW, deadline)) {
			X52DebugPrintln("Error waiting for C02=0 while sending the first bit of the joystick state.");
			return X52_JOYSTICK_UNRESPONSIVE_MICROS;
		}

		// sending the rest of the joystick state
		for (int i=1; i<JoystickState::NUM_BITS; i++) {

			// I don't have an X52 throttle to test this but the throttle must
			// be sampling C03 between falling-C04 and falling-C02.
			// The other sensible option (between rising-C04 and rising-C02)
			// wouldn't work because the joystick often removes the data bit
			// from C03 before the rising-C02 edge.

			// The data bit has to be on C03 before the falling edge of C04.
			digitalWrite(PIN_C03, send_buf.Bit(i));

			digitalWrite(PIN_C04, HIGH);
			if (!wait_for_pin_state(PIN_C02, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C02=1 while sending the joystick state. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C04, LOW);
				return X52_JOYSTICK_UNRESPONSIVE_MICROS;
			}

			digitalWrite(PIN_C04, LOW);
			// This is where the throttle samples C03 for the data bit
			if (!wait_for_pin_state(PIN_C02, LOW, deadline)) {
				X52DebugPrint("Error waiting for C02=0 while sending the joystick state. Clock cycle: ");
				X52DebugPrintln(i);
				return X52_JOYSTICK_UNRESPONSIVE_MICROS;
			}
		}

		// The second C04 pulse that doesn't require an ACK from the throttle
		digitalWrite(PIN_C04, HIGH);
		// The original joystick uses a >=50us pulse and I don't have a throttle to test shorter pulses.
		delayMicroseconds(X52_SECOND_C04_PULSE_MICROS);
		digitalWrite(PIN_C04, LOW);

		JoystickConfig::Binary recv_buf;

		// receiving the config from the throttle
		for (int i=0; i<JoystickConfig::NUM_BITS; i++) {
			if (!wait_for_pin_state(PIN_C02, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C02=1 while receiving the joystick config. Clock cycle: ");
				X52DebugPrintln(i);
				return X52_JOYSTICK_UNRESPONSIVE_MICROS;
			}
			// The joystick samples C01 between rising-C02 and rising-C04 (last 8 rising edges of C02)
			recv_buf.SetBit(i, bool(digitalRead(PIN_C01)));
			digitalWrite(PIN_C04, HIGH);
			if (!wait_for_pin_state(PIN_C02, LOW, deadline)) {
				X52DebugPrint("Error waiting for C02=0 while receiving the joystick config. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C04, LOW);
				return X52_JOYSTICK_UNRESPONSIVE_MICROS;
			}
			digitalWrite(PIN_C04, LOW);
		}

		cfg.SetFromBinary(recv_buf);
		return 0;
	}

	// IsPollInProgress returns true if the throttle is waiting for the
	// JoystickState on the other side of the connection. In that situation
	// a call to SendJoystickState is less likely to block in a waiting state
	// (or fail as a result of wait timeout).
	bool IsPollInProgress() {
		return bool(digitalRead(PIN_C02));
	}
};


}  // namespace std
}  // namespace x52
