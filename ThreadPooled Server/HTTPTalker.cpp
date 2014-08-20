//
//  HTTPParser.cpp
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/14/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#define MAX_SIZE 4096

/* Allows up to 10 MB requests */
#define MAX_BODY_LEN (10 * 1024 * 1024)

#include "HTTPTalker.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

HTTPTalker::HTTPTalker(int connfd):
connfd(connfd)
{
    this->content = nullptr;
    this->content_length = 0;
    this->request_line = "";
    this->to_keep_alive = false;
}

void HTTPTalker::reset_values() {
    this->header_values.clear();
    this->content = nullptr;
    this->content_length = 0;
    this->request_line = "";
    this->to_keep_alive = false;
}

bool HTTPTalker::keep_alive() const {
    return this->to_keep_alive;
}

/*
 * params:
 *  header: char pointer pointing to start of header that is terminated with \0 where it should be terminated by \r\n\r\n
 * Responsible for parsing header values into internal data structure
 * Also responsible for setting this->content_length and this->to_keep_alive appropriately
 */
void HTTPTalker::parse_header(char *header) {
    printf("HTTPTalker::parse_header() %d{ message: \"%s\" }\n", 106, "called parse_header()");
    
    // First, parse request line
    char *request_line_end = strstr(header, "\r\n");
    if (request_line_end == NULL) {
        // Happens if request line is the only line in the header
        this->request_line = header;
        this->content_length = 0; // This function is responsible for setting content_length
        return;
    }
    *request_line_end = '\0'; // replace \r with a \0 to terminate it
    this->request_line = header; // Will do a deep copy of first line in header
    
    // Now, parse header key-values
    // Will keep incrementing header as we parse through line by line
    header = request_line_end + 2; // Point to new header line
    char *line_end = NULL;
    while ( (line_end = strstr(header, "\r\n")) != NULL ) {
        *line_end = '\0'; // terminate at \r
        // Now header points to a single key-value line in the header
        char *colon = strchr(header, ':'); // Find colon that separate key-value
        *colon = '\0';
        
        string key   = header;
        string value = (colon + 1); // (colon + 1) points to start of value in key-value pair
        
        std::pair<string, string> kv_pair = {key, value};
        this->header_values.insert(kv_pair);
        
        header = line_end + 2; // Point to start of next line
    }
    
    // Above loop only parses through first n-1 key-value lines, where n is total number of key-value lines
    // Must parse last line now
    char *colon = strchr(header, ':'); // Find colon that separate key-value
    *colon = '\0';
    
    string key   = header;
    string value = (colon + 1); // (colon + 1) points to start of value in key-value pair
    
    std::pair<string, string> kv_pair = {key, value};
    this->header_values.insert(kv_pair);
    
    // Set content_length appropriately
    string content_length_str = this->header_values["Content-Length"];
    
    // May not have been given a Content-Length, in which case set length value to 0
    if (content_length_str.length() > 0)
        this->content_length = stoi(content_length_str);
    else
        this->content_length = 0;
    
    string keep_alive_str = this->header_values["Connection"];
    
    if (keep_alive_str.length() > 0)
        this->to_keep_alive = (keep_alive_str == "keep-alive");
    else
        this->to_keep_alive = false; // If Connection not given, set to_keep_alive to false by default
    
    printf("HTTPTalker::parse_header() %d{ message: \"%s\" }\n", 158, "finished with parse_header()");
}

bool HTTPTalker::receive_request() {
    printf("HTTPTalker::receive_request() %d{ message: \"%s\" }\n", 162, "called receive_request()");
    
    this->reset_values();
    
    string header = "";
    int buffer_size = 50;
    char buffer[buffer_size];
    char *header_end = nullptr;
    ssize_t bytes_read;
    while ((bytes_read = read(this->connfd, buffer, buffer_size - 1)) > 0 ) {
        buffer[bytes_read] = '\0';
        header += buffer;
        
        if ( (header_end = strstr(header.c_str(), "\r\n\r\n")) != NULL) {
            *header_end = '\0'; // terminate string at first \r in \r\n\r\n
            header_end += 4; // Increment 4 chars to point to index after last \n
            break;
        }
    }
    
    if (bytes_read < 0) {
        printf("HTTPTalker::receive_request() 183: { message: \"%s\" }\n", "bytes_read was less than 0");
        return false;
    }
    
    // Recieved header ... parse values into internal data structure
    this->parse_header( (char *) header.c_str());
    
    if (this->content_length == 0)
        return true; // There is no body to parse...we're done here
    
    string content = header_end; // Points to start of body
    size_t bytes_to_read = this->content_length - content.length(); // (Total size of content) - (amount of content we already have)
    
    while ( bytes_to_read > 0 ) {
        bytes_read = read(this->connfd, buffer, buffer_size - 1);
        
        if (bytes_read < 0) {
            printf("HTTPTalker::receive_request() 200: { message: \"%s\" }\n", "bytes_read was less than 0");
            return false;
        }
        
        buffer[bytes_read] = '\0';
        content += buffer;
        bytes_to_read -= bytes_read;
    }
    
    this->content = (void *) content.c_str();
    
    return true;
}

string HTTPTalker::get_header_value(const string & key) {
    return this->header_values[key];
}

bool HTTPTalker::send_response(void *response_content, size_t res_length, MIME_TYPES mime_type, int status_code) {
    printf("HTTPTalker::send_response() 206: { message: \"%s\" }\n", "Called send_response");
    
    bool free_content = false;
    
    if (response_content == NULL || res_length <= 0) {
        free_content = true;
        FILE *song_file = fopen("/Users/sourabhdesai/Music/00 First song.mp3", "r");
        if (song_file == NULL) {
            printf("HTTPTalker::send_response() 222: { message: \"%s\" }\n", "song_file was null");
        }
        response_content = read_file(song_file, (long *) &res_length);
        fclose(song_file);
    }
    
    long response_size;
    char *response = construct_response(status_code, status_code_to_desc(status_code), mime_type, this->to_keep_alive, response_content, res_length, &response_size);
    
    bool success = true;
    
    char *response_ptr = response;
    size_t bytes_to_send = response_size;
    while (bytes_to_send > 0) {
        ssize_t bytes_sent = send(this->connfd, response_ptr, bytes_to_send, 0);
        if (bytes_sent < 1) {
            printf("HTTPTalker::send_response 220: { message: \"%s\", socket_fd: %d }\n", "Got error from send()", this->connfd);
            success = false;
            break;
        }
        response_ptr += bytes_sent;
        bytes_to_send -= bytes_sent;
    }
    
    free(response);
    if (free_content)
        free(response_content);

    return success;
}