//
//  http_helpers.c
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/19/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//
#include "http_helpers.h"

// From http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
char *status_code_to_desc(int status_code) {
    switch (status_code) {
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocols";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 305:
            return "Use Proxy";
        case 307:
            return "Temporary Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Request Entity Too Large";
        case 414:
            return "Request-URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        default:
            return "Internal Server Error"; // Default to 500
    }
}

char *mime_type_to_string(MIME_TYPES type) {
    switch (type) {
        case TEXT_HTML:
            return "text/html";
        case TEXT_CSS:
            return "text/css";
        case TEXT_PLAIN:
            return "text/plain";
        case IMAGE_JPEG:
            return "image/jpeg";
        case IMAGE_PNG:
            return "image/png";
        case AUDIO_MPEG:
            return "audio/mpeg";
    }
    return "text/plain"; // should never be reached really
}

char *construct_response(int response_code, char *response_code_string, MIME_TYPES type, int keep_alive, void *content, long content_length, long *response_size_ptr) {
    char status_line[200];
    sprintf(status_line, "HTTP/1.1 %d %s\r\n", response_code, response_code_string);
    char content_type[200];
    sprintf(content_type, "Content-Type: %s\r\n", mime_type_to_string(type));
    char content_length_str[200];
    sprintf(content_length_str, "Content-Length: %zu\r\n", content_length);
    char connection[200];
    sprintf(connection, "Connection: %s\r\n\r\n", keep_alive ? "Keep-Alive" : "close");
    
    size_t response_size = content_length + strlen(status_line) + strlen(content_type) + strlen(content_length_str) + strlen(connection);
    
    char *response = (char *) malloc( (response_size + 1) * sizeof(char)); // plus one for null terminating char
    
    *response_size_ptr = response_size;
    
    sprintf(response, "%s%s%s%s", status_line, content_type, content_length_str, connection); // Just concats all the strings
    
    size_t content_start_idx = response_size - content_length; // Index of null terminating character placed by sprintf(...)
    
    memcpy(response + content_start_idx, content, content_length); // Sets content part of response
    
    return response;
}

// reads file into buffer that is stored on the heap (client must free() it)
// file ptr param cannot be null. Will seg fault otherwise
void *read_file(FILE *file, long *buffer_size_ptr) {
    // Init buffer values
    void *buffer = NULL;
    long buffer_size = 0;
    // Find size of file to read
    fseek(file, 0, SEEK_END);
    buffer_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    *buffer_size_ptr = buffer_size;
    // Allocate appropriate space for buffer
    buffer = malloc(buffer_size);
    // Read file contents into buffer all at once
    fread( buffer, 1, buffer_size, file);
    // Close file stream
    fclose(file);
    // Return heap allocated buffer containing file contents
    return buffer;
}