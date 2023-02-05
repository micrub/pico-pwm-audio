#include <stdio.h>
#include "pico/stdlib.h"   // stdlib 
#include "hardware/irq.h"  // interrupts
#include "hardware/pwm.h"  // pwm 
#include "hardware/sync.h" // wait for interrupt 
 
// Audio PIN is to match some of the design guide shields. 
#define AUDIO_PIN 28  // you can change this to whatever you like

#define C_PIN 8
#define D_PIN 9
#define E_PIN 10
#define F_PIN 11
#define G_PIN 12
#define A_PIN 13
#define B_PIN 14
#define C1_PIN 15

/* 
 * This include brings in static arrays which contain audio samples. 
 * if you want to know how to make these please see the python code
 * for converting audio samples into static arrays. 
 */
#include "sample.h"
int wav_position = 0;

/*
 * PWM Interrupt Handler which outputs PWM level and advances the 
 * current sample. 
 * 
 * We repeat the same value for 8 cycles this means sample rate etc
 * adjust by factor of 8   (this is what bitshifting <<3 is doing)
 * 
 */
void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));    
    if (wav_position < (WAV_DATA_LENGTH<<3) - 1) { 
        // set pwm level 
        // allow the pwm value to repeat for 8 cycles this is >>3 
        pwm_set_gpio_level(AUDIO_PIN, WAV_DATA[wav_position>>3]);  
        wav_position++;
    } else {
        // reset to start
        wav_position = 0;
    }
}
static char event_str[128];

void gpio_event_string(char *buf, uint32_t events);

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    printf("GPIO %d %s\n", gpio, event_str);
    if (event_str == EDGE_RISE) {
      pwm_config_set_clkdiv_int_frac()
      
    }
}
int main(void) {
    /* Overclocking for fun but then also so the system clock is a 
     * multiple of typical audio sampling rates.
     */
    stdio_init_all();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(C_PIN);
    gpio_set_dir(C_PIN,GPIO_IN);
    gpio_pull_down(C_PIN);
    gpio_set_irq_enabled_with_callback(C_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);


    gpio_init(D_PIN);
    gpio_set_dir(D_PIN,GPIO_IN);
    gpio_pull_down(D_PIN);
    gpio_set_irq_enabled_with_callback(D_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(E_PIN);
    gpio_set_dir(E_PIN,GPIO_IN);
    gpio_pull_down(E_PIN);
    gpio_set_irq_enabled_with_callback(E_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(F_PIN);
    gpio_set_dir(F_PIN,GPIO_IN);
    gpio_pull_down(F_PIN);
    gpio_set_irq_enabled_with_callback(F_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(D_PIN);
    gpio_set_dir(D_PIN,GPIO_IN);
    gpio_pull_down(D_PIN);
    gpio_set_irq_enabled_with_callback(D_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(G_PIN);
    gpio_set_dir(G_PIN,GPIO_IN);
    gpio_pull_down(G_PIN);
    gpio_set_irq_enabled_with_callback(G_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(A_PIN);
    gpio_set_dir(A_PIN,GPIO_IN);
    gpio_pull_down(A_PIN);
    gpio_set_irq_enabled_with_callback(A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(B_PIN);
    gpio_set_dir(B_PIN,GPIO_IN);
    gpio_pull_down(B_PIN);
    gpio_set_irq_enabled_with_callback(B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(C1_PIN);
    gpio_set_dir(C1_PIN,GPIO_IN);
    gpio_pull_down(C1_PIN);
    gpio_set_irq_enabled_with_callback(C1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    set_sys_clock_khz(176000, true); 
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Setup PWM for audio output
    pwm_config config = pwm_get_default_config();
    /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
     * to set the interrupt rate. 
     * 
     * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
     * 
     * 
     * So clkdiv should be as follows for given sample rate
     *  8.0f for 11 KHz
     *  4.0f for 22 KHz
     *  2.0f for 44 KHz etc
     */
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    while(1) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        __wfi(); // Wait for Interrupt
    }
    return 0;
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}
