#include <queue>
#include <vector>

#include "state.h"

template<typename T,
         typename Sequence = std::vector<T> >
class myQueue : public std::priority_queue<T, Sequence, myComparison> {
public:

    bool remove(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                this->c.erase(it);
                std::make_heap(this->c.begin(), this->c.end(), this->comp);
                return true;
            }
        }
        return false;
    }

    // Add by Louie Fu
    bool find(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                return true;
            }
        }
        return false;
    }

    T getState(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                return *it;
            }
        }
        return value;
    }
    

    void update(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                *it = value;
                std::make_heap(this->c.begin(), this->c.end(), this->comp);
                return;
            }
        }

        return;
    }

};
