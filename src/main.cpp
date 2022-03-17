#include <Arduino.h>

#include <sys/time.h>

extern "C" {
	#include "tamalib.h"
	#include "rom.h"
}


#ifdef USE_LCD
	#include <ESP32-Chimera-Core.h>
#endif

log_level_t log_levels = LOG_ERROR;

static void* hal_malloc(u32_t size) {
	return malloc(size);
}

static void hal_free(void* ptr) {
	free(ptr);
}

static void hal_halt(void) {
	// exit(EXIT_SUCCESS);
}

static bool_t hal_is_log_enabled(log_level_t level) {
	return !!(log_levels & level);
}

static void hal_log(log_level_t level, char* buff, ...) {
	va_list arglist;

	if (!(log_levels & level)) {
		return;
	}

	// va_start(arglist, buff);

	// vfprintf((level == LOG_ERROR) ? stderr : stdout, buff, arglist);

	// va_end(arglist);
}

static timestamp_t hal_get_timestamp(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

// #define NO_SLEEP 

static void hal_sleep_until(timestamp_t ts) {
	#ifndef NO_SLEEP
		int32_t remaining = (int32_t)(ts - hal_get_timestamp());

		/* Sleep for a bit more than what is needed */
		if (remaining > 0) {
			delayMicroseconds(remaining);
		}
	#else
		/* Wait instead of sleeping to get the highest possible accuracy
		* NOTE: the accuracy still depends on the timestamp_t resolution.
		*/
		while ((int32_t)(ts - hal_get_timestamp()) > 0);
	#endif
}
static bool_t matrix_buffer[LCD_HEIGHT][LCD_WIDTH] = { {0} };

static void hal_update_screen(void) {
	// for (int x=0; x<LCD_WIDTH; x++) {
	// 	for (int y=0; y<LCD_HEIGHT; y++) {
	// 		// Serial.printf("%d %d\n", x, y);
	// 		M5.Lcd.fillRect(
	// 			x + x * 1,
	// 			y + y * 1,
	// 			1,
	// 			1,
	// 			!matrix_buffer[y][x] ? 0x0 : 0xffffff
	// 		);
	// 	}
	// }
	

}


static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
	matrix_buffer[y][x] = val;
	M5.Lcd.fillRect(
		x + x * 8,
		y + y * 8,
		8,
		8,
		!val ? 0x0 : 0xffffff
	);
}

static void hal_set_lcd_icon(u8_t icon, bool_t val) {
	// icon_buffer[icon] = val;
}

static void hal_set_frequency(u32_t freq) {
	// if (current_freq != freq) {
	// 	current_freq = freq;
	// 	sin_pos = 0;
	// }
}

static void hal_play_frequency(bool_t en) {
	// if (is_audio_playing != en) {
	// 	is_audio_playing = en;
	// }
}

uint8_t a_is_press = 0;
uint8_t b_is_press = 0;
uint8_t c_is_press = 0;

static int hal_handler(void) {
	if (a_is_press != M5.BtnA.isPressed()) {
		if (M5.BtnA.isPressed())
			tamalib_set_button(BTN_LEFT, BTN_STATE_PRESSED);
		else
			tamalib_set_button(BTN_LEFT, BTN_STATE_RELEASED);
		a_is_press = M5.BtnA.isPressed();
	}

	if (b_is_press != M5.BtnB.isPressed()) {
		if (M5.BtnB.isPressed())
			tamalib_set_button(BTN_MIDDLE, BTN_STATE_PRESSED);
		else
			tamalib_set_button(BTN_MIDDLE, BTN_STATE_RELEASED);
		b_is_press = M5.BtnB.isPressed();
	}

	if (c_is_press != M5.BtnStart.isPressed()) {
		if (M5.BtnStart.isPressed())
			tamalib_set_button(BTN_RIGHT, BTN_STATE_PRESSED);
		else
			tamalib_set_button(BTN_RIGHT, BTN_STATE_RELEASED);
		c_is_press = M5.BtnStart.isPressed();
	}

	M5.update();
	return 0;
}

static hal_t hal = {
	.malloc = &hal_malloc,
	.free = &hal_free,
	.halt = &hal_halt,
	.is_log_enabled = &hal_is_log_enabled,
	.log = &hal_log,
	.sleep_until = &hal_sleep_until,
	.get_timestamp = &hal_get_timestamp,
	.update_screen = &hal_update_screen,
	.set_lcd_matrix = &hal_set_lcd_matrix,
	.set_lcd_icon = &hal_set_lcd_icon,
	.set_frequency = &hal_set_frequency,
	.play_frequency = &hal_play_frequency,
	.handler = &hal_handler,
};

static breakpoint_t* g_breakpoints = NULL;

// static u12_t* g_program = NULL;		// The actual program that is executed
// static uint32_t g_program_size = 0;

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");

	#ifdef USE_LCD
		pinMode(TFT_BL, OUTPUT);
		digitalWrite(TFT_BL, 1);
	#endif
	#ifdef USE_LCD
		M5.begin();
	#endif

	

	tamalib_register_hal(&hal);
	tamalib_init(g_program, g_breakpoints, 1000000); // my_breakpoints can be NULL, 1000000 means that timestamps will be expressed in us
	tamalib_mainloop();
}

void loop() {
	// tamalib_step();
	// hal_handler();
	// hal_update_screen();
	// vTaskDelay(1 / portTICK_PERIOD_MS);
}	