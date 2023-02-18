#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <ctype.h>

char* take_cookie(char* response) {
    char* cookie = (response + 277);

    int i;
    for(i = 0; i < strlen(cookie) - 1; i++) {
        if (strncmp((cookie + i), ";", 1) == 0) {
            break;
        }
    }

    cookie[i] = '\0';

    return cookie;
}

char* take_JWT(char* response) {
    char* token = (response + 472);

    int i;
    for(i = 0; i < strlen(token) - 1; i++) {
        if (strncmp((token + i), "\"", 1) == 0) {
            break;
        }
    }

    token[i] = '\0';

    char *a = "Bearer ";
    char *jwt = malloc(strlen(a) + strlen(token) + 1);
    strcpy(jwt, a);
    strcat(jwt, token);

    return jwt;
}


void set_register_object (JSON_Value *register_value, JSON_Object *register_object) {

    char* username;
    size_t user_len = 0;
    user_jmp:
    printf("username=");
    getline(&username, &user_len, stdin);
    username[strlen(username) - 1] = '\0';
    if (strlen(username) < 4) {
        printf("Username too short! Please try another one!\n");
        goto user_jmp;
    }

    char* password;
    size_t pass_len = 0;
    pass_jmp:
    printf("password=");
    getline(&password, &pass_len, stdin);
    password[strlen(password) - 1] = '\0';
    if (strlen(password) < 4) {
        printf("Password too short! Please try another one!\n");
        goto pass_jmp;
    }

    json_object_set_string(register_object, "username", username);
    json_object_set_string(register_object, "password", password);
}

void set_login_object (JSON_Value *login_value, JSON_Object *login_object) {
    char* username;
    size_t user_len = 0;
    user_jmp:
    printf("username=");
    getline(&username, &user_len, stdin);
    username[strlen(username) - 1] = '\0';
    if (strlen(username) < 4) {
        printf("Username too short! Please try another one!\n");
        goto user_jmp;
    }

    char* password;
    size_t pass_len = 0;
    pass_jmp:
    printf("password=");
    getline(&password, &pass_len, stdin);
    password[strlen(password) - 1] = '\0';
    if (strlen(password) < 4) {
        printf("Password too short! Please try another one!\n");
        goto pass_jmp;
    }

    json_object_set_string(login_object, "username", username);
    json_object_set_string(login_object, "password", password);
}

void print_all_books (char* response) {
    char* books = strstr(response, "[");    
    JSON_Value *books_value = json_parse_string(books);
    char *payload = json_serialize_to_string_pretty(books_value);
    printf("%s\n", payload);
}

void print_one_book (char* response) {
    char* book = strstr(response, "{");    
    JSON_Value *book_value = json_parse_string(book);
    char *payload = json_serialize_to_string_pretty(book_value);
    printf("%s\n", payload);
}

void set_book_object (JSON_Value *book_value, JSON_Object *book_object) {
    char* title;
    size_t title_len = 0;
    printf("title=");
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    char* author;
    size_t author_len = 0;
    printf("author=");
    getline(&author, &author_len, stdin);
    author[strlen(author) - 1] = '\0';

    char* genre;
    size_t genre_len = 0;
    printf("genre=");
    getline(&genre, &genre_len, stdin);
    genre[strlen(genre) - 1] = '\0';

    char* publisher;
    size_t publisher_len = 0;
    printf("publisher=");
    getline(&publisher, &publisher_len, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    char* page_count;
    size_t page_count_len = 0;
    write:
    printf("page_count=");
    getline(&page_count, &page_count_len, stdin);
    page_count[strlen(page_count) - 1] = '\0';
    for (size_t i = 0; i < strlen(page_count); i++)
    {
        if (!isdigit(page_count[i])) {
            printf("Please write a valid number for page_count!\n");
            goto write;
        }
    }
    json_object_set_string(book_object, "title", title);
    json_object_set_string(book_object, "author", author);
    json_object_set_string(book_object, "genre", genre);
    json_object_set_string(book_object, "publisher", publisher);
    json_object_set_string(book_object, "page_count", page_count);
}

void print_server_response (char* response, char* request_type) {
    //  Make a copy of response
    char* copy = calloc(LINELEN, sizeof(char));
    memcpy(copy, response, LINELEN);

    char* response_tk = strtok(response, " ");
    response_tk = strtok(NULL, " ");
    
    if (strncmp(request_type, "register", 8) == 0) {
        if (strcmp(response_tk, "201") == 0) {
            printf("Register with success!\n");
        } else {
            printf("Error: This username is taken!\n");
        }

    } else if (strncmp(request_type, "login", 5) == 0) {
        if (strcmp(response_tk, "200") == 0) {
            printf("Loggin with success!\n");
        } else {
            printf("Error: Username/Password are not good! Try again!\n");
        }

    } else if (strncmp(request_type, "enter_library", 13) == 0) {
        if (strcmp(response_tk, "200") == 0) {
            printf("Enter in library with success!\n");
        } else {
            printf("Error: You are not logged in!\n");
        }

    } else if (strncmp(request_type, "get_books", 9) == 0) {
        if (strcmp(response_tk, "200") == 0) {
            print_all_books(copy);
            printf("Get books with success!\n");
        } else {
            printf("Error: You do not have access to library!\n");
        }

    } else if (strncmp(request_type, "get_book", 8) == 0 && request_type[9] != '\n') {
        if (strcmp(response_tk, "200") == 0) {
            print_one_book(copy);
            printf("Get book with success!\n");
        } else if (strcmp(response_tk, "403") == 0){
            printf("Error: You do not have access to library!\n");
        } else {
            printf("Error: No book with this ID was found!\n");
        }

    } else if (strncmp(request_type, "add_book", 8) == 0) {
        if (strcmp(response_tk, "200") == 0) {
            printf("Add book with success!\n");
        } else if (strcmp(response_tk, "403") == 0){
            printf("Error: You do not have access to library!\n");
        } else if (strcmp(response_tk, "429") == 0){
            printf("Error: Too many requests!\n");
        } else {
            printf("Error: Book incomplete/wrong format!\n");
        }

    } else if (strncmp(request_type, "delete_book", 11) == 0) {
        if (strcmp(response_tk, "200") == 0) {
            printf("Delete book with success!\n");
        } else if (strcmp(response_tk, "403") == 0){
            printf("Error: You do not have access to library!\n");
        } else {
            printf("Error: No book with this ID was found!\n");
        }

    } else if (strncmp(request_type, "logout", 6) == 0) {
         if (strcmp(response_tk, "200") == 0) {
            printf("Logout with success!\n");
        } else {
            printf("Error: You are not logged in!\n");
        }
    }
}


int main (int argc, char *argv[]) {

    char **cookies = calloc(1, sizeof(char *));
    cookies[0] = calloc(LINELEN, sizeof(char));

    char **tokens = calloc(1, sizeof(char *));
    tokens[0] = calloc(LINELEN, sizeof(char));

    while (1)
    {
        char* cmd = NULL;
        size_t len = 0;
        getline(&cmd, &len, stdin);

        int sockfd;
        char *message = calloc(BUFLEN, sizeof(char));
        char *response = calloc(BUFLEN, sizeof(char));
        
        if (strncmp(cmd, "register", 8) == 0) {

            // Create JSON object for register
            JSON_Value *register_value = json_value_init_object();
            JSON_Object *register_object = json_value_get_object(register_value);
            set_register_object(register_value, register_object);

            // Create payload for request
            char **payload = calloc(1, sizeof(char *));
            payload[0] = calloc(LINELEN, sizeof(char));
            *payload = json_serialize_to_string_pretty(register_value);
            
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", payload, 1, NULL, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Print error or success message
            print_server_response(response, cmd);
        }

        if (strncmp(cmd, "login", 5) == 0) {

            // Create JSON object for login
            JSON_Value *login_value = json_value_init_object();
            JSON_Object *login_object = json_value_get_object(login_value);
            set_login_object(login_value, login_object);            

            // Create payload for request
            char **payload = calloc(1, sizeof(char *));
            payload[0] = calloc(LINELEN, sizeof(char));
            *payload = json_serialize_to_string_pretty(login_value);

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", payload, 1, NULL, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            // Take cookies
            strcpy(cookies[0], take_cookie(response));

            // Print error or success message
            print_server_response(response, cmd);

        }

        if (strncmp(cmd, "enter_library", 13) == 0) {
            
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Take JWT token
            strcpy(tokens[0], take_JWT(response));

            // Print error or success message
            print_server_response(response, cmd);

        }

        if (strncmp(cmd, "get_books", 9) == 0) {

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, cookies, 1, tokens);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Print error or success message
            print_server_response(response, cmd);

        }

        if (strncmp(cmd, "get_book", 8) == 0 && cmd[9] != '\n') {
            
            // If we have JWT that means we have acces in library
            if(strncmp(tokens[0], "Bearer", 6) == 0) {
                char* id;
                size_t id_len = 0;
                printf("id=");
                getline(&id, &id_len, stdin);
                id[strlen(id) - 1] = '\0';

                // Set URL
                char* url = calloc(BUFLEN, sizeof(char));
                sprintf(url, "/api/v1/tema/library/books/%s", id);

                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request("34.241.4.235", url, NULL, cookies, 1, tokens);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Print error or success message
                print_server_response(response, cmd);
            } else {
                printf("Error: You do not have access to library!\n");
            }
        }

        if (strncmp(cmd, "add_book", 8) == 0) {
            
            // If we have JWT that means we have acces in library
            if(strncmp(tokens[0], "Bearer", 6) == 0) {
                // Create JSON book
                JSON_Value *book_value = json_value_init_object();
                JSON_Object *book_object = json_value_get_object(book_value);
                set_book_object(book_value, book_object);
            
                // Create payload for adding a book
                char **payload = calloc(1, sizeof(char *));
                payload[0] = calloc(LINELEN, sizeof(char));
                *payload = json_serialize_to_string_pretty(book_value);

                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", payload, 1, NULL, 1, tokens);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Print error or success message
                print_server_response(response, cmd);
                
            } else {
                printf("Error: You do not have access to library!\n");
            }
            
        }

        if (strncmp(cmd, "delete_book", 11) == 0) {
            
            // If we have JWT that means we have acces in library
            if(strncmp(tokens[0], "Bearer", 6) == 0) {
                char* id;
                size_t id_len = 0;
                printf("id=");
                getline(&id, &id_len, stdin);
                id[strlen(id) - 1] = '\0';
                
                // Set URL
                char* url = calloc(BUFLEN, sizeof(char));
                sprintf(url, "/api/v1/tema/library/books/%s", id);

                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_delete_request("34.241.4.235", url, NULL, cookies, 1, tokens);    
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Print error or success message
                print_server_response(response, cmd);
            } else {
                printf("Error: You do not have access to library!\n");
            }

        }

        if (strncmp(cmd, "logout", 6) == 0) {

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", "application/json", cookies, 1, tokens);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Delete tokens and cookies from memory
            memset(tokens[0], 0, LINELEN);
            memset(cookies[0], 0, LINELEN);

            // Print error or succes message
            print_server_response(response, cmd);
        }

        if (strncmp(cmd, "exit", 4) == 0) {
            break;
        }

        free(cmd);
        free(message);
        free(response);
    }
    

    free(cookies[0]);
    free(cookies);
    free(tokens[0]);
    free(tokens);

    return 0;
}