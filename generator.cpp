#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <string>

using namespace std;

void generate_file(const string& filename, int count, const vector<double>& weights) {
    ofstream file(filename);
    random_device rd;
    mt19937 gen(rd());

    discrete_distribution<> d(weights.begin(), weights.end());
    uniform_int_distribution<> val_dist(1, 100);

    for (int i = 0; i < count; ++i) {
        int op = d(gen);
        switch (op) {
        case 0: file << "read 0\n"; break;
        case 1: file << "write 0 " << val_dist(gen) << "\n"; break;
        case 2: file << "read 1\n"; break;
        case 3: file << "write 1 " << val_dist(gen) << "\n"; break;
        case 4: file << "read 2\n"; break;
        case 5: file << "write 2 " << val_dist(gen) << "\n"; break;
        case 6: file << "string\n"; break;
        }
    }
    file.close();
    cout << "Generated " << filename << endl;
}

int main() {
    int N = 500000;

    vector<double> weights_a = { 20, 5, 20, 5, 20, 5, 25 };
    generate_file("test_a.txt", N, weights_a);

    vector<double> weights_b = { 1, 1, 1, 1, 1, 1, 1 };
    generate_file("test_b.txt", N, weights_b);

    vector<double> weights_c = { 1, 90, 1, 2, 1, 2, 3 };
    generate_file("test_c.txt", N, weights_c);S

    return 0;
}