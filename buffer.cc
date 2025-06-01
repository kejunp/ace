#include "buffer.h"
#include <cstring>
#include <iostream>

EditorBuffer::EditorBuffer() {
    buffer_ = new Node[CHUNK_CAPACITY_];
    for (size_t i = 0; i < CHUNK_CAPACITY_; ++i) {
        buffer_[i].used = false;
    }
    list_ = alloc_node('\0');
    list_->prev = list_;
    list_->next = list_;
    cursor = list_;
    free_head_ = nullptr;
}

EditorBuffer::EditorBuffer(const char* src) : EditorBuffer() {
    for (const char* p = src; *p; ++p) {
        insert(*p);
    }
}

EditorBuffer::~EditorBuffer() {
    delete[] buffer_;
}

EditorBuffer::Node* EditorBuffer::alloc_node(char ch) {
    for (size_t i = 0; i < CHUNK_CAPACITY_; ++i) {
        if (!buffer_[i].used) {
            buffer_[i].used = true;
            buffer_[i].data = ch;
            buffer_[i].prev = buffer_[i].next = nullptr;
            return &buffer_[i];
        }
    }
    return nullptr;
}

void EditorBuffer::free_node(Node* node) {
    node->used = false;
}

void EditorBuffer::insert(char ch) {
    Node* new_node = alloc_node(ch);
    if (!new_node) return;  // out of space

    new_node->prev = cursor;
    new_node->next = cursor->next;

    cursor->next->prev = new_node;
    cursor->next = new_node;

    cursor = new_node;
}

void EditorBuffer::delete_char() {
    if (cursor->next != list_) {
        Node* to_delete = cursor->next;
        cursor->next = to_delete->next;
        to_delete->next->prev = cursor;
        free_node(to_delete);
    }
}

void EditorBuffer::move_left() {
    if (cursor != list_) cursor = cursor->prev;
}

void EditorBuffer::move_right() {
    if (cursor->next != list_) cursor = cursor->next;
}

void EditorBuffer::move_start() {
    cursor = list_;
}

void EditorBuffer::move_end() {
    while (cursor->next != list_) cursor = cursor->next;
}

void EditorBuffer::move_up() {
    Node* temp = cursor;
    while (temp != list_ && temp->data != '\n') temp = temp->prev;
    if (temp == list_) return;
    int col = 0;
    Node* back = cursor;
    while (back != temp) {
        ++col;
        back = back->prev;
    }
    temp = temp->prev;
    while (temp != list_ && temp->data != '\n') temp = temp->prev;
    if (temp == list_) temp = list_;
    else temp = temp->next;
    for (int i = 0; i < col && temp != list_ && temp->data != '\n'; ++i) temp = temp->next;
    cursor = temp;
}

void EditorBuffer::move_down() {
    Node* temp = cursor;
    while (temp != list_ && temp->data != '\n') temp = temp->prev;
    if (temp == list_) temp = list_;
    else temp = temp->next;
    int col = 0;
    Node* back = cursor;
    while (back != temp) {
        ++col;
        back = back->prev;
    }
    temp = cursor;
    while (temp != list_ && temp->data != '\n') temp = temp->next;
    if (temp == list_) return;
    temp = temp->next;
    for (int i = 0; i < col && temp != list_ && temp->data != '\n'; ++i) temp = temp->next;
    cursor = temp;
}

void EditorBuffer::substitute_all(const std::string& pattern, const std::string& replacement) {
    Node* p = list_->next;
    while (p != list_) {
        Node* start = p;
        Node* check = p;
        size_t match_len = 0;
        while (check != list_ && match_len < pattern.size() && check->data == pattern[match_len]) {
            ++match_len;
            check = check->next;
        }
        if (match_len == pattern.size()) {
            for (size_t i = 0; i < match_len; ++i) {
                Node* next = start->next;
                start->prev->next = start->next;
                start->next->prev = start->prev;
                free_node(start);
                start = next;
            }
            Node* prev = p->prev;
	for (char rc : replacement) {
                Node* n = alloc_node(rc);
                n->prev = prev;
                n->next = prev->next;
                prev->next->prev = n;
                prev->next = n;
                prev = n;
            }
            p = prev->next;
        } else {
            p = p->next;
        }
    }
}

std::string EditorBuffer::get_string() const {
    std::string result;
    result.reserve(CHUNK_CAPACITY_);
    Node* sentinel = list_;
    for (Node* p = sentinel->next; p != sentinel; p = p->next) {
        if (p->used) {
            result.push_back(p->data);
        }
    }
    return result;
}

EditorBuffer::Node* EditorBuffer::get_cursor() const {
    return cursor;
}

EditorBuffer::Node* EditorBuffer::get_head() const {
    return list_;
}


void EditorBuffer::backspace_char() {
    if (cursor != list_) {
        Node* to_delete = cursor;
        cursor = cursor->prev;
        cursor->next = to_delete->next;
        to_delete->next->prev = cursor;
        free_node(to_delete);
    }
}