#include "string.h"
#include "cdc_uart.h"
#include "av-generic.h"

cdc_uart_t g_cdc_uart1;

static int UART_CDC_UPSTREAM_AVAILABLE(uart_cdc_upstream_t *us)
{
    /* Ensure usart_wr_idx read only once.
     * Otherwise it need to protected by interrupt dis/en */
    int uart_wr_idx = us->uart_wr_idx;

    return (uart_wr_idx >= us->usbd_rd_idx) ?
                (uart_wr_idx - us->usbd_rd_idx) :
                (uart_wr_idx - us->usbd_rd_idx + UART_CDC_UPSTREAM_BUFF_SIZE);
}


static void cdc_uart_upstream_rx_cont(uart_cdc_upstream_t *upstream)
{
	int d_free, d_border;
	int bytes_to_rx;

	int uart_wr_idx = upstream->uart_wr_idx;

	__disable_irq(); // AV: REmove me

	/* Get number of free bytes in buffer */
	d_free = upstream->usbd_rd_idx - uart_wr_idx - 1;
	if (d_free < 0) d_free += UART_CDC_UPSTREAM_BUFF_SIZE;

	if (d_free == 0) {
		/* Overflow - something wrong with buffer unloading */
		/* Discard oldest chunk and continue RX as usually */
		upstream->usbd_rd_idx += UART_CDC_UPSTREAM_CHUNK_SIZE;
		if (upstream->usbd_rd_idx >= UART_CDC_UPSTREAM_BUFF_SIZE) {
			upstream->usbd_rd_idx -= UART_CDC_UPSTREAM_BUFF_SIZE;
		}
		d_free = UART_CDC_UPSTREAM_CHUNK_SIZE;
		upstream->uart_ovfl_bytes += UART_CDC_UPSTREAM_CHUNK_SIZE;
		upstream->uart_ovfl_cnt ++;
	}

	/* Get number of free bytes till border */
	d_border = UART_CDC_UPSTREAM_BUFF_SIZE - uart_wr_idx;

	bytes_to_rx = MIN(UART_CDC_UPSTREAM_CHUNK_SIZE, d_free);
	bytes_to_rx = MIN(bytes_to_rx, d_border);

	HAL_UART_Receive_IT(upstream->huart, &upstream->buff[uart_wr_idx], bytes_to_rx);

	__enable_irq(); // AV: REmove me

	return;
}

void cdc_uart_upstream_send (uart_cdc_upstream_t *us)
{
	USBD_Status usbd_rc;
	int bytes_available;
	int bytes_to_tx;
	int till_border;
	int uart_wr_idx = us->uart_wr_idx;  /* Ensure atomic usage */

	__disable_irq(); // AV: TODO: remove me and test carefully
	bytes_available = UART_CDC_UPSTREAM_AVAILABLE(us);

	till_border = UART_CDC_UPSTREAM_BUFF_SIZE - us->usbd_rd_idx;

	/* (CDC_DATA_IN_PACKET_SIZE - 1) is to avoid ZLP exchange. */
	bytes_to_tx = MIN(CDC_DATA_IN_PACKET_SIZE - 1, bytes_available);
	bytes_to_tx = MIN(bytes_to_tx, till_border);

	usbd_rc = USBD_CDC_TransmitPacket(
			us->hcdc,
			&us->buff[us->usbd_rd_idx],
			(uint16_t)bytes_to_tx);

	if (usbd_rc == USBD_OK) {
		us->stat_usbd_tx_bytes += bytes_to_tx;

		us->usbd_rd_idx += bytes_to_tx;
		assert_param(us->usbd_rd_idx <= UART_CDC_UPSTREAM_BUFF_SIZE);
		if (us->usbd_rd_idx >= UART_CDC_UPSTREAM_BUFF_SIZE) {
			us->usbd_rd_idx -= UART_CDC_UPSTREAM_BUFF_SIZE;
		}

		/* Stop t/o timer if no more data */
		if (us->usbd_rd_idx == uart_wr_idx) {
			us->timeout = -1;
		}
	}
	__enable_irq(); // AV: TODO: remove me and test carefully

}

int8_t cdc_uart_get_line_encoding (UART_HandleTypeDef *huart, pstn_line_coding_t *lc)
{
	return 0;
}

int8_t cdc_uart_set_line_encoding (UART_HandleTypeDef *huart, pstn_line_coding_t *lc)
{
	return 0;
}

void cdc_uart_downstream_on_idle(uart_cdc_downstream_t *ds)
{
    if (ds->uart_ready_to_tx) {
    	/* If no more data for UART TX request next data bunch from USB */
	    if (USBD_OK == USBD_CDC_ReceivePacket(ds->hcdc)) {
	  	    ds->uart_ready_to_tx = 0;
	    }
    }

    if (ds->cdc_data_received) {
    	__disable_irq();

    	HAL_UART_Transmit_DMA(ds->huart,
            ds->buff, ds->cdc_data_received);

	    ds->cdc_data_received = 0;

	    __enable_irq();

    }
}


void cdc_uart_downstream_init (
		uart_cdc_downstream_t *ds,
		USBD_CDC_Handle *hcdc,
		UART_HandleTypeDef *huart)
{
	ds->hcdc = hcdc;
	ds->huart = huart;
	ds->uart_ready_to_tx = 1;
}

void cdc_uart_upstream_init (
		uart_cdc_upstream_t *us,
		USBD_CDC_Handle *hcdc,
		UART_HandleTypeDef *huart)
{
	us->hcdc = hcdc;
	us->huart = huart;
}

/*
 * UART RX transaction not finished within reasonable time since
 * the last USB transaction.
 * Steal data from UART transaction and legalize them in the buffer.
 * The data will be transmitted in the idle loop
 */
void uart_cdc_upstream_timer (uart_cdc_upstream_t *us)
{
	UART_HandleTypeDef *huart = us->huart;
	int bytes_avail;

	if (us->timeout == 0) {
		/* Timer already triggered */
		return;
	}

	/*
	 *  Check number of bytes received by UART.
	 *  Timeout should be initialized at the very 1st byte received
	 *  by the UART, but there is no callback in ISR. So, let's check it
	 *  periodically.
	 */
	bytes_avail = huart->RxXferSize - huart->RxXferCount;
	if (us->timeout == -1) {
	    if (bytes_avail) {
	        us->timeout = UART_CDC_TIMEOUT_PERIOD_MS;
	        us->timeout_ts = HAL_GetTick() + UART_CDC_TIMEOUT_PERIOD_MS;
	    }
	    return;
	}

    /* if (us->timeout == 1) */
	/* Timer is running */
	if (us->timeout_ts < HAL_GetTick() && bytes_avail) {
		/* Timeout - steal data from UART */
		us->timeout = 0;

		__disable_irq();
            huart->RxXferSize = huart->RxXferCount;

            us->stat_uart_rx_bytes += bytes_avail;
            us->uart_wr_idx += bytes_avail;

            assert_param(us->uart_wr_idx <= UART_CDC_UPSTREAM_BUFF_SIZE);
            if (us->uart_wr_idx >= UART_CDC_UPSTREAM_BUFF_SIZE) {
                us->uart_wr_idx -= UART_CDC_UPSTREAM_BUFF_SIZE;
            }
        __enable_irq();
	}

	return;

}

/*****************************************************************************
 * Interrupt callbacks
 *
 *****************************************************************************/
static uart_cdc_upstream_t *get_us_by_dfi (cdc_dfi_t *dfi)
{
#if MULTI_UART
    uart_cdc_upstream_t *us = &((cdc_uart_t)dfi->ctx)->us;
    return us;
#else
    return &g_cdc_uart1.us;
#endif
}

static uart_cdc_downstream_t *get_ds_by_dfi (cdc_dfi_t *dfi)
{
#if MULTI_UART
    uart_cdc_downstream_t *ds = &((cdc_uart_t)dfi->ctx)->ds;
    return us;
#else
    return &g_cdc_uart1.ds;
#endif
}

static uart_cdc_upstream_t *get_us_by_huart(UART_HandleTypeDef *huart)
{
#if MULTI_UART
    uart_cdc_upstream_t *us =
            (g_cdc_uart1.us.huart == huart) ? &g_cdc_uart1.us : NULL;
    assert(us);
    return us;
#else
    return &g_cdc_uart1.us;
#endif
}

static uart_cdc_downstream_t *get_ds_by_huart(UART_HandleTypeDef *huart)
{
#if MULTI_UART
    uart_cdc_downstream_t *ds =
            (g_cdc_uart1.ds.huart == huart) ? &g_cdc_uart1.ds : NULL;
    assert(ds);
    return ds;
#else
    return &g_cdc_uart1.ds;
#endif
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uart_cdc_downstream_t *ds = get_ds_by_huart(huart);
	ds->uart_ready_to_tx = 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	uart_cdc_upstream_t *us = get_us_by_huart(huart);
	us->uart_err_cnt++;
}

void HAL_UART_RxCpltCallback (UART_HandleTypeDef *huart)
{
    uart_cdc_upstream_t *us = get_us_by_huart(huart);

	us->stat_uart_rx_bytes += huart->RxXferSize;
	us->uart_wr_idx += huart->RxXferSize;
	assert_param(us->uart_wr_idx <= UART_CDC_UPSTREAM_BUFF_SIZE);
	if (us->uart_wr_idx >= UART_CDC_UPSTREAM_BUFF_SIZE) {
		us->uart_wr_idx -= UART_CDC_UPSTREAM_BUFF_SIZE;
	}

	/* Restart timer if stopped */
	if (us->timeout == -1 ) {
        us->timeout = 1;
        us->timeout_ts = UART_CDC_TIMEOUT_PERIOD_MS + HAL_GetTick();
	}

	huart->RxXferSize = 0;
	huart->RxXferCount = 0;
	us->cont_rx = 1;

}

void cdc_uart_dfi_ds_on_rx (cdc_dfi_t *cdc_dfi, uint32_t len)
{
	uart_cdc_downstream_t *ds = get_ds_by_dfi(cdc_dfi);
	ds->cdc_data_received = len;
}

static void cdc_uart_dfi_on_control(cdc_dfi_t *cdc_dfi, uint8_t cmd, uint8_t *buf, uint16_t length)
{
	uart_cdc_upstream_t *us = get_us_by_dfi(cdc_dfi);
	UART_HandleTypeDef *huart = us->huart;

	switch (cmd) {
	case CDC_SET_LINE_CODING:
		/* assert(length == sizeof(pstn_line_coding_t)) */
		cdc_uart_set_line_encoding(huart, (pstn_line_coding_t*) buf);
		break;

	case CDC_GET_LINE_CODING:
		/* assert length == sizeof(pstn_line_coding_t) */
		cdc_uart_get_line_encoding(huart, (pstn_line_coding_t*) buf);
		break;

	case CDC_SET_CONTROL_LINE_STATE:
	case CDC_SEND_ENCAPSULATED_COMMAND:
	case CDC_GET_ENCAPSULATED_RESPONSE:
	case CDC_SET_COMM_FEATURE:
	case CDC_GET_COMM_FEATURE:
	case CDC_CLEAR_COMM_FEATURE:
	case CDC_SEND_BREAK:
	default:
		break;
	}

}


static uint8_t* cdc_uart_dfi_ds_get_buff(struct cdc_dfi_s *cdc_dfi)
{
	uart_cdc_downstream_t *ds = get_ds_by_dfi(cdc_dfi);
	return ds->buff;
}

/*
 * ISR context on CDC_Init
 * DFI callback
 */
static void cdc_uart_dfi_us_rx_start(struct cdc_dfi_s *cdc_dfi)
{
	uart_cdc_upstream_t *us = get_us_by_dfi(cdc_dfi);

	us->usbd_rd_idx = 0;
	us->uart_wr_idx = 0;

	us->uart_err_cnt = 0;
	us->uart_ovfl_bytes = 0;
	us->uart_ovfl_cnt = 0;
	us->timeout = -1;
	us->cont_rx = 1;
}

/*
 * ISR context on CDC_DeInit
 * DFI call back
 */
static void cdc_uart_dfi_us_rx_stop(struct cdc_dfi_s *cdc_dfi)
{
	uart_cdc_upstream_t *us = get_us_by_dfi(cdc_dfi);
	HAL_UART_Abort_IT(us->huart);
	us->timeout = -1;
}

/*
 * DFI interface callback
 *
 */
static void cdc_uart_dfi_on_idle (struct cdc_dfi_s *cdc_dfi)
{
	uart_cdc_upstream_t *us = get_us_by_dfi(cdc_dfi);
	int bytes_available;

	/* Initiate new RX transaction on UART if previous finished */
	if (us->cont_rx) {
		us->cont_rx = 0;
		cdc_uart_upstream_rx_cont(us);
	}

	/* Send data upstream when available */
	bytes_available = UART_CDC_UPSTREAM_AVAILABLE(us);
	if (bytes_available) {
		if (us->timeout == 0 || bytes_available >= CDC_DATA_IN_PACKET_SIZE) {
			cdc_uart_upstream_send(us);
		}
	}

	uart_cdc_upstream_timer(us);
}

void cdc_dfi_uart_init (cdc_dfi_t *cdc_dfi, cdc_uart_t *cdc_uart)
{
	cdc_dfi->on_idle       = cdc_uart_dfi_on_idle;
	cdc_dfi->us_start_rx   = cdc_uart_dfi_us_rx_start;
	cdc_dfi->us_stop_rx    = cdc_uart_dfi_us_rx_stop;
	cdc_dfi->ds_on_rx      = cdc_uart_dfi_ds_on_rx;
	cdc_dfi->ds_get_buffer = cdc_uart_dfi_ds_get_buff;
	cdc_dfi->ds_on_control = cdc_uart_dfi_on_control;
	cdc_dfi->ctx.cdc_uart  = cdc_uart;
}

void cdc_uart_init(cdc_uart_t *cdc_uart, USBD_CDC_Handle *hcdc, UART_HandleTypeDef *huart)
{
	cdc_uart_downstream_init(&cdc_uart->ds, hcdc, huart);
	cdc_uart_upstream_init(&cdc_uart->us, hcdc, huart);

	cdc_dfi_uart_init(&cdc_uart->dfi, cdc_uart);

	hcdc->dfi = &cdc_uart->dfi;
}
