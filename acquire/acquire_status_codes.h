#ifndef ACQUIRE_STATUS_CODES_H
#define ACQUIRE_STATUS_CODES_H

/**
 * @file acquire_status_codes.h
 * @brief Defines all status and error codes for libacquire operations.
 */

/**
 * @brief Describes the current state of an asynchronous operation.
 */
enum acquire_status {
  ACQUIRE_IDLE = 32,   /* The handle is ready but no operation is active. */
  ACQUIRE_IN_PROGRESS, /* The operation is running. */
  ACQUIRE_COMPLETE,    /* The operation finished successfully. */
  ACQUIRE_ERROR        /* The operation failed. See error details in handle. */
};

/**
 * @brief Defines numeric error codes for all libacquire operations.
 * A value of ACQUIRE_OK indicates success.
 */
enum acquire_error_code {
  ACQUIRE_OK = 0,
  ACQUIRE_ERROR_UNKNOWN = 1,
  ACQUIRE_ERROR_OUT_OF_MEMORY = 2,
  ACQUIRE_ERROR_CANCELLED = 3,
  ACQUIRE_ERROR_INVALID_ARGUMENT = 4,
  ACQUIRE_ERROR_FILE_OPEN_FAILED = 100,
  ACQUIRE_ERROR_FILE_WRITE_FAILED = 101,
  ACQUIRE_ERROR_FILE_READ_FAILED = 102,
  ACQUIRE_ERROR_DIRECTORY_CREATE_FAILED = 103,
  ACQUIRE_ERROR_NETWORK_INIT_FAILED = 200,
  ACQUIRE_ERROR_URL_PARSE_FAILED = 201,
  ACQUIRE_ERROR_HOST_NOT_FOUND = 202,
  ACQUIRE_ERROR_NETWORK_FAILURE = 203,
  ACQUIRE_ERROR_HTTP_FAILURE = 204,
  ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED = 300,
  ACQUIRE_ERROR_ARCHIVE_READ_HEADER_FAILED = 301,
  ACQUIRE_ERROR_ARCHIVE_EXTRACT_FAILED = 302,
  ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT = 303
};

#endif /* !ACQUIRE_STATUS_CODES_H */
