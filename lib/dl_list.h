// -*- c++ -*-
// dl_list - Double Linked List
//
// kasemir@lanl.gov

// This list looks like the std::list as far as the
// method names/semantics are concerned.
// Differences/characteristics/limitations:
// - slightly faster and more memory friendly
//   since it's intrusive (each list item contains the node information)
// - therefore each list item has to contain a DLNode.
// - limited to pointer lists.
// - Item can contain several DLNodes -> be in different list
// - Offset of node has to be passed to DLList
// - only few std::list methods are implemented
//
// Comparison:
//
// To put a class X into this list, it has to contain
// an element
//    DLNode node;
//
// Then list and DLList are interchangable like this:
//
//    typedef std::list<X*>              SList;
//    typedef DLList<X,offsetof(X,node)> XList;
//
// ALMOST:
// Turns out that GCC 3.1 does not like the use of "offsetof"
// with classes: "invalid offsetof from non-POD type ...,
//                use pointer to member instead"
// So it's now best to use DLList only with structs
// or 'simple' classes (no virtuals, no base classes?)

#ifndef __DL_LIST_H__
#define __DL_LIST_H__

#include<stddef.h>
#include<string.h>

struct DLNode
{
    DLNode *prev, *next;
};

template <class T, size_t node_offset>
class DLList
{
public:
    class iterator
    {
    public:
        iterator()                           { this->node = 0; };
        iterator(DLNode *node)               { this->node = node; };
        T *operator * ()                     { return get_item(node); }
        iterator & operator ++ ()            { node=node->next; return *this; }
        iterator & operator -- ()            { node=node->prev; return *this; }
        iterator & operator = (const iterator &rhs) { node = rhs.node; return *this; }
        bool operator == (const iterator &rhs)  { return rhs.node == node; }
        bool operator != (const iterator &rhs)  { return rhs.node != node; }
    private:
        friend class DLList;
        DLNode *node;
    };

    DLList();
    bool empty() const               { return anchor.next==&anchor; }
    size_t size() const              { return elements; }
    T *front()                       { return get_item(anchor.next); }
    iterator begin()                 { return iterator(anchor.next); }
    iterator end()                   { return iterator(&anchor); }
    void push_back(T *item);
    void pop_front();
    iterator erase(iterator position)
    {
        DLNode *node = position.node;
        DLNode *prev = node->prev;
        DLNode *next = node->next;
        
        prev->next = next;
        next->prev = prev;
        node->prev = node->next = 0;
        --elements;
        return iterator(next);
    }       
    
private:
    friend class iterator; // needed for g++ 2.91.66
    static DLNode *get_node(T *item);
    static T *get_item(DLNode *node);
    DLNode anchor;
    size_t elements;
};

// -- inlines ------------------------------------

template <class T, size_t node_offset>
inline DLList<T, node_offset>::DLList()
{
    anchor.prev = &anchor;
    anchor.next = &anchor;
    elements = 0;
}
    
template <class T, size_t node_offset>
inline DLNode *DLList<T, node_offset>::get_node(T *item)
{
    return (DLNode*) ((char *)item + node_offset);
}

template <class T, size_t node_offset>
inline T *DLList<T, node_offset>::get_item(DLNode *node)
{
    return (T *) ((char *)node - node_offset);
}

template <class T, size_t node_offset>
inline void DLList<T, node_offset>::push_back(T *item)
{
    DLNode *node = get_node(item);
    if (anchor.prev==0)
    {
        anchor.next = node;
        node->prev=&anchor;
        node->next=&anchor;
        anchor.prev = node;
    }
    else
    {
        node->prev = anchor.prev;
        node->next = &anchor;
        node->prev->next = node;
        anchor.prev = node;
    }
    ++elements;
}

template <class T, size_t node_offset>
inline void DLList<T, node_offset>::pop_front()
{
    DLNode *node = anchor.next;
    if (node)
    {
        anchor.next = node->next;
        anchor.next->prev = &anchor;
        node->prev = node->next = 0;
    }
    --elements;
}

#endif
