#include <unordered_map>
#include <vector>

#include "state.h"
#include "step.h"

class myMap {
    unordered_map<unsigned long long, char> Map;
public:

    bool find(State s){
        return Map.find(getKey(s)) != Map.end();
    }

    Step get(State s){
        return getStep(Map[getKey(s)]);
    }

    void update(State state, Step step){
        Map[getKey(state)] = getHash(step);
        return;
    }
};
