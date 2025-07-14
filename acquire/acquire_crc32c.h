#ifndef LIBACQUIRE_ACQUIRE_CRC32C_H
#define LIBACQUIRE_ACQUIRE_CRC32C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_common_defs.h"

struct acquire_handle; /* Forward declaration */

#if defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C
int _crc32c_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash);
enum acquire_status _crc32c_verify_async_poll(struct acquire_handle *handle);
void _crc32c_verify_async_cancel(struct acquire_handle *handle);
#endif /* defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C */

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_CRC32C) &&    \
    LIBACQUIRE_USE_CRC32C

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "acquire_handle.h"
#include "acquire_string_extras.h"

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif /* !CHUNK_SIZE */

struct checksum_backend {
  FILE *file;
  uint32_t crc;
  char expected_hash[9];
};

static const uint32_t crc32c_table[256] = {
    0x00000000U, 0xF26B8303U, 0xE13B70F7U, 0x1350F3F4U, 0xC79A971FU,
    0x35F1141CU, 0x26A1E7E8U, 0xD4CA64EBU, 0x8AD958CFU, 0x78B2DBCCU,
    0x6BE22838U, 0x9989AB3BU, 0x4D43CFD0U, 0xBF284CD3U, 0xAC78BF27U,
    0x5E133C24U, 0x105EC76FU, 0xE235446CU, 0xF165B798U, 0x030E349BU,
    0xD7C45070U, 0x25AFD373U, 0x36FF2087U, 0xC494A384U, 0x9A879FA0U,
    0x68EC1CA3U, 0x7BBCEF57U, 0x89D76C54U, 0x5D1D08BFU, 0xAF768BBCU,
    0xBC267848U, 0x4E4DFB4BU, 0x20BD8EDEU, 0xD2D60DDDU, 0xC186FE29U,
    0x33ED7D2AU, 0xE72719C1U, 0x154C9AC2U, 0x061C6936U, 0xF477EA35U,
    0xAA64D611U, 0x580F5512U, 0x4B5FA6E6U, 0xB93425E5U, 0x6DFE410EU,
    0x9F95C20DU, 0x8CC531F9U, 0x7EAEB2FAU, 0x30E349B1U, 0xC288CAB2U,
    0xD1D83946U, 0x23B3BA45U, 0xF779DEAEU, 0x05125DADU, 0x1642AE59U,
    0xE4292D5AU, 0xBA3A117EU, 0x4851927DU, 0x5B016189U, 0xA96AE28AU,
    0x7DA08661U, 0x8FCB0562U, 0x9C9BF696U, 0x6EF07595U, 0x417B1DBCU,
    0xB3109EBFU, 0xA0406D4BU, 0x522BEE48U, 0x86E18AA3U, 0x748A09A0U,
    0x67DAFA54U, 0x95B17957U, 0xCBA24573U, 0x39C9C670U, 0x2A993584U,
    0xD8F2B687U, 0x0C38D26CU, 0xFE53516FU, 0xED03A29BU, 0x1F682198U,
    0x5125DAD3U, 0xA34E59D0U, 0xB01EAA24U, 0x42752927U, 0x96BF4DCCU,
    0x64D4CECFU, 0x77843D3BU, 0x85EFBE38U, 0xDBFC821CU, 0x2997011FU,
    0x3AC7F2EBU, 0xC8AC71E8U, 0x1C71F743U, 0xEE1A7440U, 0xFF4A87B4U,
    0x0D2104B7U, 0x79EB605CU, 0x8B80E35FU, 0x98D010ABU, 0x6ABBD3A8U,
    0x3E71B743U, 0xCC1A3440U, 0xDF4A67B4U, 0x2D21E4B7U, 0x59EBA05CU,
    0xAB80235FU, 0xB8D0D0ABU, 0x4ABE53A8U, 0x64DAB3E0U, 0x96B130E3U,
    0x85E16317U, 0x778AE014U, 0xF4F10ACFU, 0x069A89CCU, 0x15CA7A38U,
    0xE7A1F93BU, 0xB3E8D5D0U, 0x418356D3U, 0x52D3A527U, 0xA0B82624U,
    0x747242CFU, 0x8619C1CCU, 0x95493238U, 0x6722B13BU, 0xD4158D1FU,
    0x267E0E1CU, 0x352EFDE8U, 0xC7457EEBU, 0x138F1A00U, 0xE1E49903U,
    0xF2B46AF7U, 0x00DFEDF4U, 0x84427D3FU, 0x7629F43CU, 0x657907C8U,
    0x971284CBU, 0x43D8E020U, 0xB1B36323U, 0xA2E390D7U, 0x508813D4U,
    0xDB23E5AU,  0xEF496C59U, 0xFC199FAFU, 0x0E721CA8U, 0x8021308CU,
    0x724AB38FU, 0x611A407BU, 0x9371C378U, 0x47BBE793U, 0xB5D06490U,
    0xA6809764U, 0x54EA1467U, 0x2087CF2CU, 0xD2E84C2FU, 0xC1B8BFDBU,
    0x33D33CD8U, 0xF94F00B3U, 0x0B2483B0U, 0x18747044U, 0xEA1FF347U};

static uint32_t crc32c_init(void) { return 0xFFFFFFFFU; }
static uint32_t crc32c_update(uint32_t crc, const unsigned char *data,
                              size_t len) {
  size_t i;
  for (i = 0; i < len; ++i) {
    uint8_t idx = (uint8_t)((crc ^ data[i]) & 0xFFU);
    crc = crc32c_table[idx] ^ (crc >> 8);
  }
  return crc;
}
static uint32_t crc32c_finalize(uint32_t crc) { return crc ^ 0xFFFFFFFFU; }

static void cleanup_crc32c_backend(struct acquire_handle *handle) {
  struct checksum_backend *be;
  if (!handle || !handle->backend_handle)
    return;
  be = (struct checksum_backend *)handle->backend_handle;
  if (be->file)
    fclose(be->file);
  free(be);
  handle->backend_handle = NULL;
}

int _crc32c_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash) {
  struct checksum_backend *be;
  if (algorithm != LIBACQUIRE_CRC32C)
    return -1;
  if (strlen(expected_hash) != 8) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
                             "Invalid hash length for CRC32C");
    return -1;
  }
  if (!handle || !filepath || !expected_hash) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments");
    return -1;
  }
  be = (struct checksum_backend *)calloc(1, sizeof(struct checksum_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "Out of memory");
    return -1;
  }
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    const errno_t err = fopen_s(&be->file, filepath, "rb");
    if (err != 0 || be->file == NULL) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                               "Cannot open file: %s", filepath);
      free(be);
      return -1;
    }
  }
#else
  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             "Cannot open file: %s", strerror(errno));
    free(be);
    return -1;
  }
#endif
  be->crc = crc32c_init();
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strncpy_s(be->expected_hash, sizeof be->expected_hash, expected_hash, 8);
#else
  strncpy(be->expected_hash, expected_hash, 8);
#endif
  be->expected_hash[8] = '\0';
  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _crc32c_verify_async_poll(struct acquire_handle *handle) {
  struct checksum_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Operation cancelled");
    cleanup_crc32c_backend(handle);
    return ACQUIRE_ERROR;
  }
  be = (struct checksum_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);
  if (bytes_read > 0) {
    be->crc = crc32c_update(be->crc, buffer, bytes_read);
    handle->bytes_processed += (off_t)bytes_read;
    return ACQUIRE_IN_PROGRESS;
  }
  if (ferror(be->file)) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    char error_code[256];
    strerror_s(error_code, sizeof(error_code), errno);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED,
                             "File read error: %s", error_code);
#else
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED,
                             "File read error: %s", strerror(errno));
#endif
  } else {
    char computed_hex[9];
    uint32_t final_crc = crc32c_finalize(be->crc);
    snprintf(computed_hex, sizeof(computed_hex), "%08x", final_crc);
    if (strncasecmp(computed_hex, be->expected_hash, 8) == 0) {
      handle->status = ACQUIRE_COMPLETE;
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "CRC32C mismatch: expected %s, got %s",
                               be->expected_hash, computed_hex);
    }
  }
  cleanup_crc32c_backend(handle);
  return handle->status;
}

void _crc32c_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_CRC32C) \
          && LIBACQUIRE_USE_CRC32C */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_CRC32C_H */
