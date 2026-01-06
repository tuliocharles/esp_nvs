#include <stdio.h>
#include "esp_nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>

static const char *TAG = "NVS_INIT";

typedef struct esp_nvs_t esp_nvs_t;

struct esp_nvs_t{
    char name_space[64];
    char key[64];
    char *value_ptr;
    size_t value_size;
    size_t max_value_size;
    
};

#define MAX_NAMESPACES 16
#define MAX_NAMESPACE_LEN NVS_KEY_NAME_MAX_SIZE

esp_err_t init_esp_nvs(esp_nvs_config_t *config, esp_nvs_handle_t *handle)
{
    esp_err_t ret = ESP_OK;
    esp_nvs_t *esp_nvs = NULL;
    ESP_GOTO_ON_FALSE(config && handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid arguments");
    esp_nvs = calloc(1, sizeof(esp_nvs_t));


    memcpy(esp_nvs->name_space, config->name_space, sizeof(esp_nvs->name_space));
    memcpy(esp_nvs->key, config->key, sizeof(esp_nvs->key));

    esp_nvs->value_size = config->value_size;
    esp_nvs->max_value_size = config->value_size;
    
    esp_nvs->value_ptr = malloc(esp_nvs->value_size);
    

    // Initialize NVS
    ret = nvs_flash_init();
    // Handle NVS initialization errors
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // Check if NVS is initialized successfully
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    *handle = esp_nvs;
    ESP_LOGI(TAG, "NVS initialized successfully");
    ret =  ESP_OK;
    return ret;
err:
    if (esp_nvs && esp_nvs->value_ptr) {
        free(esp_nvs->value_ptr);
    }

    if (esp_nvs) {
        free(esp_nvs);
        esp_nvs = NULL;
    }


    ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
    return ret;

}

void plot_nvs_stats(void)
{
    nvs_stats_t nvs_stats;
    esp_err_t ret = nvs_get_stats(NULL, &nvs_stats);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get NVS stats: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "NVS stats: %d number of name space , %d used_entries", nvs_stats.namespace_count, nvs_stats.used_entries);
}

void esp_nvs_write_string(const char *value, esp_nvs_handle_t esp_nvs_handle)
{
    nvs_handle_t handle;

    esp_err_t ret = nvs_open(esp_nvs_handle->name_space, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return;
    }

    ret = nvs_set_str(handle, esp_nvs_handle->key, value);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write string: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Wrote string: %s", value);
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes: %s", esp_err_to_name(ret));
    }

    nvs_close(handle);
}

esp_err_t esp_nvs_read_string(esp_nvs_handle_t esp_nvs_handle, char **out_str)
{

    esp_nvs_handle->value_size = esp_nvs_handle->max_value_size; 
    
    nvs_handle_t handle;

    esp_err_t ret = nvs_open(esp_nvs_handle->name_space, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return ret;
    }

    size_t required_size;
    ret = nvs_get_str(handle, esp_nvs_handle->key, NULL, &required_size);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get string size: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    if (required_size > esp_nvs_handle->value_size)
    {
        ESP_LOGE(TAG, "Buffer too small for string value");
        nvs_close(handle);
        return ret;
        }
    
        
    ret = nvs_get_str(handle, esp_nvs_handle->key, esp_nvs_handle->value_ptr, &esp_nvs_handle->value_size);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get string: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Read string: %s", esp_nvs_handle->value_ptr);
    }

    nvs_close(handle);

    *out_str = esp_nvs_handle->value_ptr;
    ESP_LOGI(TAG, "Read string: %s", *out_str);
    ret = ESP_OK;
    return ret;
}

void esp_nvs_delete_string(esp_nvs_handle_t esp_nvs_handle)
{
    nvs_handle_t handle;

    esp_err_t ret = nvs_open(esp_nvs_handle->name_space, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return;
    }

    ret = nvs_erase_key(handle, esp_nvs_handle->key);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete string: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Deleted string with key: %s", esp_nvs_handle->key);
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes: %s", esp_err_to_name(ret));
    }

    nvs_close(handle);
}
    
void esp_nvs_deinit(esp_nvs_handle_t esp_nvs_handle)
{
    if (esp_nvs_handle && esp_nvs_handle->value_ptr) {
        free(esp_nvs_handle->value_ptr);
    }

    if (esp_nvs_handle) {
        free(esp_nvs_handle);
        esp_nvs_handle = NULL;
    }
}

void esp_nvs_change_key(const char *new_key, esp_nvs_handle_t esp_nvs_handle){
    
    memcpy(esp_nvs_handle->key, new_key, sizeof(esp_nvs_handle->key));
    
}

void esp_nvs_list_namespaces(void)
{    
    
    nvs_iterator_t it = NULL;
    size_t count = 0;
    esp_err_t err;

    err = nvs_entry_find(NVS_DEFAULT_PART_NAME, 
                         NULL,                  
                         NVS_TYPE_ANY,          
                         &it);                  

    if (err != ESP_OK || it == NULL)
    {
        ESP_LOGW(TAG, "No NVS entries found or an error occurred (%s)", esp_err_to_name(err));
        return;
    }

    if (it == NULL)
    {
        ESP_LOGE(TAG, "Failed to find NVS entry");
        return;
    }

    size_t max_namespaces = 64;
    char **namespaces = malloc(max_namespaces * sizeof(char *));
    for (size_t i = 0; i < max_namespaces; i++) {
        namespaces[i] = malloc(64 * sizeof(char)); 
    }

    
    do
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        bool seen = false;
        for (size_t i = 0; i < count; ++i)
        {
            if (strcmp(namespaces[i], info.namespace_name) == 0)
            {
                seen = true;
                break;
            }
        }
        if (!seen && count < max_namespaces)
        {
            strncpy(namespaces[count], info.namespace_name, MAX_NAMESPACE_LEN);
            namespaces[count][MAX_NAMESPACE_LEN - 1] = '\0';
            ESP_LOGI(TAG, "[%u] namespace: %s", (unsigned)count, namespaces[count]);
            count++;
        }

        err = nvs_entry_next(&it); 
    } while (err == ESP_OK && it != NULL);

    ESP_LOGI(TAG, "Total unique namespaces: %u", (unsigned)count); 
    for (size_t i = 0; i < count; ++i)
    {
        ESP_LOGI(TAG, "Unique namespace: %s", namespaces[i]);
    }
    
    nvs_release_iterator(it);
    
    for (size_t i = 0; i < max_namespaces; i++) {
        free(namespaces[i]);
    }
    free(namespaces);
    
    return;
}



