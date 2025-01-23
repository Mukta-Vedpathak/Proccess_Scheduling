#ifdef fifo_1
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct Process {
    int arrivalTime;
    vector<int> cpuBursts;
    vector<int> ioBursts;
    int currentCpuBurst;
    int currentIoBurst;
    int remainingTime;
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    bool operator<(const Process& other) const {
        return remainingTime > other.remainingTime;
    }
};

bool containsHtmlTag(const string& line) {
    return line.find("<html>") != string::npos ||
           line.find("<body>") != string::npos ||
           line.find("<pre>") != string::npos ||
           line.find("</html>") != string::npos ||
           line.find("</body>") != string::npos ||
           line.find("</pre>") != string::npos;
}

vector<Process> readInputFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        exit(1);
    }

    string line;
    stringstream cleanedData;
    
    while (getline(file, line)) {
        if (!containsHtmlTag(line)) {
            cleanedData << line << " ";
        }
    }
     
    file.close();

    stringstream ss(cleanedData.str());
    while (!ss.eof()) {
        Process p;
        int burst;
        ss >> p.arrivalTime;
        if (ss.fail()) break;

        p.cpuBursts.clear();
        p.ioBursts.clear();
        p.currentCpuBurst = 0;
        p.currentIoBurst = 0;
        p.remainingTime = 0;

        bool isCpuBurst = true;
        while (ss >> burst && burst != -1) {
            if (isCpuBurst) {
                p.cpuBursts.push_back(burst);
                p.remainingTime += burst;
            } else {
                p.ioBursts.push_back(burst);
            }
            isCpuBurst = !isCpuBurst;
        }

        p.completionTime = 0;
        p.waitingTime = 0;
        p.turnaroundTime = 0;
        processes.push_back(p);
    }

    return processes;
}

void printProcesses(const vector<Process>& processes) {
    for (size_t i = 0; i < processes.size(); ++i) {
        cout << "Process " << i + 1 << ":" << endl;
        cout << "  Arrival Time: " << processes[i].arrivalTime << endl;
        cout << "  CPU Bursts: ";
        for (int burst : processes[i].cpuBursts) {
            cout << burst << " ";
        }
        cout << endl;
        cout << "  IO Bursts: ";
        for (int burst : processes[i].ioBursts) {
            cout << burst << " ";
        }
        cout << endl;
        cout << "  Remaining Time: " << processes[i].remainingTime << endl;
        cout << "  Completion Time: " << processes[i].completionTime << endl;
        cout << "  Waiting Time: " << processes[i].waitingTime << endl;
        cout << "  Turnaround Time: " << processes[i].turnaroundTime << endl;
        cout << endl;
    }
}

void fifoScheduling(vector<Process>& processes) {
    int start_time=0;
    int currentTime = 0;
    int cpuRunTime = 0;
    queue<pair<Process*, bool>> readyQueue; 
    queue <Process> waitingQueue;
    queue <Process> runningQueue;
    vector<int> completionTimes;

    for (auto& p : processes) {
        readyQueue.push({&p, true});
    }

    while (!readyQueue.empty()) {
        auto currentEntry = readyQueue.front();
        Process* currentProcess = currentEntry.first;
        bool isCpuBurst = currentEntry.second;
        readyQueue.pop();

        if (currentProcess->arrivalTime > currentTime) {
            currentTime = currentProcess->arrivalTime;
        }

        if (isCpuBurst) {
            //cout<<" "<<currentTime<<endl;
            int cpuBurstTime = currentProcess->cpuBursts[currentProcess->currentCpuBurst++];
            start_time = currentTime;
            currentTime += cpuBurstTime;
            cpuRunTime += cpuBurstTime;

            cout <<"P" << (currentProcess - &processes[0]) << ","<< currentProcess->currentCpuBurst << "\t " << start_time << "\t " << currentTime -1  << endl;
            //cout<<" "<<currentTime<<endl;
            //cout << "Time " << start_time << ": Process " << (currentProcess - &processes[0]) << " starts CPU burst for " << cpuBurstTime << " units." << endl;

            if (currentProcess->currentCpuBurst <= currentProcess->ioBursts.size()) {
                readyQueue.push({currentProcess, false}); // Enqueue I/O burst
            }
            //cout<<" "<<currentTime;

        } else {
            int ioBurstTime = currentProcess->ioBursts[currentProcess->currentIoBurst++];
            //cout<<" "<<currentTime<<endl;
            
            if (currentProcess->currentIoBurst < currentProcess->cpuBursts.size()) {
                readyQueue.push({currentProcess, true});
            }
        }

        if (currentProcess->currentCpuBurst == currentProcess->cpuBursts.size() &&
            currentProcess->currentIoBurst == currentProcess->ioBursts.size()) {
            currentProcess->completionTime = currentTime;
            currentProcess->turnaroundTime = currentProcess->completionTime - currentProcess->arrivalTime;
            currentProcess->waitingTime = currentProcess->turnaroundTime - currentProcess->remainingTime;
            completionTimes.push_back(currentProcess->completionTime);
            //cout <<"P" << (currentProcess - &processes[0]) << ","<< currentProcess->currentCpuBurst << " " << start_time << " " << currentProcess->completionTime -1 << endl;
        }
    }

    double totalWaitingTime = 0, totalTurnaroundTime = 0;
    int maxWaitingTime = 0, maxCompletionTime = 0;
    for (auto& p : processes) {
        totalWaitingTime += p.waitingTime;
        totalTurnaroundTime += p.turnaroundTime;
        maxWaitingTime = max(maxWaitingTime, p.waitingTime);
        maxCompletionTime = max(maxCompletionTime, p.completionTime-p.arrivalTime);
    }

    double avgWaitingTime = totalWaitingTime / processes.size();
    double avgCompletionTime = totalTurnaroundTime / processes.size();
    int makespan = *max_element(completionTimes.begin(), completionTimes.end());

    cout << "Makespan: " << makespan << endl;
    cout << "Average Completion Time: " << avgCompletionTime << endl;
    cout << "Maximum Completion Time: " << maxCompletionTime << endl;
    cout << "Average Waiting Time: " << avgWaitingTime << endl;
    cout << "Maximum Waiting Time: " << maxWaitingTime << endl;
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>" << endl;
        return 1;
    }

    string algorithm = argv[1];
    string filename = argv[2];

    vector<Process> processes = readInputFile(filename);
    //printProcesses(processes);

    if (algorithm == "fifo") {
        fifoScheduling(processes);
    } else {
        cerr << "Unknown scheduling algorithm!" << endl;
        return 1;
    }

    return 0;
}

#endif

#ifdef sjf_1
#include <iostream>
#include <limits.h>

#include <fstream>

#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct Process {
     int id;
    int arrivalTime;
    vector<int> cpuBursts;
    vector<int> ioBursts;
    int currentCpuBurst = 0; 
    int currentIoBurst = 0;   
    int remainingTime = 0;    
    int completionTime = 0;
    int waitingTime = 0;
    int turnaroundTime = 0;
    bool isInIo = false;       
    int ioBurstTimeRemaining = 0; 
    bool operator<(const Process& other) const {
        return remainingTime > other.remainingTime; 
    }
};

bool containsHtmlTag(const string& line) {
    return line.find("<html>") != string::npos ||
           line.find("<body>") != string::npos ||
           line.find("<pre>") != string::npos ||
           line.find("</html>") != string::npos ||
           line.find("</body>") != string::npos ||
           line.find("</pre>") != string::npos;
}

vector<Process> readInputFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        exit(1);
    }

    string line;
    stringstream cleanedData;
    
    while (getline(file, line)) {
        if (!containsHtmlTag(line)) {
            cleanedData << line << " ";
        }
    }
     
    file.close();

    stringstream ss(cleanedData.str());
    int processID = 1;
    while (!ss.eof()) {
        Process p;
        int burst;
        ss >> p.arrivalTime;
        if (ss.fail()) break;

        p.id = processID++;
        p.cpuBursts.clear();
        p.ioBursts.clear();
        p.currentCpuBurst = 0;
        p.currentIoBurst = 0;
        p.remainingTime = 0;

        bool isCpuBurst = true;
        while (ss >> burst && burst != -1) {
            if (isCpuBurst) {
                p.cpuBursts.push_back(burst);
                p.remainingTime += burst;
            } else {
                p.ioBursts.push_back(burst);
            }
            isCpuBurst = !isCpuBurst;
        }

        p.completionTime = 0;
        p.waitingTime = 0;
        p.turnaroundTime = 0;
        processes.push_back(p);
    }

    return processes;
}

void printProcesses(const vector<Process>& processes) {
    for (size_t i = 0; i < processes.size(); ++i) {
        cout << "Process " << i + 1 << ":" << endl;
        cout << "  Arrival Time: " << processes[i].arrivalTime << endl;
        cout << "  CPU Bursts: ";
        for (int burst : processes[i].cpuBursts) {
            cout << burst << " ";
        }
        cout << endl;
        cout << "  IO Bursts: ";
        for (int burst : processes[i].ioBursts) {
            cout << burst << " ";
        }
        cout << endl;
        cout << "  Remaining Time: " << processes[i].remainingTime << endl;
        cout << "  Completion Time: " << processes[i].completionTime << endl;
        cout << "  Waiting Time: " << processes[i].waitingTime << endl;
        cout << "  Turnaround Time: " << processes[i].turnaroundTime << endl;
        cout << endl;
    }
}

void sjfScheduling(vector<Process>& processes) {
    int start_time=0;
    int currentTime = 0;
    int completed = 0;
    vector<int> completionTimes;

    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    while (completed < processes.size()) {
        for (auto& p : processes) {
            if (p.isInIo) {
                if (--p.ioBurstTimeRemaining == 0) {
                    p.isInIo = false;
                }
            }
        }

        int minBurstTime = INT_MAX;
        int selectedProcess = -1;

        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].isInIo) {
                if (processes[i].currentCpuBurst < processes[i].cpuBursts.size()) {
                    int burstTime = processes[i].cpuBursts[processes[i].currentCpuBurst];
                    if (burstTime < minBurstTime) {
                        minBurstTime = burstTime;
                        selectedProcess = i;
                    }
                }
            }
        }

        if (selectedProcess == -1) {
            currentTime++;
            continue;
        }

        Process& p = processes[selectedProcess];
        int burstTime = p.cpuBursts[p.currentCpuBurst++];
        start_time=currentTime;
        //cout << "Time " << currentTime << ": Process " << p.id << " starts CPU burst for " << burstTime << " units." << endl;

        currentTime += burstTime;
        p.remainingTime -= burstTime;
        cout << "P" << p.id << "," << p.currentCpuBurst << "\t"<< start_time<<  "\t" << start_time+burstTime<< endl;

        if (p.currentCpuBurst < p.cpuBursts.size()) {
            if (p.currentIoBurst < p.ioBursts.size()) {
                p.isInIo = true; 
                p.ioBurstTimeRemaining = p.ioBursts[p.currentIoBurst++];
                //cout << "P" << p.id << "," << p.currentCpuBurst << "\t"<< start_time<<  "\t" << start_time+currentTime+p.ioBurstTimeRemaining << endl;
            }
        }

        if (p.currentCpuBurst == p.cpuBursts.size()) {
            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - (p.remainingTime + burstTime);
            completionTimes.push_back(p.completionTime);
            completed++;

           // cout << "P" << p.id << "," << p.currentCpuBurst << "\t" << start_time << "\t" << start_time+burstTime << endl;
        }
    }

    double totalWaitingTime = 0, totalTurnaroundTime = 0;
    int maxWaitingTime = 0, maxCompletionTime = 0;
    for (auto& p : processes) {
        totalWaitingTime += p.waitingTime;
        totalTurnaroundTime += p.turnaroundTime;
        maxWaitingTime = max(maxWaitingTime, p.waitingTime);
        maxCompletionTime = max(maxCompletionTime, p.completionTime-p.arrivalTime);
    }

    double avgWaitingTime = totalWaitingTime / processes.size();
    double avgCompletionTime = totalTurnaroundTime / processes.size();
    int makespan = *max_element(completionTimes.begin(), completionTimes.end());

    cout << "Makespan: " << makespan << endl;
    cout << "Average Completion Time: " << avgCompletionTime << endl;
    cout << "Maximum Completion Time: " << maxCompletionTime << endl;
    cout << "Average Waiting Time: " << avgWaitingTime << endl;
    cout << "Maximum Waiting Time: " << maxWaitingTime << endl;
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>" << endl;
        return 1;
    }

    string algorithm = argv[1];
    string filename = argv[2];

    vector<Process> processes = readInputFile(filename);
    //printProcesses(processes);

    if (algorithm == "sjf") {
        sjfScheduling(processes);
    } else {
        cerr << "Unknown scheduling algorithm!" << endl;
        return 1;
    }

    return 0;
}


#endif

#ifdef psjf_1
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <numeric>
#include <queue>
#include <algorithm>

using namespace std;

struct Process {
    int id;
    int arrivalTime;
    vector<int> cpuBursts;     
    vector<int> ioBursts;      
    vector<int> remainingTimes; 
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    int currentCpuBurst;     
    int currentIoBurst;       
    int ioCompletionTime;      

    Process() : completionTime(0), waitingTime(0), turnaroundTime(0),
                currentCpuBurst(0), currentIoBurst(0), ioCompletionTime(0) {}

    int totalBurstTime() const {
        return accumulate(cpuBursts.begin(), cpuBursts.end(), 0);
    }
};

struct CompareRemainingTime {
    bool operator()(Process* p1, Process* p2) {
        return p1->remainingTimes[p1->currentCpuBurst] > p2->remainingTimes[p2->currentCpuBurst];
    }
};

bool containsHtmlTag(const string& line) {
    return line.find("<html>") != string::npos ||
           line.find("<body>") != string::npos ||
           line.find("<pre>") != string::npos ||
           line.find("</html>") != string::npos ||
           line.find("</body>") != string::npos ||
           line.find("</pre>") != string::npos;
}

vector<Process> readInputFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        exit(1);
    }

    string line;
    int processId = 1;
    while (getline(file, line)) {
        if (!containsHtmlTag(line) && !line.empty()) {
            stringstream ss(line);
            Process p;
            p.id = processId++;
            ss >> p.arrivalTime;
            int burst;

            bool isCpuBurst = true;
            while (ss >> burst && burst != -1) {
                if (isCpuBurst) {
                    p.cpuBursts.push_back(burst);
                    p.remainingTimes.push_back(burst);
                } else {
                    p.ioBursts.push_back(burst);
                }
                isCpuBurst = !isCpuBurst;
            }

            processes.push_back(p);
        }
    }

    return processes;
}

void calculateSRTF(vector<Process>& processes) {
    int n = processes.size(); 
    int time = 0;            
    int completed = 0;       

    vector<Process*> readyQueue; 
    vector<Process*> waitingQueue; 
    Process* runningProcess = nullptr; 
    int start_time = -1;
    int totalRunTime = 0;

    while (completed < n) {
        for (auto& process : processes) {
            if (process.arrivalTime == time) {
                readyQueue.push_back(&process);
            }
        }

        for (auto it = waitingQueue.begin(); it != waitingQueue.end();) {
            Process* process = *it;
            if (process->ioCompletionTime == time) {
                readyQueue.push_back(process); 
                it = waitingQueue.erase(it); 
            } else {
                ++it; 
            }
        }

        if (!readyQueue.empty()) {
            sort(readyQueue.begin(), readyQueue.end(), [](Process* p1, Process* p2) {
                return p1->remainingTimes[p1->currentCpuBurst] < p2->remainingTimes[p2->currentCpuBurst];
            });
        }

        if (runningProcess) {
            if (!readyQueue.empty() && 
                readyQueue.front()->remainingTimes[readyQueue.front()->currentCpuBurst] < 
                runningProcess->remainingTimes[runningProcess->currentCpuBurst]) {
                
                    cout << "P" << runningProcess->id << "," << runningProcess->currentCpuBurst + 1
                     << "\t" << start_time << "\t" << time - 1 << endl;

                start_time=time;

                readyQueue.push_back(runningProcess);
                runningProcess = nullptr; 
            }
        }

        if (!runningProcess && !readyQueue.empty()) {
            runningProcess = readyQueue.front(); 
            readyQueue.erase(readyQueue.begin()); 

            if (runningProcess->remainingTimes[runningProcess->currentCpuBurst] == 
                runningProcess->cpuBursts[runningProcess->currentCpuBurst]) {
                start_time = time; 
                if (runningProcess->currentCpuBurst == 0) {
                    runningProcess->waitingTime += (time - runningProcess->arrivalTime);
                } else {
                    runningProcess->waitingTime += (time - runningProcess->ioCompletionTime);
                }
            }
        }

        if (runningProcess) {
            runningProcess->remainingTimes[runningProcess->currentCpuBurst]--; 
            totalRunTime++; 
            if (runningProcess->remainingTimes[runningProcess->currentCpuBurst] == 0) {
                int ioBurstTime = 0; 
                if (runningProcess->currentIoBurst < runningProcess->ioBursts.size()) {
                    ioBurstTime = runningProcess->ioBursts[runningProcess->currentIoBurst];
                    runningProcess->ioCompletionTime = time + 1 + ioBurstTime; 
                    runningProcess->currentIoBurst++; 
                    waitingQueue.push_back(runningProcess); 
                } else {
                    runningProcess->completionTime = time + 1;
                    runningProcess->turnaroundTime = runningProcess->completionTime - runningProcess->arrivalTime;
                    runningProcess->waitingTime = runningProcess->turnaroundTime - runningProcess->totalBurstTime();
                }

                cout << "P" << runningProcess->id << "," << runningProcess->currentCpuBurst + 1
                     << "\t" << start_time << "\t" << time << endl;

                runningProcess->currentCpuBurst++; 

                if (runningProcess->currentCpuBurst < runningProcess->cpuBursts.size()) {
                    runningProcess->remainingTimes[runningProcess->currentCpuBurst] = 
                        runningProcess->cpuBursts[runningProcess->currentCpuBurst];
                } else {
                    completed++; 
                }

                runningProcess = nullptr; 
            }
        }

        time++; 
    }

    int makespan = 0, totalCompletionTime = 0, maxCompletionTime = 0;
    int totalWaitingTime = 0, maxWaitingTime = 0;

    for (const auto& process : processes) {
        makespan = max(makespan, process.completionTime); 
        totalCompletionTime += process.completionTime; 
        maxCompletionTime = max(maxCompletionTime, process.completionTime); 
        totalWaitingTime += process.waitingTime; 
        maxWaitingTime = max(maxWaitingTime, process.waitingTime); 
    }

    double avgCompletionTime = static_cast<double>(totalCompletionTime) / n;
    double avgWaitingTime = static_cast<double>(totalWaitingTime) / n;

    cout << "Makespan: " << makespan << endl;
    cout << "Completion Time (Average): " << avgCompletionTime << endl;
    cout << "Completion Time (Maximum): " << maxCompletionTime << endl;
    cout << "Waiting Time (Average): " << avgWaitingTime << endl;
    cout << "Waiting Time (Maximum): " << maxWaitingTime << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>" << endl;
        return 1;
    }

    string algorithm = argv[1];
    string filename = argv[2];

    vector<Process> processes = readInputFile(filename);

    if (algorithm == "srtf") {
        calculateSRTF(processes);
    } else {
        cerr << "Error: Unknown scheduling algorithm: " << algorithm << endl;
        return 1;
    }

    return 0;
}


#endif

#ifdef rr_1
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <numeric>
#include <queue>
#include <algorithm>

using namespace std;

struct Process {
    int id;
    int arrivalTime;
    vector<int> cpuBursts;
    vector<int> ioBursts;
    vector<int> remainingTimes;
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    int currentCpuBurst;
    int currentIoBurst;
    int ioCompletionTime;
    int remainingQuantum;

    Process() : completionTime(0), waitingTime(0), turnaroundTime(0),
                currentCpuBurst(0), currentIoBurst(0), ioCompletionTime(0), remainingQuantum(0) {}

    int totalBurstTime() const {
        return accumulate(cpuBursts.begin(), cpuBursts.end(), 0);
    }
};

bool containsHtmlTag(const string& line) {
    return line.find("<html>") != string::npos ||
           line.find("<body>") != string::npos ||
           line.find("<pre>") != string::npos ||
           line.find("</html>") != string::npos ||
           line.find("</body>") != string::npos ||
           line.find("</pre>") != string::npos;
}

vector<Process> readInputFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        exit(1);
    }

    string line;
    int processId = 1;
    while (getline(file, line)) {
        if (!containsHtmlTag(line) && !line.empty()) {
            stringstream ss(line);
            Process p;
            p.id = processId++;
            ss >> p.arrivalTime;
            int burst;

            bool isCpuBurst = true;
            while (ss >> burst && burst != -1) {
                if (isCpuBurst) {
                    p.cpuBursts.push_back(burst);
                    p.remainingTimes.push_back(burst);
                } else {
                    p.ioBursts.push_back(burst);
                }
                isCpuBurst = !isCpuBurst;
            }

            processes.push_back(p);
        }
    }

    return processes;
}

void printProcesses(const vector<Process>& processes) {
    cout << setw(10) << "Process ID"
         << setw(15) << "Arrival Time"
         << setw(20) << "Completion Time"
         << setw(20) << "Turnaround Time"
         << setw(15) << "Waiting Time" << endl;

    for (const auto& p : processes) {
        cout << setw(10) << p.id
             << setw(15) << p.arrivalTime
             << setw(20) << p.completionTime
             << setw(20) << p.turnaroundTime
             << setw(15) << p.waitingTime << endl;
    }
}

void calculateRoundRobin(vector<Process>& processes, int timeQuantum) {
    int start_time=0;
    int n = processes.size();
    int time = 0;
    int completed = 0;
    vector<int> completionTimes;

    queue<Process*> readyQueue;
    vector<Process*> waitingQueue;
    Process* runningProcess = nullptr;

    while (completed < n) {
        for (auto& process : processes) {
            if (process.arrivalTime == time) {
                readyQueue.push(&process);
                process.remainingQuantum = timeQuantum;
            }
        }

        for (auto it = waitingQueue.begin(); it != waitingQueue.end();) {
            Process* process = *it;
            if (process->ioCompletionTime == time) {
                readyQueue.push(process);
                process->remainingQuantum = timeQuantum;
                it = waitingQueue.erase(it);
            } else {
                ++it;
            }
        }

        if (!runningProcess && !readyQueue.empty()) {
            runningProcess = readyQueue.front();
            readyQueue.pop();
            start_time = time;
        }

        if (runningProcess) {
            runningProcess->remainingTimes[runningProcess->currentCpuBurst]--;
            runningProcess->remainingQuantum--;

            if (runningProcess->remainingTimes[runningProcess->currentCpuBurst] == 0) {
                runningProcess->currentCpuBurst++;
                cout << "P" << runningProcess->id <<"," << runningProcess->currentCpuBurst << "\t" << start_time<< "\t" << time + 1 << endl;

                if (runningProcess->currentIoBurst < runningProcess->ioBursts.size()) {
                    runningProcess->ioCompletionTime = time + 1 + runningProcess->ioBursts[runningProcess->currentIoBurst];
                    start_time = time;
                    runningProcess->currentIoBurst++;
                    waitingQueue.push_back(runningProcess);
                    runningProcess = nullptr;
                } else if (runningProcess->currentCpuBurst < runningProcess->cpuBursts.size()) {
                    runningProcess->remainingTimes[runningProcess->currentCpuBurst] = runningProcess->cpuBursts[runningProcess->currentCpuBurst];
                    readyQueue.push(runningProcess);
                    runningProcess = nullptr;
                } else {
                    runningProcess->completionTime = time + 1;
                    runningProcess->turnaroundTime = runningProcess->completionTime - runningProcess->arrivalTime;
                    runningProcess->waitingTime = runningProcess->turnaroundTime - runningProcess->totalBurstTime() - accumulate(runningProcess->ioBursts.begin(), runningProcess->ioBursts.end(), 0);
                    completionTimes.push_back(runningProcess->completionTime);
                    cout << "P" << runningProcess->id <<","<<runningProcess->currentCpuBurst<< "\t"<<start_time<<"\t" << time + 1 << endl;
                    completed++;
                    runningProcess = nullptr;
                }
            } else if (runningProcess->remainingQuantum == 0) {
                cout << "P" << runningProcess->id <<","<<runningProcess->currentCpuBurst << "\t"<<start_time<<"\t" << time + 1 << endl;
                if (runningProcess->remainingTimes[runningProcess->currentCpuBurst] > 0) {
                    readyQueue.push(runningProcess);
                    runningProcess->remainingQuantum = timeQuantum;
                }
                runningProcess = nullptr;
            }
        }

        time++;
    }

    double totalWaitingTime = 0, totalTurnaroundTime = 0;
    int maxWaitingTime = 0, maxCompletionTime = 0;
    for (auto& p : processes) {
        totalWaitingTime += p.waitingTime;
        totalTurnaroundTime += p.turnaroundTime;
        maxWaitingTime = max(maxWaitingTime, p.waitingTime);
        maxCompletionTime = max(maxCompletionTime, p.completionTime-p.arrivalTime);
    }

    double avgWaitingTime = totalWaitingTime / processes.size();
    double avgCompletionTime = totalTurnaroundTime / processes.size();
    int makespan = *max_element(completionTimes.begin(), completionTimes.end());

    cout << "Makespan: " << makespan << endl;
    cout << "Average Completion Time: " << avgCompletionTime << endl;
    cout << "Maximum Completion Time: " << maxCompletionTime << endl;
    cout << "Average Waiting Time: " << avgWaitingTime << endl;
    cout << "Maximum Waiting Time: " << maxWaitingTime << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <time-quantum> <path-to-workload-description-file>" << endl;
        return 1;
    }

    string algorithm = argv[1];
    int timeQuantum = stoi(argv[2]);
    string filename = argv[3];

    vector<Process> processes = readInputFile(filename);

    if (algorithm == "rr") {
        calculateRoundRobin(processes, timeQuantum);
    } else {
        cerr << "Error: Unknown scheduling algorithm: " << algorithm << endl;
        return 1;
    }

    return 0;
}


#endif

#ifdef fifo_2

#include<bits/stdc++.h>
#include<fstream>


using namespace std;

// Function to parse process details from an input file
vector<vector<int>> parse_process_data(const string &file_name) {
    ifstream infile(file_name);
    vector<vector<int>> process_data;
    string line;
    int process_id = 1;
    while (getline(infile, line)) {
        istringstream linestream(line);
        vector<int> process_info;
        int value;
        while (linestream >> value) {
            process_info.push_back(value);
        }
        if (!process_info.empty()) {
            process_info.push_back(process_id);  // Assign process ID at the end
            process_id++;
            process_data.push_back(process_info);
        }
    }
    infile.close();
    return process_data;
}

int main(int argc, char *argv[]) {

    string input_filename = argv[1];
    vector<vector<int>> processes = parse_process_data(input_filename);
    
    // Insert unique process ID at the start of each process entry
    for (int i = 0; i < processes.size(); i++) {
        processes[i].insert(processes[i].begin(), i + 1);
    }

    queue<vector<int>> ready_queue;
    ready_queue.push(processes[0]);  // Load the first process into the ready queue
    int cpu1_time = processes[0][1];  // Start time of CPU 1
    int cpu2_time = processes[0][1];  // Start time of CPU 2
    int process_counter = 1;
    bool cpu1_idle = true, cpu2_idle = true;
    vector<string> cpu1_schedule, cpu2_schedule;

    // For calculating scheduling metrics
    int total_duration = cpu1_time;
    vector<int> waiting_times(processes.size(), 0);
    vector<int> finish_times(processes.size(), 0);

    while (!ready_queue.empty() || process_counter < processes.size()) {
        if (ready_queue.empty() && process_counter < processes.size()) {
            ready_queue.push(processes[process_counter]);
            process_counter++;
        }

        if (!ready_queue.empty()) {
            vector<int> current_process = ready_queue.front();
            ready_queue.pop();
            int burst_index = 2;

            // Skip zero burst times until a valid burst is found
            while (burst_index < current_process.size() &&current_process[burst_index] == 0) {
                burst_index += 2;
            }
            if (burst_index >= current_process.size())
                continue;

            int selected_burst = burst_index / 2;
            int cpu_selected = 0; 
            stringstream schedule_log;

            // Assign to CPU 1 if available and less busy than CPU 2
            if (cpu1_idle && (cpu1_time<=cpu2_time)) {
                cpu_selected = 1;
                schedule_log << "P" << current_process[0] << "," << selected_burst << "  "
                             << cpu1_time << "  " << cpu1_time + current_process[burst_index];
                
                // Calculate waiting time and update it
                if (cpu1_time > current_process[1]) {
                    waiting_times[current_process[0] - 1] += (cpu1_time - current_process[1]);
                }

                finish_times[current_process[0] - 1] = cpu1_time + current_process[burst_index];
                cpu1_time += current_process[burst_index];
                cpu1_idle = false;
                cpu1_schedule.push_back(schedule_log.str());
            }
            // Otherwise, assign to CPU 2
            else if (cpu2_idle) {
                cpu_selected = 2;
                schedule_log << "P" << current_process[0] << "," << selected_burst
                             << "  " << cpu2_time << "  " << cpu2_time + current_process[burst_index];

                if (cpu2_time > current_process[1]) {
                    waiting_times[current_process[0] - 1] += (cpu2_time - current_process[1]);
                }

                finish_times[current_process[0] - 1] = cpu2_time + current_process[burst_index];
                cpu2_time += current_process[burst_index];
                cpu2_idle = false;
                cpu2_schedule.push_back(schedule_log.str());
            }

            current_process[burst_index] = 0;  // Mark this burst as completed

            // Load additional processes into the ready queue if their arrival time is valid
            while (process_counter < processes.size()) {
                if (processes[process_counter][1] <= min(cpu1_time, cpu2_time)) {
                    ready_queue.push(processes[process_counter]);
                    process_counter++;
                } else {
                    break;
                }
            }

            // Reinsert into queue if there are more bursts (I/O wait) remaining
            if (burst_index+1< current_process.size() &&current_process[burst_index + 1] !=-1) {
                ready_queue.push(current_process);
            }

            // Mark the corresponding CPU as available after executing
            if (cpu_selected == 1) cpu1_idle = true;
            if (cpu_selected == 2) cpu2_idle = true;
        }
    }

    // Calculate makespan (total runtime) of all processes
    total_duration = max(cpu1_time, cpu2_time) - total_duration;

    // Compute metrics
    int total_waiting_time = 0;
    for(auto time : waiting_times){
        total_waiting_time += time;
    }

    int total_finish_time = 0;
    for(auto time : finish_times){
        total_finish_time += time;
    }

    // Output the CPU schedules
    cout << "CPU1 Schedule:\n";
    for (const auto &entry : cpu1_schedule) {
        cout << entry << endl;
    }

    cout << "\n\nCPU2 Schedule:\n";
    for (const auto &entry : cpu2_schedule) {
        cout << entry << endl;
    }

    // Output calculated metrics
    cout << endl;
    printf("MakeSpan: %d\n", total_duration);
    printf("Avg waiting time: %f\n", (double)total_waiting_time / (double)processes.size());
    printf("Avg completion time: %f\n", (double)total_finish_time / (double)processes.size());


    return 0;
}

#endif

#ifdef sjf_2

#include <bits/stdc++.h>

#include <fstream>

using namespace std;

// Function to load process data from the file into a vector of vectors
vector<vector<int>> load_process_data(string file_path) {
    ifstream file(file_path);
    vector<vector<int>> process_data;
    string line;
    int process_id = 1;

    // Reading each line and parsing into a list of integers
    while (getline(file, line)) {
        istringstream iss(line);
        vector<int> values;
        int value;

        while (iss >> value) {
            values.push_back(value);
        }

        if (!values.empty()) {
            values.insert(values.begin(), process_id);  // Prepend process ID
            process_id++;
            process_data.push_back(values);
        }
    }

    file.close();
    return process_data;
}

// Comparator to prioritize processes based on their remaining burst time
struct CompareBurstTime {
    bool operator()(const vector<int>& p1, const vector<int>& p2) {
        int i1 = 2, i2 = 2;
        while (i1 < p1.size() && p1[i1] == 0) i1 += 2;
        while (i2 < p2.size() && p2[i2] == 0) i2 += 2;

        if (i1 >= p1.size() || i2 >= p2.size()) return false;

        return p1[i1] > p2[i2];  // Process with smaller burst time gets priority
    }
};

// Comparator to prioritize processes based on their arrival time
struct CompareArrivalTime {
    bool operator()(const vector<int>& p1, const vector<int>& p2) {
        return p1[1] > p2[1];  // Process with earlier arrival time gets priority
    }
};

int main(int argc, char *argv[]) {
    string file_path = argv[1];
    vector<vector<int>> process_list = load_process_data(file_path);

    // Priority queues to manage waiting and ready queues
    priority_queue<vector<int>, vector<vector<int>>, CompareArrivalTime> waiting_queue(
        process_list.begin(), process_list.end());
    priority_queue<vector<int>, vector<vector<int>>, CompareBurstTime> ready_queue;

    vector<string> log_cpu0, log_cpu1;
    int cpu1_time = waiting_queue.top()[1];  // Initial time for CPU 1
    int cpu2_time = waiting_queue.top()[1];  // Initial time for CPU 2
    int simulation_start = cpu1_time;

    ready_queue.push(waiting_queue.top());
    waiting_queue.pop();

    vector<int> wait_times(process_list.size(), 0);
    vector<int> exec_times(process_list.size(), 0);

    for (const auto& process : process_list) {
        exec_times.push_back(process[1]);
    }

    while (!ready_queue.empty() || !waiting_queue.empty()) {
        int index = 2;

        // If ready queue is not empty, fetch the next process for execution
        if (!ready_queue.empty()) {
            vector<int> current_process = ready_queue.top();
            ready_queue.pop();

            while (index < current_process.size() && current_process[index] == 0) index += 2;

            if (index >= current_process.size()) continue;

            stringstream output;
            int selected_cpu = 0;
            int burst_index = index / 2;

            // Assign process to CPU1 or CPU2 based on availability
            if (cpu1_time<= cpu2_time) {
                selected_cpu=1;
                output << "P" << current_process[0] << "," << burst_index;
                output << " " << cpu1_time << " " << cpu1_time + current_process[index];

                wait_times[current_process[0] - 1] += (cpu1_time - current_process[1]);
                cpu1_time += current_process[index] + 1;  // Include 1 unit overhead time
                log_cpu0.push_back(output.str());
            } else {
                selected_cpu = 2;
                output << "P" << current_process[0] << "," << burst_index;
                output << " " << cpu2_time << " " << cpu2_time + current_process[index];

                wait_times[current_process[0] - 1] += (cpu2_time - current_process[1]);
                cpu2_time += current_process[index] + 1;  // Include 1 unit overhead time
                log_cpu1.push_back(output.str());
            }

            current_process[index] = 0;

            // If the process has further phases, reinsert it into the waiting queue
            if (current_process[index + 1]!=-1 &&current_process[index + 2] != -1) {
                current_process[1] =max(cpu1_time, cpu2_time) + current_process[index +1];
                waiting_queue.push(current_process);
            } else {
                exec_times[current_process[0] - 1] = max(cpu1_time, cpu2_time) - exec_times[current_process[0] - 1];
            }
        } else {
            // If no processes are ready, move time forward for both CPUs
            cpu2_time++;
            cpu1_time++;
        }

        // Load processes into ready queue when arrival time is less than or equal to CPU times
        while (!waiting_queue.empty() && min(cpu1_time, cpu2_time) >= waiting_queue.top()[1]) {
            ready_queue.push(waiting_queue.top());
            waiting_queue.pop();
        }
    }

    // Calculate performance metrics
    int makespan = max(cpu1_time, cpu2_time) - simulation_start;
    int total_waiting_time = 0;
    for(auto time : wait_times){
        total_waiting_time += time;
    }

    int total_finish_time = 0;
    for(auto time : exec_times){
        total_finish_time += time;
    }



    // Output logs for both CPUs
    cout << "CPU 0 Log:\n";
    for (const string& log : log_cpu0) {
        cout << log << endl;
    }

    cout << "\nCPU 1 Log:\n";
    for (const string& log : log_cpu1) {
        cout << log << endl;
    }

    // Output performance metrics
    cout << endl;
    printf("Makespan: %d\n", makespan);
    printf("Average Waiting Time: %f\n", (double)total_waiting_time/ process_list.size());
    printf( "Average Execution Time: %f\n", (double)total_finish_time/ process_list.size());

    return 0;
}


#endif

#ifdef psjf_2

#include <bits/stdc++.h>
#include <fstream>
using namespace std;

// Function to read data from the file and return a 2D vector
vector<vector<int>> read_file(string filename) {
    ifstream file(filename);
    vector<vector<int>> data;
    string line;
    int process_id = 1;

    while (getline(file, line)) {
        istringstream lineStream(line);
        vector<int> numbers;
        int number;

        while (lineStream >> number) {
            numbers.push_back(number);
        }

        if (!numbers.empty()) {
            numbers.insert(numbers.begin(), process_id); // Prepend process ID
            process_id++;
            data.push_back(numbers);
        }
    }

    file.close();
    return data;
}

// Comparator to compare burst times
struct CompareBurstTime {
    bool operator()(vector<int> &p1, vector<int> &p2) {
        int idx1 = 2;
        while (idx1 < p1.size() && p1[idx1] == 0) idx1 += 2;

        int idx2 = 2;
        while (idx2 < p2.size() && p2[idx2] == 0) idx2 += 2;

        if (idx1 >= p1.size() || idx2 >= p2.size()) return false;

        return p1[idx1] > p2[idx2];
    }
};

// Comparator to compare arrival times
struct CompareArrivalTime {
    bool operator()(vector<int> &p1, vector<int> &p2) {
        return p1[1] > p2[1]; // Order by arrival time
    }
};

int main(int argc, char *argv[]) {
    
    string filename = argv[1];
    vector<vector<int>> processes = read_file(filename);

    // Initializing priority queues for waiting and ready states
    priority_queue<vector<int>, vector<vector<int>>, CompareArrivalTime> wait_queue(processes.begin(), processes.end());
    priority_queue<vector<int>, vector<vector<int>>, CompareBurstTime> ready_queue;

    vector<string> output_cpu0, output_cpu1;

    // Initialize start and current times for both processors
    int cpu_time1 = wait_queue.top()[1];
    int cpu_time2 = wait_queue.top()[1];

    int start_time1 = cpu_time1;
    int start_time2 = cpu_time2;

    int process_id1 = -1, process_id2 = -1;
    int _burst1 = 0, _burst2 = 0;

    // Tracking waiting and running times
    vector<int> waiting_time(processes.size(), 0);
    vector<int> running_time(processes.size(), 0);
    vector<int> start_times(processes.size(), -1);
    int total_wait_time = 0, total_run_time = 0;

    // Scheduling loop
    while (!ready_queue.empty() || !wait_queue.empty()) {
        // Move processes to ready queue based on arrival times
        while (!wait_queue.empty() && (cpu_time1 >= wait_queue.top()[1] || cpu_time2 >= wait_queue.top()[1])) {
            ready_queue.push(wait_queue.top());
            wait_queue.pop();
        }

        int index = 2;
        stringstream output;
        
        // CPU1 executes next process
        if (!ready_queue.empty() && (cpu_time1 <= cpu_time2 || process_id2 == -1)) {
            vector<int> current_process = ready_queue.top();
            ready_queue.pop();

            while (index < current_process.size() && current_process[index] == 0) index += 2;
            if (index >= current_process.size()) continue;

            if (process_id1 != current_process[0] || _burst1 != index / 2) {
                if (process_id1 != -1) {
                    output << "P" << process_id1 << "," << _burst1 << " " << start_time1 << " " << cpu_time1 << endl;
                    output_cpu0.push_back(output.str());
                }

                start_time1 = cpu_time1;
                process_id1 = current_process[0];
                _burst1 = index / 2;
            }

            if (start_times[process_id1 - 1] == -1) {
                start_times[process_id1 - 1] = cpu_time1;
                waiting_time[process_id1 - 1] = start_time1 - current_process[1];
            }

            current_process[index]--;
            cpu_time1++;
            running_time[process_id1 - 1]++;

            if (current_process[index] > 0) {
                current_process[1] = cpu_time1;
                ready_queue.push(current_process);
            } else if (current_process[index + 1] != -1 && current_process[index + 2] != -1) {
                current_process[1] = cpu_time1 + current_process[index + 1];
                wait_queue.push(current_process);
            }
        }

        // CPU2 executes next process
        if (!ready_queue.empty() && (cpu_time2 < cpu_time1 || process_id1 == -1)) {
            vector<int> current_process = ready_queue.top();
            ready_queue.pop();

            while (index < current_process.size() && current_process[index] == 0) index += 2;
            if (index >= current_process.size()) continue;

            if (process_id2 != current_process[0] || _burst2 != index / 2) {
                if (process_id2 != -1) {
                    output << "P" << process_id2 << "," << _burst2 << " " << start_time2 << " " << cpu_time2 << endl;
                    output_cpu1.push_back(output.str());
                }

                start_time2 = cpu_time2;
                process_id2 = current_process[0];
                _burst2 = index / 2;
            }

            if (start_times[process_id2 - 1] == -1) {
                start_times[process_id2 - 1] = cpu_time2;
                waiting_time[process_id2 - 1] = start_time2 - current_process[1];
            }

            current_process[index]--;
            cpu_time2++;
            running_time[process_id2 - 1]++;

            if (current_process[index] > 0) {
                current_process[1] = cpu_time2;
                ready_queue.push(current_process);
            } else if (current_process[index + 1] != -1 && current_process[index + 2] != -1) {
                current_process[1] = cpu_time2 + current_process[index + 1];
                wait_queue.push(current_process);
            }
        }

        // Handle idle CPUs
        if (ready_queue.empty()) {
            if (!wait_queue.empty()) {
                int next_arrival = wait_queue.top()[1];
                cpu_time1 = max(cpu_time1, next_arrival);
                cpu_time2 = max(cpu_time2, next_arrival);
            }
        }
    }

    // Output remaining processes from each CPU
    if (process_id1 != -1) {
        stringstream output;
        output << "P" << process_id1 << "," << _burst1 << " " << start_time1 << " " << cpu_time1 << endl;
        output_cpu0.push_back(output.str());
    }
    if (process_id2 != -1) {
        stringstream output;
        output << "P" << process_id2 << "," << _burst2 << " " << start_time2 << " " << cpu_time2 << endl;
        output_cpu1.push_back(output.str());
    }

    // Calculate makespan and other metrics
    int makespan = max(cpu_time1, cpu_time2);
    double avg_wait_time = accumulate(waiting_time.begin(), waiting_time.end(), 0.0) / waiting_time.size();
    double avg_run_time = accumulate(running_time.begin(), running_time.end(), 0.0) / running_time.size();

    // Output for both CPUs
    cout << "CPU0\n" << endl;
    for (const string &out : output_cpu0) cout << out;
    cout << "\nCPU1\n" << endl;
    for (const string &out : output_cpu1) cout << out;

    // Print metrics
    printf( "\nMakespan: %d\n",  makespan);
    printf("Average Waiting Time: %f\n", avg_wait_time);
    printf("Average Running Time: %f\n", avg_run_time);

    return 0;
}

#endif

#ifdef rr_2
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include<bits/stdc++.h>
#include<fstream>
#include <set>
using namespace std;

struct process {
    int id;
    int arrival_time;
    vector<int> cpu_bursts;
    vector<int> io_bursts;
    int cpu_idx = 0;
    int io_idx = 0;
    int completion_time = 0;
    int turn_around_time = 0;
    int waiting_time = 0;
    int remaining_time = 0;
    int remaining_wait = 0; // For I/O burst wait tracking

    process(int _id, int _arrival_time) {
        id = _id;
        arrival_time = _arrival_time;
    }
};



int read_dat_file(string filename, vector<process>& process_vector, vector<int>& arrival_time_vector){

    int process_counter = 0;
    ifstream file;
    file.open(filename);// <process-arrival-time> <cpu-burst-1-duration> <io-burst-1-duration> <cpu-burst-2-duration> <io-burst-2-duration> … -1
    string  line;
    while(getline(file, line)){
        if(!line.find("<html>")){continue;}
        if(!line.find("<pre>")){continue;}
        if(!line.find("<body>")){continue;}
        if(!line.find("</pre></body></html>")){continue;}

        // defining new process
        process new_process(process_counter, 0);
        // content inside pre-tags
        istringstream data(line);
        int number, index = 0, remainTime = 0;
        vector<int> cpu, io;
        while(data >> number){
            if(index == 0){
                new_process.arrival_time = number;
                arrival_time_vector.push_back(number);
            }else{
                if(index % 2 == 1){
                    new_process.cpu_bursts.push_back(number);
                    remainTime += number;
                } else {
                    new_process.io_bursts.push_back(number);
                }
            }
            index++;
        }
        new_process.remaining_time = remainTime;
        process_vector.push_back(new_process);
        process_counter++;
    }
    return process_counter;
    
}



// Round Robin with two processor cores
void round_robin(vector<process>& process_vector, int time_quantum) {
    int currentTime = 0;
    int completedProcesses = 0;
    queue<process*> readyQueue;
    vector<process*> waitingQueue; // To handle I/O operations
    vector<process*> processors(2, nullptr); // Simulating two processor cores
    set<int> visited;

    // Sort processes by arrival time initially
    sort(process_vector.begin(), process_vector.end(), [](const process& a, const process& b) {
        return a.arrival_time < b.arrival_time;
    });

    while (completedProcesses < process_vector.size()) {
        // Add processes to the ready queue if they have arrived
        for (int i = 0; i < process_vector.size(); i++) {
            if (visited.find(i) == visited.end() && process_vector[i].arrival_time <= currentTime) {
                readyQueue.push(&process_vector[i]);
                visited.insert(i);
            }
        }

        // Assign processes to free processors if available
        for (int i = 0; i < 2; i++) {
            if (processors[i] == nullptr && !readyQueue.empty()) {
                processors[i] = readyQueue.front();
                readyQueue.pop();
            }
        }

        // Simulate execution on the processors
        for (int i = 0; i < 2; i++) {
            if (processors[i] != nullptr) {
                process* currentProcess = processors[i];

                // Execute for time quantum or until the end of the current CPU burst
                int remainingBurstTime = currentProcess->cpu_bursts[currentProcess->cpu_idx];
                int executionTime = min(time_quantum, remainingBurstTime);
                currentProcess->cpu_bursts[currentProcess->cpu_idx] -= executionTime;
                currentTime += executionTime;

                // If CPU burst is finished, move the process to I/O
                if (currentProcess->cpu_bursts[currentProcess->cpu_idx] == 0) {
                    currentProcess->cpu_idx++;
                    
                    if (currentProcess->cpu_idx == currentProcess->cpu_bursts.size()) {
                        // If all CPU bursts are finished, mark the process as completed
                        currentProcess->completion_time = currentTime;
                        currentProcess->turn_around_time = currentProcess->completion_time - currentProcess->arrival_time;
                        currentProcess->waiting_time = currentProcess->turn_around_time - currentProcess->remaining_time;
                        completedProcesses++;
                        cout << "Process " << currentProcess->id + 1 << " completes at time " << currentTime << "." << endl;
                        processors[i] = nullptr;
                    } else {
                        // Process moves to I/O wait
                        int burstTime = currentProcess->io_bursts[currentProcess->io_idx++];
                        if (burstTime != -1) {
                            currentProcess->remaining_wait = currentTime;
                            waitingQueue.push_back(currentProcess);
                        }
                        processors[i] = nullptr;
                    }
                } else {
                    // If the CPU burst is not done, move it to the back of the ready queue
                    readyQueue.push(currentProcess);
                    processors[i] = nullptr;
                }
            }
        }

        // Manage the waiting queue for I/O completion
        if (!waitingQueue.empty()) {
            for (auto it = waitingQueue.begin(); it != waitingQueue.end();) {
                process* proc = *it;
                if (currentTime - proc->remaining_wait >= proc->io_bursts[proc->io_idx - 1]) {
                    // Move process back to the ready queue after I/O completion
                    readyQueue.push(proc);
                    it = waitingQueue.erase(it); // Remove from the waiting queue
                } else {
                    ++it;
                }
            }
        }

        // Fast-forward time if all queues and processors are idle
        if (readyQueue.empty() && processors[0] == nullptr && processors[1] == nullptr && !waitingQueue.empty()) {
            currentTime = waitingQueue[0]->remaining_wait + waitingQueue[0]->io_bursts[waitingQueue[0]->io_idx - 1];
        }
    }

    // Output results
    double totalWaitingTime = 0, totalTurnAroundTime = 0;
    for (auto& p : process_vector) {
        totalWaitingTime += p.waiting_time;
        totalTurnAroundTime += p.turn_around_time;
    }

    cout << "Round Robin Results" << endl;
    cout << "Average waiting time: " << totalWaitingTime / process_vector.size() << endl;
    cout << "Average turn around time: " << totalTurnAroundTime / process_vector.size() << endl;
    cout << "MakeSpan: " << currentTime << endl;
}

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>" << endl;
        return 1;
    }

    int time_quantum = stoi(argv[1]);
    string filename = argv[2];

   

    vector<process> process_vector;
    vector<int> arrival_time_vector;
    // Define a time quantum
    
    int number_of_processes = read_dat_file(filename, process_vector, arrival_time_vector);;
    //int time_quantum = 20;

    // Call Round Robin scheduling
    round_robin(process_vector, time_quantum);

    return 0;
}

#endif