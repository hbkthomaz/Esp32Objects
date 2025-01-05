#ifndef BLE_MANAGER_HPP
#define BLE_MANAGER_HPP

#include <string>
#include "CommandManager.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "esp_bt.h"

/**
 * @class BleManager
 * @brief Manages Bluetooth Low Energy (BLE) operations including initialization, handling callbacks, and managing notifications.
 *
 * The BleManager class encapsulates the functionality required to set up and manage BLE services,
 * handle GATT server events, process write events, and send notifications to connected BLE clients.
 */
class BleManager
{
  public:
    /**
     * @brief Constructs a new BleManager object.
     *
     * Initializes member variables and prepares the BLE manager for initialization.
     */
    BleManager();

    /**
     * @brief Destructs the BleManager object.
     *
     * Cleans up resources such as semaphores and ensures proper shutdown of BLE operations.
     */
    ~BleManager();

    /**
     * @brief Initializes the BLE manager.
     *
     * Sets up BLE configurations, registers GATT callbacks, and starts advertising.
     * This method should be called after constructing the BleManager object to begin BLE operations.
     */
    void Init();

  private:
    /**
     * @brief Static GATT Server callback function.
     *
     * Serves as a static entry point for GATT server events and forwards them to the instance's callback handler.
     *
     * @param event The GATT server event type.
     * @param gatts_if The GATT interface.
     * @param param Pointer to the event parameters.
     */
    static void GATTSCallbackStatic(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

    /**
     * @brief Instance GATT Server callback handler.
     *
     * Handles GATT server events specific to this BleManager instance.
     *
     * @param event The GATT server event type.
     * @param gatts_if The GATT interface.
     * @param param Pointer to the event parameters.
     */
    void GATTSCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

    /**
     * @brief Handles write events from BLE clients.
     *
     * Processes data written by connected BLE clients and delegates commands to the CommandManager.
     *
     * @param write The parameters associated with the write event.
     */
    void HandleWriteEvent(const esp_ble_gatts_cb_param_t::gatts_write_evt_param &write);

    /**
     * @brief Sends a notification to connected BLE clients.
     *
     * Transmits a notification containing the specified response string to all clients that have notifications enabled.
     *
     * @param response The response string to send as a notification.
     */
    void SendNotification(const std::string &response);

    /// Singleton instance of BleManager
    static BleManager *instance;

    /// Manages command processing and execution
    CommandManager commandManager;

    // UUIDs for BLE service and characteristics
    static const uint16_t SERVICE_UUID;
    static const uint16_t CHAR_UUID_WRITE;
    static const uint16_t CHAR_UUID_NOTIFY;
    static const uint16_t GATT_UUID_PRI_SERVICE;
    static const uint16_t GATT_UUID_CHAR_DECLARE;
    static const uint16_t GATT_UUID_CHAR_CLIENT_CONFIG;

    /// Represents an invalid connection identifier
    static const uint16_t INVALID_CONN_ID;

    /// Profile application index
    static const uint8_t PROFILE_APP_IDX;

    /// Service instance identifier
    static const uint8_t SVC_INST_ID;

    /// Properties for the write characteristic
    static uint8_t char_prop_write;

    /// Properties for the notify characteristic
    static uint8_t char_prop_notify;

    /// Current connection identifier
    static uint16_t connection_id;

    /// Handle for the notify characteristic
    uint16_t notify_handle;

    /// Global GATT interface
    esp_gatt_if_t gatts_if_global;

    /// Flag indicating if notifications are enabled
    bool notifications_enabled;

    /// Flag indicating if a notification is currently in progress
    bool notification_in_progress;

    /// Mutex to protect shared resources
    SemaphoreHandle_t resource_mutex;

    /// Advertising data array
    static uint8_t adv_data[];

    /// Advertising parameters
    static esp_ble_adv_params_t adv_params;

    /// GATT database configuration
    static esp_gatts_attr_db_t gatt_db[];
};

#endif // BLE_MANAGER_HPP
