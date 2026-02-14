#ifndef MINIGC_HPP_
#define MINIGC_HPP_

#include <array>
#include <cstddef>

/**
Minimal implementation of a garbage collector. Needs to be filled actively
(realtime safe) and to be emptied actively (not realtime same).
@tparam T   Data type (not the pointer).
@tparam N   Max number of data to be stored.
 */
template<class T, size_t N = 16>
class MiniGC : protected std::array<T*, N>
{
protected:
    /* Actual number of data in the MiniGC */
    size_t gc_sz_ = 0;

public:
    inline ~MiniGC() {purge();}

    /**
    Adds data to be scheduled for deletion. Assumed to be realtime safe.
    @param ptr  Pointer to the heap data.
    @return     True, if schedule succeded, otherwise false.
     */
    inline bool add(const T* ptr)
    {
        // Already full?
        if (gc_sz_ == N) return false;
        
        // Check if already inside
        for (size_t i = 0; i < gc_sz_; ++i) if (this->operator[](i) == ptr) return false;

        this->operator[](gc_sz_) = ptr;
        ++gc_sz_;
        return true;
    }

    /**
    Empties the MiniGC by deleting all data in there. Call it from outside of
    a realtime thread.
     */
    inline void purge()
    {
        while(gc_sz_ > 0)
        {
            delete this->operator[](gc_sz_ - 1);
            --gc_sz_;
        }
    }
};

#endif /* MINIGC_HPP_*/