// by @xMasterX

#include <furi.h>
#include <furi_hal_power.h>
#include <furi_hal_gpio.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/elements.h>

GpioPin flipper_swclk_pin;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    uint8_t power_lvl;
    bool is_on;
} PluginState;

static void render_callback(Canvas* const canvas, void* ctx) {
    PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Flashlight");

    canvas_set_font(canvas, FontSecondary);

    char text_buf[14];

    elements_button_left(canvas, "-");
    elements_button_right(canvas, "+");

    if(!plugin_state->is_on) {
        elements_button_center(canvas, "Enable");
        snprintf(text_buf, sizeof(text_buf), "%d LEDs off", plugin_state->power_lvl);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, text_buf);
    } else {
        elements_button_center(canvas, "Disable");
        snprintf(text_buf, sizeof(text_buf), "%d LEDs on", plugin_state->power_lvl);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, text_buf);
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void initialize_flashlights() {
    furi_hal_gpio_write(&gpio_ext_pc3, false);
    furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pa7, false);
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pa6, false);
    furi_hal_gpio_init(&gpio_ext_pa6, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pb2, false);
    furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pb3, false);
    furi_hal_gpio_init(&gpio_ext_pb3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    flipper_swclk_pin = (GpioPin){.port = GPIOA, .pin = LL_GPIO_PIN_14}; // Thanks Dr Zlo
    furi_hal_gpio_write(&flipper_swclk_pin, false);
    furi_hal_gpio_init(&flipper_swclk_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pc0, false);
    furi_hal_gpio_init(&gpio_ext_pc0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&gpio_ext_pc1, false);
    furi_hal_gpio_init(&gpio_ext_pc1, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(&ibutton_gpio, false);
    furi_hal_gpio_init(&ibutton_gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
}

void flash_reset() {
    furi_hal_gpio_write(&gpio_ext_pa6, false);
    furi_hal_gpio_write(&gpio_ext_pb3, false);
    furi_hal_gpio_write(&gpio_ext_pc3, false);
    furi_hal_gpio_write(&flipper_swclk_pin, false);
    furi_hal_gpio_write(&gpio_ext_pc1, false);
    furi_hal_gpio_write(&ibutton_gpio, false);
}

static void flash_switch(PluginState* plugin_state) {
    flash_reset();
    switch(plugin_state->power_lvl) {
    case 1:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        break;
    case 2:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    case 3:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&flipper_swclk_pin, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    case 4:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
        furi_hal_gpio_write(&flipper_swclk_pin, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    case 5:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&gpio_ext_pb3, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
        furi_hal_gpio_write(&flipper_swclk_pin, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    case 6:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&gpio_ext_pb3, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
        furi_hal_gpio_write(&flipper_swclk_pin, true);
        furi_hal_gpio_write(&gpio_ext_pc1, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    default:
        furi_hal_gpio_write(&gpio_ext_pa6, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
        furi_hal_gpio_write(&flipper_swclk_pin, true);
        furi_hal_gpio_write(&ibutton_gpio, true);
        break;
    }
}

static void flash_toggle(PluginState* plugin_state) {
    initialize_flashlights(); // Initialize all needed pins

    if(plugin_state->is_on) {
        flash_reset();
        plugin_state->is_on = false;
    } else {
        flash_switch(plugin_state);
        plugin_state->is_on = true;
    }
}

int32_t flashlight_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("flashlight", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    if(furi_hal_gpio_read(&gpio_ext_pa6)) {
        plugin_state->is_on = true;
    }

    plugin_state->power_lvl = 6;

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(plugin_state->power_lvl >= 6) {
                            plugin_state->power_lvl = 6;
                        } else {
                            plugin_state->power_lvl += 1;
                        }

                        if(plugin_state->is_on) {
                            flash_switch(plugin_state);
                        }
                        break;
                    case InputKeyLeft:
                        if(plugin_state->power_lvl <= 1) {
                            plugin_state->power_lvl = 1;
                        } else {
                            plugin_state->power_lvl -= 1;
                        }

                        if(plugin_state->is_on) {
                            flash_switch(plugin_state);
                        }
                        break;
                    case InputKeyOk:
                        flash_toggle(plugin_state);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);

    return 0;
}
