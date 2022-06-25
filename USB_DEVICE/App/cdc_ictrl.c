#include <stdarg.h>

#include "av-generic.h"
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "cdc_ictrl.h"
#include "imon.h"

cdc_ictrl_t g_cdc_ictrl;
#define ICTRL_CDC_TX_TIMEOUT_MS 16
#define ICTRL_REV 0

static int ICTRL_CDC_UPSTREAM_AVAILABLE(ictrl_cdc_upstream_t *us)
{
    /* Ensure ictrl_wr_idx read only once.
     * Otherwise it need to protected by interrupt dis/en */
    int ictrl_wr_idx = us->ictrl_wr_idx;

    return (ictrl_wr_idx >= us->usbd_rd_idx) ?
                (ictrl_wr_idx - us->usbd_rd_idx) :
                (ictrl_wr_idx - us->usbd_rd_idx + ICTRL_CDC_UPSTREAM_BUFF_SIZE);
}

static void cdc_ictrl_upstream_send (ictrl_cdc_upstream_t *us)
{
    USBD_Status usbd_rc;
    int bytes_available;
    int bytes_to_tx;
    int till_border;

    bytes_available = ICTRL_CDC_UPSTREAM_AVAILABLE(us);

    till_border = ICTRL_CDC_UPSTREAM_BUFF_SIZE - us->usbd_rd_idx;

    /* (CDC_DATA_IN_PACKET_SIZE - 1) is to avoid ZLP exchange. */
    bytes_to_tx = MIN(CDC_DATA_IN_PACKET_SIZE - 1, bytes_available);
    bytes_to_tx = MIN(bytes_to_tx, till_border);

    usbd_rc = USBD_CDC_TransmitPacket(
            us->hcdc,
            &us->buff[us->usbd_rd_idx],
            (uint16_t)bytes_to_tx);

    if (usbd_rc == USBD_OK) {
        us->stat_tx_bytes += bytes_to_tx;

        us->usbd_rd_idx += bytes_to_tx;
        // AV: CIRC INC
        assert_param(us->usbd_rd_idx <= ICTRL_CDC_UPSTREAM_BUFF_SIZE);
        if (us->usbd_rd_idx >= ICTRL_CDC_UPSTREAM_BUFF_SIZE) {
            us->usbd_rd_idx -= ICTRL_CDC_UPSTREAM_BUFF_SIZE;
        }
    }
}

void cdc_ictrl_dfi_us_rx_start (struct cdc_dfi_s *cdc_dfi)
{
	// ictrl_cdc_upstream_t *us = &cdc_dfi->ctx.cdc_ictrl->us;
    ictrl_cdc_upstream_t *us = &g_cdc_ictrl.us;

	us->usbd_rd_idx = 0;
	us->ictrl_wr_idx = 0;

	/* Statistics counters */
	us->stat_rx_bytes = 0;
	us->stat_tx_bytes = 0;

	/* Error counters */
	us->ictrl_err_cnt = 0;
	us->ictrl_ovfl_bytes = 0;
	us->ictrl_ovfl_cnt = 0;

	return;
}

void cdc_ictrl_dfi_us_rx_stop (struct cdc_dfi_s *cdc_dfi)
{
	return;
}

uint8_t *cdc_ictrl_dfi_ds_get_buff(struct cdc_dfi_s *cdc_dfi)
{
    // ictrl_cdc_downstream_t *ds = &cdc_dfi->ctx.cdc_ictrl->ds;
    ictrl_cdc_downstream_t *ds = &g_cdc_ictrl.ds;
    int wr_idx = ds->usbd_wr_idx;

    if (wr_idx >= CDC_ICTRL_DS_BUFF_SIZE) {
        // assert(0)
        wr_idx = 0;
    }

	return &ds->buff[wr_idx];
}

extern int g_dev0_dbg;

static void ictrl_on_command(const char *cmd, int len)
{
    if (len != 0 && cmd[0] == '\e') {
        return;
    }

    if (len == 0) {
        ictrl_print_out("\r\n", 2);
    } else if (0 == strncmp(cmd, "sign", len)) {
        ictrl_printf_nonisr("\r\nICTR V%d\r\n", ICTRL_REV);
    } else if (0 == strncmp(cmd, "help", len)) {
        ictrl_printf_nonisr("\r\nhelp me too...\r\n");
    } else if (0 == strncmp(cmd, "imonc", len)) {
        g_dev0_dbg = 1 - g_dev0_dbg;
        ictrl_printf_nonisr("\r\nIMON %s\r\n", g_dev0_dbg ? "DIS" : "EN");
    } else if (0 == strncmp(cmd, "imon", len)) {
        static int cnt = 0;
        imon_t *imon = &g_imon;
        ictrl_printf_nonisr("\r\n[%d] %dC, %dmV\r\n",
                cnt++, imon->temp_degc, imon->vref);
    } else if (0 == strncmp(cmd, "ledred", len)) {
        ictrl_printf_nonisr("\r\nRED LED Toggle\r\n");
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    } else if (0 == strncmp(cmd, "ledgreen", len)) {
        ictrl_printf_nonisr("\r\nGREEN LED Toggle\r\n");
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    } else {
        // Command not recognized
        ictrl_print_out("\r\n", 2);
    }
    ictrl_printf_nonisr("ictrl>");
}

void ds_on_idle()
{
    ictrl_cdc_downstream_t *ds = &g_cdc_ictrl.ds;

    if (ds->ictrl_rd_idx == ds->usbd_wr_idx) {
        if (!ds->rx_pending) {
            if (USBD_OK == USBD_CDC_ReceivePacket(ds->hcdc)) {
                ds->rx_pending = 1;
            }
        }
        return;
    }

    /* Some data received */
    // assert(ds->rx_pending == 0);

    if (ds->state == ICTRL_DS_STATE_SYNCING) {
        // Scan RX for sync
        int rd_idx = ds->ictrl_rd_idx;
        int sync_idx = -1;

        while(rd_idx != ds->usbd_wr_idx) {
            if (ds->buff[rd_idx++] == '\r') {
                sync_idx = rd_idx;
                break;
            }
        }
        if (sync_idx != -1) {
            // Sync found shift remaining data to the buffer's start
            int wr_idx = 0;
            while(rd_idx != ds->usbd_wr_idx) {
                ds->buff[wr_idx++]  = ds->buff[rd_idx++];
            }
            ds->usbd_wr_idx = wr_idx;
            ds->state = ICTRL_DS_STATE_RX;
            ictrl_printf_nonisr("\nictrl>");
        } else {
            // Sync not found - discard all data and get next packet
            ds->usbd_wr_idx = 0;
        }
    }

    if (ds->state == ICTRL_DS_STATE_RX) {
        int rd_idx = ds->ictrl_rd_idx;
        int cmd_idx = 0;
        int echo_idx = 0;

        while(rd_idx != ds->usbd_wr_idx) {
            if (ds->buff[rd_idx] == '\r' || ds->buff[rd_idx] == '\n') {
                // Command received. Make it zero terminated
                ds->buff[rd_idx] = 0;

                // Print out echo till the end of command
                echo_idx = (cmd_idx == 0) ? ds->ictrl_rd_idx : cmd_idx;
                ictrl_print_out((char*)&ds->buff[echo_idx], rd_idx - echo_idx);

                ictrl_on_command((char*)&ds->buff[cmd_idx], rd_idx - cmd_idx);
                cmd_idx = rd_idx + 1;
            }
            rd_idx++;
        }

        /*
         *  Echo back typed data starting from the end of last
         *  received command or, if no command were received from
         *  the start of received data
         */
        echo_idx = (cmd_idx == 0) ? ds->ictrl_rd_idx : cmd_idx;
        ictrl_print_out((char*)&ds->buff[echo_idx], ds->usbd_wr_idx - echo_idx);

        if (cmd_idx) {
            // One or more commands received. Shift remaining tail
            int wr_idx = 0;
            while(rd_idx != ds->usbd_wr_idx) {
                ds->buff[wr_idx++]  = ds->buff[rd_idx++];
            }
            ds->usbd_wr_idx = wr_idx;
        }
    }


    {  // If no more space, then clear sync and reset buffer indices
        int free_space = CDC_ICTRL_DS_BUFF_SIZE - ds->usbd_wr_idx - 1;
        if (free_space < CDC_DATA_OUT_PACKET_SIZE) {
            ds->state = ICTRL_DS_STATE_SYNCING;
            ds->usbd_wr_idx = 0;
        }
    }

    // Mark all newly received data as processed
    ds->ictrl_rd_idx = ds->usbd_wr_idx;

    if (USBD_OK == USBD_CDC_ReceivePacket(ds->hcdc)) {
        ds->rx_pending = 1;
    }

}

void  cdc_ictrl_dfi_on_idle (struct cdc_dfi_s *cdc_dfi)
{
    // ictrl_cdc_upstream_t *us = &cdc_dfi->ctx.cdc_ictrl->us;
    ictrl_cdc_upstream_t *us = &g_cdc_ictrl.us;
	int bytes_available;

	/* Send data upstream when available */
	bytes_available = ICTRL_CDC_UPSTREAM_AVAILABLE(us);
	if (bytes_available) {
		uint32_t now = HAL_GetTick();
		if ((now - us->last_tx_ts) > ICTRL_CDC_TX_TIMEOUT_MS ||
			bytes_available >= CDC_DATA_IN_PACKET_SIZE) {
			us->last_tx_ts = now;
			cdc_ictrl_upstream_send(us);
		}
	}

	ds_on_idle();

	return;
}

/* ISR context */
void cdc_ictrl_dfi_ds_on_control(struct cdc_dfi_s *cdc_dfi, uint8_t cmd, uint8_t* buf, uint16_t len)
{
	return;
#if 0
    // ictrl_cdc_upstream_t *us = &cdc_dfi->ctx.cdc_ictrl->us;
    ictrl_cdc_upstream_t *us = &g_cdc_ictrl.us;

	switch (cmd) {
	case CDC_SET_LINE_CODING:
	case CDC_GET_LINE_CODING:
	case CDC_SEND_ENCAPSULATED_COMMAND:
	case CDC_GET_ENCAPSULATED_RESPONSE:
	case CDC_SET_COMM_FEATURE:
	case CDC_GET_COMM_FEATURE:
	case CDC_CLEAR_COMM_FEATURE:
	case CDC_SET_CONTROL_LINE_STATE:
	case CDC_SEND_BREAK:
	default:
		break;
	}
#endif
	return;
}

void cdc_ictrl_dfi_ds_on_rx(struct cdc_dfi_s *cdc_dfi, uint32_t len)
{
    // ictrl_cdc_downstream_t *ds = &cdc_dfi->ctx.cdc_ictrl->ds;
    ictrl_cdc_downstream_t *ds = &g_cdc_ictrl.ds;

    ds->usbd_wr_idx += len;

    if (ds->usbd_wr_idx > CDC_ICTRL_DS_BUFF_SIZE) {
        // assert(0);
        ds->usbd_wr_idx = 0;
    }

    // assert(ds->rx_pending == 1);
    ds->rx_pending = 0;
	return;
}

void cdc_dfi_ictrl_init (cdc_dfi_t *cdc_dfi, cdc_ictrl_t *ictrl)
{
	cdc_dfi->ctx.cdc_ictrl = ictrl;       // Not in use, but let's initialize anyway

	cdc_dfi->on_idle 		= cdc_ictrl_dfi_on_idle;
	cdc_dfi->us_start_rx 	= cdc_ictrl_dfi_us_rx_start;
	cdc_dfi->us_stop_rx 	= cdc_ictrl_dfi_us_rx_stop;
	cdc_dfi->ds_get_buffer 	= cdc_ictrl_dfi_ds_get_buff;
	cdc_dfi->ds_on_control  = cdc_ictrl_dfi_ds_on_control;
	cdc_dfi->ds_on_rx 		= cdc_ictrl_dfi_ds_on_rx;
}

void ictrl_downstrem_init(ictrl_cdc_downstream_t *ds, USBD_CDC_Handle *hcdc)
{
    memset(ds, 0, sizeof(ictrl_cdc_downstream_t));
    ds->hcdc = hcdc;
}
void ictrl_upstream_init(ictrl_cdc_upstream_t *us, USBD_CDC_Handle *hcdc)
{
    memset(us, 0, sizeof(ictrl_cdc_upstream_t));
	us->hcdc = hcdc;
}

void cdc_ictrl_init(USBD_CDC_Handle *hcdc)
{
	cdc_ictrl_t *ictrl = &g_cdc_ictrl;

	cdc_dfi_ictrl_init(&ictrl->dfi, ictrl);

	ictrl_downstrem_init(&ictrl->ds, hcdc);
	ictrl_upstream_init(&ictrl->us, hcdc);

	hcdc->dfi = &ictrl->dfi;
}

/* Note: Care should be taken if global interrupts
 *       isn't always on.
 */

int ictrl_print_out(const char *in_buff, int in_buff_len)
{
    ictrl_cdc_upstream_t *us = &g_cdc_ictrl.us;

    int dst_free;
    int ovfl;
    int till_border;
    int ictrl_wr_idx;
    int bytes_to_wr1, bytes_to_wr2;

    /* Complete index operation ASAP and release interrupts */
    __disable_irq();
    ictrl_wr_idx = us->ictrl_wr_idx;
    dst_free = us->usbd_rd_idx - ictrl_wr_idx - 1;
    if (dst_free < 0) dst_free += ICTRL_CDC_UPSTREAM_BUFF_SIZE;

    /* Discard excessive data */
    ovfl = in_buff_len - dst_free;
    if (ovfl > 0) {
        in_buff_len -= ovfl;
    }

    us->ictrl_wr_idx += in_buff_len;
    if (us->ictrl_wr_idx >= ICTRL_CDC_UPSTREAM_BUFF_SIZE)
        us->ictrl_wr_idx -= ICTRL_CDC_UPSTREAM_BUFF_SIZE;
    __enable_irq();

    if (ovfl > 0) {
        us->ictrl_ovfl_bytes += ovfl;
        us->ictrl_ovfl_cnt ++;
    }

    /* Split write into 2 parts - till border and from begging */
    till_border = ICTRL_CDC_UPSTREAM_BUFF_SIZE - ictrl_wr_idx;

    bytes_to_wr1 = MIN(in_buff_len, till_border);
    memcpy(&us->buff[ictrl_wr_idx], in_buff, bytes_to_wr1);

    bytes_to_wr2 = in_buff_len - bytes_to_wr1;
    if (bytes_to_wr2) {
        memcpy(&us->buff[0], &in_buff[bytes_to_wr1], bytes_to_wr2);
    }

    us->stat_rx_bytes += in_buff_len;

    return in_buff_len;
}

/*
 * Note: Should not be called from ISR context due to global upstream buffer
 *       usage. If necessary prepare data inside an interrupt and call
 *       ictrl_print_out()
 */
int ictrl_printf_nonisr(const char *format, ...)
{
    // ictrl_cdc_upstream_t *us = &((cdc_ictrl_t*)cdc_dfi->ctx)->us;
    ictrl_cdc_upstream_t *us = &g_cdc_ictrl.us;

    va_list  va_args;
	int bytes_to_wr_tot;

    va_start(va_args, format);
    bytes_to_wr_tot = vsnprintf(us->tmp_str_buff, sizeof(us->tmp_str_buff), format, va_args);
    va_end(va_args);

	if (bytes_to_wr_tot < 0) {
		us->ictrl_err_cnt ++;
		return -1;
	}

	return ictrl_print_out(us->tmp_str_buff, bytes_to_wr_tot);
}
