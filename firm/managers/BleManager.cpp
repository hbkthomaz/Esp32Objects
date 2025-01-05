#include "BleManager.hpp"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
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

uint16_t BleManager::connection_ids[MAX_CONNECTIONS]        = {BleManager::INVALID_CONN_ID};
bool     BleManager::notifications_enabled[MAX_CONNECTIONS] = {false};

static uint8_t  char_value_write[128]  = {0};
static uint8_t  char_value_notify[128] = {0};
static uint16_t ccc_value_notify       = 0x0000;

uint8_t BleManager::adv_data[] = {0x02, 0x01, 0x06, 0x03, 0x03, 0xFF, 0x00, 0x0A, 0x09, 'E', 'S', 'P', '3', '2', '_', 'B', 'L', 'E'};

esp_ble_adv_params_t BleManager::adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
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

BleManager::BleManager() : notify_handle(0), gatts_if_global(0), notification_in_progress(false), resource_mutex(nullptr)
{
    if (instance == nullptr)
    {
        instance = this;
    }
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        connection_ids[i]        = INVALID_CONN_ID;
        notifications_enabled[i] = false;
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
    do
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
            break;
        }

        ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        if (ret != ESP_OK)
        {
            break;
        }

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ret                               = esp_bt_controller_init(&bt_cfg);
        if (ret != ESP_OK)
        {
            break;
        }

        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret != ESP_OK)
        {
            break;
        }

        ret = esp_bluedroid_init();
        if (ret != ESP_OK)
        {
            break;
        }
        ret = esp_bluedroid_enable();
        if (ret != ESP_OK)
        {
            break;
        }

        if (resource_mutex == nullptr)
        {
            resource_mutex = xSemaphoreCreateMutex();
            if (resource_mutex == nullptr)
            {
                break;
            }
        }

        ret = esp_ble_gatts_register_callback(GATTSCallbackStatic);
        if (ret != ESP_OK)
        {
            break;
        }

        ret = esp_ble_gatts_app_register(PROFILE_APP_IDX);
        if (ret != ESP_OK)
        {
            break;
        }

        commandManager.Init();

    } while (0);
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
            AddConnection(param->connect.conn_id);
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            RemoveConnection(param->disconnect.conn_id);
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GATTS_WRITE_EVT:
            HandleWriteEvent(param->write);
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
            uint16_t ccc_value = (write.value[1] << 8) | write.value[0];
            for (int i = 0; i < MAX_CONNECTIONS; ++i)
            {
                if (connection_ids[i] == write.conn_id)
                {
                    notifications_enabled[i] = (ccc_value == 0x0001);
                    break;
                }
            }
        }
    }
    else if (write.handle == notify_handle - 2)
    {
        std::string input(reinterpret_cast<const char *>(write.value), write.len);
        std::string response = commandManager.ProcessCommand(input);
        SendNotification(response, write.conn_id);
    }
}

void BleManager::SendNotification(const std::string &response, uint16_t conn_id)
{
    do
    {
        if (notify_handle == 0)
        {
            break;
        }

        if (xSemaphoreTake(resource_mutex, portMAX_DELAY) != pdTRUE)
        {
            break;
        }

        for (int i = 0; i < MAX_CONNECTIONS; ++i)
        {
            if (connection_ids[i] == conn_id && notifications_enabled[i])
            {
                uint16_t len  = response.size();
                uint8_t *data = new uint8_t[len];
                memcpy(data, response.c_str(), len);

                esp_ble_gatts_send_indicate(gatts_if_global, conn_id, notify_handle, len, data, false);

                delete[] data;
                break;
            }
        }
        xSemaphoreGive(resource_mutex);
    } while (0);
}

void BleManager::AddConnection(uint16_t conn_id)
{
    do
    {
        if (conn_id == INVALID_CONN_ID)
        {
            break;
        }

        for (int i = 0; i < MAX_CONNECTIONS; ++i)
        {
            if (connection_ids[i] == INVALID_CONN_ID)
            {
                connection_ids[i] = conn_id;
                break;
            }
        }
    } while (0);
}

void BleManager::RemoveConnection(uint16_t conn_id)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (connection_ids[i] == conn_id)
        {
            connection_ids[i]        = INVALID_CONN_ID;
            notifications_enabled[i] = false;
            break;
        }
    }
}
