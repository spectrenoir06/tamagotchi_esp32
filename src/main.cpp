#include <Arduino.h>
#include <sys/time.h>
#include "SPIFFS.h"

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

	const int icone_size[8] = {
		icone0_png_end-icone0_png_start,
		icone1_png_end-icone1_png_start,
		icone2_png_end-icone2_png_start,
		icone3_png_end-icone3_png_start,
		icone4_png_end-icone4_png_start,
		icone5_png_end-icone5_png_start,
		icone6_png_end-icone6_png_start,
		icone7_png_end-icone7_png_start
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

u32_t g_freq = 0;

static void hal_set_frequency(u32_t freq) {
	g_freq = freq;
}

static void hal_play_frequency(bool_t en) {
	if (en) {
		M5.Speaker.tone(g_freq/10);
	} else{
		M5.Speaker.mute();
		// dacWrite(25, 0);
	}
}

uint8_t a_is_press = 0;
uint8_t b_is_press = 0;
uint8_t c_is_press = 0;

uint8_t speed_is_press = 0;

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

	if (speed_is_press != M5.BtnVolume.isPressed()) {
		if (M5.BtnVolume.isPressed())
			tamalib_set_speed(0);
		else
			tamalib_set_speed(1);
		speed_is_press = M5.BtnVolume.isPressed();
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



void lua_tamalib_state_save(uint8_t* buf) {
	state_t* state;
	uint32_t i;

	state = tamalib_get_state();

	uint32_t ctn = 0;

	buf[ctn++] = *(state->pc) & 0xFF;
	buf[ctn++] = (*(state->pc) >> 8) & 0x1F;

	buf[ctn++] = *(state->x) & 0xFF;
	buf[ctn++] = (*(state->x) >> 8) & 0xF;

	buf[ctn++] = *(state->y) & 0xFF;
	buf[ctn++] = (*(state->y) >> 8) & 0xF;

	buf[ctn++] = *(state->a) & 0xF;

	buf[ctn++] = *(state->b) & 0xF;

	buf[ctn++] = *(state->np) & 0x1F;

	buf[ctn++] = *(state->sp) & 0xFF;

	buf[ctn++] = *(state->flags) & 0xF;

	buf[ctn++] = *(state->tick_counter) & 0xFF;
	buf[ctn++] = (*(state->tick_counter) >> 8) & 0xFF;
	buf[ctn++] = (*(state->tick_counter) >> 16) & 0xFF;
	buf[ctn++] = (*(state->tick_counter) >> 24) & 0xFF;

	buf[ctn++] = *(state->clk_timer_timestamp) & 0xFF;
	buf[ctn++] = (*(state->clk_timer_timestamp) >> 8) & 0xFF;
	buf[ctn++] = (*(state->clk_timer_timestamp) >> 16) & 0xFF;
	buf[ctn++] = (*(state->clk_timer_timestamp) >> 24) & 0xFF;

	buf[ctn++] = *(state->prog_timer_timestamp) & 0xFF;
	buf[ctn++] = (*(state->prog_timer_timestamp) >> 8) & 0xFF;
	buf[ctn++] = (*(state->prog_timer_timestamp) >> 16) & 0xFF;
	buf[ctn++] = (*(state->prog_timer_timestamp) >> 24) & 0xFF;

	buf[ctn++] = *(state->prog_timer_enabled) & 0x1;

	buf[ctn++] = *(state->prog_timer_data) & 0xFF;

	buf[ctn++] = *(state->prog_timer_rld) & 0xFF;

	buf[ctn++] = *(state->call_depth) & 0xFF;
	buf[ctn++] = (*(state->call_depth) >> 8) & 0xFF;
	buf[ctn++] = (*(state->call_depth) >> 16) & 0xFF;
	buf[ctn++] = (*(state->call_depth) >> 24) & 0xFF;

	for (i = 0; i < INT_SLOT_NUM; i++) {
		buf[ctn++] = state->interrupts[i].factor_flag_reg & 0xF;


		buf[ctn++] = state->interrupts[i].mask_reg & 0xF;


		buf[ctn++] = state->interrupts[i].triggered & 0x1;

	}

	/* First 640 half bytes correspond to the RAM */
	for (i = 0; i < MEM_RAM_SIZE; i++) {
		buf[ctn++] = state->memory[i + MEM_RAM_ADDR] & 0xF;

	}

	/* I/Os are from 0xF00 to 0xF7F */
	for (i = 0; i < MEM_IO_SIZE; i++) {
		buf[ctn++] = state->memory[i + MEM_IO_ADDR] & 0xF;
	}
	// Serial.printf("save\n");
}


void lua_tamalib_state_load(uint8_t* buf) {
	state_t* state;
	uint32_t i;
	uint32_t ctn = 0;

	state = tamalib_get_state();

	*(state->pc) = buf[ctn++] | ((buf[ctn++] & 0x1F) << 8);

	*(state->x) = buf[ctn++] | ((buf[ctn++] & 0xF) << 8);

	*(state->y) = buf[ctn++] | ((buf[ctn++] & 0xF) << 8);

	*(state->a) = buf[ctn++] & 0xF;

	*(state->b) = buf[ctn++] & 0xF;

	*(state->np) = buf[ctn++] & 0x1F;

	*(state->sp) = buf[ctn++];

	*(state->flags) = buf[ctn++] & 0xF;

	*(state->tick_counter) = buf[ctn++] | (buf[ctn++] << 8) | (buf[ctn++] << 16) | (buf[ctn++] << 24);

	*(state->clk_timer_timestamp) = buf[ctn++] | (buf[ctn++] << 8) | (buf[ctn++] << 16) | (buf[ctn++] << 24);

	*(state->prog_timer_timestamp) = buf[ctn++] | (buf[ctn++] << 8) | (buf[ctn++] << 16) | (buf[ctn++] << 24);

	*(state->prog_timer_enabled) = buf[ctn++] & 0x1;

	*(state->prog_timer_data) = buf[ctn++];

	*(state->prog_timer_rld) = buf[ctn++];

	*(state->call_depth) = buf[ctn++] | (buf[ctn++] << 8) | (buf[ctn++] << 16) | (buf[ctn++] << 24);

	for (i = 0; i < INT_SLOT_NUM; i++) {

		state->interrupts[i].factor_flag_reg = buf[ctn++] & 0xF;


		state->interrupts[i].mask_reg = buf[ctn++] & 0xF;


		state->interrupts[i].triggered = buf[ctn++] & 0x1;
	}

	/* First 640 half bytes correspond to the RAM */
	for (i = 0; i < MEM_RAM_SIZE; i++) {

		state->memory[i + MEM_RAM_ADDR] = buf[ctn++] & 0xF;
	}

	/* I/Os are from 0xF00 to 0xF7F */
	for (i = 0; i < MEM_IO_SIZE; i++) {
		state->memory[i + MEM_IO_ADDR] = buf[ctn++] & 0xF;
	}

	printf("load %d\n", ctn);

	tamalib_refresh_hw();
}


void tamagotchi_cpu_task(void* parameter) {
	uint32_t timer = millis() + 10000;
	for (;;) {
		tamalib_step();

		if (millis() > timer) {
			File file = SPIFFS.open("/save.state", FILE_WRITE);
			if (!file) {
				Serial.println("Failed to open savestate");
				// return;
			}
			else {
				char* buf = (char*)malloc(1000);
				lua_tamalib_state_save((uint8_t*)buf);
				file.write((uint8_t*)buf, 816);
				file.close();
				free(buf);
			}
			timer = millis() + 10000;
		}
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
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");

	#ifdef USE_LCD
		M5.begin();
	#endif

	if(!SPIFFS.begin(true)){
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}

	tamalib_register_hal(&hal);
	tamalib_init(g_program, g_breakpoints, 1000000); // my_breakpoints can be NULL, 1000000 means that timestamps will be expressed in us

	File file = SPIFFS.open("/save.state");
	if (!file) {
		Serial.println("Failed to open savestate");
		// return;
	} else {
		Serial.println("load savestate");
		char *buf = (char*)malloc(1000);
		file.readBytes(buf, 816);
		lua_tamalib_state_load((uint8_t*)buf);
		file.close();
		free(buf);
	}

		(
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