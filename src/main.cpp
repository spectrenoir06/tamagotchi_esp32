#include <Arduino.h>

#include <sys/time.h>

extern "C" {
	#include "tamalib.h"
	#include "rom.h"
}


#ifdef USE_LCD
	#include <ESP32-Chimera-Core.h>

	extern const char icone0_png_start[] asm("_binary_res_icone0_png_start");
	extern const char icone0_png_end[]   asm("_binary_res_icone0_png_end");

	extern const char icone1_png_start[] asm("_binary_res_icone1_png_start");
	extern const char icone1_png_end[]   asm("_binary_res_icone1_png_end");

	extern const char icone2_png_start[] asm("_binary_res_icone2_png_start");
	extern const char icone2_png_end[]   asm("_binary_res_icone2_png_end");

	extern const char icone3_png_start[] asm("_binary_res_icone3_png_start");
	extern const char icone3_png_end[]   asm("_binary_res_icone3_png_end");

	extern const char icone4_png_start[] asm("_binary_res_icone4_png_start");
	extern const char icone4_png_end[]   asm("_binary_res_icone4_png_end");

	extern const char icone5_png_start[] asm("_binary_res_icone5_png_start");
	extern const char icone5_png_end[]   asm("_binary_res_icone5_png_end");

	extern const char icone6_png_start[] asm("_binary_res_icone6_png_start");
	extern const char icone6_png_end[]   asm("_binary_res_icone6_png_end");

	extern const char icone7_png_start[] asm("_binary_res_icone7_png_start");
	extern const char icone7_png_end[]   asm("_binary_res_icone7_png_end");

	const char* icone[8] = {
		icone0_png_start,
		icone1_png_start,
		icone2_png_start,
		icone3_png_start,
		icone4_png_start,
		icone5_png_start,
		icone6_png_start,
		icone7_png_start
	};

	const uint32_t icone_size[8] = {
		icone0_png_end - icone0_png_start,
		icone1_png_end - icone1_png_start,
		icone2_png_end - icone2_png_start,
		icone3_png_end - icone3_png_start,
		icone4_png_end - icone4_png_start,
		icone5_png_end - icone5_png_start,
		icone6_png_end - icone6_png_start,
		icone7_png_end - icone7_png_start
	};
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
	return esp_timer_get_time();
}

// #define NO_SLEEP 

static void hal_sleep_until(timestamp_t ts) {
	#ifndef NO_SLEEP
		int32_t remaining = (int32_t)(ts - hal_get_timestamp());
		if (remaining > 0) {
			ets_delay_us(remaining);
		}
	#else
		/* Wait instead of sleeping to get the highest possible accuracy
		* NOTE: the accuracy still depends on the timestamp_t resolution.
		*/
		while ((int32_t)(ts - hal_get_timestamp()) > 0);
	#endif
}
static bool_t matrix_buffer[LCD_HEIGHT][LCD_WIDTH] = { {0} };
static bool_t matrix_buffer_old[LCD_HEIGHT][LCD_WIDTH] = { {0} };

static bool_t icone_buffer[8] = {0};
static bool_t icone_buffer_old[8] = {0};

static void hal_update_screen(void) {
	for (int x=0; x<LCD_WIDTH; x++) {
		for (int y=0; y<LCD_HEIGHT; y++) {
			// Serial.printf("%d %d\n", x, y);
			if (matrix_buffer_old[y][x] != matrix_buffer[y][x]) {
				M5.Lcd.fillRect(
					x + x * 8,
					48 + y + y * 8,
					8,
					8,
					matrix_buffer[y][x] ? 0x0000000 : 0xffffff
				);
				matrix_buffer_old[y][x] = matrix_buffer[y][x];
			}
		}
	}
	for (int i=0; i < 8; i++) {
		if (icone_buffer[i] != icone_buffer_old[i]) {
			int y = 0;
			if (i >= 4)
				y = 192;

			if (icone_buffer[i]) {
				M5.Lcd.drawPng(
					(uint8_t*)icone[i],
					icone_size[i],
					64+i%4*48,
					y
				);
			} else {
				M5.Lcd.fillRect(
					64+i%4*48,
					y,
					48,
					48,
					0x000000
				);
			}
			icone_buffer_old[i] = icone_buffer[i];
		}
	}
}


static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
	matrix_buffer[y][x] = val;
}

static void hal_set_lcd_icon(u8_t icon, bool_t val) {
	icone_buffer[icon] = val;
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

void tamagotchi_cpu_task(void* parameter) {
	for (;;) {
		tamalib_step();
	}
}

void tamagotchi_input_task(void* parameter) {
	for (;;) {
		hal_handler();
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}

void tamagotchi_render_task(void* parameter) {
	for (int x = 0; x < LCD_WIDTH; x++) {
		for (int y = 0; y < LCD_HEIGHT; y++) {
				M5.Lcd.fillRect(
					x + x * 8,
					48 + y + y * 8,
					8,
					8,
					0xffffff
				);
			}
	}
	for (;;) {
		hal_update_screen();
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");

	#ifdef USE_LCD
		M5.begin();
	#endif

	tamalib_register_hal(&hal);
	tamalib_init(g_program, g_breakpoints, 1000000); // my_breakpoints can be NULL, 1000000 means that timestamps will be expressed in us

	xTaskCreatePinnedToCore(
		tamagotchi_input_task,    // Function that should be called
		"tamagotchi_input_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		0          // Core you want to run the task on (0 or 1)
	);

	xTaskCreatePinnedToCore(
		tamagotchi_cpu_task,    // Function that should be called
		"tamagotchi_cpu_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		1          // Core you want to run the task on (0 or 1)
	);

	xTaskCreatePinnedToCore(
		tamagotchi_render_task,    // Function that should be called
		"tamagotchi_render_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		0          // Core you want to run the task on (0 or 1)
	);
}

void loop() {
	vTaskDelay(1 / portTICK_PERIOD_MS);
}	