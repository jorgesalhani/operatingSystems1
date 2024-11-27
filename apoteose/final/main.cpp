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
  "Duda", "Anitta", "Maria", "Madu", "Dado", "Pablo", "Guilherme", "Yuri"
};

vector<string> SURNAMES = {
  "Ribeiro", "Costa", "Salgado", "Lastra", "Righetto", "Silva", "Dirceu", "Lula", "Marinho",
  "Lobeiro", "Pinhal", "Vitória", "Steven", "Hilton", "Grande", "Santos", "Oliveira", "Oliveti", 
  "Lacerda", "Bardo", "Botelho", "Salhani", "Saffi", "Borges", "Brito", "Pinto", "Tadeu", "Cruz",
  "Vittar"
};

vector<string> MSG_PERSONAL = {
    "Andar com fé eu vou, a fé não costuma falhar.",
    "A paz invadiu meu coração, como uma surpresa.",
    "Vamos fugir, pra outro lugar, baby.",
    "Domingo no parque, o amor está em festa.",
    "Se eu quiser falar com Deus, preciso me silenciar.",
    "Expresso 2222, partindo para a liberdade e o sonho.",
    "Toda menina baiana tem um jeito especial de ser.",
    "Eu quero ser feliz agora, não depois.",
    "O verdadeiro amor é vão",
    "Não chores mais, ouça o canto da salvação.",
    "Primavera, te amo, meu amor."
    "Você e eu, eu e você, juntinhos."
    "Não quero dinheiro, só quero amar."
    "Sossego, eu quero sossego."
    "Me dê motivo pra ir embora."
    "Descobridor dos sete mares, navegar eu quero."
    "Ela partiu, e nunca mais voltou."
    "Mas é você que ama o passado e que não vê",
    "Choram Marias e Clarices no solo do Brasil",
    "São as águas de março fechando o verão",
    "O meu peito percebeu que o mar é uma gota",
    "Eu quero um homem de cor, um Deus negro",
    "Tudo é perigoso, tudo é divino maravilhoso.",
    "Enquanto eles se batem, dê um rolê.",
    "É preciso estar atento e forte.",
    "Eu não preciso de muito dinheiro, graças a Deus.",
    "Eu sou, eu sou amor, da cabeça aos pés.",
    "A noite vai ser longa, o que fazer?",
    "O sol vai brilhar, mas a chuva já passou.",
    "Tudo passa, tudo passará, tudo passará.",
    "Somos feitos da mesma substância, eu e você.",
    "Amanhã, o dia vai clarear, será diferente."
};

int MAX_WAIT_TIME = 10;

struct Person {
    string name;
    string surname;
    int wait_time;
    long int elapsed_time = 0;
    string msgPersonal = "";
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

float game_score = 0.0;

int dificult = 1;
int game_max_time = 100;

// Function to generate a new person
Person generatePerson(int id) {
    return Person{
      NAMES[rand() % NAMES.size()],
      SURNAMES[rand() % SURNAMES.size()],
      rand() % MAX_WAIT_TIME + 1,
      0,
      MSG_PERSONAL[rand() % MSG_PERSONAL.size()],
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

string handleInput(string message) {
    int inputPosLine = 1;
    int inputPosCol = 1;

    setCursorPosition(inputPosLine, inputPosCol);  // Move to line 1, column 1 (first line)
    
    cout << string(80, ' ');
    
    setCursorPosition(inputPosLine, inputPosCol);
    
    cout << message ;
    string input;
    getline(cin, input);
    return input;
}

void printPersons(const vector<Person> persons) {
    int personsPosLine = 4;
    int personsPosCol = 1;
    setCursorPosition(personsPosLine, personsPosCol);  // Move to line 3, column 1 (third line)

    // Print header
    cout << left << setw(setWval) << "Nome" 
         << setw(setWval) << "Sobrenome" 
         << setw(setWval) << "Tempo de espera" 
         << setw(setWval) << "Tempo decorrido" 
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
    int timerPosLine = 3;
    int timerPosCol = 1;
    setCursorPosition(timerPosLine, timerPosCol);
    cout << "Timer: " << setw(timerPosLine) << seconds << "s";
}

void printScore(float score) {
    int scorePosLine = 3;
    int scorePosCol = 15;
    setCursorPosition(scorePosLine, scorePosCol);  
    cout << "Score: " << setw(scorePosCol) << score;
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
  printScore(game_score);
  printTimer(elapsed);
  printPersons(persons);
}

void updateScore(Person p) {
    float ca, cb;

    string inputed_str = p.name;
    if (dificult == 1) {
        ca = 1;
        cb = 1;
    } else 
    if (dificult == 2) {
        ca = 1.2;
        cb = 0.8;
        inputed_str += p.surname;
    } else {
        ca = 1.5;
        cb = 0.5;
        inputed_str += p.surname + p.msgPersonal;
    }

    // * score = ca.sn - cb.|ts - tw|
    game_score = game_score + (ca * inputed_str.size()) - (cb * abs(p.wait_time - p.elapsed_time));
}

void printMsgPersonal(Person p) {
    int inputMsgPersonLine = 2;
    int inputMsgPersonCol = 1;

    setCursorPosition(inputMsgPersonLine, inputMsgPersonCol);
    cout << p.msgPersonal;
}

// Consumer function
void consumer() {
    while (running) {
        unique_lock<mutex> lock(persons_mutex);

        // Wait until there is at least one person in the vector
        cv_consumer.wait(lock, []() { return !persons.empty(); });

        if (!running) break; // Exit loop if the program is stopping

        CheckElapsedTime(persons);
        string name = handleInput("> Atender pessoa: ");

        // Remove the first person from the vector
        auto it = find_if(persons.begin(), persons.end(), [&](const Person& person) {
            if (dificult == 1) return person.name == name;
            return person.name + " " + person.surname == name;
        });

        string msgPersonal = "";
        if (dificult == 3) {
            printMsgPersonal(*it);
            msgPersonal = handleInput("> Mensagem personalizada: ");
            if (msgPersonal != (*it).msgPersonal) (*it).msgPersonal = "";
        }

        if (it != persons.end()) {
            updateScore(*it);
            persons.erase(it);
        }

        // cout << "[Consumer] Removed: " << p.name
        //      << " (Wait Time: " << p.wait_time << ")\n";

        // Notify the producer that space is available
        cv_producer.notify_one();

        // Simulate consumption delay
        this_thread::sleep_for(milliseconds(500));
    }
}

void displayBanner() {
    int bannerPosLine = 2;
    int bannerPosCol = 1;
    setCursorPosition(bannerPosLine, bannerPosCol);  
    cout << R"(
            ________   ______                          
           / ____/ /  / ____/___ _______________  ____ 
          / __/ / /  / / __/ __ `/ ___/ ___/ __ \/ __ \
         / /___/ /  / /_/ / /_/ / /  / /__/ /_/ / / / /
        /_____/_/   \____/\__,_/_/  s\___/\____/_/ /_/                     
   )" << endl;

    cout << R"(
                        (\     .-. 
                         )\   (o.o) 
                        (__) (   )o
                           '---'-" 
                           
                Seja bem-vinda ao nosso bistrô!
                Ficaremos felizes em serví-la!

   )" << endl;
}


void printGreeting() {
    displayBanner();
    cout << R"(
    ╔═════════════════╗
    ║       MENU      ║
    ╟─────────────────╢
    ║ 1. Começar!     ║
    ║ 2. Regras       ║
    ║ 3. Dificuldade  ║
    ║ 4. Sobre        ║
    ╚═════════════════╝
   )" << endl;
}

void displayMenu(int selectedOption) {
    system("clear"); // Clear the terminal
    displayBanner();

    cout << "\n    ╔═════════════════╗" << endl;
    cout << "    ║       MENU      ║" << endl;
    cout << "    ╟─────────────────╢" << endl;
    cout << "    ║ ";
    if (selectedOption == 1) cout << "\033[1m1. Começar!\033[0m   * ║" << endl; // Bold
    else cout << "1. Começar!     ║" << endl;

    cout << "    ║ ";
    if (selectedOption == 2) cout << "\033[1m2. Regras\033[0m     * ║" << endl; // Bold
    else cout << "2. Regras       ║" << endl;

    cout << "    ║ ";
    if (selectedOption == 3) cout << "\033[1m3. Dificuldade\033[0m* ║" << endl; // Bold
    else cout << "3. Dificuldade  ║" << endl;

    cout << "    ║ ";
    if (selectedOption == 4) cout << "\033[1m4. Sobre\033[0m      * ║" << endl; // Bold
    else cout << "4. Sobre        ║" << endl;

    cout << "    ╚═════════════════╝" << endl;
}

void backToMainMenu(string msg) {
    handleInput(msg);
}

void displayRules() {
    system("clear");
    displayBanner();
    cout << R"(
        REGRAS
        ------------------------------------------------------------------
        O sistema chama-garçom quebrou e você deve informar manualmente o 
        nome das pessoas para serem atendidas a cada momento

        A cada instante você deverá digitar no campo
        
        > Atender pessoa: _______

        o nome de uma das pessoas listadas. Por exemplo

        > Atender pessoa: Pepe

        Timer:   9s                  Score: 0
        Name           Surname        Wait Time      Elapsed Time
        -----------------------------------------------------------------
        Steven         Vittar         10             0
        Anitta         Steven         4              0
        Erick          Vitória        3              0
        Pepe           Vittar         2              0
        Steven         Oliveira       9              0
        Gabriela       Steven         2              0
        Franklina      Cruz           5              0
        Lele           Tadeu          8              0
        Anitta         Botelho        4              0
        Maristela      Lula           5              0

        Sua meta é obter a maior pontuação possível. Calculamos abaixo:
        
        * score = ca.sn - cb.|ts - tw|

        Onde as variáveis representam:

        - sn: tamanho do nome digitado
        - tw: tempo de espera que cada cliente está disposta 
              a esperar sem sofrer
        - ts: tempo decorrido desde a chegada da cliente

        DIFICULDADE
        -----------------------------------------------------------
        Lembrando que 

        [1] Nomes simples 
        - sn = NOME                         |  ca = 1  |  cb = 1  |
        
        [2] Nomes compostos 
        - sn = NOME + SOBRENOME             | ca = 1.2 | cb = 0.8 |

        [3] Nomes compostos com 
           mensagem personalizada
        - sn = NOME + SOBRENOME + MENSAGEM  | ca = 1.5 | cb = 0.5 | 


        Voltar ao menu inicial... [qualquer tecla]
    )" << endl;
        
    backToMainMenu("");
}

void displayDificulties() {
    system("clear");
    displayBanner();
        cout << R"(
        REGRAS
        -----------------------------------------------------------
        Sua meta é obter a maior pontuação possível:
        
        - sn: tamanho do nome digitado
        - tw: tempo de espera que cada cliente está disposta 
              a esperar sem sofrer
        - ts: tempo decorrido desde a chegada da cliente

        * score = ca.sn - cb.|ts - tw|

        DIFICULDADE
        -----------------------------------------------------------
        Lembrando que 
        [1] Nomes simples 
        - sn = NOME                         |  ca = 1  |  cb = 1  |
        
        [2] Nomes compostos 
        - sn = NOME + SOBRENOME             | ca = 1.2 | cb = 0.8 |

        [3] Nomes compostos com 
           mensagem personalizada
        - sn = NOME + SOBRENOME + MENSAGEM  | ca = 1.5 | cb = 0.5 | 

    )" << endl;

    string str_dificult = handleInput("Mudar dificuldade? [1|2|3] ");
    dificult = stoi(str_dificult);
    dificult = dificult <= 1 ? 1 : dificult >= 3 ? 3 : 2;

}

void displayAbout() {
    system("clear");
    displayBanner();
    
    auto elapsed = duration_cast<seconds>(steady_clock::now() - start_time).count();
    cout << R"(
        ----------------------------------------------------------------
        Bom dia!! Você acordou no corpo de Milton, uma baratinha garçom!

        Você poderia ter acordado em um dia de folga de Milton, mas isso 
        somente aconteceria com uma chance de ~0.1667%, devido à escala
        excessiva de trabalho no universo das baratinhas

        Uma das suas amigas está lutando muito para que seus dias de 
        descanso possam ser mais dignos, mas enquanto isso, precisamos 
        atender nossas clientes
        ----------------------------------------------------------------
        A população de baratinhas é grande e seu bistrô é excepcional, 
        mas sua sorte não está boa e o sistema chama-garçom automático 
        quebrou. Sua tarefa é informar manualmente o nome da cliente a
        ser atendida. A cada segundo uma nova cliente aparece. )" << endl;
        
    cout << "\n        Neste instante já se passaram " << elapsed << " segundos" << endl;
    
    cout << R"(
        Uma verdadeira multidão se aperta para saborear todas as 
        delícias do bistrô. Não fique parada!
        ----------------------------------------------------------------
        )" << endl;
        
    backToMainMenu("Voltar ao menu inicial... [qualquer tecla] ");
}

void menuInteraction() {
    while (true) {
        system("clear");
        printGreeting();
        string str_input = handleInput("Qual opção? [1|2|3|4] ");
        int input = stoi(str_input);
        displayMenu(input);
        if ((int) input < 0 || (int) input > 4) continue;

        str_input = handleInput("Confirma? [s|n] ");
        if (str_input != "s") continue;

        if (input == 2) displayRules();
        if (input == 3) displayDificulties();
        if (input == 4) displayAbout();

        if (input == 1) {
            string str_game_max_time = handleInput("Duração do jogo? (segundos) ");
            game_max_time = stoi(str_game_max_time);
            str_input = handleInput("Começar? ");
            system("clear");
            break;
        }
    }
}

int main() {
    // Seed the random number generator for wait times
    srand(time(0));
    system("clear");
    printGreeting();
    menuInteraction();

    // Launch producer and consumer threads
    thread producer_thread(producer);
    thread consumer_thread(consumer);

    // Let the simulation run for 15 seconds
    this_thread::sleep_for(seconds(game_max_time));

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
