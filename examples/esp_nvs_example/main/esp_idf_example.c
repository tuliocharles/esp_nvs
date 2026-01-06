#include <stdio.h>
#include "esp_nvs.h"


void app_main(void)
{
    esp_nvs_handle_t esp_nvs_handle;
    esp_nvs_config_t esp_nvs_config = {
        .name_space = "storage",
        .key = "key-1",
        .value_size = 64,
    };
    
    init_esp_nvs(&esp_nvs_config, &esp_nvs_handle);

    plot_nvs_stats();
    
    esp_nvs_write_string( "example_value", esp_nvs_handle);
    
    char *value = NULL;
    esp_nvs_read_string(esp_nvs_handle, &value);
    printf("Read string: %s\n", value);

    //esp_nvs_delete_string(esp_nvs_handle);

    esp_nvs_read_string(esp_nvs_handle, &value);
    printf("Read string: %s\n", value);

    esp_nvs_change_key("key-2", esp_nvs_handle);

    esp_nvs_write_string( "example_value2", esp_nvs_handle);

    esp_nvs_read_string(esp_nvs_handle, &value);
    printf("Read string: %s\n", value);

    esp_nvs_delete_string(esp_nvs_handle);

    esp_nvs_read_string(esp_nvs_handle, &value);
    printf("Read string: %s\n", value);

    esp_nvs_list_namespaces();

    esp_nvs_deinit(esp_nvs_handle);
   
}
