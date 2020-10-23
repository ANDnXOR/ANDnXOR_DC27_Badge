/*****************************************************************************
 * Made with beer and late nights in California.
 *
 * (C) Copyright 2017-2019 AND!XOR LLC (https://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 11th, 2019 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge, or just think it's neat. Consider buying us a beer
 * (or two) and/or a badge (or two). We are just as obsessed with collecting
 * badges as we are in making them.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@bender_andnxor
 * 	@lacosteaef
 *  @f4nci3
 *  @Cr4bf04m
 *****************************************************************************/
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <posix/time.h>
#include <stdio.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_proxy);

#include "../ff_post.h"
#include "../ff_settings.h"
#include "../ff_time.h"
#include "../system.h"
#include "ff_mesh.h"
#include "ff_mesh_model_c2.h"
#include "ff_mesh_model_status.h"
#include "ff_mesh_model_time.h"

// BT Advertised data
static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x28, 0x18),
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, 0x27, 0xDC),
    BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA, 0x9e, 0x04,
                  VERSION_INT), //[Manufacturer ID 0x49E][FW Version]
};
#define ADV_DURATION (20 * 1000)
#define ADV_PERIOD (120 * 1000) 

static struct bt_conn *m_conn = NULL;
static struct bt_gatt_ccc_cfg blvl_ccc_cfg[BT_GATT_CCC_MAX] = {};

// BT Connection callbacks
static void __connected(struct bt_conn *conn, u8_t err);
static void __disconnected(struct bt_conn *conn, u8_t reason);

// Declare the read badge info callback
static ssize_t __read_badge_info(struct bt_conn *conn,
                                 const struct bt_gatt_attr *attr, void *buf,
                                 u16_t len, u16_t offset);

// Declare the read mesh callback
static ssize_t __read_mesh(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, void *buf,
                           u16_t len, u16_t offset);

// Declare the write mesh callback
static ssize_t __write_mesh(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr, const void *buf,
                            u16_t len, u16_t offset, u8_t flags);

// Declare the read time callback
static ssize_t __read_time(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, void *buf,
                           u16_t len, u16_t offset);

// Declare the write time callback
static ssize_t __write_time(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr, const void *buf,
                            u16_t len, u16_t offset, u8_t flags);

// Delcare the write C2 callback
static ssize_t __write_c2(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          const void *buf, u16_t len, u16_t offset, u8_t flags);

/**
 * @brief Background task that occasionally interrupts mesh to advertise BLE
 * stuff
 */
static void __adv_task() {
  int err;

  // delay before advertising for some reason it must happen here and not when
  // thread created below
  // :(
  k_sleep(15 * 1000);
  while (1) {
    // Don't run proxy if disabled
    if (ff_settings_ptr_get()->proxy) {
      // Startup the advertiser, this will break mesh only while it's
      // advertising
      LOG_DBG("Proxy advertising");
      err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);

      if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
      } else {
        k_sleep(ADV_DURATION);
        bt_le_adv_stop();
      }

      // Stop it and let mesh take over again
      LOG_DBG("Proxy advertising done.");
    } else {
      LOG_DBG("Proxy disabled, not advertising");
    }

    k_sleep(ADV_PERIOD);
  }
}

// Declare the config changed callback
static void __ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value);

// Proxy service
static struct bt_gatt_attr m_attrs[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(0x3000)),

    // Define time characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x3001),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, __read_time,
                           __write_time, NULL),

    // Define cloud <--> mesh characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x3002), BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE, __read_mesh, __write_mesh, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_DECLARE_16(0x3002),
                       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, __read_mesh,
                       __write_mesh, NULL),

    BT_GATT_CCC(blvl_ccc_cfg, __ccc_cfg_changed),

};

// Badge Info Service
static struct bt_gatt_attr bis_attrs[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(0x4000)),

    // Define badge info characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x4001), BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, __read_badge_info, NULL, NULL),

    // Define C2 characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x4002),
                           BT_GATT_CHRC_WRITE_WITHOUT_RESP, BT_GATT_PERM_WRITE,
                           NULL, __write_c2, NULL),
};

// Define the services using the attributes above
static struct bt_gatt_service m_bis_service = BT_GATT_SERVICE(bis_attrs);
static struct bt_gatt_service m_proxy_service = BT_GATT_SERVICE(m_attrs);

// Connection callbacks
static struct bt_conn_cb m_conn_callbacks = {
    .connected = __connected,
    .disconnected = __disconnected,
};

/**
 * @brief Handler for when BLE connection established
 * @param conn    Pointer to the BLE conn object
 * @param err     Error code for connection
 */
static void __connected(struct bt_conn *conn, u8_t err) {
  LOG_INF("Proxy Connected!");
  if (!m_conn) {
    m_conn = bt_conn_ref(conn);
  }
}

/**
 * @brief Handler for when BLE connection dropped
 * @param conn    Pointer to BLE conn object
 * @param reason  Reason code for disconnection
 */
static void __disconnected(struct bt_conn *conn, u8_t reason) {
  LOG_INF("Proxy Disconnected. Reason: %d", reason);
  if (m_conn) {
    // Cleanup
    bt_conn_unref(m_conn);
    m_conn = NULL;
  }
}

/**
 * @brief Callback for reading badge info
 * @param conn    Pointer to bluetooth connection
 * @param attr    The ble attribute
 * @param buf     Pointer to buffer to use for badge info
 * @param len     Length of the buffer
 * @param offset  Offset for long reads
 * @return Size of the data read
 */
static ssize_t __read_badge_info(struct bt_conn *conn,
                                 const struct bt_gatt_attr *attr, void *buf,
                                 u16_t len, u16_t offset) {
  mesh_model_status_t status;

  status.address = ff_mesh_addr_get();
  status.fw_version_int = VERSION_INT;
  status.post = ff_post_state_get();
  status.score = sys_rand32_get() % 2000;
  memcpy(status.name, "BENDER", 6);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &status,
                           sizeof(mesh_model_status_t));
}

/**
 * @brief Callback for when the mesh data characteristic is read
 * @param conn    : Current BLE connectiong
 * @param attr    : Gatt connection attributes
 * @param buf     : The buffer to write to to support the read - probably for
 * long reads or streaming?
 * @param len     : Length of the buffer
 * @param offset  : Offset inside the buffer
 * @return The number of bytes read or negative if an error
 */
static ssize_t __read_mesh(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, void *buf,
                           u16_t len, u16_t offset) {
  uint32_t d = k_uptime_get_32();
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &d, sizeof(d));
}

/**
 * @brief Callback for when the characteristic for data is written to by passing
 * the data directly onto the mesh.
 * @param conn    : Current BLE connectiong
 * @param attr    : Gatt connection attributes
 * @param buf     : The buffer to write to to support the write - probably for
 * long writes or streaming?
 * @param len     : Length of the buffer
 * @param offset  : Offset inside the buffer
 * @param flags   : Something something, not sure
 * @return The number of bytes read or negative if an error
 */
static ssize_t __write_mesh(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr, const void *buf,
                            u16_t len, u16_t offset, u8_t flags) {
  LOG_WRN("%s not implemented", __func__);
  return 0;
}

/**
 * @brief Callback for when the current time characteristic is read
 * @param conn    : Current BLE connectiong
 * @param attr    : Gatt connection attributes
 * @param buf     : The buffer to write to to support the read - probably for
 * long reads or streaming?
 * @param len     : Length of the buffer
 * @param offset  : Offset inside the buffer
 * @return The number of bytes read or negative if an error
 */
static ssize_t __read_time(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, void *buf,
                           u16_t len, u16_t offset) {
  struct timespec now = ff_time_now_get();
  LOG_DBG("Read Time, returning %d", now.tv_sec);

  return bt_gatt_attr_read(conn, attr, buf, len, offset, &now,
                           sizeof(struct timespec));
}

/**
 * @brief Callback for when the characteristic for time is written. Update
 * internal time and push that out over the mesh
 * @param conn    : Current BLE connectiong
 * @param attr    : Gatt connection attributes
 * @param buf     : The buffer to write to to support the write - probably for
 * long writes or streaming?
 * @param len     : Length of the buffer
 * @param offset  : Offset inside the buffer
 * @param flags   : Something something, not sure
 * @return The number of bytes read or negative if an error
 */
static ssize_t __write_time(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr, const void *buf,
                            u16_t len, u16_t offset, u8_t flags) {
  LOG_DBG("%s len=%d offset=%d flags=0x%02x", __func__, len, offset, flags);

  struct timespec new_time;
  if (len == sizeof(struct timespec)) {
    memcpy(&new_time, buf, len);
    LOG_DBG("Parsed time %d.%d", new_time.tv_sec, new_time.tv_nsec);
    ff_time_now_set(&new_time);

    // Share our time with the mesh
    ff_mesh_model_time_publish_now();

    LOG_DBG("Time now sec since epoc = %d", ff_time_now_get().tv_sec);
    return len;
  }

  return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  // memcpy(&value, buf, len);

  // if (value + 1 > NUMBER_OF_SLOTS) {
  //   return BT_GATT_ERR(BT_ATT_ERR_WRITE_NOT_PERMITTED);
  // }

  // eds_active_slot = value;

  return len;
}

/**
 * @brief Callback for when the characteristic for C2 is written. Perform the
 * action and spread the C2 command
 * @param conn    : Current BLE connectiong
 * @param attr    : Gatt connection attributes
 * @param buf     : The buffer to write to to support the write - probably for
 * long writes or streaming?
 * @param len     : Length of the buffer
 * @param offset  : Offset inside the buffer
 * @param flags   : Something something, not sure
 * @return The number of bytes read or negative if an error
 */
static ssize_t __write_c2(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          const void *buf, u16_t len, u16_t offset,
                          u8_t flags) {
  if (len != 2) {
    return -1;
  }

  mesh_model_c2_t *p_c2 = (mesh_model_c2_t *)buf;
  ff_mesh_model_c2_publish_now(p_c2->command, p_c2->data);

  return len;
}

/**
 * @brief Handle ble configuration changed event
 * @param attr  Pointer to gatt parameters
 * @param value The new value
 */
static void __ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value) {
  // LOG_DBG("%s changed. value=%d", __func__, value);
}

/**
 * @brief Initialize the mesh proxy services
 */
void ff_mesh_proxy_init() {
  bt_gatt_service_register(&m_proxy_service);
  bt_gatt_service_register(&m_bis_service);
  bt_conn_cb_register(&m_conn_callbacks);

  LOG_INF("Mesh Proxy Service Initialized");
}

/**
 * @brief Notify the peer of new proxy data
 * @param opcode  16-bit opcode this data belongs to
 * @param data    Buffer of data to send to the peer
 * @param len     Length of data in buffer
 * @param ttl     Time to live of the packet we're notifying with
 */
void ff_mesh_proxy_notify(uint16_t opcode, uint8_t *p_data, uint16_t len,
                          uint8_t ttl) {
  if (m_conn) {

    uint8_t packet_len = len + 2 + 1; // Packet is [opcode 2B][packet nB][TTL
                                      // 1B]
    uint8_t packet[packet_len];

    // Prepend opcode
    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, p_data, len);
    memcpy(packet + packet_len - 1, &ttl, 1); //append ttl to end

    int err = bt_gatt_notify(m_conn, &m_attrs[3], packet, packet_len);
    LOG_DBG("Notifying with data, len = %d result = %d", packet_len, err);
  }
}

K_THREAD_DEFINE(ble_adv, 1024, __adv_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 10000);