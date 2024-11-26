#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <semaphore>
#include <vector>
#include <iomanip>

using namespace std;
using namespace chrono;

// Struct to define a person
struct Person {
    string name;
    int wait_time; // Time (in seconds) the person will simulate work
    long int elapsed_time = 0;
    int id = 0;
};

auto start_time = steady_clock::now();
bool game_over = false;

atomic<int> acquired_count(0);

// Map to store start times for each person
map<string, time_point<steady_clock>> start_times;
mutex time_mutex; // Mutex to protect access to start_times
 
vector<thread> workers;
 
// object counts are set to zero
// objects are in non-signaled state
counting_semaphore<10> 
    smphSignalMainToThread{0},
    smphSignalThreadToMain{0};
 
void ThreadProc(Person person)
{
    // wait for a signal from the main proc
    // by attempting to decrement the semaphore
    smphSignalMainToThread.acquire();

    {
        // Store the start time when the thread begins work
        // start_times is a shared variable, so each thread must
        // access it once at time
        lock_guard<mutex> guard(time_mutex);
        start_times[person.name] = steady_clock::now();
    }
 
    // cout << "[Worker: " << person.name << "] Got the signal, starting work\n";

    // Simulate work for the wait time defined in the Person struct
    this_thread::sleep_for(seconds(person.wait_time));

    // cout << "[Worker: " << person.name << "] Finished work (wait time: " 
    //      << person.wait_time << " seconds), sending signal back to main\n";
 
    // signal the main proc back
    smphSignalThreadToMain.release();

    acquired_count++;
}

// Function to set the cursor position
void setCursorPosition(int x, int y) {
    cout << "\033[" << x << ";" << y << "H";  // Move cursor to (x, y)
}

string handleInput() {
    setCursorPosition(1, 1);  // Move to line 1, column 1 (first line)
    cout << string(80, ' ');
    setCursorPosition(1, 1);
    cout << "Enter command: " ;
    // You can add logic for handling input here if needed
    // For now, it will just ask for a command
    string input;
    getline(cin, input);
    return input;
}

void printPersons(const vector<Person> persons) {
    setCursorPosition(4, 1);  // Move to line 3, column 1 (third line)

    // Print header
    cout << left << setw(10) << "Name" 
         << setw(10) << "Wait Time" 
         << setw(10) << "Elapsed Time" 
         << endl;
    cout << "------------------------" << endl;  // Divider

    // Print each person's details
    for (const auto& person : persons) {
        cout << setw(10) << person.name 
             << setw(10) << person.wait_time 
             << setw(10) << person.elapsed_time
             << endl;
    }
}

void printTimer(long int seconds) {
    setCursorPosition(3, 1);  // Move to line 2, column 1 (second line)

    // Print the timer with fixed formatting
    cout << "Timer: " << setw(3) << seconds << "s";
}

void CheckElapsedTime(vector<Person> persons) {
  // start_times is going to be read, so each thread must 
  // read it once at a time
  lock_guard<mutex> guard(time_mutex);
  for (auto& person : persons) {
    if (start_times.find(person.name) != start_times.end()) {
      auto now = steady_clock::now();
      auto elapsed = duration_cast<seconds>(now - start_times[person.name]).count();
      person.elapsed_time = (long int) elapsed;
    }
  }

  auto elapsed = duration_cast<seconds>(steady_clock::now() - start_time).count();
  printTimer(elapsed);
  printPersons(persons);
}

// Function to remove a person by their name
bool removePerson(vector<Person>& persons, const string name) {
    lock_guard<mutex> guard(time_mutex);
    auto it = find_if(persons.begin(), persons.end(), [&](const Person& person) {
        return person.name == name;
    });

    if (it != persons.end()) {
        persons.erase(it);
        cout << "Removed person: " << name << endl;
        system("clear");
        return true;
    }

    cout << "Person not found: " << name << endl;
    return false;
}

int main() 
{

    int numWorkers = 10;

    vector<thread> workers;

    system("clear");

    // Create a list of persons
    vector<Person> persons = {
        {"Alice", 300}, {"Bob", 200}, {"Charlie", 4}, {"Diana", 10}, {"Eve", 5},
        {"Frank", 2}, {"Grace", 30}, {"Hank", 400}, {"Ivy", 2}, {"Jack", 1}
    };

    // Create worker threads
    for (int i = 0; i < numWorkers; ++i) {
        workers.emplace_back(ThreadProc, persons[i]);
    }
 
    cout << "[main] Send the signal\n"; // message
 
    // signal the worker thread to start working
    // by increasing the semaphore's count

    for (int i = 0; i < numWorkers; ++i) {
      if (i >= 6) this_thread::sleep_for(seconds(2));
      smphSignalMainToThread.release();
    }

    while (acquired_count < 5) {
      CheckElapsedTime(persons);

      string word = handleInput();
      bool removed = removePerson(persons, word);
    }
 
    // wait until the worker thread is done doing the work
    // by attempting to decrement the semaphore's count
    for (int i = 0; i < numWorkers; ++i) {
      smphSignalThreadToMain.acquire();
    }

    cout << "end" << endl;
 
    cout << "[main] Got the signal\n"; // response message
    for (auto& worker : workers) {
      if (worker.joinable()) {
        worker.join();
      }
    }

    return 0; 
}
