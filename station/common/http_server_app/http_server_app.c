#include "http_server_app.h"

/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
// #include "protocol_examples_common.h"
// #include "protocol_examples_utils.h"
// #include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_event.h"
// #include "esp_netif.h"
// #include "esp_tls.h"
#include "esp_check.h"

#if !CONFIG_IDF_TARGET_LINUX
#include <esp_wifi.h>
#include <esp_system.h>
#include "nvs_flash.h"
// #include "esp_eth.h"
#endif  // !CONFIG_IDF_TARGET_LINUX

#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)


/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static httpd_req_t *REQ;

static const char *TAG = "example";
static httpd_handle_t server = NULL;

static http_post_callback_t http_post_switch_callback = NULL;
static http_get_callback_t http_get_dht11_callback = NULL;
static http_post_callback_t http_post_slider_callback = NULL;
static http_post_callback_t http_post_wifi_callback = NULL;
static http_get_data_callback_t http_get_rgb_data_callback = NULL;

// extern const uint8_t index_html_start[] asm("_binary_anh_jpg_start");
// extern const uint8_t index_html_end[] asm("_binary_anh_jpg_end");
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char* resp_str = (const char*) "Hello world\n";
    // httpd_resp_send(req, resp_str, strlen(resp_str));
    // httpd_resp_set_type(req, "image/jpg");
    // httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}

void dht11_response (char *data, int len)
{
    httpd_resp_send(REQ, data, len);
}

static const httpd_uri_t get_dht11 = {
    .uri       = "/dht11",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static esp_err_t dht11_get_handler(httpd_req_t *req)
{
    // const char* resp_str = (const char*) "{\"temperature\": \"27.3\", \"humidity\": \"80\"}";
    // httpd_resp_send(req, resp_str, strlen(resp_str));
    // httpd_resp_set_type(req, "image/jpg");
    // httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);
    REQ = req;
    http_get_dht11_callback();


    return ESP_OK;
}

static const httpd_uri_t get_data_dht11 = {
    .uri       = "/getdatadht11",
    .method    = HTTP_GET,
    .handler   = dht11_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static esp_err_t rgb_get_handler(httpd_req_t *req)
{
    char*  buf;
    int buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char value[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "color", value, sizeof(value)) == ESP_OK) {
                http_get_rgb_data_callback(value, 6);
            }
        }
        free(buf);
    }
    return ESP_OK;
}

static const httpd_uri_t get_data_rgb = {
    .uri       = "/rgb",
    .method    = HTTP_GET,
    .handler   = rgb_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


static esp_err_t data_post_handler(httpd_req_t *req)
{
    char buf[100];

        /* Read the data for the request */
    httpd_req_recv(req, buf,req->content_len);
    printf("data: %s\n", buf);

        /* Send back the same data */
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t post_data = {
    .uri       = "/post",
    .method    = HTTP_POST,
    .handler   = data_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static esp_err_t sw1_post_handler(httpd_req_t *req)
{
    char buf[100];

        /* Read the data for the request */
    httpd_req_recv(req, buf,req->content_len);
    http_post_switch_callback(buf, req->content_len);
    // printf("data: %s\n", buf);

        /* Send back the same data */
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t sw1_post_data = {
    .uri       = "/switch1",
    .method    = HTTP_POST,
    .handler   = sw1_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static esp_err_t slider_post_handler(httpd_req_t *req)
{
    char buf[100];

        /* Read the data for the request */
    httpd_req_recv(req, buf,req->content_len);
    http_post_slider_callback(buf, req->content_len);
    // printf("data: %s\n", buf);

        /* Send back the same data */
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t slider_post_data = {
    .uri       = "/slider",
    .method    = HTTP_POST,
    .handler   = slider_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static esp_err_t wifi_post_handler(httpd_req_t *req)
{
    char buf[100];
    httpd_req_recv(req, buf,req->content_len);
    // printf("data: %s\n", buf);
    http_post_wifi_callback(buf, req->content_len);

    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t wifi_post_data = {
    .uri       = "/wifiinfo",
    .method    = HTTP_POST,
    .handler   = wifi_post_handler,
    .user_ctx  = NULL
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/dht11", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/dht11 URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } 
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}



void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable  = true;
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_data_dht11);
        httpd_register_uri_handler(server, &get_dht11);
        httpd_register_uri_handler(server, &post_data);
        httpd_register_uri_handler(server, &sw1_post_data);
        httpd_register_uri_handler(server, &slider_post_data);
        httpd_register_uri_handler(server, &wifi_post_data);
        httpd_register_uri_handler(server, &get_data_rgb);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else{
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}

void http_set_callback_switch(void *cb)
{
    http_post_switch_callback = cb;
}

void http_set_callback_dht11(void *cb)
{
    http_get_dht11_callback = cb;
}

void http_set_callback_slider(void *cb)
{
    http_post_slider_callback = cb;
}

void http_set_callback_wifiinfo(void *cb)
{
    http_post_wifi_callback = cb;
}

void http_set_callback_rgb(void *cb)
{
    http_get_rgb_data_callback = cb;
}