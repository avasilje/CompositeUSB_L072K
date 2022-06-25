#pragma once

#include "usbd_def.h"
#include "usbd_cdc.h"

#define UART_CDC_TIMEOUT_PERIOD_MS 16
#if 0
#define UART_CDC_UPSTREAM_AVAILABLE(up) \
        (up->uart_wr_idx >= up->usbd_rd_idx ? \
        (up->uart_wr_idx - up->usbd_rd_idx) : \
        (up->uart_wr_idx - up->usbd_rd_idx + UART_CDC_UPSTREAM_BUFF_SIZE))
#endif

#define UART_CDC_UPSTREAM_BUFF_SIZE     1024
#define UART_CDC_UPSTREAM_CHUNK_SIZE    16

#define CDC_UART_DOWN_BUFF_SIZE         64U

typedef struct uart_cdc_downstream_s {

    UART_HandleTypeDef *huart;
    USBD_CDC_Handle   *hcdc;

    int uart_ready_to_tx;                   /* Flag is set by UART upon DMA transfer completed.
                                             * Cleared when new data bunch requested from USB host. */

    int cdc_data_received;                  /* Set upon new data bunch received from USB host.
                                             * Cleared when newly received data bunch passed to UART DMA */

    uint8_t buff[CDC_UART_DOWN_BUFF_SIZE];  /* Buffer to hold data forwarded from a USBD host to the USART */
} uart_cdc_downstream_t;

typedef struct uart_cdc_upstream_s {
    UART_HandleTypeDef *huart;
    USBD_CDC_Handle *hcdc;

    uint8_t buff[UART_CDC_UPSTREAM_BUFF_SIZE];

    volatile int uart_wr_idx;               /* Updated from ISR context */
    int usbd_rd_idx;
    int timeout;                            /* T>0 waiting; T==0 timeout; T<0 stopped
                                             * Timer to steal data from UART buffer if transfer is too slow
                                             * Also works as a timer to initiate transaction when received
                                             * less data than USB_PACKET_SIZE.
                                             */
    uint32_t timeout_ts;
    int cont_rx;

    /* Statistics counters */
    uint32_t stat_uart_rx_bytes;
    uint32_t stat_usbd_tx_bytes;

    /* Error counters */
    uint32_t uart_err_cnt;
    uint32_t uart_ovfl_cnt;
    uint32_t uart_ovfl_bytes;
} uart_cdc_upstream_t;

typedef struct cdc_uart_s {
    uart_cdc_upstream_t   us;
    uart_cdc_downstream_t ds;
    cdc_dfi_t dfi;
} cdc_uart_t;

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

extern void cdc_uart_init(cdc_uart_t *cdc_uart, USBD_CDC_Handle *hcdc, UART_HandleTypeDef *huart);
