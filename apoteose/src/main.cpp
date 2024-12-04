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
#include <algorithm>

using namespace std;
using namespace chrono;

/**
 * "BANCO DE DADOS"
 * ----------------
 * 
 * Dados que serão escolhidos arbitrariamente no jogo
 */
vector<string> NAMES = {
  "Gabs", "Jorge", "Gabi", "Kalinka", "Ciro", "Luan", "Milene", "Lara", "Maristela", "Waleska",
  "Larinha", "Gabriela", "Ginho", "Sara", "Murilo", "Murilinho", "Silvio", "João", "Jão", "Wandressa",
  "Erick", "Júlia", "Vitória", "Letícia", "Lele", "Pepe", "Mercedes", "Mariana", "Marina", "Alessandra"
  "Machado", "José", "Zé", "Steven", "Finn", "Erika", "Arlindo", "Péricles", "Chico", "Francisco", "Cams"
  "Rita", "Pedro", "Pedrinho", "Laiane", "Thais", "Ariane", "Ariana", "Lula", "Franklina", "Eduardo",
  "Duda", "Anitta", "Maria", "Madu", "Dado", "Pablo", "Guilherme", "Yuri", "Marília", "Laura", "Tatiana",
  "Tatiane", "Abel", "Fernando", "Fernanda", "Fefe", "Dráuzio", "Bob", "Alice", "Carlos", "Matheus", "Mateus",
  "Camila", "Policarpo", "Regina", "Malu", "Emanuelle", "Emanuel", "Manuela", "Manu", "Emma", "Barbie",
  "Beyoncé", "Britney", "Madonna", "Stephany", "Letrux", "Glória", "Maicon", "Bebel", "Isabel", "Isa"
};

vector<string> SURNAMES = {
  "Ribeiro", "Costa", "Salgado", "Lastra", "Righetto", "Silva", "Dirceu", "Lula", "Marinho",
  "Tanimoto", "Lobeiro", "Pinhal", "Vitória", "Steven", "Hilton", "Grande", "Santos", "Oliveira", "Oliveti", 
  "Lacerda", "Bardo", "Botelho", "Salhani", "Saffi", "Borges", "Brito", "Pinto", "Tadeu", "Cruz",
  "Vittar", "Quaresma", "Edwiges", "Cândido", "Holmes", "Potter", "Belinati", "Brack", "Pompeu", "Mata",
  "Regina", "Braga", "Spears", "Gaspar", "Gaspari", "Bartolomeu", "Abreu", "Pires", "Ferrari", 
  "Ferrarini", "Maggi", "Campos", "Vítor", "Victor", "Augusto", "Barna", "Amarantos", "Picoli"
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

/**
 * GLOBAIS
 * -------
 * 
 * Variáveis globais
 */
int WIDTH_OFFSET = 20;
int MAX_WAIT_TIME = 10;
int MAX_PERSONS = 20;
float GAME_TOTAL_SCORE = 0.0;
int GAME_TOTAL_CORRECT_PERSON = 0;
int GAME_DIFFICULT = 1;
int GAME_MAX_TIME = 100;
auto GAME_INIT_TIME = steady_clock::now();
map<string, time_point<steady_clock>> START_TIMES;

/**
 * STRUCTS
 * -------
 * 
 * Estrutura de dados relativa a cada 'objeto pessoa'
 */
struct Person {
    int id = 0;
    string name;
    string surname;
    int wait_time;
    long int elapsed_time = 0;
    string msgPersonal = "";
};

vector<Person> PERSONS;

/**
 * VARIÁVEIS DE CONCORRÊNCIA (threads)
 * -----------------------------------
 * 
 * MUTEX
 */
mutex PERSONS_MUTEX;    // Controle de acesso para vetor de pessoas
mutex TIME_MUTEX;       // Controle de acesso para vetor de tempos de espera
mutex SCORE_MUTEX;      // Controle de acesso para valor de pontuação

/**
 * VARIÁVEIS CONDICIONAIS
 */
condition_variable 
    CONDITIONAL_VAR_PRODUCER, // Sinalizador para hibilitar produção
    CONDITIONAL_VAR_CONSUMER; // Sinalizador para habilitar consumo

/**
 * VARIÁVEIS ATOMICAS
 */
atomic<bool> IS_RUNNING(true);  // tipo atomico para que escrita / leitura simultanea
                                // por threads diferentes não resulte em comportamento atipico

/**
 * Construção de um objeto 'Pessoa' com campos arbitrariamente escolhidos
 * utilizando os "BANCOS DE DADOS" acima definidos
 */
Person generatePerson(int id) {
    return Person{
        id,
        NAMES[rand() % NAMES.size()],
        SURNAMES[rand() % SURNAMES.size()],
        rand() % MAX_WAIT_TIME + 1,
        0,
        MSG_PERSONAL[rand() % MSG_PERSONAL.size()],
    };
}

/**
 * Responsável pela produção de novas pessoas enquanto condição de limite máximo
 * de população não é atingida
 */
void producer() {
    int person_id = 1;

    while (IS_RUNNING) {
        /**
         * lock_persons: mutex
         * 
         * Vetor PERSONS variável global e compartilhada entre produtor e consumidor
         * assim a geração e adição ao vetor de uma nova 'person' somente deverá
         * ocorrer caso o vetor esteja livre para uso
         */
        unique_lock<mutex> lock_persons(PERSONS_MUTEX);

        /**
         * CONDITIONAL_VAR_PRODUCER: conditional variable
         * 
         * Permite a espera para que a ação do producer ocorra apenas enquanto o tamanho 
         * máximo não é atingido. O comportamento é equivalente ao 'busy wait'
         */
        CONDITIONAL_VAR_PRODUCER.wait(lock_persons, []() { return (int) PERSONS.size() < MAX_PERSONS; });

        if (!IS_RUNNING) break;

        Person new_person = generatePerson(person_id++);
        
        /**
         * lock_time: mutex
         * 
         * Vetor START_TIMES contém o tempo de início da espera associado a cada pessoa
         * Como será atualizado por cada thread, precisa apresentar exclusao mutua
         */
        {
            unique_lock<mutex> lock_time(TIME_MUTEX);
            START_TIMES[new_person.name] = steady_clock::now();

            PERSONS.push_back(new_person);
        }

        if (PERSONS.size() >= 8) 
          this_thread::sleep_for(seconds(1));

        /**
         * CONDITIONAL_VAR_CONSUMER: conditional variable
         * 
         * Notifica um dos threads consumidores (aleatoriamente) que uma nova pessoa 
         * está disponível
         */
        CONDITIONAL_VAR_CONSUMER.notify_one();

    }
}

/**
 * Função para fixar o cursor do terminal em uma posição específica
 * @param int x: posição horizontal
 * @param int y: posição vertical
 */
void setCursorPosition(int x, int y) {
    cout << "\033[" << x << ";" << y << "H";  // Move cursor to (x, y)
}

/**
 * Função para obter input no início fixo do terminal
 * @param string message: Mensagem ao usuario
 * @returns string Resposta do usuario
 */
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

/**
 * Função para impressão padrão de um objeto "pessoa"
 */
void printPersons() {
    int personsPosLine = 5;
    int personsPosCol = 1;
    setCursorPosition(personsPosLine, personsPosCol);  // Move to line 3, column 1 (third line)

    // Print header
    cout << left << setw(WIDTH_OFFSET) << "Nome" 
         << setw(WIDTH_OFFSET) << "Sobrenome" 
         << setw(WIDTH_OFFSET) << "Tempo de espera" 
         << setw(WIDTH_OFFSET) << "Tempo decorrido" 
         << endl;
    cout << "-----------------------------------------------------------------" << endl;  // Divider

    // Print each person's details
    for (const auto& person : PERSONS) {
        cout << setw(WIDTH_OFFSET) << person.name 
             << setw(WIDTH_OFFSET) << person.surname 
             << setw(WIDTH_OFFSET) << person.wait_time 
             << setw(WIDTH_OFFSET) << person.elapsed_time
             << endl;
    }
}

/**
 * Função para impressão padrão do tempo transcorrido
 */
void printTimer(long int seconds) {
    int timerPosLine = 4;
    int timerPosCol = 1;
    setCursorPosition(timerPosLine, timerPosCol);
    cout << "Timer: " << setw(timerPosLine) << seconds << "s";
}

/**
 * Função para impressão padrão do score atual
 */
void printScore(float score) {
    int scorePosLine = 4;
    int scorePosCol = 15;
    setCursorPosition(scorePosLine, scorePosCol);  
    cout << "Score: " << setw(scorePosCol) << score;
}

/**
 * Função base para impressão e atualização do tempo transcorrido
 * tanto para o contador global quanto para cada pessoa
 */
void CheckElapsedTime() {
  for (auto& person : PERSONS) {
    if (START_TIMES.find(person.name) != START_TIMES.end()) {
      auto now = steady_clock::now();
      auto elapsed = duration_cast<seconds>(now - START_TIMES[person.name]).count();
      person.elapsed_time = (long int) elapsed;
    }
  }

  auto elapsed = duration_cast<seconds>(steady_clock::now() - GAME_INIT_TIME).count();
  
  printScore(GAME_TOTAL_SCORE);
  printTimer(elapsed);
  printPersons();
}

/**
 * Atualização do score global
 * @param Person p Pessoa atendida corretamente e que será removida da lista
 */
void updateScore(Person p) {
    float ca, cb;

    string inputed_str = p.name;
    if (GAME_DIFFICULT == 1) {
        ca = 1;
        cb = 1;
    } else 
    if (GAME_DIFFICULT == 2) {
        ca = 2;
        cb = 0.8;
        inputed_str += p.surname;
    } else {
        ca = 3;
        cb = 0.5;
        inputed_str += p.surname + p.msgPersonal;
    }
    
    // * score = ca.sn - cb.|ts - tw|
    GAME_TOTAL_SCORE = GAME_TOTAL_SCORE + (ca * inputed_str.size()) - (cb * abs(p.elapsed_time - p.wait_time));
}

/**
 * Função para impressão da mensagem pessoal
 * @param string msg: Mensagem a ser impressa
 */
void printMsgPersonal(string msg) {
    int inputMsgPersonLine = 2;
    int inputMsgPersonCol = 1;

    setCursorPosition(inputMsgPersonLine, inputMsgPersonCol);
    cout << string(80, ' ');
    setCursorPosition(inputMsgPersonLine, inputMsgPersonCol);

    cout << msg;
}

/**
 * Função consumidor que será executada dentro de cada thread
 */
void consumer() {
    while (IS_RUNNING) {
        /**
         * PERSONS_MUTEX: mutex
         * 
         * bloqueio para que possa ser consumida a pessoa que foi corretamente
         * selecionada. Logo, como o vetor PERSONS será atualizado, deve ser
         * bloqueado para apenas um thread por vez
         */
        unique_lock<mutex> lock_persons(PERSONS_MUTEX);

        /**
         * CONDITIONAL_VAR_CONSUMER: conditional variable
         * 
         * Aguarda para que apenas seja possível excluir um elemento do vetor PERSONS
         * caso o mesmo tenha algum elemento. Similar ao CONDIT_VAR_PROD, é equivalente
         * ao comportamento do busy wait
         */
        CONDITIONAL_VAR_CONSUMER.wait(lock_persons, []() { return !PERSONS.empty(); });

        if (!IS_RUNNING) break;

        /**
         * TIME_MUTEX: mutex
         * PERSONS_MUTEX: mutex
         * 
         * Como o vetor START_TIMES será lido e o vetor PERSONS será lido
         * e atualizado, ambas variáveis devem ser bloqueadas caso estejam 
         * em uso por algum thread
         * 
         * Como a função é chamada pelo consumer que já apresenta em seu 
         * contexto mutex para PERSONS, aqui não é necessário que seja sinalizado
         */
        {
            unique_lock<mutex> lock_time(TIME_MUTEX);

            CheckElapsedTime();
        }
        string name = handleInput("> Atender pessoa: ");

        auto it = find_if(PERSONS.begin(), PERSONS.end(), [&](const Person& person) {
            if (GAME_DIFFICULT == 1) return person.name == name;
            return person.name + " " + person.surname == name;
        });

        string msgPersonal = "";
        if (GAME_DIFFICULT == 3 && it != PERSONS.end()) {
            printMsgPersonal(it->msgPersonal);
            msgPersonal = handleInput("> Mensagem personalizada: ");
            printMsgPersonal("");
            if (msgPersonal != it->msgPersonal) msgPersonal = "";
        }

        
        /**
         * SCORE_MUTEX: mutex
         * 
         * Como a variável global de score GAME_TOTAL_SCORE será atualizada
         * apenas um thread deverá ler e atualiza-la
         */
        {
            unique_lock<mutex> lock_score(SCORE_MUTEX);
            if (it != PERSONS.end()) {
                if (GAME_DIFFICULT < 3) {
                    updateScore(*it);
                    PERSONS.erase(it);
                    GAME_TOTAL_CORRECT_PERSON++;  
                } else {
                    if (msgPersonal.length() > 0) {
                        updateScore(*it);
                        PERSONS.erase(it);
                        GAME_TOTAL_CORRECT_PERSON++;
                    }
                }
            }
        }

        /**
         * CONDITIONAL_VAR_PRODUCER: conditional variable
         * 
         * Notifica um dos threads produtores (aleatoriamente) que um novo
         * espaço foi liberado no vetor PERSONS e está disponível para produção
         * de um novo elemento
         */
        CONDITIONAL_VAR_PRODUCER.notify_one();

        this_thread::sleep_for(milliseconds(500));
    }
}

/**
 * Função para exibir banner do jogo
 */
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

/**
 * Função para exibir tela inicial com menu de seleção
 */
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

/**
 * Função para exibir menu interativo, com seleção de opções
 * @param int selectedOption: Opção selecionada 
 */
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

/**
 * Função para exibir mensagem de volta ao menu principal
 */
void backToMainMenu(string msg) {
    handleInput(msg);
}

/**
 * Função para exibir regras do jogo
 */
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
        - sn = NOME + SOBRENOME             | ca = 2 | cb = 0.8 |

        [3] Nomes compostos com 
           mensagem personalizada
        - sn = NOME + SOBRENOME + MENSAGEM  | ca = 3 | cb = 0.5 | 


        Voltar ao menu inicial... [qualquer tecla]
    )" << endl;
        
    backToMainMenu("");
}

/**
 * Função para exibir cálculo do score e seleção de dificuldade
 */
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
        - sn = NOME + SOBRENOME             | ca = 2 | cb = 0.8 |

        [3] Nomes compostos com 
           mensagem personalizada
        - sn = NOME + SOBRENOME + MENSAGEM  | ca = 3 | cb = 0.5 | 

    )" << endl;

    string str_dificult = handleInput("Mudar dificuldade? [1|2|3] ");
    GAME_DIFFICULT = stoi(str_dificult);
    GAME_DIFFICULT = GAME_DIFFICULT <= 1 ? 1 : GAME_DIFFICULT >= 3 ? 3 : 2;

}

/**
 * Função para exibir história do jogo
 */
void displayAbout() {
    system("clear");
    displayBanner();
    
    auto elapsed = duration_cast<seconds>(steady_clock::now() - GAME_INIT_TIME).count();
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

/**
 * Função para exibir fim do jogo, com métricas finais calculadas
 */
void printGameEnd() {
    system("clear");
    displayBanner();

    auto person_max_time = max_element(
        PERSONS.begin(), PERSONS.end(), 
        [](Person& a, Person& b) { return a.elapsed_time < b.elapsed_time;}
    );
    
    cout << R"(
        ----------------------------------------------------------------
        O expediente chegou ao fim!

        Foi uma verdadeira luta para que o bistrô não ficasse sem todo o
        suporte necessário, mas parabéns, você se esforçou muito!
        ----------------------------------------------------------------

        PONTUAÇÃO FINAL
        )" << endl;

        cout << "        Descrição" << "\t\t\t\t" << "Pontuação" << endl;
        cout << "        " << string(65, '-') << endl;

        cout << "        Pontuação final" << "\t\t\t\t" << GAME_TOTAL_SCORE << endl;
        cout << "        Número de pessoas atendidas" << "\t\t" << GAME_TOTAL_CORRECT_PERSON << endl;
        cout << "        Número de pessoas não atendidas" << "\t\t" << PERSONS.size() << endl;
        cout << "        Maior tempo de espera" << "\t\t\t" << person_max_time->elapsed_time << endl;
        cout << "        Pessoa com maior tempo de espera" << "\t" << person_max_time->name << " " << person_max_time->surname << endl;
    
    cout << R"(
        Tenha uma ótima noite!
        ----------------------------------------------------------------
        )" << endl;
}

/**
 * Função para interação contínua do menu inicial
 */
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
            GAME_MAX_TIME = stoi(str_game_max_time);
            str_input = handleInput("Começar? ");
            system("clear");
            break;
        }
    }
}


int main() {
    srand(time(0));
    system("clear");
    
    printGreeting();
    menuInteraction();

    /**
     * Inicialização dos threads produtor / consumidor
     */
    thread producer_thread(producer);
    thread consumer_thread(consumer);

    /**
     * Definiação de tempo de espera até que o jogo finalize
     */
    this_thread::sleep_for(seconds(GAME_MAX_TIME));

    IS_RUNNING = false;

    /**
     * Notificar todos threads construídos para todas leiam a variável
     * IS_RUNNING = false e possam finalizar a execução
     */
    CONDITIONAL_VAR_PRODUCER.notify_all();
    CONDITIONAL_VAR_CONSUMER.notify_all();

    /**
     * Sinaliza para que todo thread seja bloqueado até que a execução 
     * de todos ocorra (sincronização)
     */    
    producer_thread.join();
    consumer_thread.join();

    printGameEnd();
    return 0;
}
