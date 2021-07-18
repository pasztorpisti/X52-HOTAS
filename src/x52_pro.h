#pragma once

#include "x52_common.h"


// This value is hardcoded into the firmware of my original X52 Pro throttle.
#ifndef X52_PRO_THROTTLE_TIMEOUT_MICROS
	#define X52_PRO_THROTTLE_TIMEOUT_MICROS 17000
#endif

// This value is hardcoded into the firmware of my original X52 Pro joystick.
#ifndef X52_PRO_JOYSTICK_TIMEOUT_MICROS
	#define X52_PRO_JOYSTICK_TIMEOUT_MICROS 23000
#endif

#ifndef X52_PRO_THROTTLE_UNRESPONSIVE_MICROS
	#define X52_PRO_THROTTLE_UNRESPONSIVE_MICROS (X52_PRO_JOYSTICK_TIMEOUT_MICROS + 5000)
#endif

// This value is hardcoded into the firmware of my original X52 Pro joystick.
#ifndef X52_PRO_JOYSTICK_UNRESPONSIVE_MICROS
	#define X52_PRO_JOYSTICK_UNRESPONSIVE_MICROS 2000
#endif

// This value is hardcoded into the firmware of my original X52 Pro joystick.
// It has to be greater than X52_PRO_THROTTLE_TIMEOUT_MICROS.
#ifndef X52_PRO_JOYSTICK_DESYNC_UNRESPONSIVE_MICROS
	#define X52_PRO_JOYSTICK_DESYNC_UNRESPONSIVE_MICROS 23000
#endif

// Enabling this feature allows the ThrottleClient to behave differently from
// the original joystick in order to make the desync detection more reliable.
#ifndef X52_PRO_IMPROVED_THROTTLE_CLIENT_DESYNC_DETECTION
	#define X52_PRO_IMPROVED_THROTTLE_CLIENT_DESYNC_DETECTION 0
#endif

// Enabling this feature allows the JoystickClient to behave differently from
// the original throttle in order to make the desync detection more reliable.
#ifndef X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION
	#define X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION 0
#endif

#ifndef X52_PRO_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS
	#define X52_PRO_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS 25000
#endif

#ifndef X52_PRO_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS
	#define X52_PRO_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS 25000
#endif


namespace x52 {
namespace pro {


enum LEDColor {
	Amber = 0,  // 00
	Green = 1,  // 10
	Red = 2,    // 01
	Off = 3,    // 11
};


// JoystickState is the data sent by the joystick through the PS/2 cable.
struct JoystickState {
	static constexpr uint16_t MAX_X = 1023;
	static constexpr uint16_t MAX_Y = 1023;
	static constexpr uint16_t MAX_Z = 1023;

	static constexpr uint16_t CENTER_X = 512;
	static constexpr uint16_t CENTER_Y = 512;
	static constexpr uint16_t CENTER_Z = 512;

	uint16_t x;  // valid_range: 0..1023(MAX_X)  center: 512(CENTER_X)
	uint16_t y;  // valid_range: 0..1023(MAX_Y)  center: 512(CENTER_Y)
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

	static constexpr int NUM_BITS = 56;
	typedef BitField<NUM_BITS> Binary;

	void SetFromBinary(const Binary&);
	void ToBinary(Binary&) const;
};


// JoystickConfig is the data sent by the throttle through the PS/2 cable.
struct JoystickConfig {
	static constexpr uint8_t MAX_LED_BRIGHTNESS = 31;

	uint8_t led_brightness;   // valid range: 0..31 (MAX_LED_BRIGHTNESS)
	bool pov_1_led_blinking;  // blinks about 4 times per second
	bool button_fire_led;

	LEDColor pov_2_led: 2;
	LEDColor button_a_led: 2;
	LEDColor button_b_led: 2;
	LEDColor button_t1_t2_led: 2;
	LEDColor button_t3_t4_led: 2;
	LEDColor button_t5_t6_led: 2;

	JoystickConfig() {
		led_brightness = MAX_LED_BRIGHTNESS;
		pov_1_led_blinking = false;
		button_fire_led = true;
		pov_2_led = Green;
		button_a_led = Green;
		button_b_led = Green;
		button_t1_t2_led = Green;
		button_t3_t4_led = Green;
		button_t5_t6_led = Green;
	}

	// Constructor for the pros who believe they know what they are doing.
	JoystickConfig(Uninitialized) {}

	static constexpr int NUM_BITS = 19;
	typedef BitField<NUM_BITS> Binary;

	void SetFromBinary(const Binary&);
	void ToBinary(Binary&) const;
};


inline void JoystickConfig::SetFromBinary(const Binary& b) {
	led_brightness = uint8_t(b.UInt(0, 5));
	pov_1_led_blinking = b.Bit(5);
	button_a_led = LEDColor(b.UInt(6, 2));
	pov_2_led = LEDColor(b.UInt(8, 2));
	button_fire_led = !b.Bit(10);
	button_b_led = LEDColor(b.UInt(11, 2));
	button_t1_t2_led = LEDColor(b.UInt(13, 2));
	button_t3_t4_led = LEDColor(b.UInt(15, 2));
	button_t5_t6_led = LEDColor(b.UInt(17, 2));
}


inline void JoystickConfig::ToBinary(Binary& b) const {
	b.SetUInt(0, 5, led_brightness);
	b.SetBit(5, pov_1_led_blinking);
	b.SetUInt(6, 2, button_a_led);
	b.SetUInt(8, 2, pov_2_led);
	b.SetBit(10, !button_fire_led);
	b.SetUInt(11, 2, button_b_led);
	b.SetUInt(13, 2, button_t1_t2_led);
	b.SetUInt(15, 2, button_t3_t4_led);
	b.SetUInt(17, 2, button_t5_t6_led);
}


inline void JoystickState::SetFromBinary(const Binary& b) {
	x = uint16_t(b.UInt(0, 8) | (b.UInt(16, 2) << 8));
	y = uint16_t(b.UInt(8, 8) | (b.UInt(18, 2) << 8));
	z = uint16_t(b.UInt(24, 8) | (b.UInt(22, 2) << 8));

	switch (b.UInt(32, 4)) {
		case 1: pov_1 = Down; break;
		case 2: pov_1 = DownRight; break;
		case 3: pov_1 = Right; break;
		case 4: pov_1 = UpRight; break;
		case 5: pov_1 = Up; break;
		case 6: pov_1 = UpLeft; break;
		case 7: pov_1 = Left; break;
		case 8: pov_1 = DownLeft; break;
		default: pov_1 = NoDirection; break;
	}

	pov_2 = Direction(
		(-b.Bit(36) & Up) |
		(-b.Bit(37) & Right) |
		(-b.Bit(38) & Down) |
		(-b.Bit(39) & Left)
	);

	switch (b.UInt(45, 3)) {
		case 1: mode = Mode1; break;
		case 2: mode = Mode2; break;
		case 4: mode = Mode3; break;
		default: mode = ModeUndefined; break;
	}

	trigger_stage_1 = b.Bit(40);
	button_fire = b.Bit(41);
	button_a = b.Bit(42);
	button_c = b.Bit(43);
	trigger_stage_2 = b.Bit(44);
	button_b = b.Bit(48);
	pinkie_switch = b.Bit(49);
	button_t1 = b.Bit(50);
	button_t2 = b.Bit(51);
	button_t3 = b.Bit(52);
	button_t4 = b.Bit(53);
	button_t5 = b.Bit(54);
	button_t6 = b.Bit(55);
}


inline void JoystickState::ToBinary(Binary& b) const {
#if X52_DEBUG
	if (x > MAX_X) {
		X52DebugPrint("x52::pro::JoystickState.x exceeds the maximum value. x=");
		X52DebugPrintln(x);
	}
	if (y > MAX_Y) {
		X52DebugPrint("x52::pro::JoystickState.y exceeds the maximum value. y=");
		X52DebugPrintln(y);
	}
	if (z > MAX_Z) {
		X52DebugPrint("x52::pro::JoystickState.z exceeds the maximum value. z=");
		X52DebugPrintln(z);
	}
#endif

	b.SetUInt(0, 8, x);
	b.SetUInt(8, 8, y);
	b.SetUInt(24, 8, z);

	b.SetUInt(16, 2, x >> 8);
	b.SetUInt(18, 2, y >> 8);
	b.SetUInt(20, 2, 0);
	b.SetUInt(22, 2, z >> 8);

	uint8_t pov;
	switch (pov_1) {
		case Down: pov = 1; break;
		case DownRight: pov = 2; break;
		case Right: pov = 3; break;
		case UpRight: pov = 4; break;
		case Up: pov = 5; break;
		case UpLeft: pov = 6; break;
		case Left: pov = 7; break;
		case DownLeft: pov = 8; break;
		default: pov = 0; break;
	}
	b.SetUInt(32, 4, pov);

	b.SetBit(36, bool(pov_2 & Up));
	b.SetBit(37, bool(pov_2 & Right));
	b.SetBit(38, bool(pov_2 & Down));
	b.SetBit(39, bool(pov_2 & Left));

	b.SetBit(40, trigger_stage_1);
	b.SetBit(41, button_fire);
	b.SetBit(42, button_a);
	b.SetBit(43, button_c);
	b.SetBit(44, trigger_stage_2);
	b.SetBit(45, mode == Mode1);
	b.SetBit(46, mode == Mode2);
	b.SetBit(47, mode == Mode3);
	b.SetBit(48, button_b);
	b.SetBit(49, pinkie_switch);
	b.SetBit(50, button_t1);
	b.SetBit(51, button_t2);
	b.SetBit(52, button_t3);
	b.SetBit(53, button_t4);
	b.SetBit(54, button_t5);
	b.SetBit(55, button_t6);
}


// JoystickClient makes it possible to use some of your Arduino pins as a
// connection to the PS/2 socket of an X52 Pro Joystick.
//
// These pin names (C01..C04) were printed on the PCB of my X52 joystick.
// The pinout of the PS/2 connector is the same as that of the X52 non-Pro
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
// My X52 Pro throttle uses 4.1-4.2V for both power and GPIO but the joystick
// works with 3.3V too.
template <int PIN_C01, int PIN_C02, int PIN_C03, int PIN_C04>
class JoystickClient {
public:
	// Call Setup from the setup function of your Arduino project to initialize
	// a JoystickClient instance.
	void Setup() {
		pinMode(PIN_C01, OUTPUT);
		pinMode(PIN_C02, OUTPUT);
		// On the teensy the digitalWrite seems to work only after pinMode.
		digitalWrite(PIN_C02, LOW);
#if X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION
		digitalWrite(PIN_C01, HIGH);
#endif
		pinMode(PIN_C03, INPUT);
		pinMode(PIN_C04, INPUT);
	}

	// PollJoystickState polls the joystick for its state. It creates a frame
	// transmission request, waits for the joystick to respond and handles the
	// bidirectional data transfer: sends the JoystickConfig and receives the
	// JoystickState. Returns zero on success.
	//
	// A nonzero return value means error and gives the recommended number
	// of microseconds to wait before calling PollJoystickState again.
	// In that situation the value of the JoystickState is undefined.
	unsigned long PollJoystickState(JoystickState& state, const JoystickConfig& cfg, unsigned long wait_micros=X52_PRO_DEFAULT_POLL_JOYSTICK_STATE_WAIT_MICROS) {
		// PIN_C02 has to be LOW when this function returns.
		//
		// If X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION==1
		// then PIN_C01 has to be HIGH when this function returns.

		JoystickState::Binary recv_buf;
		JoystickConfig::Binary send_buf;
		cfg.ToBinary(send_buf);

		// deadline for the whole frame transmission
		unsigned long deadline = micros() + wait_micros;

		// A frame consists of 76 clock pulses on both C02 and C04.
		for (int i=0; i<76; i++) {
#if !X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION
			// This is what the original throttle does but we have a potentially better solution.
			if (i == 1)
				digitalWrite(PIN_C01, HIGH);
			else
#endif
			if (i >= 57)
				digitalWrite(PIN_C01, send_buf.Bit(i-57));

			digitalWrite(PIN_C02, HIGH);

			// The original joystick samples C01 here between the
			// rising edge of C02 and the rising edge of C04.

			if (!wait_for_pin_state(PIN_C04, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C04=1. Clock cycle: ");
				X52DebugPrintln(i);
#if X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION
				if (i >= 57)
					digitalWrite(PIN_C01, HIGH);
#endif
				digitalWrite(PIN_C02, LOW);
				// Timing out with i==0 means that the joystick didn't respond to our
				// initial C02=1 request within the available time frame defined by `wait_micros`.
				if (i == 0)
					return 1;
				// We are in the middle of a frame (already started talking with the joystick).
				// This means that the joystick will also time out and the throttle should
				// should try to initiate a new frame only after the joystick's timeout.
				return X52_PRO_THROTTLE_UNRESPONSIVE_MICROS;
			}

			if (i == 0)
				// The original throttle times out if the whole frame isn't
				// transmitted within X52_PRO_THROTTLE_TIMEOUT_MICROS
				// measured from the first falling edge of C02.
				// The previous deadline value was set up using `wait_micros`,
				// that timeout applies only to the first rising edge of C04.
				deadline = micros() + X52_PRO_THROTTLE_TIMEOUT_MICROS;
			else if (i == 56)
				digitalWrite(PIN_C01, LOW);  // desync detection: the joystick becomes unresponsive if this isn't LOW
#if X52_PRO_IMPROVED_JOYSTICK_CLIENT_DESYNC_DETECTION
			// The original throttle doesn't do this but perhaps it should
			// because this makes desync detection more reliable.
			// The last iteration of this for loop ends with C01=HIGH
			// and C01 stays that way until the i==56 of the next frame.
			else if (i >= 57)
				digitalWrite(PIN_C01, HIGH);
#endif

			digitalWrite(PIN_C02, LOW);

			if (!wait_for_pin_state(PIN_C04, LOW, deadline)) {
				X52DebugPrint("Error waiting for C04=0. Clock cycle: ");
				X52DebugPrintln(i);
				return X52_PRO_THROTTLE_UNRESPONSIVE_MICROS;
			}

			// The original throttle samples C03 here between the
			// falling edge of C04 and the rising edge of C02.
			if (i < JoystickState::NUM_BITS)
				recv_buf.SetBit(i, bool(digitalRead(PIN_C03)));
		}

		state.SetFromBinary(recv_buf);
		return 0;
	}

	void PrepareForPoll() {
		digitalWrite(PIN_C02, HIGH);
	}
};


// ThrottleClient makes it possible to use some of your Arduino pins as a
// connection to the PS/2 socket of an X52 Pro Throttle.
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
	unsigned long SendJoystickState(const JoystickState& state, JoystickConfig& cfg, unsigned long wait_micros=X52_PRO_DEFAULT_SEND_JOYSTICK_STATE_WAIT_MICROS) {
		// waiting for the throttle's poll
		if (!wait_for_pin_state(PIN_C02, HIGH, micros()+wait_micros))
			return 1;

		JoystickConfig::Binary recv_buf;
		JoystickState::Binary send_buf;
		state.ToBinary(send_buf);

		auto deadline = micros() + X52_PRO_JOYSTICK_TIMEOUT_MICROS;

		// A frame consists of 76 clock pulses on both C02 and C04.
		for (int i=0; i<76; i++) {
			if (i < JoystickState::NUM_BITS)
				digitalWrite(PIN_C03, send_buf.Bit(i));
			else if (i >= 57)
				// The original joystick samples C01 here between the
				// rising edge of C02 and the rising edge of C04.
				recv_buf.SetBit(i-57, bool(digitalRead(PIN_C01)));

			digitalWrite(PIN_C04, HIGH);

			if (!wait_for_pin_state(PIN_C02, LOW, deadline)) {
				X52DebugPrint("Error waiting for C02=0. Clock cycle: ");
				X52DebugPrintln(i);
				digitalWrite(PIN_C04, LOW);
				return X52_PRO_JOYSTICK_UNRESPONSIVE_MICROS;
			}

#if X52_PRO_IMPROVED_THROTTLE_CLIENT_DESYNC_DETECTION
			if (i >= 1 && i <= 55) {
				// This is something that the original joystick doesn't do.
				// This method leads to very quick and reliable desync detection.
				// It's based on the assumption that the X52 Pro always sends
				// ones over C01 while the joystick is sending its state over C03.
				if (!digitalRead(PIN_C01)) {
					X52DebugPrint("Desync detected: bits 1..55 aren't all ones. Timing out to force a resync. Clock cycle: ");
					X52DebugPrintln(i);
					digitalWrite(PIN_C04, LOW);
					return X52_PRO_JOYSTICK_DESYNC_UNRESPONSIVE_MICROS;
				}
			} else
#endif
			if (i == 56) {
				// This is something that the original joystick also does.
				if (digitalRead(PIN_C01)) {
					X52DebugPrintln("Desync detected: bit 56 isn't zero. Timing out to force a resync.");
					digitalWrite(PIN_C04, LOW);
					return X52_PRO_JOYSTICK_DESYNC_UNRESPONSIVE_MICROS;
				}
			}

			digitalWrite(PIN_C04, LOW);

			// The original throttle samples C03 here between the
			// falling edge of C04 and the rising edge of C02.

			if (!wait_for_pin_state(PIN_C02, HIGH, deadline)) {
				X52DebugPrint("Error waiting for C02=1. Clock cycle: ");
				X52DebugPrintln(i);
				return X52_PRO_JOYSTICK_UNRESPONSIVE_MICROS;
			}
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


}  // namespace pro
}  // namespace x52
