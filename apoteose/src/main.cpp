#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <semaphore.h>

using namespace std;

struct Person {
    string name;
    int dressing_time;
    int travel_time;
    int work_time;
};

queue<Person> person_queue;  
mutex queue_mutex;           
condition_variable cv;       
sem_t population_semaphore;  

bool game_running = true;    
int timestep = 0;            
int score = 0;
int game_run_time = 0;

void decrement_dressing_time() {
    lock_guard<mutex> lock(queue_mutex);
    queue<Person> updated_queue;

    while (!person_queue.empty()) {
        Person p = person_queue.front();
        person_queue.pop();

        p.dressing_time++;
        p.work_time--;

        if (p.work_time <= -10) {
            cout << p.name << " desistiu de esperar e cancelou sua viagem!\n";
            score += -10;  
            sem_post(&population_semaphore);
        } else if (p.work_time <= -5) {
            score += -5;  
            updated_queue.push(p);
        } else if (p.work_time < 0) {
            updated_queue.push(p);
        } else {
            updated_queue.push(p);  
        }
    }

    person_queue = updated_queue;
}

void producer() {
    vector<string> names = {
        "Gabriela", "Kalinka", "Duda", "Gabs", "Maju", "Priscila", 
        "Silvio", "Bruno", "Pedro", "Murilo","Ginho", "Jorge",
        "Ana Ju", "Milene", "Carol", "Pietra", "Pepe", "Lula",
        "Maristela", "Ana", "Andreia", "Franklina", "Mari", "João"};

    int person_count = 0;

    while (game_running) {
        sem_wait(&population_semaphore); 

        {
            lock_guard<mutex> lock(queue_mutex);

            Person p;
            p.name = names[rand() % names.size()];
            p.dressing_time = - 4 + rand() % 3; 
            p.travel_time = 2 + rand() % 3;   
            p.work_time = 8 + timestep + rand() % 6;     

            person_queue.push(p);
            person_count++;
        }

        cv.notify_one(); 
        this_thread::sleep_for(chrono::seconds(5)); 
    }
}

void consumer() {
    while (game_running) {
        Person selected_person;
        bool person_selected = false;

        {
            unique_lock<mutex> lock(queue_mutex);

            cv.wait(lock, [] { return !person_queue.empty() || !game_running; });

            if (!game_running) break;

            system("clear");

            cout << "\nScore total: " << score << endl
                 << "Tempo disponível: " << game_run_time - timestep << endl
                 << "Tempo atual: " << timestep << endl
                 << "Pessoas disponíveis:" << endl;

            queue<Person> temp_queue = person_queue;
            int idx = 1;
            while (!temp_queue.empty()) {
                Person p = temp_queue.front();
                cout << "[" << idx++ << "] " << p.name
                     << " (Vestir: " << p.dressing_time 
                     << ", Trajeto: " << p.travel_time
                     << ", Trabalho: " << p.work_time << ")\n";
                temp_queue.pop();
            }

            cout << "\nQuem conduziremos ao trabalho? ";
            int choice;
            cin >> choice;

            if (choice > 0 && choice <= (int)person_queue.size()) {
                for (int i = 1; i < choice; ++i) {
                    person_queue.push(person_queue.front());
                    person_queue.pop();
                }
                selected_person = person_queue.front();
                person_queue.pop();
                person_selected = true;

                sem_post(&population_semaphore);
            } else {
                cout << "Escolha inválida. Pulando corrida...\n";
            }
        }

        if (person_selected) {
            while (selected_person.dressing_time < 0) {
                cout << selected_person.name << " está terminando de se vestir...\n";
                this_thread::sleep_for(chrono::seconds(5));
                timestep++;
                selected_person.dressing_time++;
                selected_person.work_time--;
            }

            cout << selected_person.name << " está indo ao trabalho...\n";
            this_thread::sleep_for(chrono::seconds(selected_person.travel_time));
            timestep += selected_person.travel_time;


            int person_score = selected_person.work_time - selected_person.dressing_time;
            score += person_score;
            cout << selected_person.name << " chegou no trabalho a tempo! Score da corrida: " << person_score << "\n";
        }
    }
}

int main() {
    cout << "\nQuanto tempo teremos de corrida? ";
    cin >> game_run_time;

    srand(time(0));
    sem_init(&population_semaphore, 0, 5); 

    thread producer_thread(producer);
    thread consumer_thread(consumer);

    while (game_running) {
        this_thread::sleep_for(chrono::seconds(5)); 
        timestep++;

        decrement_dressing_time(); 

        if (timestep >= game_run_time) { 
            game_running = false;
            cv.notify_all();
        }
    }

    producer_thread.join();
    consumer_thread.join();

    sem_destroy(&population_semaphore);

    cout << "\nFim do jogo! Final score: " << score << "\n";
    return 0;
}
