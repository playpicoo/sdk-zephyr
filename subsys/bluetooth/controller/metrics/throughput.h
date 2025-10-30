/* Simple throughput instrumentation for Bluetooth ACL data
 * Provides atomic counters for TX and RX bytes and simple sampling API.
 */
#ifndef ZEPHYR_SUBSYS_BLUETOOTH_CTRL_THROUGHPUT_H_
#define ZEPHYR_SUBSYS_BLUETOOTH_CTRL_THROUGHPUT_H_

#include <stdint.h>

void bt_throughput_init(void);
void bt_throughput_tx_add(uint32_t bytes);
void bt_throughput_rx_add(uint32_t bytes);

/* Snapshot counters and return bytes tx/rx since last call (in bytes) */
void bt_throughput_snapshot(uint32_t *tx_bytes, uint32_t *rx_bytes);

#endif /* ZEPHYR_SUBSYS_BLUETOOTH_CTRL_THROUGHPUT_H_ */
