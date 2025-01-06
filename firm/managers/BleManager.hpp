#ifndef BLE_MANAGER_HPP
#define BLE_MANAGER_HPP

#include <string>
#include <vector>
#include "CommandManager.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "esp_bt.h"

/**
 * @class BleManager
 * @brief Manages Bluetooth Low Energy (BLE) operations.
 */
class BleManager
{
  public:
    /**
     * @brief Constructs a new BleManager object.
     */
    BleManager();

    /**
     * @brief Destructs the BleManager object.
     */
    ~BleManager();

    /**
     * @brief Initializes the BLE manager.
     */
    void Init();

  private:
    /**
     * @brief Static GATT Server callback function.
     * @param event The GATT server event type.
     * @param gatts_if The GATT interface.
     * @param param Pointer to the event parameters.
     */
    static void GATTSCallbackStatic(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

    /**
     * @brief Instance GATT Server callback handler.
     * @param event The GATT server event type.
     * @param gatts_if The GATT interface.
     * @param param Pointer to the event parameters.
     */
    void GATTSCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    /**
     * @brief Verifies the integrity of received data using a Block Check Character (BCC).
     *
     * @param data The vector of bytes containing the data to validate.
     * @return true If the data's checksum is valid.
     * @return false If the data's checksum is invalid.
     */
    bool VerifyBCC(const std::vector<uint8_t> &data);

    /**
     * @brief Handles write events from BLE clients.
     * @param write The parameters associated with the write event.
     */
    void HandleWriteEvent(const esp_ble_gatts_cb_param_t::gatts_write_evt_param &write);

    /**
     * @brief Sends a notification to connected BLE clients.
     * @param response The response string to send as a notification.
     */
    void SendNotification(const std::string &response, uint16_t conn_id);

    /**
     * @brief Adds a connection to the connection list.
     * @param conn_id The connection ID to add.
     */
    void AddConnection(uint16_t conn_id);

    /**
     * @brief Removes a connection from the connection list.
     * @param conn_id The connection ID to remove.
     */
    void RemoveConnection(uint16_t conn_id);

    static BleManager *instance;

    CommandManager commandManager;

    static const uint16_t SERVICE_UUID;
    static const uint16_t CHAR_UUID_WRITE;
    static const uint16_t CHAR_UUID_NOTIFY;
    static const uint16_t GATT_UUID_PRI_SERVICE;
    static const uint16_t GATT_UUID_CHAR_DECLARE;
    static const uint16_t GATT_UUID_CHAR_CLIENT_CONFIG;

    static const uint16_t INVALID_CONN_ID;
    static const uint8_t  PROFILE_APP_IDX;
    static const uint8_t  SVC_INST_ID;

    static uint8_t char_prop_write;
    static uint8_t char_prop_notify;

    static const int MAX_CONNECTIONS = 9;
    static uint16_t  connection_ids[MAX_CONNECTIONS];
    static bool      notifications_enabled[MAX_CONNECTIONS];

    static std::vector<uint8_t> command_buffers[MAX_CONNECTIONS];

    static uint8_t              adv_data[];
    static esp_ble_adv_params_t adv_params;

    static esp_gatts_attr_db_t gatt_db[];

    uint16_t          notify_handle;
    esp_gatt_if_t     gatts_if_global;
    bool              notification_in_progress;
    SemaphoreHandle_t resource_mutex;
};

#endif // BLE_MANAGER_HPP
