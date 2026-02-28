/*
 * error_handling.c - Error handling utilities for VMManager
 */

#include "../include/vmmanager.h"

/* ── Error Code to String ───────────────────────────────────── */
const char *error_code_str(ErrorCode code) {
    switch (code) {
        case ERR_NONE:               return "No Error";
        case ERR_CONNECTION_FAILED:  return "Connection Failed";
        case ERR_PERMISSION_DENIED:  return "Permission Denied";
        case ERR_NOT_FOUND:          return "Not Found";
        case ERR_ALREADY_EXISTS:     return "Already Exists";
        case ERR_INVALID_INPUT:      return "Invalid Input";
        case ERR_OPERATION_FAILED:   return "Operation Failed";
        case ERR_TIMEOUT:            return "Timeout";
        case ERR_OUT_OF_MEMORY:      return "Out of Memory";
        case ERR_SERVICE_UNAVAILABLE: return "Service Unavailable";
        default:                     return "Unknown Error";
    }
}

/* ── Set Error ──────────────────────────────────────────────── */
void error_set(AppError *err, ErrorCode code, const char *msg, const char *suggestion) {
    if (!err) return;
    
    err->code = code;
    err->recoverable = (code != ERR_OUT_OF_MEMORY);
    
    if (msg) {
        strncpy(err->message, msg, sizeof(err->message) - 1);
        err->message[sizeof(err->message) - 1] = '\0';
    } else {
        snprintf(err->message, sizeof(err->message), "%s", error_code_str(code));
    }
    
    if (suggestion) {
        strncpy(err->suggestion, suggestion, sizeof(err->suggestion) - 1);
        err->suggestion[sizeof(err->suggestion) - 1] = '\0';
    } else {
        /* Default suggestions based on error code */
        switch (code) {
            case ERR_CONNECTION_FAILED:
                strncpy(err->suggestion, "Check if the service is running and try again.",
                        sizeof(err->suggestion) - 1);
                break;
            case ERR_PERMISSION_DENIED:
                strncpy(err->suggestion, "Run with appropriate permissions or check user groups.",
                        sizeof(err->suggestion) - 1);
                break;
            case ERR_NOT_FOUND:
                strncpy(err->suggestion, "The requested resource does not exist.",
                        sizeof(err->suggestion) - 1);
                break;
            case ERR_SERVICE_UNAVAILABLE:
                strncpy(err->suggestion, "Ensure the required service is installed and running.",
                        sizeof(err->suggestion) - 1);
                break;
            default:
                err->suggestion[0] = '\0';
                break;
        }
    }
}

/* ── Clear Error ────────────────────────────────────────────── */
void error_clear(AppError *err) {
    if (!err) return;
    memset(err, 0, sizeof(AppError));
    err->code = ERR_NONE;
}

/* ── Get libvirt error message ──────────────────────────────── */
const char *get_libvirt_error(virConnectPtr conn) {
    virErrorPtr err = virGetLastError();
    if (err && err->message) {
        return err->message;
    }
    return "Unknown libvirt error";
}

/* ── Map libvirt error to our error codes ───────────────────── */
ErrorCode map_libvirt_error(int vir_error_code) {
    switch (vir_error_code) {
        case VIR_ERR_NO_DOMAIN:
            return ERR_NOT_FOUND;
        case VIR_ERR_OPERATION_INVALID:
            return ERR_INVALID_INPUT;
        case VIR_ERR_OPERATION_DENIED:
            return ERR_PERMISSION_DENIED;
        case VIR_ERR_NO_CONNECT:
        case VIR_ERR_INVALID_CONN:
            return ERR_CONNECTION_FAILED;
        default:
            return ERR_OPERATION_FAILED;
    }
}
