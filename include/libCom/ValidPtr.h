#ifndef LIBCOM_VALIDPTR_H
#define LIBCOM_VALIDPTR_H

#include <mutex>
#include <vector>
#include <algorithm>

#define _LOCK_SCOPE(x) std::lock_guard<std::mutex> _hopefullyUnusedVariableName_(x);

template <typename T>
class ValidPtr {
public:
    ValidPtr(T* instance) : mInstance(instance) {
        instance->subscribeValid(*this);
    }

    ~ValidPtr() {
        _LOCK_SCOPE(busMutex);
        if(mInstance && valid)
            mInstance->unsubscribeValid(*this);
    }

    /**
     * called by encapsulated class to notify of destruction
     */
    inline void setInvalid() { _LOCK_SCOPE(busMutex); valid = false; }

    /**
     * throw caller under bus if he calls on invalid ptr
     */
    inline T* operator->() { if(valid) return mInstance; else throw std::invalid_argument("Attempt to dereference an invalid pointer"); }

    /**
     * @return indicator whether object is valid
     * Note: use with lock only
     */
    inline bool operator()() { return valid; }

    /**
     * To minimize risk of race condition between usage of () operator and -> operator we need the caller to lock
     * this mutex in order to be able to work on the object.
     *
     * @return a unique lock ensuring the objects validity for the locks lifetime
     */
    inline std::unique_lock<std::mutex> getLock() {
        return std::unique_lock<std::mutex>(busMutex);
    }

private:
    // members for objects and validity
    T* mInstance;
    bool valid = true;

    // mutex for not throwing other ppl under bus
    std::mutex busMutex;
};

template <typename T>
class ValidPtrObject {
public:
    virtual ~ValidPtrObject() {
        _LOCK_SCOPE(_mmtxs);
        for (auto p : _mptrs)
            p->setInvalid();
    }

    /**
     * Subscribes a ValidPtr to this object's validness
     * @param p ValidPtr to subscribe
     */
    inline void subscribeValid(ValidPtr<T> &p) {
        _LOCK_SCOPE(_mmtxs);
        _mptrs.push_back(&p);
    }

    /**
     * Unsubscribes a ValidPtr from this object's validness
     * @param p ValidPtr to unsubscribe
     */
    inline void unsubscribeValid(ValidPtr<T> &p) {
        _LOCK_SCOPE(_mmtxs);
        // remove/erase idiom
        _mptrs.erase(std::remove(_mptrs.begin(), _mptrs.end(), &p), _mptrs.end());
    }

protected:
    std::vector<ValidPtr<T>*> _mptrs;
    std::mutex                _mmtxs;
};

#endif //LIBCOM_VALIDPTR_H
