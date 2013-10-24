// -*- c++ -*-
// STL-like Hashtable class for pointers to type T
//
// Requirements for class T:
// - must contain DLNode for DLList<T,o>
//   (see dl_list.h for offset o to DLNode)
// - size_t hash(const T *item, size_t N)
// - bool equals(const T *lhs, const T *rhs)
//
// N (size of hash table) should be prime
//
// kasemir@lanl.gov

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include"dl_list.h"

//#include <iostream.h>
#include <iostream>

// the following was introduced to get edm
// to compile under gcc 3.4.1
#if (__GNUC__==3)&&(__GNUC_MINOR__==4)
using std::cout;
#endif

// the following was introduced to get edm
// to compile under gcc 4.0.X and higher
#if (__GNUC__>=4)
using std::cout;
#endif

// the following was introduced to get edm
// to compile under gcc 4.1.X and higher
#if (__GNUC__==4)&&(__GNUC_MINOR__==1)
using std::cout;
#endif

// the following was introduced to get edm
// to compile under aCC on HP_UX
#ifdef HP_UX
using std::cout;
#endif

template <class T, size_t o, size_t N=5>
class Hashtable
{
    public:
    class iterator
    {
    public:
        iterator();
        //iterator(Hashtable *_hash, size_t _n, DLList<T,o>::iterator _pos);
        iterator(Hashtable *_hash, size_t _n, typename DLList<T,o>::iterator _pos);
                T * operator * ()   { return *pos; }
        iterator &operator ++ () {
                        ++pos;
                        if (pos != hash->bucket[n].end())
                                return *this;
                        // find next valid bucket
                        while (++n < N)
                                if (!hash->bucket[n].empty())
                                {
                                        pos = hash->bucket[n].begin();
                                        return *this;
                                }
                        // Move to end()
                        n = N-1;
                        pos = hash->bucket[N-1].end();
                        return *this;
                }
        bool operator == (const iterator &rhs)     { return pos == rhs.pos; }
        bool operator != (const iterator &rhs)     { return pos != rhs.pos; }
        iterator & operator = (const iterator &rhs) {
                         hash = rhs.hash; n = rhs.n; pos = rhs.pos; return *this; }
    private:
        //friend class Hashtable<T,o,N>;
        friend class Hashtable;
        Hashtable *hash;
        size_t n;
        typename DLList<T,o>::iterator pos;
    };
    void insert(T *item);
    iterator find(const T *item){
                // Item that uses Hashtable has to provide
                //  size_t hash(const T *item, size_t N);
                size_t h = hash(item, N);
                typename DLList<T,o>::iterator i = bucket[h].begin();
                while (i != bucket[h].end())
                {
                        // Item has to provide
                        // bool equals(const T *lhs, const T *rhs);
                        if (equals(*i, item))
                                return iterator(this, h, i);
                        ++i;
                }
                return end();
        }
    iterator begin(){
                for (size_t n=0; n<N; ++n)
                {
                        if (!bucket[n].empty())
                                {
                                        return iterator(this, n, bucket[n].begin());
                                }
                }
                return end();
        }
    iterator end(){
        return iterator(this, N-1, bucket[N-1].end());
        }
    void erase(iterator& it);
    void info();
private:
    friend class iterator;
    DLList<T,o> bucket[N];
};

// Helper: Compute hash value of string.
inline size_t generic_string_hash(const char *s, size_t N)
{
    size_t h = 0;
    while (*s != '\0')
    {
        h = 31 * h + *s;
        ++s;
    }
    return h % N;
    // From Kerninghan/Pike, "The Practice of Programming":
    //    "It's hard to construct a hash function that does appreciably
    //     better than the one above, but it's easy to make one that
    //     does worse."
}

// inlines --------------------------------------------------------------

template <class T, size_t o, size_t N>
inline Hashtable<T,o,N>::iterator::iterator()
{
    hash = 0;
    n = 0;
}


template <class T, size_t o, size_t N>
inline Hashtable<T,o,N>::iterator::iterator(Hashtable<T,o,N> *_hash, size_t _n,
                                        typename DLList<T,o>::iterator _pos)
{
    hash = _hash;
    n = _n;
    pos = _pos;
}


template <class T, size_t o, size_t N>
inline void Hashtable<T,o,N>::insert(T *item)
{
    // Item that uses Hashtable has to provide
    //  size_t hash(const T *item, size_t N);
    size_t h = hash(item, N);
    // 
    bucket[h].push_back(item);
}


template <class T, size_t o, size_t N>
inline void Hashtable<T,o,N>::erase(typename Hashtable<T,o,N>::iterator& it)
{
    bucket[it.n].erase(it.pos);
}


template <class T, size_t o, size_t N>
inline void Hashtable<T,o,N>::info()
{
    cout << "Hashtable info: " << N << " buckets\n";
    for (size_t n=0; n<N; ++n)
    {
        cout << n << ":  "
                  << bucket[n].size() << " elements\n";
    }
}

#endif
