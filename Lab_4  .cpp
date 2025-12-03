#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <iomanip>

using namespace std;

class OptimizedDataStructure {
private:
    int m_data[3];
    mutable shared_mutex m_mutexes[3];

public:
    OptimizedDataStructure() {
        for (int i = 0; i < 3; ++i) m_data[i] = 0;
    }

    int read(int index) const {
        if (index < 0 || index >= 3) return -1;
        shared_lock<shared_mutex> lock(m_mutexes[index]);
        return m_data[index];
    }

    void write(int index, int value) {
        if (index < 0 || index >= 3) return;
        unique_lock<shared_mutex> lock(m_mutexes[index]);
        m_data[index] = value;
    }

    operator string() const {
        // Блокуємо всі м'ютекси у фіксованому порядку (0 -> 1 -> 2) щоб уникнути Deadlock
        shared_lock<shared_mutex> lock0(m_mutexes[0]);
        shared_lock<shared_mutex> lock1(m_mutexes[1]);
        shared_lock<shared_mutex> lock2(m_mutexes[2]);

        stringstream ss;
        ss << "[" << m_data[0] << ", " << m_data[1] << ", " << m_data[2] << "]";
        return ss.str();
    }
};

enum CommandType { CMD_READ, CMD_WRITE, CMD_STRING };

struct Command {
    CommandType type;
    int index;
    int value;
};

void worker(OptimizedDataStructure& ds, const vector<Command>& commands, int start, int end) {
    for (int i = start; i < end; ++i) {
        const auto& cmd = commands[i];
        switch (cmd.type) {
        case CMD_READ:
            ds.read(cmd.index);
            break;
        case CMD_WRITE:
            ds.write(cmd.index, cmd.value);
            break;
        case CMD_STRING:
        {
            string s = ds;
            volatile size_t len = s.length();
            (void)len;
        }
        break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <filename> <threads_count>" << endl;
        return 1;
    }

    string filename = argv[1];
    int thread_count = stoi(argv[2]);

    vector<Command> commands;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return 1;
    }

    string line, cmdStr;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        ss >> cmdStr;
        Command cmd;
        if (cmdStr == "read") {
            cmd.type = CMD_READ;
            ss >> cmd.index;
        }
        else if (cmdStr == "write") {
            cmd.type = CMD_WRITE;
            ss >> cmd.index >> cmd.value;
        }
        else if (cmdStr == "string") {
            cmd.type = CMD_STRING;
        }
        commands.push_back(cmd);
    }
    file.close();

    cout << "File loaded: " << commands.size() << " operations." << endl;
    cout << "Threads: " << thread_count << endl;

    OptimizedDataStructure ds;
    vector<thread> threads;

    int cmds_per_thread = commands.size() / thread_count;

    auto start_time = chrono::high_resolution_clock::now();

    for (int i = 0; i < thread_count; ++i) {
        int start_idx = i * cmds_per_thread;
        int end_idx = (i == thread_count - 1) ? commands.size() : (i + 1) * cmds_per_thread;
        threads.emplace_back(worker, ref(ds), ref(commands), start_idx, end_idx);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end_time - start_time;

    cout << "Time: " << fixed << setprecision(2) << duration.count() << " ms" << endl;
    cout << "-----------------------------------" << endl;

    return 0;
}