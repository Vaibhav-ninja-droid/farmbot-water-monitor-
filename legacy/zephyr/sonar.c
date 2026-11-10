#include "sonar.h"
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#define SONAR_NODE DT_ALIAS(sonar_pw)
static const struct gpio_dt_spec input = GPIO_DT_SPEC_GET(SONAR_NODE, gpios);
static struct gpio_callback callback;
static struct k_spinlock lock;
static uint32_t rise_cycles, overruns;
static bool rise_seen, ready;
static struct sonar_sample pending;
static void edge(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    int level = gpio_pin_get_dt(&input);
    uint32_t now = k_cycle_get_32();
    k_spinlock_key_t key = k_spin_lock(&lock);
    if (level > 0 && !rise_seen) {
        rise_cycles = now;
        rise_seen = true;
        k_spin_unlock(&lock, key);
        return;
    }
    bool valid = level == 0 && rise_seen;
    uint32_t pulse = valid ? k_cyc_to_us_floor32(now - rise_cycles) : 0;
    rise_seen = false;
    if (ready)
        overruns++;
    pending = (struct sonar_sample){pulse, (uint64_t)k_uptime_get(),
                                    valid && pulse > 0 && pulse <= 100000};
    ready = true;
    k_spin_unlock(&lock, key);
}
int sonar_init(void) {
    if (!gpio_is_ready_dt(&input))
        return -ENODEV;
    int ret = gpio_pin_configure_dt(&input, GPIO_INPUT);
    if (ret)
        return ret;
    gpio_init_callback(&callback, edge, BIT(input.pin));
    ret = gpio_add_callback(input.port, &callback);
    if (ret)
        return ret;
    return gpio_pin_interrupt_configure_dt(&input, GPIO_INT_EDGE_BOTH);
}
bool sonar_get_sample(struct sonar_sample *sample) {
    k_spinlock_key_t key = k_spin_lock(&lock);
    bool available = ready;
    if (available) {
        *sample = pending;
        ready = false;
    }
    k_spin_unlock(&lock, key);
    return available;
}
uint32_t sonar_overruns(void) {
    k_spinlock_key_t key = k_spin_lock(&lock);
    uint32_t value = overruns;
    k_spin_unlock(&lock, key);
    return value;
}
