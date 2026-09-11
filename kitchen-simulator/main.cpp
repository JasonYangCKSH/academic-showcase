// 第1組 11220105 楊耀甯 11227250 劉語涵
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <iomanip>
//#include <exception>
using namespace std;

struct Job {
    string oid;
    string arrival;
    string duration;
    string timeout;
    string cid;
};
class JobList {
public:
    vector<Job> jobs;

    void reset() { jobs.clear();}

    bool ReadFile(string num, bool read_inputfile) {
        double rstart, rend;
        rstart = clock();

        ifstream in;
        if (read_inputfile) {
            in.open(("input" + num + ".txt").c_str());
        } else {
            in.open(("sorted" + num + ".txt").c_str());
        }
        if (!in.is_open()) {
            if (read_inputfile)
                cout << endl << "### input" << num << ".txt does not exist! ###" << endl;
            else
                cout << endl << "### sorted" << num << ".txt does not exist! ###" << endl;
            return false;
        }
        string line;
        getline(in, line); // skip first line
        while (getline(in, line)) {
            stringstream ss(line);
            string token1;
            getline(ss, token1, '\t');
            string token2;
            getline(ss, token2, '\t');
            string token3;
            getline(ss, token3, '\t');
            string token4;
            getline(ss, token4, '\t');
            jobs.push_back({token1, token2, token3, token4});
        }

        rend = clock();
        if (read_inputfile)
            cout << "\nReading data: " << rend - rstart << " clocks ("
                 << (rend - rstart) / CLOCKS_PER_SEC * 1000 << " ms)." << endl;

        return true;
    }
    void ShellSort() {
        double sstart, send;
        sstart = clock();

        int n = jobs.size();
        for (int gap = n / 2; gap > 0; gap /= 2) {
            for (int i = gap; i < n; i++) {
                Job temp = jobs[i];
                int j;
                for (j = i; j >= gap; j -= gap) {
                    int a1 = atoi(jobs[j - gap].arrival.c_str());
                    int a2 = atoi(temp.arrival.c_str());
                    int o1 = atoi(jobs[j - gap].oid.c_str());
                    int o2 = atoi(temp.oid.c_str());

                    // 比較 Arrival，若相同則比較 OID
                    if (a1 > a2 || (a1 == a2 && o1 > o2)) {
                        jobs[j] = jobs[j - gap];
                    } else {
                        break;
                    }
                }
                jobs[j] = temp;
            }
        }
        send = clock();
        cout << "\nSorting data: " << send - sstart << " clocks ("
             << (send - sstart) / CLOCKS_PER_SEC * 1000 << " ms)." << endl;
        return;
    }
    void SaveSortedFile(string num) {
        double wstart, wend;
        wstart = clock();
        ofstream out;
        out.open(("sorted" + num + ".txt").c_str());
        if (!out.is_open()) {
            return;
        }
        out << "OID\tArrival\tDuration\tTimeOut\n";
        for (int i = 0; i < jobs.size(); i++) {
            out << jobs[i].oid << "\t"
                << jobs[i].arrival << "\t"
                << jobs[i].duration << "\t"
                << jobs[i].timeout << "\n";
        }
        out.close();

        wend = clock();


        cout << "\nWriting data: " << wend - wstart << " clocks ("
             << (wend - wstart) / CLOCKS_PER_SEC * 1000 << " ms)." << endl;
        return;
    }



};
struct Abort {
    string oid;
    string delay;
    string abort;
    string cid;

};
class AbortList {
public:
    vector<Abort> ajobs;
    void Sort() {
        bool sorted = false;

        while (!sorted) {
            sorted = true;
            for (int i = 0; i < ajobs.size() - 1; i++) {
                if (stoi(ajobs[i].abort) > stoi(ajobs[i + 1].abort)) {
                    Abort temp = ajobs[i];
                    ajobs[i] = ajobs[i + 1];
                    ajobs[i + 1] = temp;
                    sorted = false;
                }
            }
        }
        return;

    }
};

struct TimeOut {
    string oid;
    string delay;
    string departure;
    string cid;
};
class TimeOutList {
public:
    vector<TimeOut> tojobs;
    void Sort() {
        bool sorted = false;

        while (!sorted) {
            sorted = true;
            for (int i = 0; i < tojobs.size() - 1; i++) {
                if (stoi(tojobs[i].departure) > stoi(tojobs[i + 1].departure)) {
                    TimeOut temp = tojobs[i];
                    tojobs[i] = tojobs[i + 1];
                    tojobs[i + 1] = temp;
                    sorted = false;
                }
            }
        }
        return;

    }
};
string itos(int num) {
    stringstream ss;
    ss << num;
    return ss.str();
}
// 一個廚師擁有一個佇列
class Queue {
public:
    vector<Job> qu;
    int idle_time;  //閒置時刻
    int cid;
    Queue (int cid_num) {idle_time = 0; cid = cid_num;}
    void reset() {
        qu.clear();
    }
    int getLength() {
        return qu.size();
    }
    bool isFull() {
        if (qu.size() >= 3)
            return true;
        return false;
    }
    bool isEmpty() {
        if (qu.size() == 0)
            return true;
        return false;
    }
    Job getFront() {
        return qu[0];
    }
    Job deQueue() {
        Job temp;
        temp = qu[0];
        qu.erase(qu.begin());
        return temp;
    }
    void enQueue(Job temp) {
        qu.push_back(temp);
    }


};
void SaveOutputFile(string num, JobList&  joblist, AbortList& abortlist, TimeOutList& timeoutlist) {
    ofstream out;
    out.open("output" + num + ".txt");
    out << "\t[Abort List]" << endl;
    out << "\tOID\tDelay\tAbort\n";
    for (int i = 1; i <= abortlist.ajobs.size(); i++) {
        out << "[" << i << "]\t" << abortlist.ajobs[i - 1].oid << "\t"
            << abortlist.ajobs[i - 1].delay << "\t"
            << abortlist.ajobs[i - 1].abort << "\n";
    }
    out << "\t[Timeout List]" << endl;
    out << "\tOID\tDelay\tDeparture\n";
    for (int i = 1; i <= timeoutlist.tojobs.size(); i++) {
        out << "[" << i << "]\t" << timeoutlist.tojobs[i - 1].oid << "\t"
            << timeoutlist.tojobs[i - 1].delay << "\t"
            << timeoutlist.tojobs[i - 1].departure << "\n";
    }

    out << "[Total Delay]" << endl;
    int total_delay = 0;
    for (int i = 0; i < abortlist.ajobs.size(); i++) {
        int delay = stoi(abortlist.ajobs[i].delay);
        total_delay += delay;
    }
    for (int i = 0; i < timeoutlist.tojobs.size(); i++) {
        int delay = stoi(timeoutlist.tojobs[i].delay);
        total_delay += delay;
    }
    out << total_delay << " min." << endl;

    out << "[Failure Percentage]" << endl;
    double sum_of_abort = double(abortlist.ajobs.size());
    double sum_of_timeout = double(timeoutlist.tojobs.size());
    double sum_of_all = 0;

    // take out error data
    for (int i = 0; i < joblist.jobs.size(); i++) {
        Job currentjob = joblist.jobs[i];
        int arrival = stoi(currentjob.arrival);
        int duration = stoi(currentjob.duration);
        int timeout = stoi(currentjob.timeout);
        if (arrival + duration <= timeout) {
            sum_of_all++;
        }
    }

    if (sum_of_all > 0) {
        double failure_percentage = 100.0 * (sum_of_abort + sum_of_timeout) / sum_of_all;
        out << fixed << setprecision(2) << failure_percentage << " %\n";
    }
    out.close();
    return;
}
void SaveCooksFile(string num, JobList&  joblist, AbortList& abortlist, TimeOutList& timeoutlist) {
    ofstream out;
    out.open("cooks" + num + ".txt");
    out << "\t[Abort List]" << endl;
    out << "\tOID\tCID\tDelay\tAbort\n";
    for (int i = 1; i <= abortlist.ajobs.size(); i++) {
        out << "[" << i << "]\t" << abortlist.ajobs[i - 1].oid << "\t"
            << abortlist.ajobs[i - 1].cid << "\t"
            << abortlist.ajobs[i - 1].delay << "\t"
            << abortlist.ajobs[i - 1].abort << "\n";
    }
    out << "\t[Timeout List]" << endl;
    out << "\tOID\tCID\tDelay\tDeparture\n";
    for (int i = 1; i <= timeoutlist.tojobs.size(); i++) {
        out << "[" << i << "]\t" << timeoutlist.tojobs[i - 1].oid << "\t"
            << timeoutlist.tojobs[i - 1].cid << "\t"
            << timeoutlist.tojobs[i - 1].delay << "\t"
            << timeoutlist.tojobs[i - 1].departure << "\n";
    }

    out << "[Total Delay]" << endl;
    int total_delay = 0;
    for (int i = 0; i < abortlist.ajobs.size(); i++) {
        int delay = stoi(abortlist.ajobs[i].delay);
        total_delay += delay;
    }
    for (int i = 0; i < timeoutlist.tojobs.size(); i++) {
        int delay = stoi(timeoutlist.tojobs[i].delay);
        total_delay += delay;
    }
    out << total_delay << " min." << endl;

    out << "[Failure Percentage]" << endl;
    double sum_of_abort = double(abortlist.ajobs.size());
    double sum_of_timeout = double(timeoutlist.tojobs.size());
    double sum_of_all = 0;

    // take out error data
    for (int i = 0; i < joblist.jobs.size(); i++) {
        Job currentjob = joblist.jobs[i];
        int arrival = stoi(currentjob.arrival);
        int duration = stoi(currentjob.duration);
        int timeout = stoi(currentjob.timeout);
        if (arrival + duration <= timeout) {
            sum_of_all++;
        }
    }

    if (sum_of_all > 0) {
        double failure_percentage = 100.0 * (sum_of_abort + sum_of_timeout) / sum_of_all;
        out << fixed << setprecision(2) << failure_percentage << " %\n";
    }
    out.close();
    return;

}
void HandleJobInQueue(Job& currentjob, Queue& chef, AbortList& abortlist) {
    // 若 Queue 已滿
    if (chef.isFull()) {
        abortlist.ajobs.push_back({
            currentjob.oid, "0", currentjob.arrival, currentjob.cid
        });
    }
    // Queue非滿
    else {
        chef.enQueue(currentjob);
    }
    return;
}
void ProcessChefQueue(Queue& chef, AbortList& abortlist, TimeOutList& timeoutlist, int arrival) {
    // Queue 中的工作可以按到達時間出列
    while (!chef.isEmpty() && chef.idle_time <= arrival) {

        Job dequeuedjob = chef.deQueue();
        int dequeued_timeout = stoi(dequeuedjob.timeout);

        if (dequeued_timeout <= chef.idle_time) {
            int delay = chef.idle_time - stoi(dequeuedjob.arrival);
            abortlist.ajobs.push_back({
                dequeuedjob.oid,
                itos(delay),
                itos(chef.idle_time),
                itos(chef.cid)
            });
            continue;
        }

        chef.idle_time += stoi(dequeuedjob.duration);
        if (chef.idle_time > dequeued_timeout) {
            timeoutlist.tojobs.push_back({
                dequeuedjob.oid,
                itos(chef.idle_time - stoi(dequeuedjob.arrival) - stoi(dequeuedjob.duration)),
                itos(chef.idle_time),
                itos(chef.cid)
            });
        }
    }
    return;
}
void AssignJobToChef(Job& currentjob, Queue& chef, AbortList& abortlist) {
    // 更新 idle_time 為當前到達時間
    int arrival = stoi(currentjob.arrival);
    chef.idle_time = max(chef.idle_time, arrival);
    // 嘗試將當前工作加入 Queue 或加入 AbortList
    if (chef.isFull()) {
        abortlist.ajobs.push_back({
            currentjob.oid, "0", currentjob.arrival, currentjob.cid
        });
    } else {
        chef.enQueue(currentjob);
    }
    return;
}
void ProcessRemainingJob(Queue& chef, AbortList& abortlist, TimeOutList& timeoutlist) {
    // 處理 Queue 中剩餘的工作
    while (!chef.isEmpty() /*&& chef.idle_time <= stoi(chef.qu[0].arrival)*/) {
        Job dequeuedjob = chef.deQueue();
        int duration = stoi(dequeuedjob.duration);
        int timeout = stoi(dequeuedjob.timeout);
        int arrival = stoi(dequeuedjob.arrival);
        // 如果發現訂單逾時 (timeout <= idle_time)
        if (timeout <= chef.idle_time) {
            int delay = chef.idle_time - arrival;
            abortlist.ajobs.push_back({
                dequeuedjob.oid,
                itos(delay),
                itos(chef.idle_time),
                itos(chef.cid)
            });
            continue; // 跳過後續處理
        }
        chef.idle_time += duration;
        if (chef.idle_time > timeout) {
            timeoutlist.tojobs.push_back({
                dequeuedjob.oid,
                itos(chef.idle_time - stoi(dequeuedjob.arrival) - duration),
                itos(chef.idle_time),
                itos(chef.cid)
            });
        }

    }

}

void Command2(string num) {
    JobList joblist;
    AbortList abortlist;
    TimeOutList timeoutlist;
    Queue chef(1);


    // 嘗試讀取檔案
    if (!joblist.ReadFile(num, false)) {
        return;
    }

    // 處理每一個工作
    for (int i = 0; i < joblist.jobs.size(); i++) {
        Job currentjob = joblist.jobs[i];
        // 確保資料解析正確
        int arrival = stoi(currentjob.arrival);
        int duration = stoi(currentjob.duration);
        int timeout = stoi(currentjob.timeout);
        // illegal
        if (arrival + duration > timeout) {
            continue;
        }
        // chef在工作
        if (chef.idle_time > arrival) {
            // 把一新任務加到queue()
            currentjob.cid = "1";
            HandleJobInQueue(currentjob, chef, abortlist);
        }
        // chef空閒
        else {
            // 處理queue中可以在currentjob.arrival之前完成的任務
            ProcessChefQueue(chef, abortlist, timeoutlist, arrival);
            currentjob.cid = "1";
            // 將新任務分配給指定的queue
            AssignJobToChef(currentjob, chef, abortlist);
        }

    }

    // 處理剩餘在queue中的任務
    ProcessRemainingJob(chef, abortlist, timeoutlist);
    abortlist.Sort();

    // 測試輸出結果
    SaveOutputFile(num, joblist, abortlist, timeoutlist);

}
void Command3(string num) {
    JobList joblist;
    AbortList abortlist;
    TimeOutList timeoutlist;
    Queue chef1(1), chef2(2);


    // 嘗試讀取檔案
    if (!joblist.ReadFile(num, false)) {
        return;
    }

    Queue* selectedchef = NULL;
    for (int i = 0; i < joblist.jobs.size(); i++) {
        Job currentjob = joblist.jobs[i];
        int arrival = stoi(currentjob.arrival);
        int duration = stoi(currentjob.duration);
        int timeout = stoi(currentjob.timeout);

        // illegal
        if (arrival + duration > timeout) {
            continue;
        }

        ProcessChefQueue(chef1, abortlist, timeoutlist, arrival);
        ProcessChefQueue(chef2, abortlist, timeoutlist, arrival);


        selectedchef = NULL;
        // case1: 找到閒置廚師
        if (chef1.idle_time <= arrival && chef1.isEmpty()) {
            selectedchef = &chef1;
        }
        if (chef2.idle_time <= arrival && chef2.isEmpty()) {
            if (selectedchef == NULL) {
                selectedchef = &chef2;
            } /*else {
                selectedchef = &chef1;
            }*/
        }

        // case3: 無閒置廚師，then choose the shortest queue
        if (selectedchef == NULL) {
            if (!chef1.isFull() && !chef2.isFull()) {
                if (chef1.getLength() < chef2.getLength()) {
                    selectedchef = &chef1;
                } else if (chef1.getLength() > chef2.getLength()) {
                    selectedchef = &chef2;

                }
                // chef1.getLength() == chef2.getLength()
                else if (chef1.cid < chef2.cid) {
                    selectedchef = &chef1;
                } else {
                    selectedchef = &chef2;
                }
            }
            //chef2 full
            else if (!chef1.isFull()) {
                selectedchef = &chef1;
            }
            // chef1 full
            else if (!chef2.isFull()) {
                selectedchef = &chef2;
            }
        }
        // Case 4: 若無法分配，取消該任務
        if (selectedchef == NULL) {
            currentjob.cid = "0"; // 表示無法分配
            abortlist.ajobs.push_back({
                currentjob.oid, "0", currentjob.arrival, currentjob.cid
            });
        } else {
            // 將任務分配給選中的廚師佇列
            currentjob.cid = itos(selectedchef->cid);

            AssignJobToChef(currentjob, *selectedchef, abortlist);
        }

    }
    // 處理兩位廚師佇列中剩餘的任務


    ProcessRemainingJob(chef1, abortlist, timeoutlist);
    ProcessRemainingJob(chef2, abortlist, timeoutlist);


    abortlist.Sort();

    // 儲存結果
    SaveCooksFile(num, joblist, abortlist, timeoutlist);
}

void Menu() {
    cout << endl;
    cout << "**** Simulate FIFO Queues by SQF *****" << endl;
    cout << "* 0. Quit                            *" << endl;
    cout << "* 1. Sort a file                     *" << endl;
    cout << "* 2. Simulate one FIFO queue         *" << endl;
    cout << "* 3. Simulate two queues by SQF      *" << endl;
    cout << "*******************************" << endl;
    cout << "Input a command(0, 1, 2, 3): ";
    return;
}

int main() {
    JobList joblist;

    int command = -1;

    while (command != 0) {
        joblist.reset();

        Menu();
        cin >> command;
        if (cin.fail()) break;
        if (command != 0 && command != 1 && command != 2 && command != 3) {
            cout << "\nCommand does not exist!\n";
        }
        if (command == 1) {
            string num;
            cout << endl << "Input a file number (e.g., 401, 402, 403, ...): ";
            cin >> num;

            if (joblist.ReadFile(num, true)) {
                joblist.ShellSort();
                joblist.SaveSortedFile(num);
            } else {
                continue;
            }
        } else if (command == 2) {
            string num;
            cout << endl << "Input a file number (e.g., 401, 402, 403, ...): ";
            cin >> num;
            Command2(num);
        } else if (command == 3) {
            string num;
            cout << endl << "Input a file number (e.g., 401, 402, 403, ...): ";
            cin >> num;
            Command3(num);
        }
    }
    return 0;
}
