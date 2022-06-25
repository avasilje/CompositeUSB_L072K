#pragma once

#define ICTRL_CDC_UPSTREAM_BUFF_SIZE 512

typedef struct ictrl_cdc_upstream_s {

	USBD_CDC_Handle   *hcdc;

	char tmp_str_buff[ICTRL_CDC_UPSTREAM_BUFF_SIZE];		/* Temporary buffer for printf. Must be less or equal to buff */

	uint8_t buff[ICTRL_CDC_UPSTREAM_BUFF_SIZE];
	int ictrl_wr_idx;
	int usbd_rd_idx;
	uint32_t last_tx_ts;

	/* Statistics counters */
	uint32_t stat_rx_bytes;
	uint32_t stat_tx_bytes;

	/* Error counters */
	uint32_t ictrl_err_cnt;
	uint32_t ictrl_ovfl_cnt;
	uint32_t ictrl_ovfl_bytes;

} ictrl_cdc_upstream_t;

#define CDC_ICTRL_DS_BUFF_SIZE         256U

enum ictrl_ds_state {
    ICTRL_DS_STATE_SYNCING,
    ICTRL_DS_STATE_RX
};
typedef struct ictrl_cdc_downstream_s {
    USBD_CDC_Handle   *hcdc;
    int rx_pending;
    enum ictrl_ds_state state;
    int usbd_wr_idx;
    int ictrl_rd_idx;
    uint8_t buff[CDC_ICTRL_DS_BUFF_SIZE];
} ictrl_cdc_downstream_t;

typedef struct cdc_ictrl_s {
	ictrl_cdc_upstream_t	us;
	ictrl_cdc_downstream_t	ds;
	cdc_dfi_t dfi;
} cdc_ictrl_t;

/* There is no reason to keep multiple ictrl instances, thus
 * don't pass its context to functions, but get the context
 * as a global variable instead */
extern void cdc_ictrl_init(USBD_CDC_Handle *hcdc);
extern int ictrl_printf_nonisr(const char *format, ...);
extern int ictrl_print_out(const char *in_buff, int in_buff_len);
