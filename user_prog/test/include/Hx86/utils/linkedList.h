#pragma once
#include <Hx86/stdint.h>
#include <Hx86/memory.h>

template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& _data) : data(_data), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    uint32_t size;

    void CopyFrom(const LinkedList& other) {
        Node* current = other.head;
        while (current) {
            PushBack(current->data);
            current = current->next;
        }
    }

public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    ~LinkedList() {
        Clear();
    }

    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        CopyFrom(other);
    }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            Clear();
            CopyFrom(other);
        }
        return *this;
    }

    void Add(const T& item) {
        Node* newNode = new Node(item);
        newNode->next = head;
        head = newNode;
        if (!tail) tail = head;
        size++;
    }

    void PushBack(const T& item) {
        Node* newNode = new Node(item);
        if (!tail) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }

    template <typename Func>
    T Find(Func condition) const {
        Node* current = head;
        while (current) {
            if (condition(current->data)) {
                return current->data;
            }
            current = current->next;
        }
        return T{};
    }

    template <typename Func>
    bool Remove(Func condition) {
        Node* current = head;
        Node* prev = nullptr;

        while (current) {
            if (condition(current->data)) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    head = current->next;
                }
                if (current == tail) {
                    tail = prev;
                }
                delete current;
                size--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    uint32_t GetSize() const {
        return size;
    }

    bool IsEmpty() const {
        return size == 0;
    }

    template <typename Func>
    void ForEach(Func func) const {
        Node* current = head;
        while (current) {
            func(current->data);
            current = current->next;
        }
    }

    template <typename Func>
    T Take(Func condition) {
        Node* current = head;
        Node* prev = nullptr;

        while (current) {
            if (condition(current->data)) {
                T result = current->data;

                if (prev) {
                    prev->next = current->next;
                } else {
                    head = current->next;
                }
                if (current == tail) {
                    tail = prev;
                }

                delete current;
                size--;
                return result;
            }

            prev = current;
            current = current->next;
        }

        return T{};
    }

    T PopFront() {
        if (!head) return T{};
        Node* temp = head;
        T result = head->data;
        head = head->next;
        if (!head) tail = nullptr;
        delete temp;
        size--;
        return result;
    }

    T GetFront() {
        if (!head) return T{};
        T result = head->data;
        return result;
    }

    void Clear() {
        Node* current = head;
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        head = tail = nullptr;
        size = 0;
    }

    // ----------- Iterator Support -----------
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* start) : current(start) {}

        T& operator*() const {
            return current->data;
        }

        Iterator& operator++() {
            if (current) current = current->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    Iterator begin() const {
        return Iterator(head);
    }

    Iterator end() const {
        return Iterator(nullptr);
    }

    template <typename Func>
    void ReverseForEach(Func func) const {
        T* stack[128]; // fixed-size stack
        uint32_t count = 0;

        Node* current = head;
        while (current && count < 128) {
            stack[count++] = &current->data;
            current = current->next;
        }

        for (int32_t i = count - 1; i >= 0; --i) {
            func(*stack[i]);
        }
    }
};
