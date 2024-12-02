#include <iostream>
#include <vector>
#include "random/random.hpp"
using namespace std;

void printingVector(vector<int> list) {
    for (auto i = list.begin(); i != list.end(); i++) {
        cout << *i << endl;
    }
}

int main() {
    vector<int> data(10);
    for (int i = 1; i <= 10; i++) {
        data.push_back(i);
    }

    for (auto it = data.begin(); it != data.end(); it++) {
        if (*it % 2 == 0) {
            data.erase(it);
            it--;
        }
    }

    try {
        auto it = data.begin();
        data.erase(it+10);
    } catch (const std::exception &e) {
        cout << e.what() << endl;
    }

    printingVector(data);

    cout << "BEGINN" << endl;
}
