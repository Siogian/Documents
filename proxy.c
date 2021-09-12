#include "proxy.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define MAX_PENDING_CNT 6000

static uint16_t g_pending = 0;
static uint16_t g_did = 0;
static uint8_t g_confirm = 0;

#define REPLY_RSTP  (uint8_t)(1 << 0)
#define REPLY_EVENT (uint8_t)(1 << 1)

typedef bool (*ReadData)(uint8_t *data, uint16_t dataLen);

typedef struct {
    uint16_t did;
    uint8_t mask;
    uint8_t taskId;
    ReadData readData;
} ReplyConfig;


static const ReplyConfig g_replyConfig[] = {
    { 0x1003, REPLY_RSTP | REPLY_EVENT, 1, NULL },
    { 0x1004, REPLY_RSTP | REPLY_EVENT, 2, NULL }
};

static const ReplyConfig * FindReplyConfig(uint16_t did)
{
    for (uint16_t i = 0; i < ARRAY_SIZE(g_replyConfig); ++i) {
        if (did == g_replyConfig[i].did) {
            return &(g_replyConfig[i]);
        }
    }
    return NULL;
}

static bool RstpRequest(void);
static bool EventRequest(void);

static bool MesConfirm(void);
static bool RstpConfirm(void);
static bool EventConfirm(void);

bool MesRequest(uint16_t did)
{
    const ReplyConfig * const replyConfig = FindReplyConfig(did);
    if (replyConfig == NULL) {
        printf("%s: did %x is not supported", __FUNCTION__, did);
        return false;
    }
    g_did = did;
    g_confirm = 0;
    g_pending = 0;
    if ((REPLY_RSTP & replyConfig->mask) != 0) {
        if (!RstpRequest()) {
            return false;
        }
    }
    if ((REPLY_EVENT & replyConfig->mask) != 0) {
        if (!EventRequest()) {
            return false;
        }
    }
    return true;
}

/* 所有的请求都已经收到响应 */
static bool MesConfirm(void)
{
    const ReplyConfig * const replyConfig = FindReplyConfig(g_did);
    if (replyConfig == NULL) {
        printf("%s: did %x is not supported", __FUNCTION__, g_did);
        return false;
    }
    if ((REPLY_RSTP & replyConfig->mask) != 0) {
        if (!RstpConfirm()) {
            return false;
        }
    }
    if ((REPLY_EVENT & replyConfig->mask) != 0) {
        if (!EventConfirm()) {
            return false;
        }
    }
    return true;
}

static bool RstpConfirm(void)
{
    return (g_confirm & REPLY_RSTP) == 1;
}

static bool EventConfirm(void)
{
    return (g_confirm & REPLY_EVENT) == 1;
}

bool MesPending(void)
{
    if (g_pending > MAX_PENDING_CNT) {
        return false;
    }
    ++g_pending;
    return true;
}

Std_ReturnType MesReadData(uint16_t did, uint8_t *data, uint16_t dataLen)
{
    const ReplyConfig * const replyConfig = FindReplyConfig(did);
    if (replyConfig == NULL) {
        printf("%s: did %x is not supported", __FUNCTION__, did);
        return E_NOT_OK;
    }
    if (!MesConfirm()) {
        if (!MesPending()) {
            return E_NOT_OK;
        }
        return DCM_PENDING;
    }
    if (replyConfig->readData == NULL) {
        return E_NOT_OK;
    }
    if (!replyConfig->readData(data, dataLen)) {
        return E_NOT_OK;
    }
    return E_OK;
}

static bool RstpRequest(void)
{
    return false;
}
static bool EventRequest(void)
{
    return false;
}