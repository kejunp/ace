#pragma once

#include <cstddef>
#include <iostream>
#include <string>

class EditorBuffer {
public:
    struct Node {
        Node* prev;
        Node* next;
        char data;
        bool used;
    };

    static const size_t CHUNK_CAPACITY_ = 1026 * 8;
    static const size_t MAX_CHUNK_INDEX_ = 1026;

    EditorBuffer();
    EditorBuffer(const char* src);
    ~EditorBuffer();

    void insert(char ch);
    void delete_char();
    void delete_before();
    void move_left();
    void move_right();
    void move_start();
    void move_end();
    void move_up();
    void move_down();
    void substitute_all(const std::string& pattern, const std::string& replacement);
    std::string get_string() const;

    Node* get_cursor() const;
    Node* get_head() const;

    friend std::ostream& operator<<(std::ostream& os, const EditorBuffer& src);

private:
    Node* list_;
    Node* buffer_;
    Node* free_head_;
    Node* cursor;

    Node* alloc_node(char ch);
    void free_node(Node* node);
};
