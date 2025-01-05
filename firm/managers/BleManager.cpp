#include "BleManager.hpp"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include <cstring>

BleManager *BleManager::instance = nullptr;

const uint16_t BleManager::SERVICE_UUID                 = 0x00FF;
const uint16_t BleManager::CHAR_UUID_WRITE              = 0xFF01;
const uint16_t BleManager::CHAR_UUID_NOTIFY             = 0xFF02;
const uint16_t BleManager::GATT_UUID_PRI_SERVICE        = 0x2800;
const uint16_t BleManager::GATT_UUID_CHAR_DECLARE       = 0x2803;
const uint16_t BleManager::GATT_UUID_CHAR_CLIENT_CONFIG = 0x2902;

const uint16_t BleManager::INVALID_CONN_ID = 0xFFFF;
const uint8_t  BleManager::PROFILE_APP_IDX = 0;
const uint8_t  BleManager::SVC_INST_ID     = 0;

uint8_t BleManager::char_prop_write  = ESP_GATT_CHAR_PROP_BIT_WRITE;
uint8_t BleManager::char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;

uint16_t BleManager::connection_id = BleManager::INVALID_CONN_ID;

static uint8_t  char_value_write[128]  = {0};
static uint8_t  char_value_notify[128] = {0};
static uint16_t ccc_value_notify       = 0x0000;

uint8_t BleManager::adv_data[] = {0x02, 0x01, 0x06, 0x03, 0x03, 0xFF, 0x00, 0x0A, 0x09, 'E', 'S', 'P', '3', '2', '_', 'B', 'L', 'E'};

esp_ble_adv_params_t BleManager::adv_params = {
    .adv_int_min       = 0x30,
    .adv_int_max       = 0x70,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr         = {0x00},
    .peer_addr_type    = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_gatts_attr_db_t BleManager::gatt_db[] = {
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::GATT_UUID_PRI_SERVICE, ESP_GATT_PERM_READ, sizeof(SERVICE_UUID), sizeof(SERVICE_UUID),
     (uint8_t *)&SERVICE_UUID}                      },
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::GATT_UUID_CHAR_DECLARE, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t),
     (uint8_t *)&BleManager::char_prop_write}       },
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::CHAR_UUID_WRITE, ESP_GATT_PERM_WRITE, sizeof(char_value_write), sizeof(char_value_write),
     char_value_write}                              },
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::GATT_UUID_CHAR_DECLARE, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t),
     (uint8_t *)&BleManager::char_prop_notify}      },
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::CHAR_UUID_NOTIFY, ESP_GATT_PERM_READ, sizeof(char_value_notify), sizeof(char_value_notify),
     char_value_notify}                             },
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&BleManager::GATT_UUID_CHAR_CLIENT_CONFIG, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t),
     sizeof(uint16_t), (uint8_t *)&ccc_value_notify}},
};

BleManager::BleManager() :
    notify_handle(0), gatts_if_global(0), notifications_enabled(false), notification_in_progress(false), resource_mutex(nullptr)
{
    if (instance == nullptr)
    {
        instance = this;
    }
}

BleManager::~BleManager()
{
    if (resource_mutex != nullptr)
    {
        vSemaphoreDelete(resource_mutex);
        resource_mutex = nullptr;
    }
    instance = nullptr;
}

void BleManager::Init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ret = nvs_flash_erase();
        if (ret == ESP_OK)
        {
            ret = nvs_flash_init();
        }
    }
    if (ret != ESP_OK)
    {
        return;
    }

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK)
    {
        return;
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret                               = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK)
    {
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK)
    {
        return;
    }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK)
    {
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK)
    {
        return;
    }

    if (resource_mutex == nullptr)
    {
        resource_mutex = xSemaphoreCreateMutex();
        if (resource_mutex == nullptr)
        {
            return;
        }
    }

    ret = esp_ble_gatts_register_callback(GATTSCallbackStatic);
    if (ret != ESP_OK)
    {
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_APP_IDX);
    if (ret != ESP_OK)
    {
        return;
    }

    commandManager.Init();
}

void BleManager::GATTSCallbackStatic(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (instance != nullptr)
    {
        instance->GATTSCallback(event, gatts_if, param);
    }
}

void BleManager::GATTSCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
        case ESP_GATTS_REG_EVT:
            esp_ble_gap_set_device_name("ESP32_BLE");
            esp_ble_gap_config_adv_data_raw(adv_data, sizeof(adv_data));
            esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, sizeof(gatt_db) / sizeof(gatt_db[0]), SVC_INST_ID);
            gatts_if_global = gatts_if;
            break;

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            if (param->add_attr_tab.status == ESP_GATT_OK)
            {
                esp_ble_gatts_start_service(param->add_attr_tab.handles[0]);
                notify_handle = param->add_attr_tab.handles[4];
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;

        case ESP_GATTS_CONNECT_EVT:
        {
            esp_ble_conn_update_params_t conn_params;
            memset(&conn_params, 0, sizeof(conn_params));
            conn_params.latency = 0;
            conn_params.min_int = adv_params.adv_int_min;
            conn_params.max_int = adv_params.adv_int_max;
            conn_params.timeout = 1000;
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));

            esp_ble_gap_update_conn_params(&conn_params);
            connection_id = param->connect.conn_id;
            break;
        }

        case ESP_GATTS_WRITE_EVT:
            HandleWriteEvent(param->write);
            break;

        case ESP_GATTS_CONF_EVT:
            if (param->conf.status == ESP_GATT_OK)
                notification_in_progress = false;
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            connection_id            = INVALID_CONN_ID;
            notifications_enabled    = false;
            notification_in_progress = false;
            esp_ble_gap_start_advertising(&adv_params);
            break;

        default:
            break;
    }
}

void BleManager::HandleWriteEvent(const esp_ble_gatts_cb_param_t::gatts_write_evt_param &write)
{
    if (write.handle == (notify_handle + 1))
    {
        if (write.len == 2)
        {
            uint16_t ccc_value    = (write.value[1] << 8) | write.value[0];
            notifications_enabled = (ccc_value == 0x0001);
        }
    }
    else if (write.len > 0)
    {
        std::string input(reinterpret_cast<const char *>(write.value), write.len);
        std::string response = commandManager.ProcessCommand(input);
        SendNotification(response);
    }
}

void BleManager::SendNotification(const std::string &response)
{
    if (connection_id == INVALID_CONN_ID || notify_handle == 0 || !notifications_enabled)
    {
        return;
    }

    if (xSemaphoreTake(resource_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (!notification_in_progress)
        {
            notification_in_progress = true;
            uint16_t len             = response.size();
            uint8_t *data            = new uint8_t[len];
            memcpy(data, response.c_str(), len);
            esp_err_t ret = esp_ble_gatts_send_indicate(gatts_if_global, connection_id, notify_handle, len, data, false);
            delete[] data;
            if (ret != ESP_OK)
            {
                notification_in_progress = false;
            }
        }
        xSemaphoreGive(resource_mutex);
    }
}
