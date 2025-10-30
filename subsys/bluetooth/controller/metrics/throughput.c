/* Throughput instrumentation for Bluetooth controller
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/atomic.h>
#include <zephyr/work.h>

#include "metrics/throughput.h"

/* Use a dedicated module for throughput logs so it can be enabled independently */
LOG_MODULE_REGISTER(bt_throughput, 3);

static atomic_t tx_cnt;
static atomic_t rx_cnt;

static atomic_t throughput_started;

void bt_throughput_init(void)
{
    atomic_set(&tx_cnt, 0);
    atomic_set(&rx_cnt, 0);
    /* Do not start periodic work here: start lazily on first activity */
    atomic_set(&throughput_started, 0);
}

void bt_throughput_tx_add(uint32_t bytes)
{
    atomic_add(&tx_cnt, bytes);
    /* start periodic logging on first activity */
    if (!atomic_get(&throughput_started)) {
        int expected = 0;
        if (atomic_cas(&throughput_started, &expected, 1)) {
            /* initialize and schedule */
            k_work_init_delayable(&throughput_work, throughput_work_handler);
            k_work_reschedule(&throughput_work, K_MSEC(BT_THROUGHPUT_PERIOD_MS));
        }
    }
}

void bt_throughput_rx_add(uint32_t bytes)
{
    atomic_add(&rx_cnt, bytes);
    /* start periodic logging on first activity */
    if (!atomic_get(&throughput_started)) {
        int expected = 0;
        if (atomic_cas(&throughput_started, &expected, 1)) {
            /* initialize and schedule */
            k_work_init_delayable(&throughput_work, throughput_work_handler);
            k_work_reschedule(&throughput_work, K_MSEC(BT_THROUGHPUT_PERIOD_MS));
        }
    }
}

void bt_throughput_snapshot(uint32_t *tx_bytes, uint32_t *rx_bytes)
{
    uint32_t t = (uint32_t)atomic_get(&tx_cnt);
    uint32_t r = (uint32_t)atomic_get(&rx_cnt);

    if (tx_bytes) {
        *tx_bytes = t;
        atomic_set(&tx_cnt, 0);
    }

    if (rx_bytes) {
        *rx_bytes = r;
        atomic_set(&rx_cnt, 0);
    }
}

/* Periodic logging implementation */
#ifndef BT_THROUGHPUT_PERIOD_MS
#define BT_THROUGHPUT_PERIOD_MS 1000U
#endif

static struct k_work_delayable throughput_work;

static void throughput_work_handler(struct k_work *work)
{
    uint32_t tx = 0, rx = 0;

    bt_throughput_snapshot(&tx, &rx);

    /* bits per second = bytes * 8 * (1000 / period_ms)
     * kbps = bits_per_second / 1000 = (bytes * 8) / period_ms
     */
    uint32_t tx_kbps = (BT_THROUGHPUT_PERIOD_MS > 0) ? ((tx * 8U) / BT_THROUGHPUT_PERIOD_MS) : 0U;
    uint32_t rx_kbps = (BT_THROUGHPUT_PERIOD_MS > 0) ? ((rx * 8U) / BT_THROUGHPUT_PERIOD_MS) : 0U;

    LOG_INF("BT throughput: TX=%u bytes (%u kbps), RX=%u bytes (%u kbps)", tx, tx_kbps, rx, rx_kbps);

    /* Reschedule next run */
    k_work_reschedule(&throughput_work, K_MSEC(BT_THROUGHPUT_PERIOD_MS));
}

