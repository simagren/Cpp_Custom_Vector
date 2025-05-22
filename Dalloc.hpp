#pragma once

//Stop std::vector from checking to much
//#define _ITERATOR_DEBUG_LEVEL 0

#include <string>
#include<cassert>
#include<iostream>
#include <vector>
#include "Dhelper.h"


template<class T>
bool ErrMegToUser(T msg) {
    static bool executed=false;
    if (!executed) {
        //executed = true;
        std::cout << msg << '\n';
        std::cout << "väntar på Enter"; std::cin.ignore(999, '\n');
    }
    return false;
}

template <class T>
std::string MsgAt(T file, size_t line) {
    return std::string(" in ") + file + std::string(" at ") + std::to_string(line) + ' ';
}

template<class T>
class Dalloc;
template<class T>
class PoolObj;
template<class T>
bool CheckVector(PoolObj<T>* poolPtr);
template <class T>
class VectorHack;
template <class T>
void DestroyAllocatedList(VectorHack<T>* This);


//The PoolObj is allocated together with the normal object allocated.
//It is holding data about the object to make it possible to check errors in memory handling.
template<class T>
class PoolObj {
    friend class Dalloc<T>;
    friend bool CheckVector<T>(PoolObj<T>* poolPtr);

    size_t n = (size_t)-1;   //Size of item allocated(in T)
    std::string allocFile = "";
    size_t allocLine = (size_t)-1;
    std::string  deallocFile = std::string("");
    size_t deallocLine = (size_t)-1;
    PoolObj(size_t n, std::string name, int lineNumber) :
        n(n),
        allocFile(name),
        allocLine(lineNumber) {}
    T* Get() {
        return reinterpret_cast<T*>((char*)this + sizeof(PoolObj<T>));
    }
    static PoolObj<T>* Get(T* ptr) {
        return reinterpret_cast<PoolObj<T>*>(this - sizeof(PoolObj<T>));
    }
    std::string AllocIn() {
        return std::string(" allocated ")
            + MsgAt(allocFile, allocLine);
    }
};


template <class T>
class VectorHack {
    friend void DestroyAllocatedList<T>(VectorHack<T>* This);
    friend class Dalloc<T>;
    std::vector<PoolObj<T>*> list;
    //VectorHack() {
    //}
    ~VectorHack() {
        DestroyAllocatedList<T>(this);
    }
    //size_t size() {
    //    return list.size();
    //}
};

template<class T>
class Dalloc {

    std::allocator<char> byteAllocator;
    std::allocator<T> stdalloc;
    static VectorHack<T> allocatedList;

public:

#define STDVECTOR
#pragma region Behhövs för std::vecotr när jag kör med den!
#ifdef STDVECTOR
    //En del är bortkommenterat då det inte behövdes när jag fått ordning på det
    // och gjort #define _ITERATOR_DEBUG_LEVEL 0
    using value_type = T;
    //bool operator==(const Dalloc&) { return true; }
    bool operator!=(const Dalloc&) { return false; }

    //friend class Dalloc;
    //template <class U>
    //Dalloc(const Dalloc<U>& r) :stdalloc(r.stdalloc) { allocatedList = r.allocatedList; }

    //template <class U>
    //Dalloc& operator=(const Dalloc<U>& r) {
    //    stdalloc(r.stdalloc);
    //    allocatedList = r.allocatedList;
    //    return *this;
    //}
    //Only needed if std::vector is tested!
    T* allocate(size_t n) {
        return allocate(n, "", 40);
    }
    void deallocate(T* ptr, size_t n) {
        deallocate(ptr, n, "XXX", 80);
    }
#endif
#pragma region Behhövs för std::vecotr när jag kör med den!


    T* allocate(size_t n, const char* file, int line) {
        if (n == 0) {
            ErrMegToUser("att allokera 0 bytes kan vara slöseri med minne!\n");
            return nullptr;
        }
        PoolObj<T>* ptr = reinterpret_cast<PoolObj<T>*>(byteAllocator.allocate(sizeof(PoolObj<T>) + n * sizeof(T)));
        new (ptr) PoolObj<T>(n, file, line);
        memset(ptr->Get(), 1, n * sizeof(T)); // To be able to revognize uninitiated memory!
        allocatedList.list.push_back(ptr);
        return ptr->Get();
    }

    void deallocate(T* ptr, size_t n, const char* file, size_t line) {
        if (!ptr)
            return;
        PoolObj<T>* poolPtr = reinterpret_cast<PoolObj<T>*>((char*)ptr - sizeof(PoolObj<T>));
        auto findIt = std::find(allocatedList.list.begin(), allocatedList.list.end(), poolPtr);
        if (findIt == allocatedList.list.end()) {
            assert("" == std::string("Trying to delete a non-allocated item!") + MsgAt(file, line) + '\n');
            return;
        }
        if (n != poolPtr->n) {
            std::cout << std::string("deallocate N does not match allocate N \n")
                + poolPtr->AllocIn() + " deallocated " +MsgAt(file, line) + '\n';
            __debugbreak();
        }
        auto aFile = poolPtr->allocFile;
        auto aLine = poolPtr->allocLine;

        auto deFile = poolPtr->deallocFile;
        auto deLine = poolPtr->deallocLine;
        if (deFile != "") {
            auto msg = std::string("deleting twice ") + MsgAt(file, line);
            msg += std::string("\nwas already deleted ") + MsgAt(deFile, deLine);
            msg += std::string("\nwas allocated " + poolPtr->AllocIn() + '\n');
            ErrMegToUser(msg);
            __debugbreak();
            return;
        }
        poolPtr->deallocFile = file;
        poolPtr->deallocLine = line;
    }
};
template<class T>
VectorHack<T> Dalloc<T>::allocatedList;

#undef allocate
#define allocate(x) allocate(x, __FILE__, __LINE__)
#undef deallocate
#define deallocate(x, y) deallocate(x, y, __FILE__, __LINE__)


template<class T>
bool CheckVector(PoolObj<T>* poolPtr) {
    if (poolPtr->deallocFile == "") {
        return ErrMegToUser(std::string("memory ") + poolPtr->AllocIn() + "was not deallocated!");
    }
    return true;
}
template<>
bool CheckVector<Dhelper>(PoolObj<Dhelper>* poolPtr) {
    if (poolPtr->deallocFile == "") {
        return ErrMegToUser(std::string("memory ") + poolPtr->AllocIn() + "was not deallocated!");
    }
    size_t n = poolPtr->n;
    Dhelper* ptr = poolPtr->Get();
    size_t i = 0;
    i = DD;
    i = NON;
    i = 0;
    for (; i < n; ++i,++ptr) {
        if (ptr->FLAG != DD) break;
    }
    for (; i < n; ++i, ++ptr) {
        if (ptr->FLAG != NON) break;
    }
    if (i != n) {
        return ErrMegToUser(std::string("parts of the memory ") + poolPtr->AllocIn()
            + "\nand deallocated in " + MsgAt(poolPtr->allocFile, poolPtr->deallocLine)
            + "\nwas not destroyed correct!\n");
            }
    return true;
}

template <class T>
void DestroyAllocatedList(VectorHack<T>* This) {
    std::vector<PoolObj<T>*>* allocList = &(This->list);
    for (auto it : *allocList)
        CheckVector(&*it);
        //if (!CheckVector(&*it))
        //    return;
}
