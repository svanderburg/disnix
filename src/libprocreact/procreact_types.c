#include "procreact_types.h"
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void *procreact_type_initialize_bytes(void)
{
    return calloc(1, sizeof(ProcReact_BytesState));
}

ssize_t procreact_type_append_bytes(ProcReact_Type *type, void *state, int fd)
{
    ProcReact_BytesState *bytes_state = (ProcReact_BytesState*)state;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    
    if(bytes_read > 0)
    {
        bytes_state->data = realloc(bytes_state->data, (bytes_state->data_size + bytes_read));
        memcpy(bytes_state->data + bytes_state->data_size, buffer, bytes_read);
        bytes_state->data_size = bytes_state->data_size + bytes_read;
    }
    
    return bytes_read;
}

void *procreact_type_finalize_bytes(void *state, pid_t pid, ProcReact_Status *status)
{
    ProcReact_BytesState *bytes_state = (ProcReact_BytesState*)state;
    int success = procreact_wait_for_boolean(pid, status);
    
    if(*status == PROCREACT_STATUS_OK && success)
        return bytes_state;
    else
    {
        free(bytes_state->data);
        free(bytes_state);
        return NULL;
    }
}

void *procreact_type_finalize_string(void *state, pid_t pid, ProcReact_Status *status)
{
    ProcReact_BytesState *bytes_state = (ProcReact_BytesState*)procreact_type_finalize_bytes(state, pid, status);
    
    if(bytes_state == NULL)
        return NULL;
    else
    {
        char *result;
        
        bytes_state->data = realloc(bytes_state->data, (bytes_state->data_size + 1) * sizeof(char));
        
        /* Add NUL-termination */
        result = (char*)bytes_state->data;
        result[bytes_state->data_size] = '\0';
        
        free(bytes_state);
    
        return result;
    }
}

static char *copy_substring(char *str, unsigned int start_offset, unsigned int end_offset)
{
    if(end_offset > start_offset)
    {
        unsigned int substring_len = end_offset - start_offset;
        char *substr = (char*)malloc((substring_len + 1) * sizeof(char));
        strncpy(substr, str + start_offset, substring_len);
        substr[substring_len] = '\0';
        
        return substr;
    }
    else
        return strdup("");
}

static char **increase_tokens_array(char **tokens, unsigned int *tokens_length)
{
    *tokens_length = *tokens_length + 1;
    tokens = (char**)realloc(tokens, *tokens_length * sizeof(char*));
    tokens[*tokens_length - 1] = NULL;
    
    return tokens;
}

static void append_or_concatenate_buffer(char **tokens, char *buf, unsigned int start_offset, unsigned int pos, unsigned int tokens_length)
{
    char *token = copy_substring(buf, start_offset, pos);
    char *last_token = tokens[tokens_length - 1]; /* Take the last token */
    
    if(last_token == NULL)
        tokens[tokens_length - 1] = token; /* If the last token is NULL, append token to the end */
    else
    {
        /* Otherwise, concatenate buffer to the last token */
        size_t last_token_length = strlen(last_token);
        size_t token_length = strlen(token);
        size_t concat_token_length = last_token_length + token_length;
        
        tokens[tokens_length - 1] = (char*)realloc(tokens[tokens_length - 1], concat_token_length + 1);
        strncpy(tokens[tokens_length - 1] + last_token_length, token, token_length);
        tokens[tokens_length - 1][concat_token_length] = '\0';
        
        /* Cleanup obsolete old token */
        free(token);
    }
}

static char **update_tokens_vector(char **tokens, unsigned int *tokens_length, char *buf, ssize_t buf_len, const char delimiter)
{
    int i;
    unsigned int start_offset = 0;
    
    /* Check the buffer for tokens */
    for(i = 0; i < buf_len; i++)
    {
        /* If a delimiter is found, append the token to the tokens vector, and increase the vector size */
        if(buf[i] == delimiter)
        {
            append_or_concatenate_buffer(tokens, buf, start_offset, i, *tokens_length);
            tokens = increase_tokens_array(tokens, tokens_length);
            
            start_offset = i + 1;
        }
    }
    
    /* If there is trailing stuff, append it to the tokens vector */
    if(start_offset < i)
        append_or_concatenate_buffer(tokens, buf, start_offset, i, *tokens_length);
    
    /* Return modified tokens vector */
    return tokens;
}

void *procreact_type_initialize_string_array(void)
{
    ProcReact_StringArrayState *string_array_state = (ProcReact_StringArrayState*)malloc(sizeof(ProcReact_StringArrayState));
    
    /* We need at least one element in the beginning */
    string_array_state->result_length = 0;
    string_array_state->result = increase_tokens_array(NULL, &string_array_state->result_length);
    
    return string_array_state;
}

ssize_t procreact_type_append_strings_to_array(ProcReact_Type *type, void *state, int fd)
{
    ProcReact_StringArrayState *string_array_state = (ProcReact_StringArrayState*)state;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    
    if(bytes_read > 0)
        string_array_state->result = update_tokens_vector(string_array_state->result, &string_array_state->result_length, buffer, bytes_read, type->delimiter);
    
    return bytes_read;
}

void *procreact_type_finalize_string_array(void *state, pid_t pid, ProcReact_Status *status)
{
    ProcReact_StringArrayState *string_array_state = (ProcReact_StringArrayState*)state;
    int success = procreact_wait_for_boolean(pid, status);
    char **result;
    
    /* Ensure that the result always ends with NULL termination */
    
    if(string_array_state->result[string_array_state->result_length - 1] != NULL)
        string_array_state->result = increase_tokens_array(string_array_state->result, &string_array_state->result_length);
    
    if(*status != PROCREACT_STATUS_OK || !success)
    {
        procreact_free_string_array(string_array_state->result);
        string_array_state->result = NULL;
    }
    
    result = string_array_state->result;
    free(string_array_state);
    
    return result;
}


ProcReact_Type procreact_create_bytes_type(void)
{
    ProcReact_Type type = { procreact_type_initialize_bytes, procreact_type_append_bytes, procreact_type_finalize_bytes };
    return type;
}

ProcReact_Type procreact_create_string_type(void)
{
    ProcReact_Type type = { procreact_type_initialize_bytes, procreact_type_append_bytes, procreact_type_finalize_string };
    return type;
}

ProcReact_Type procreact_create_string_array_type(char delimiter)
{
    ProcReact_Type type = { procreact_type_initialize_string_array, procreact_type_append_strings_to_array, procreact_type_finalize_string_array, delimiter };
    return type;
}

void procreact_free_string_array(char **arr)
{
    if(arr != NULL)
    {
        unsigned int count = 0;
        
        while(arr[count] != NULL)
        {
            free(arr[count]);
            count++;
        }
        
        free(arr);
    }
}
