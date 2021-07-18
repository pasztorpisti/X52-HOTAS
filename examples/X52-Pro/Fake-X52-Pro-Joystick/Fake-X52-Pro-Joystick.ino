// This example is for the X52 Pro. It isn't compatible with the non-Pro hardware.
//
// This firmware turns an Arduino-compatible board into a "Fake X52 Pro Joystick"
// that can be connected to the PS/2 (6-pin mini-DIN) socket of the X52 Pro throttle.
//
// Note: My X52 Pro throttle puts 4.1-4.2V on the pins of its PS/2 socket.
// Use electrically compatible boards/devices to interface with your throttle.
//
// The teensy 3.2 has 5V tolerant digital pins and the throttle's
// CY7C64215-56LTXC MCU (https://www.cypress.com/file/134346) has wide operating
// voltage range between 3V and 5V. Its input low and high levels are 0.8V and
// 2.1V respectively.
//
// Tested with:
// - Hardware: PJRC teensy 3.2
// - Arduino IDE 1.8.13
// - Teensyduino 1.53 (add-on for the Arduino IDE)

#define X52_DEBUG 1
#define X52_PRO_IMPROVED_THROTTLE_CLIENT_DESYNC_DETECTION 1
#include <x52_hotas.h>


// The throttle can handle only 100-125 updates per second on average so there is
// no good reason to cap the update rate. MAX_UPDATES_PER_SECOND=0 means unlimited.
#define MAX_UPDATES_PER_SECOND 0

// RATE_LOG_PERIOD_MILLIS defines the period for the update rate logger.
// Rate logging is disabled if X52_DEBUG or RATE_LOG_PERIOD_MILLIS is zero.
#define RATE_LOG_PERIOD_MILLIS 3000


// TODO: Choose your favorite digital pins on your board.
x52::pro::ThrottleClient<16, 5, 3, 2> throttle_client;


void setup() {
#if X52_DEBUG
	Serial.begin(9600);
#endif

	throttle_client.Setup();

	// TODO: Deal with unused/floating input pins if you want to do it by the book.
}


void loop() {
	if (!throttle_client.IsPollInProgress()) {
#if !X52_BUSY_WAIT
		delayMicroseconds(100);
#endif
		return;
	}

#if MAX_UPDATES_PER_SECOND
	static x52::util::RateLimiter<MAX_UPDATES_PER_SECOND,MAX_UPDATES_PER_SECOND> rate_limiter;
	unsigned long d = rate_limiter.MicrosTillNextUpdate();
	if (d > 0) {
#if !X52_BUSY_WAIT
		delayMicroseconds(d);
#endif
		return;
	}
#endif

	if (send_joystick_state()) {
#if X52_DEBUG && RATE_LOG_PERIOD_MILLIS
		static x52::util::RateLogger<RATE_LOG_PERIOD_MILLIS> rate_logger;
		rate_logger.OnUpdate();
#endif
	}
}


bool send_joystick_state() {
	x52::pro::JoystickState state;

	// TODO: Fill the JoystickState with the values read from your custom input sensors.
	//  I put here some constant values to help me with debugging.
	state.x = x52::pro::JoystickState::MAX_X / 4;
	state.y = x52::pro::JoystickState::MAX_Y / 4;
	state.z = x52::pro::JoystickState::MAX_Z * 3 / 4;
	state.pov_1 = x52::UpLeft;
	state.pov_2 = x52::DownRight;
	state.trigger_stage_1 = true;
	state.mode = x52::Mode1;

	x52::pro::JoystickConfig cfg;
	auto timeout_micros = throttle_client.SendJoystickState(state, cfg);
	if (timeout_micros) {
		Serial.println("SendJoystickState failed");
		delayMicroseconds(timeout_micros);
		return false;
	}

	// TODO: Do something with the JoystickConfig recevied from the throttle.

	// DebugPrinting after every 100th update
	static int counter = 0;
	if (counter++ % 100 == 0) {
		X52DebugPrint("Brightness:");
		X52DebugPrint(cfg.led_brightness);
		X52DebugPrint(" POV1:");
		X52DebugPrint(cfg.pov_1_led_blinking);
		X52DebugPrint(" fire:");
		X52DebugPrint(cfg.button_fire_led);
		X52DebugPrint(" POV2:");
		X52DebugPrint(cfg.pov_2_led);
		X52DebugPrint(" A:");
		X52DebugPrint(cfg.button_a_led);
		X52DebugPrint(" B:");
		X52DebugPrint(cfg.button_b_led);
		X52DebugPrint(" T1/2:");
		X52DebugPrint(cfg.button_t1_t2_led);
		X52DebugPrint(" T3/4:");
		X52DebugPrint(cfg.button_t3_t4_led);
		X52DebugPrint(" T5/6:");
		X52DebugPrintln(cfg.button_t5_t6_led);
	}
	return true;
}
