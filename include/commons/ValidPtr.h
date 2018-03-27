#ifndef COMMONS_VALIDPTR_H
#define COMMONS_VALIDPTR_H

#include <mutex>
#include <vector>
#include <algorithm>

#define _LOCK_SCOPE(x) std::lock_guard<std::mutex> _hopefullyUnusedVariableName_(x);

class ValidObject;

class ValidObjectListener {
public:
    virtual void onDestroy(ValidObject *) = 0;
};

template <typename T>
class ValidPtr : public ValidObjectListener {
public:
    ValidPtr(T* instance) : mInstance(instance) {
        instance->subscribeValid(this);
    }

    ~ValidPtr() {
        _LOCK_SCOPE(busMutex);
        if (mInstance && valid)
            mInstance->unsubscribeValid(this);
    }

    /**
     * called by encapsulated class to notify of destruction
     */
    void onDestroy(ValidObject *) override {
        _LOCK_SCOPE(busMutex);
        valid = false;
    }

    inline T* get() {
        return mInstance;
    }

    /**
     * throw caller under bus if he calls on invalid ptr
     */
    inline T* operator->() {
        if (valid)
            return mInstance;
        else
            throw std::invalid_argument("Attempt to dereference an invalid pointer");
    }

    /**
     * @return indicator whether object is valid
     * Note: use with lock only
     */
    inline bool operator()() {
        return valid;
    }

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

class ValidObject {
public:
    virtual ~ValidObject() {
        notifyDestroy();
    }

    /**
     * Subscribes a ValidPtr to this object's validness
     * @param p ValidPtr to subscribe
     */
    void subscribeValid(ValidObjectListener *p) {
        _LOCK_SCOPE(_mmtxs);
        _mptrs.push_back(p);
    }

    /**
     * Unsubscribes a ValidPtr from this object's validness
     * @param p ValidPtr to unsubscribe
     */
    void unsubscribeValid(ValidObjectListener *p) {
        _LOCK_SCOPE(_mmtxs);
        // remove/erase idiom
        _mptrs.erase(std::remove(_mptrs.begin(), _mptrs.end(), p), _mptrs.end());
    }

protected:
    void notifyDestroy() {
        _LOCK_SCOPE(_mmtxs);
        for (auto p : _mptrs)
            p->onDestroy(this);

        _mptrs.clear();
    }

    std::vector<ValidObjectListener*> _mptrs;
    std::mutex                        _mmtxs;
};

#endif //COMMONS_VALIDPTR_H
