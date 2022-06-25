#pragma once

#include "usbd_def.h"

#define CDC_BINTERVAL                  0x10U

/* CDC 1.20 */
#define CDC_DESC_TYPE_INTERFACE        0x24

#define CDC_DESC_SUB_TYPE_HDR          0        /* Header Functional Descriptor, which marks the beginning of the concatenated set of functional descriptors for the interface. */
#define CDC_DESC_SUB_TYPE_CALL_MNG     1        /* Call Management */
#define CDC_DESC_SUB_TYPE_ACM          2        /* Abstract Control Management */
#define CDC_DESC_SUB_TYPE_DLM          3        /* Direct Line Management */
#define CDC_DESC_SUB_TYPE_RINGER       4        /* Telephone Ringer */
#define CDC_DESC_SUB_TYPE_LSTATE       5        /* Telephone Call and Line State Reporting Capabilities */
#define CDC_DESC_SUB_TYPE_UNION        6

#define CDC_DESC_TYPE_ENDPOINT         0x25

#define CDC_DATA_MAX_PACKET_SIZE       64U  /* Endpoint IN & OUT Packet size */

#define CDC_CMD_PACKET_SIZE            8U  /* Control Endpoint Packet size */

#define CDC_DATA_IN_PACKET_SIZE        CDC_DATA_MAX_PACKET_SIZE
#define CDC_DATA_OUT_PACKET_SIZE       CDC_DATA_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#define CDC_SEND_ENCAPSULATED_COMMAND   0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE   0x01U
#define CDC_SET_COMM_FEATURE            0x02U
#define CDC_GET_COMM_FEATURE            0x03U
#define CDC_CLEAR_COMM_FEATURE          0x04U
#define CDC_SET_LINE_CODING             0x20U
#define CDC_GET_LINE_CODING             0x21U
#define CDC_SET_CONTROL_LINE_STATE      0x22U
#define CDC_SEND_BREAK                  0x23U

#pragma pack(push, 1)
 /*******************************************************************************/
 /* Line Coding Structure  PSTN120.pdf Table 17                                 */
 /*******************************************************************************/
typedef struct pstn_line_coding_s {
    uint32_t dwDTERate;     /* Data terminal rate, in bits per second */
    uint8_t bCharFormat;    /* Stop bits: 0 -> 1; 1 -> 1.5; 2 -> 2; */
    uint8_t bParityType;    /* Parity 0-None; 1-Odd; 2-Even; 3-Mark; 4-Space */
    uint8_t bDataBits;      /* Number Data bits (5, 6, 7, 8 or 16) */
} pstn_line_coding_t;
#pragma pack(pop)

typedef struct
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
}USBD_CDC_LineCodingTypeDef;

/* CDC down faced interface - DFI */
typedef struct cdc_dfi_s {
    union {
        struct cdc_ictrl_s *cdc_ictrl;
        struct cdc_uart_s *cdc_uart;
    } ctx;

    void (*on_idle) (struct cdc_dfi_s *dfi);
    void (*us_start_rx) (struct cdc_dfi_s *dfi);
    void (*us_stop_rx) (struct cdc_dfi_s *dfi);
    void (*ds_on_control) (struct cdc_dfi_s *dfi, uint8_t cmd, uint8_t* buf, uint16_t len);
    void (*ds_on_rx) (struct cdc_dfi_s *dfi, uint32_t len);
    uint8_t *(*ds_get_buffer) (struct cdc_dfi_s *dfi);
} cdc_dfi_t;

#pragma pack(push, 1)
typedef struct _USBD_CDC_ConfigDesc {
    USBD_IADDesc           if_assoc_desc;
    USBD_InterfaceDesc     interface_desc_cmd;
    USBD_FuncDescHdr       func_desc_header;
    USBD_FuncDescCallMng   func_desc_call_mng;
    USBD_FuncDescACM       func_desc_acm;
    USBD_FuncDescUnion     func_desc_union;
    USBD_EpDesc            cmd_ep;
    USBD_InterfaceDesc     interface_desc_data;
    USBD_EpDesc            data_ep_out;
    USBD_EpDesc            data_ep_in;
} USBD_CDC_ConfigDesc;
#pragma pack(pop)

typedef struct _USBD_CDC_Handle
{
    /* Initialized @ USBD_Composite_Init -> USBD_CDC_Init */
    uint8_t ifnum_cmd;
    uint8_t epnum_cmd;
    uint8_t ifnum_data;
    uint8_t epnum_data;
    struct _USBD_Handle *pdev;

    /* Initialized @ cdc_uart_init */
    cdc_dfi_t *dfi;

    uint8_t  data[CDC_DATA_MAX_PACKET_SIZE] __attribute__ ((aligned (4)));
    uint8_t  CmdOpCode;
    uint8_t  CmdLength;

    __IO uint32_t TxState;
    __IO uint32_t RxState;

    USBD_CDC_ConfigDesc    *cfg_desc;

} USBD_CDC_Handle;

uint8_t USBD_CDC_ReceivePacket    (USBD_CDC_Handle *hcdc);
uint8_t USBD_CDC_TransmitPacket    (USBD_CDC_Handle *hcdc, uint8_t  *buf, uint16_t len);

void CDC_Register(USBD_CDC_Handle *hcdc, usbd_intf_t *intf, USBD_ConfigDesc *config_desc, int *ifnum, int *epnum);


