#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
using namespace std;
using namespace chrono;
class DataProcessor {
private:
    vector<int> datas;
public:
    DataProcessor() {
        datas.clear();
    }
    int GetDataSize() { return datas.size();}
    vector<int> GetDatas() {return datas;}
    int* GetDatasPtr() {return datas.data();}
    int GetData(int index) { return datas[index]; }
    bool Readfile(string filename) {
        ifstream infile;
        infile.open(filename);
        if (!infile.is_open()) {
            cerr << "error\n";
            return false;
        }
        for (int value; infile >> value; ) {
            datas.push_back(value);
        }
    
        infile.close();
        return true;

    }
    void SaveFile(string filename, string method, double time) {
        ofstream outfile;
        filename.erase(filename.size() - 4);
        outfile.open(filename + "_output" + method + ".txt");
        outfile << "Sort : \n";
        for (int data : datas) {
            outfile << data << "\n";
        }
        outfile << fixed << setprecision(15);
        outfile << "CPU Time : " << time << "\n";
    
        system_clock::time_point now = system_clock::now();

  
        time_t now_time_t = system_clock::to_time_t(now);
        microseconds now_us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

        outfile << "Output Time : "
                << put_time(localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") 
                << "." << setw(6) << setfill('0') << now_us.count()
                << "+08:00";      

        outfile.close();
        return;
    }       
    void BubbleSort( int start, int end) {
        for (int i = start; i <= end; i++)
            for (int j = start; j < end - (i - start); j++) 
                if (datas[j] > datas[j + 1])
                    swap(datas[j], datas[j + 1]);
    }
    
    void BubbleSortArray(int* arr, int start, int end) {
        for (int i = start; i <= end; i++)
            for (int j = start; j < end - (i - start); j++) 
                if (arr[j] > arr[j + 1])
                    swap(arr[j], arr[j + 1]);
    }    

    void Merge(int left, int mid, int right) {
        vector<int> temp;
        int firstptr = left;
        int secondptr = mid + 1;
        while (firstptr <= mid && secondptr <= right) {
            if (datas[firstptr] <= datas[secondptr]) {
                temp.push_back(datas[firstptr]);
                firstptr++;
            } else {
                temp.push_back(datas[secondptr]);
                secondptr++;
            }
        }
        for (; firstptr <= mid; firstptr++) {
            temp.push_back(datas[firstptr]);
        }
        for (; secondptr <= right; secondptr++) {
            temp.push_back(datas[secondptr]);   
        }
        for (int i = 0; i < temp.size(); i++) {
            datas[left + i] = temp[i];
        }
    }
    void MergeArray(int *arr, int left, int mid, int right) {
        vector<int> temp;
        int firstptr = left;
        int secondptr = mid + 1;
        while (firstptr <= mid && secondptr <= right) {
            if (arr[firstptr] <= arr[secondptr]) {
                temp.push_back(arr[firstptr]);
                firstptr++;
            } else {
                temp.push_back(arr[secondptr]);
                secondptr++;
            }
        }
        for (; firstptr <= mid; firstptr++) {
            temp.push_back(arr[firstptr]);
        }
        for (; secondptr <= right; secondptr++) {
            temp.push_back(arr[secondptr]);   
        }
        for (int j = 0; j < temp.size(); j++) {
            arr[left + j] = temp[j];
        }
    }
    void MergeSort(vector<pair<int, int>>& segments) {
        if (segments.size() <= 1) return;
        vector<pair<int, int>> nextLevel;
        for (int i = 0; i < segments.size(); i += 2) {
            if (i < segments.size() - 1) {
                int left = segments[i].first;
                int mid = segments[i].second;
                int right = segments[i + 1].second;
                Merge(left, mid, right);
                nextLevel.push_back({left, right});
            } else {
                nextLevel.push_back(segments[i]);
            }
        }
        MergeSort(nextLevel);

    
    }
    void MergeSortArray(int *arr, vector<pair<int, int>>& segments) {
        if (segments.size() <= 1) return;
        vector<pair<int, int>> nextLevel;
        vector<pid_t> pids;
        for (int i = 0; i < segments.size(); i += 2) {
            if (i < segments.size() - 1) {
                int left = segments[i].first;
                int mid = segments[i].second;
                int right = segments[i + 1].second;

                pid_t pid = fork();
                if (pid == 0) {
                    
                    MergeArray(arr, left, mid, right);
                    exit(0);
                } else if (pid > 0) {
                    
                    pids.push_back(pid);
                    nextLevel.push_back({left, right});
                } else {
                    cerr << "Fork failed\n";
                    exit(1);
                }
            } else {
                nextLevel.push_back(segments[i]);
            }
        }

        for (pid_t pid : pids) waitpid(pid, nullptr, 0);

        MergeSortArray(arr, nextLevel);
    }

};
int main() {
    DataProcessor dp;
    string filename;

    do {
        cout << "請輸入檔案名稱：\n";
        cin >> filename;
    } while(!dp.Readfile(filename));
    
    int slices;
    cout << "請輸入要切成幾份：\n";
    cin >> slices;

    string method;
    cout << "請輸入方法編號：(方法1, 方法2, 方法3, 方法4)\n";
    cin >> method;

    clock_t start, end;
    
 
    if (method == "1") {
        auto start = high_resolution_clock::now();
        dp.BubbleSort(0, dp.GetDataSize() - 1);
        auto end = high_resolution_clock::now();
        double t = duration<double, milli>(end - start).count();

        dp.SaveFile(filename, method, t);
    } else if (method == "2") {
        auto start = high_resolution_clock::now();
        int sliceLength = dp.GetDataSize() / slices;
        vector<pair<int, int>> segments;
        for (int i = 0; i < slices; i++) {
            int startIndex = i * sliceLength;
            int endIndex = (i == slices - 1) ? dp.GetDataSize() - 1 : (startIndex + sliceLength - 1);
            segments.push_back({startIndex, endIndex});
            dp.BubbleSort(startIndex, endIndex);
        }
        dp.MergeSort(segments);
        auto end = high_resolution_clock::now();
        double t = duration<double, milli>(end - start).count();
        dp.SaveFile(filename, method, t);
    } else if (method == "3") {
        auto start = high_resolution_clock::now();
        int sliceLength = dp.GetDataSize() / slices;
        vector<pid_t> pids;
        vector<pair<int, int>> segments;
        int *shared = (int*)mmap(
            NULL,
            dp.GetDataSize() * sizeof(int),
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS,
            -1,
            0
        );
        if (shared == MAP_FAILED) {
            cerr << "mmap failed\n";
            exit(1);
        }
        memcpy(shared, dp.GetDatasPtr(), dp.GetDataSize() * sizeof(int));
        for (int i = 0; i < slices; i++) {
            int startIndex = i * sliceLength;
            int endIndex = (i == slices - 1) ? (dp.GetDataSize() - 1) : (startIndex + sliceLength - 1);
            pid_t pid = fork();
            
            // child process
            if (pid == 0) {
                dp.BubbleSortArray(shared, startIndex, endIndex);
                exit(0);
            } 
            // parent process
            else if (pid > 0) {
                pids.push_back(pid);
                segments.push_back({startIndex, endIndex});
            } else {
                cerr << "Fork failed!\n";
                exit(1);
            }
        }

        

        for (pid_t pid : pids) waitpid(pid, nullptr, 0);

        dp.MergeSortArray(shared, segments);
        memcpy(dp.GetDatasPtr(), shared, dp.GetDataSize()* sizeof(int));
        
        auto end = high_resolution_clock::now();
        double t = duration<double, milli>(end - start).count();
        dp.SaveFile(filename, method, t);
    } else if (method == "4") {

        auto start = high_resolution_clock::now();
        int sliceLength = dp.GetDataSize() / slices;
        vector<thread> threads;
        vector<pair<int, int>> segments;
        for (int i = 0; i < slices; i++) {
            int startIndex = i * sliceLength;
            int endIndex = (i == slices - 1) ? dp.GetDataSize() - 1 : (startIndex + sliceLength - 1);
            segments.push_back({startIndex, endIndex});  
            threads.emplace_back(&DataProcessor::BubbleSort, &dp, startIndex, endIndex);
        }
        
        for (thread& th : threads) {
            if (th.joinable()) 
                th.join();
        }

        
        vector<pair<int, int>> currentLevel = segments;

        while (currentLevel.size() > 1) {
            threads.clear();
            vector<pair<int, int>> nextLevel;
            for (int j = 0; j < currentLevel.size(); j += 2) {
                if (j < currentLevel.size() - 1) {
                    int left = currentLevel[j].first;
                    int mid = currentLevel[j].second;
                    int right = currentLevel[j + 1].second;
                    threads.emplace_back(&DataProcessor::Merge, &dp, left, mid, right);
                    nextLevel.push_back({left, right});
                } else {
                    nextLevel.push_back(currentLevel[j]);
                }
            }
            for (thread& th : threads) {
                if (th.joinable()) {
                    th.join();
                }
            }
            currentLevel = nextLevel;
        }
       

        auto end = high_resolution_clock::now();
        double t = duration<double, milli>(end - start).count();
        dp.SaveFile(filename, method, t);
    } else {
        cout << "Invalid method selected.\n";
    }

    return 0;
}

