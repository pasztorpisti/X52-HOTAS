// This example is for the X52 non-Pro. It isn't compatible with the Pro hardware.
//
// This firmware turns an Arduino-compatible board into a "Fake X52 Joystick"
// that can be connected to the PS/2 (6-pin mini-DIN) socket of the X52 throttle.
//
// Note: I don't have an X52 non-Pro throttle so couldn't test this with the
// real hardware. Instead of a real throttle I used another Arduino-compatible
// board running the "Fake X52 Throttle" firmware.
//
// Tested with:
// - Hardware: PJRC teensy 3.2
// - Arduino IDE 1.8.13
// - Teensyduino 1.53 (add-on for the Arduino IDE)

#define X52_DEBUG 1
#include <x52_hotas.h>


// MAX_UPDATES_PER_SECOND=0 means unlimited.
#define MAX_UPDATES_PER_SECOND 0

// RATE_LOG_PERIOD_MILLIS defines the period for the update rate logger.
// Rate logging is disabled if X52_DEBUG or RATE_LOG_PERIOD_MILLIS is zero.
#define RATE_LOG_PERIOD_MILLIS 3000


// TODO: Choose your favorite digital pins on your board.
x52::std::ThrottleClient<16, 5, 3, 2> throttle_client;


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
	x52::std::JoystickState state;

	// TODO: Fill the JoystickState with the values read from your custom input sensors.
	//  I put here some constant values to help me with debugging.
	state.x = x52::std::JoystickState::MAX_X / 4;
	state.y = x52::std::JoystickState::MAX_Y / 4;
	state.z = x52::std::JoystickState::MAX_Z * 3 / 4;
	state.pov_1 = x52::UpLeft;
	state.pov_2 = x52::DownRight;
	state.trigger_stage_1 = true;
	state.mode = x52::Mode1;

	x52::std::JoystickConfig cfg;
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
		X52DebugPrintln(cfg.pov_1_led_blinking);
	}
	return true;
}
