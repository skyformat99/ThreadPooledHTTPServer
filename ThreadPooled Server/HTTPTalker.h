//
//  HTTPParser.h
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/14/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//
#pragma once
#ifndef ThreadPooled_Server_HTTPParser_h
#define ThreadPooled_Server_HTTPParser_h


#include <string>
#include <unordered_map>

#include "http_helpers.c"
#include "http_helpers.h"

using std::string;
using std::unordered_map;

class HTTPTalker {

public:
    
    HTTPTalker(int connfd);
    
    bool receive_request();
    bool send_response(void *res_content, size_t res_length, MIME_TYPES mime_type, int status_code);
    
    bool keep_alive() const;
    
    string get_header_value(const string & key);
    
private:
    
    static const int INITIAL_BUFFER_SIZE = 1024;

    int connfd;
    
    // All from request line
    string request_line;
    /*
    string method;
    string url;
    string version;
    */
    // Header key-values
    unordered_map<string, string> header_values;
    // Body content of request
    size_t content_length;
    bool to_keep_alive;
    
    void *content;
    
    void reset_values();
    void parse_header(char *header);
    
};

#endif
