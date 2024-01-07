#include <pico/stdlib.h>
#include <hardware/pwm.h>

#include "utils/motor.h"


#define PWM_MAX 255*255 // 16 bit


void
motor_init(struct Motor * m, uint in1, uint in2, uint en)
{
    m->in1 = in1;
    m->in2 = in2;
    m->en = en;
    m->s = 0.0f;
    
    gpio_init(in1);
    gpio_set_dir(in1, GPIO_OUT);

    gpio_init(in2);
    gpio_set_dir(in2, GPIO_OUT);

    gpio_set_function(en, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(en);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 24);
    pwm_init(slice_num, &config, true);
}


void
motor_set_speed(struct Motor * m, float s)
{
    m->s = s;

    if (s == 0) {
        gpio_put(m->in1, false);
        gpio_put(m->in2, false);
        pwm_set_gpio_level(m->en, 0);
        return;
    }

    s = MAX(-1.0f, MIN(1.0f, s)); // s = [-1..1]

    if (s < 0) {
        gpio_put(m->in1, true);
        gpio_put(m->in2, false);
        pwm_set_gpio_level(m->en, -s*PWM_MAX);
    } else {
        gpio_put(m->in1, false);
        gpio_put(m->in2, true);
        pwm_set_gpio_level(m->en, s*PWM_MAX);
    }
}


void
motor_increment_speed(struct Motor * m, float ds)
{
    motor_set_speed(m, m->s + ds);
}
