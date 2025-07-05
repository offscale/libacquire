libacquire Usage Guide
======================

`libacquire` provides a flexible, handle-based API for downloading, verifying, and extracting files. This guide demonstrates how to use the core features of the library.

## Core Concepts

### The `acquire_handle`

The `struct acquire_handle` is the heart of the library. It's an opaque state container that you create once per logical operation. It holds internal state for downloads, checksums, and extractions, and most importantly, it's how you retrieve progress and detailed error information.

- **Create a handle:** `struct acquire_handle *handle = acquire_handle_init();`
- **Clean up a handle:** `acquire_handle_free(handle);`

You should reuse the same handle for a chained operation (e.g., download -> verify -> extract).

### API Pattern: Synchronous vs. Asynchronous

Every major feature offers two ways to operate:

1.  **Synchronous (`_sync`)**: These are simple, blocking functions. They are easy to use but will freeze your application until the operation is complete.
    ```c
    int result = acquire_feature_sync(handle, ...);
    ```

2.  **Asynchronous (`_async_...`)**: These non-blocking functions allow your application to remain responsive. The pattern is always:
    - `acquire_feature_async_start()`: Kicks off the operation.
    - `acquire_feature_async_poll()`: Called in a loop to perform a piece of work and check the status.
    - `acquire_feature_async_cancel()`: Can be called from another thread or signal handler to request cancellation.

### Error Handling

When a function returns `-1` (failure), the `acquire_handle` contains the reason.

```c
if (result != 0) {
    fprintf(stderr, "Operation failed: [%d] %s\n",
            acquire_handle_get_error_code(handle),
            acquire_handle_get_error_string(handle));
}
```

---

## 0. Downloading a File

### a) Synchronous Download

This is the simplest way to download a file. The function blocks until the download is complete or an error occurs.

```c
#include <stdio.h>
#include <acquire_handle.h>
#include <acquire_download.h>

void simple_blocking_download(void) {
    struct acquire_handle *handle = acquire_handle_init();
    const char *url = "https://httpbin.org/get";
    const char *destination = "./httpbin.json";
    int result;

    printf("Starting synchronous download of '%s'...\n", url);
    
    result = acquire_download_sync(handle, url, destination);

    if (result == 0) {
        printf("Download successful! Saved to '%s'.\n", destination);
    } else {
        fprintf(stderr, "Download failed: %s\n",
                acquire_handle_get_error_string(handle));
    }

    acquire_handle_free(handle);
}
```

### b) Asynchronous Download with Progress

This example shows a non-blocking download with a progress indicator, perfect for GUI applications or services that need to remain responsive.

```c
#include <stdio.h>

#include <acquire_handle.h>
#include <acquire_download.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <unistd.h>
#endif

void non_blocking_download_with_progress(void) {
    struct acquire_handle *handle = acquire_handle_init();
    const char *url = "http://ipv4.download.thinkbroadband.com/5MB.zip";
    const char *destination = "./5MB.zip";
    enum acquire_status status;

    puts("Starting asynchronous download of a 5MB file...");
    if (acquire_download_async_start(handle, url, destination) != 0) {
        fprintf(stderr, "Failed to start async download: %s\n",
                acquire_handle_get_error_string(handle));
        acquire_handle_free(handle);
        return;
    }

    puts("Download in progress. Polling for updates...");
    do {
        status = acquire_download_async_poll(handle);

        if (handle->total_size > 0) {
            double percent = 100.0 * handle->bytes_processed / handle->total_size;
            printf("\rProgress: %lld / %lld bytes (%.0f%%)",
                   (long long)handle->bytes_processed, (long long)handle->total_size, percent);
        } else {
            printf("\rProgress: %lld bytes downloaded", (long long)handle->bytes_processed);
        }
        fflush(stdout);
        
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        Sleep(100); /* 100ms */
#else
        usleep(100000); /* 100ms */
#endif
    } while (status == ACQUIRE_IN_PROGRESS);

    putchar('\n');

    switch (status) {
        case ACQUIRE_COMPLETE:
            puts("Async download complete!");
            break;
        case ACQUIRE_ERROR:
            fprintf(stderr, "Async download failed: [%d] %s\n",
                    acquire_handle_get_error_code(handle),
                    acquire_handle_get_error_string(handle));
            break;
        default:
            fprintf(stderr, "Download finished with unexpected status: %d\n", status);
            break;
    }
    
    acquire_handle_free(handle);
}
```

---

## 1. Verifying a File Checksum

Verification follows the same sync/async pattern. This is useful for ensuring the integrity of a downloaded file.

### a) Synchronous Verification

```c
#include <stdio.h>
#include <acquire_handle.h>
#include <acquire_checksums.h>

void verify_file_integrity(const char *filepath, const char *expected_hash) {
    struct acquire_handle *handle = acquire_handle_init();
    int result;

    printf("Verifying SHA256 of '%s'...\n", filepath);
    
    result = acquire_verify_sync(handle, filepath, LIBACQUIRE_SHA256, expected_hash);

    if (result == 0) {
        puts("Verification successful: Checksum matches.");
    } else {
        fprintf(stderr, "Verification failed: %s\n",
                acquire_handle_get_error_string(handle));
    }

    acquire_handle_free(handle);
}
```

---

## 2. Extracting an Archive

### a) Synchronous Extraction

```c
#include <stdio.h>

#include <acquire_handle.h>
#include <acquire_extract.h>

void extract_an_archive(const char *archive_path, const char *destination_dir) {
    struct acquire_handle *handle = acquire_handle_init();
    int result;

    printf("Extracting '%s' to '%s'...\n", archive_path, destination_dir);

    result = acquire_extract_sync(handle, archive_path, destination_dir);

    if (result == 0) {
        printf("Extraction successful.\n");
    } else {
        fprintf(stderr, "Extraction failed: %s\n",
                acquire_handle_get_error_string(handle));
    }

    acquire_handle_free(handle);
}
```

### b) Asynchronous Extraction

This is useful for showing progress as each file is extracted from a large archive.

```c
#include <stdio.h>

#include <acquire_handle.h>
#include <acquire_extract.h>

/* For usleep/Sleep */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <unistd.h>
#endif

void non_blocking_extraction(const char *archive_path, const char *destination_dir) {
    struct acquire_handle *handle = acquire_handle_init();
    enum acquire_status status;

    printf("Starting asynchronous extraction...\n");
    if (acquire_extract_async_start(handle, archive_path, destination_dir) != 0) {
        fprintf(stderr, "Failed to start async extraction: %s\n",
                acquire_handle_get_error_string(handle));
        acquire_handle_free(handle);
        return;
    }

    do {
        status = acquire_extract_async_poll(handle);
        if (status == ACQUIRE_IN_PROGRESS) {
            printf("Extracting: %s\n", handle->current_file);
            #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(100); /* 100ms */
#else
    usleep(100000); /* 100ms */
#endif
        }
    } while (status == ACQUIRE_IN_PROGRESS);
    
    if (status == ACQUIRE_COMPLETE) {
        printf("Extraction complete.\n");
    } else {
        fprintf(stderr, "Extraction failed: [%d] %s\n",
            acquire_handle_get_error_code(handle),
            acquire_handle_get_error_string(handle));
    }

    acquire_handle_free(handle);
}

```

---

## 3. Putting It All Together: A Complete Workflow

This example shows the power of the unified handle API by performing a complete download, verify, and extract sequence for a Node.js headers package.

```c
#include <stdio.h>
#include <stdlib.h>

#include <acquire_handle.h>
#include <acquire_download.h>
#include <acquire_checksums.h>
#include <acquire_extract.h>

int download_verify_and_extract(void) {
    /* Define our target and its properties */
    const char *const url = "https://nodejs.org/dist/v18.17.1/node-v18.17.1-headers.tar.gz";
    const char *const expected_sha256 = "b6a7a13d2f9758783a3c942d5940e4f26484a95697d8b5847b3b3a7f64243a75";
    const char *const download_path = "./node-headers.tar.gz";
    const char *const extract_dir = "./node-headers-extracted";

    int result;
    
    /* 1. Create a single handle for the entire operation. */
    struct acquire_handle *handle = acquire_handle_init();
    if (!handle) {
        fprintf(stderr, "Failed to initialize acquire handle.\n");
        return 1;
    }

    /* 2. Download the file synchronously. */
    printf("--- Step 1: Downloading '%s' ---\n", url);
    result = acquire_download_sync(handle, url, download_path);
    if (result != 0) {
        fprintf(stderr, "Download failed: %s\n", acquire_handle_get_error_string(handle));
        acquire_handle_free(handle);
        return 1;
    }
    printf("Download complete.\n\n");

    /* 3. Verify the checksum (reusing the same handle). */
    printf("--- Step 2: Verifying checksum ---\n");
    result = acquire_verify_sync(handle, download_path, LIBACQUIRE_SHA256, expected_sha256);
    if (result != 0) {
        fprintf(stderr, "Verification failed: %s\n", acquire_handle_get_error_string(handle));
        acquire_handle_free(handle);
        return 1;
    }
    printf("Verification successful.\n\n");
    
    /* 4. Extract the archive (reusing the same handle). */
    printf("--- Step 3: Extracting archive ---\n");
    result = acquire_extract_sync(handle, download_path, extract_dir);
    if (result != 0) {
        fprintf(stderr, "Extraction failed: %s\n", acquire_handle_get_error_string(handle));
        acquire_handle_free(handle);
        return 1;
    }
    printf("Extraction complete.\n\n");

    /* 5. Clean up. */
    acquire_handle_free(handle);
    printf("--- Workflow successful! ---\n");
    return 0;
}

int main(void) {
    if (download_verify_and_extract() != 0) {
        fputs("The operation failed.", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```
