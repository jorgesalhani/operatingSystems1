#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <string>
#include <map>
#include <semaphore>
#include <iomanip>
#include <random>

using namespace std;
using namespace chrono;

int setWval = 15;

vector<string> NAMES = {
  "Gabs", "Jorge", "Gabi", "Kalinka", "Ciro", "Luan", "Milene", "Lara", "Maristela",
  "Larinha", "Gabriela", "Ginho", "Sara", "Murilo", "Murilinho", "Silvio", "João", "Jão",
  "Erick", "Júlia", "Vitória", "Letícia", "Lele", "Pepe", "Mercedes", "Mariana", "Marina",
  "Machado", "José", "Zé", "Steven", "Finn", "Erika", "Arlindo", "Péricles", "Chico", "Francisco",
  "Rita", "Pedro", "Pedrinho", "Laiane", "Thais", "Ariane", "Ariana", "Lula", "Franklina", "Eduardo",
  "Duda", "Anitta", "Maria", "Madu", "Dado", "Pablo"
};

vector<string> SURNAMES = {
  "Ribeiro", "Costa", "Salgado", "Lastra", "Righetto", "Silva", "Dirceu", "Lula", "Marinho",
  "Lobeiro", "Pinhal", "Vitória", "Steven", "Hilton", "Grande", "Santos", "Oliveira", "Oliveti", 
  "Lacerda", "Bardo", "Botelho", "Salhani", "Saffi", "Borges", "Brito", "Pinto", "Tadeu", "Cruz",
  "Vittar"
};

struct Person {
    string name;
    string surname;
    int wait_time;
    long int elapsed_time = 0;
    int id = 0;
};

auto start_time = steady_clock::now();
map<string, time_point<steady_clock>> start_times;
mutex time_mutex; // Mutex to protect access to start_times

// Global variables
vector<Person> persons;                      // Shared vector of persons
mutex persons_mutex;                         // Mutex for thread-safe access
condition_variable cv_producer, cv_consumer; // Condition variables for producer and consumer
atomic<bool> running(true);                  // Flag to keep threads running

// Function to generate a new person
Person generatePerson(int id) {
    return Person{
      NAMES[rand() % NAMES.size()],
      SURNAMES[rand() % SURNAMES.size()],
      rand() % 5 + 1
    }; // Random wait time between 1 and 5
}

// Producer function
void producer() {
    int person_id = 1;

    while (running) {
        unique_lock<mutex> lock(persons_mutex);

        // Wait until there is space in the vector
        cv_producer.wait(lock, []() { return persons.size() < 10; });

        if (!running) break; // Exit loop if the program is stopping

        // Add a new person to the vector
        Person new_person = generatePerson(person_id++);
        
        lock_guard<mutex> guard(time_mutex);
        start_times[new_person.name] = steady_clock::now();

        persons.push_back(new_person);

        // Notify the consumer that a person is available
        cv_consumer.notify_one();

        // Simulate production delay
        // if (persons.size() >= 5) 
        //   this_thread::sleep_for(seconds(new_person.wait_time));
    }
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
    cout << left << setw(setWval) << "Name" 
         << setw(setWval) << "Surname" 
         << setw(setWval) << "Wait Time" 
         << setw(setWval) << "Elapsed Time" 
         << endl;
    cout << "-----------------------------------------------------------------" << endl;  // Divider

    // Print each person's details
    for (const auto& person : persons) {
        cout << setw(setWval) << person.name 
             << setw(setWval) << person.surname 
             << setw(setWval) << person.wait_time 
             << setw(setWval) << person.elapsed_time
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

// Consumer function
void consumer() {
    while (running) {
        unique_lock<mutex> lock(persons_mutex);

        // Wait until there is at least one person in the vector
        cv_consumer.wait(lock, []() { return !persons.empty(); });

        if (!running) break; // Exit loop if the program is stopping

        CheckElapsedTime(persons);
        string input = handleInput();

        // Remove the first person from the vector
        auto it = find_if(persons.begin(), persons.end(), [&](const Person& person) {
            return person.name == input;
        });

        if (it != persons.end()) persons.erase(it);

        // cout << "[Consumer] Removed: " << p.name
        //      << " (Wait Time: " << p.wait_time << ")\n";

        // Notify the producer that space is available
        cv_producer.notify_one();

        // Simulate consumption delay
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

int main() {
    // Seed the random number generator for wait times
    srand(time(0));
    system("clear");

    // Launch producer and consumer threads
    thread producer_thread(producer);
    thread consumer_thread(consumer);

    // Let the simulation run for 15 seconds
    this_thread::sleep_for(chrono::seconds(100));

    // Signal threads to stop
    running = false;

    // Notify all waiting threads to ensure they exit
    cv_producer.notify_all();
    cv_consumer.notify_all();

    // Join threads
    producer_thread.join();
    consumer_thread.join();

    cout << "[Main] Simulation finished.\n";
    return 0;
}
