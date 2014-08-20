//
//  http_helpers.h
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/19/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#ifndef ThreadPooled_Server_http_helpers_h
#define ThreadPooled_Server_http_helpers_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum MIME_TYPES {
    TEXT_HTML, TEXT_CSS, IMAGE_JPEG, IMAGE_PNG, TEXT_PLAIN, AUDIO_MPEG
} MIME_TYPES;

char *status_code_to_desc(int status_code);
char *mime_type_to_string(MIME_TYPES type);
char *construct_response(int response_code, char *response_code_string, MIME_TYPES type, int keep_alive, void *content, long content_length, long *response_size_ptr);
void *read_file(FILE *file, long *buffer_size_ptr);

#endif
