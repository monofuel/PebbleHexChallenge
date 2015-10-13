#include <pebble.h>

static char int_to_hex(int);
static char safe_get_char(int);
static void update_text();
static void update_binary();
static void update_hex();
static void update_ascii();

static Window *window;
static TextLayer *text_layer;
static TextLayer *binary_layer;
static TextLayer *hex_layer;
static TextLayer *ascii_layer;

#define BUFF_SIZE 193

static char binary_buffer[BUFF_SIZE + 1];
static char hex_buffer[(BUFF_SIZE / 4) + 1];
static char ascii_buffer[(BUFF_SIZE / 8) + 1];

static struct {
	bool array[BUFF_SIZE];
	int index;
} byteData;

static void update_text() {
	update_binary();
	update_hex();
	update_ascii();

}

static void update_binary() {
	//if we're about to run over the buffer, shift it all over one.
	if (byteData.index >= BUFF_SIZE) {
		for (int i = 8; i < byteData.index; i++) {
			byteData.array[i - 8] = byteData.array[i];
		}
		byteData.index -= 8;
	}

	//update binary
	int start_index = 0;
	if (byteData.index > 24) {//limit of how many binary digits can be shown
		start_index = byteData.index - 24;
	}
	int j = 0;
	for (int i = start_index; i < byteData.index; i++) {
		if (byteData.array[i])
			binary_buffer[j++] = '1';
		else
			binary_buffer[j++] = '0';
	}
	binary_buffer[j] = '\0';
	text_layer_set_text(binary_layer, binary_buffer);
}

static void update_hex() {
	//update hex
	int hex_counter = 0;
	int bits_used = 0;
	int current_hex = 0;
	int start_index = 0;
	if (byteData.index > 21 * 4) {//limit of how many hex digits can be shown
		start_index = byteData.index - (21 * 4);
		start_index = (start_index / 4) * 4;
	}
	for (int i = start_index; i < byteData.index; i++) {
		if (bits_used >= 4) {
			hex_buffer[hex_counter++] = int_to_hex(current_hex);
			bits_used = 0;
			current_hex = 0;
		}
		bits_used++;

		current_hex = current_hex << 1;
		if (byteData.array[i])
			current_hex += 1;
		else
			current_hex += 0;
	}
	hex_buffer[hex_counter++] = int_to_hex(current_hex << (4 - bits_used));
	hex_buffer[hex_counter] = '\0';
	text_layer_set_text(hex_layer, hex_buffer);

}

static void update_ascii() {
	//update ascii
	int ascii_counter = 0;
	int bits_used = 0;
	int current_char = 0;
	int start_index = 0;
	if (byteData.index > 17 * 8) {//limit of how many ascii digits can be shown
		start_index = byteData.index - (17 * 8);
		start_index = (start_index / 8) * 8;
	}
	for (int i = start_index; i < byteData.index; i++) {
		if (bits_used >= 8) {
			ascii_buffer[ascii_counter++] = safe_get_char(current_char);
			bits_used = 0;
			current_char = 0;
		}
		bits_used++;

		current_char = current_char << 1;
		if (byteData.array[i])
			current_char += 1;
		else
			current_char += 0;
	}
	ascii_buffer[ascii_counter++] = safe_get_char(current_char << (8 - bits_used));
	ascii_buffer[ascii_counter] = '\0';
	text_layer_set_text(ascii_layer, ascii_buffer);

}

static char safe_get_char(int number) {
	if (number < ' ')
		number = '_';
	if (number > '~')
		number = '_';
	return (char)number;
}

static char int_to_hex(int number) {
	switch (number) {
	default:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	case 10:
		return 'A';
	case 11:
		return 'B';
	case 12:
		return 'C';
	case 13:
		return 'D';
	case 14:
		return 'E';
	case 15:
		return 'F';
	}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "Delete");
	if (byteData.index == 0) return;
	byteData.array[byteData.index] = '\0';
	byteData.index--;
	update_text();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "1");
	byteData.array[byteData.index++] = true;
	update_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "0");
	byteData.array[byteData.index++] = false;
	update_text();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	byteData.index = 0;
	memset(byteData.array, 0, BUFF_SIZE);


	//input prompt
	text_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text(text_layer, "Press a button");
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));

	//binary display
	binary_layer = text_layer_create((GRect) { .origin = { 0, 40 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text_alignment(binary_layer, GTextAlignmentRight);
	layer_add_child(window_layer, text_layer_get_layer(binary_layer));

	//hex display
	hex_layer = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text_alignment(hex_layer, GTextAlignmentRight);
	layer_add_child(window_layer, text_layer_get_layer(hex_layer));

	//ascii display
	ascii_layer = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text_alignment(ascii_layer, GTextAlignmentRight);
	layer_add_child(window_layer, text_layer_get_layer(ascii_layer));

	update_text();
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
}

static void init(void) {
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
			.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}