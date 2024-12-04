# Projeto Final

Aluno: Jorge Augusto Salgado Salhani

Nº USP: 8927418

## Jogo: El Garçon

### Introdução
O jogo "El Garçon" foi desenvolvido como trabalho final para a disciplina de [Sistemas Operacionais I](https://uspdigital.usp.br/jupiterweb/obterDisciplina?nomdis=&sgldis=SSC0140) para estudo (não somente, mas em especial) da utilização de threads e de suas estruturas de dados particulares, tais como semáforos, mutex, variáveis condicionais, variáveis atômicas, entre outros. Grande parte das referências utilizadas na construção do código podem ser encontradas em [cppreference](https://en.cppreference.com/w/cpp/thread), especialmente na descrição e exemplos de [mutex](https://en.cppreference.com/w/cpp/thread/mutex), [join](https://en.cppreference.com/w/cpp/thread/thread/join) e [condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable).

O código desenvolvido faz uso do modelo conhecido como Produtor-Consumidor, onde threads construídos competem por um espaço limitado de recursos (fila de tamanho máximo de pessoas, neste caso) e são responsáveis por adicionar em ou remover elementos de uma variável compartilhada ao longo do tempo

Neste jogo, o thread "Produtor" continuamente adiciona novas pessoas à fila e é papel de quem está jogando atuar como "Consumidor", removendo as pessoas da fila conforme seus nomes são digitados corretamente

### Organização (foco em threads)
Nesta seção explicamos o código desenvolvido, com ênfase às partes relacionadas ao uso de threads

```cpp
int MAX_PERSONS = 20;
float GAME_TOTAL_SCORE = 0.0;
int GAME_TOTAL_CORRECT_PERSON = 0;
auto GAME_INIT_TIME = steady_clock::now();
map<string, time_point<steady_clock>> START_TIMES;
vector<Person> PERSONS;
```

```cpp
mutex PERSONS_MUTEX;
mutex TIME_MUTEX;
mutex SCORE_MUTEX;
```

```cpp
condition_variable 
    CONDITIONAL_VAR_PRODUCER,
    CONDITIONAL_VAR_CONSUMER;
```

```cpp
atomic<bool> IS_RUNNING(true);
```

```cpp
int main() {
    thread producer_thread(producer);
    thread consumer_thread(consumer);
    this_thread::sleep_for(seconds(GAME_MAX_TIME));
    IS_RUNNING = false;
    CONDITIONAL_VAR_PRODUCER.notify_all();
    CONDITIONAL_VAR_CONSUMER.notify_all(); 
    producer_thread.join();
    consumer_thread.join();
    return 0;
}
```

```cpp
void consumer() {
    while (IS_RUNNING) {
        unique_lock<mutex> lock_persons(PERSONS_MUTEX);
        CONDITIONAL_VAR_CONSUMER.wait(lock_persons, []() { return !PERSONS.empty(); });
        if (!IS_RUNNING) break;
        {
            unique_lock<mutex> lock_time(TIME_MUTEX);
            CheckElapsedTime();
        }
        {
            unique_lock<mutex> lock_score(SCORE_MUTEX);
            updateScore(*it);
            PERSONS.erase(it);
            GAME_TOTAL_CORRECT_PERSON++;
        }
        CONDITIONAL_VAR_PRODUCER.notify_one();
        this_thread::sleep_for(milliseconds(500));
    }
}
```

```cpp
void producer() {
    int person_id = 1;
    while (IS_RUNNING) {
        unique_lock<mutex> lock_persons(PERSONS_MUTEX);
        CONDITIONAL_VAR_PRODUCER.wait(lock_persons, []() { return (int) PERSONS.size() < MAX_PERSONS; });
        if (!IS_RUNNING) break;
        Person new_person = generatePerson(person_id++);
        {
            unique_lock<mutex> lock_time(TIME_MUTEX);
            START_TIMES[new_person.name] = steady_clock::now();

            PERSONS.push_back(new_person);
        }
        if (PERSONS.size() >= 5) 
          this_thread::sleep_for(seconds(1));
        CONDITIONAL_VAR_CONSUMER.notify_one();

    }
}
```

### Como jogar
Abaixo explicamos como pode ser feita a execução do código e instruções de como jogar

#### Executando
O jogo foi desenvolvido para windows (via [Cygwin](https://cygwin.com/cygwin-ug-net/using.html)) e pode apresentar erros caso operado em sistemas unix ou iOS

Estando na raiz do projeto ('operatingSystems1')

1. Caminhar ao local do arquivo fonte
```bash
cd apoteose/src
```

2. Construir executável
```bash
make
```

3. Executar programa criado
```bash
make run
```

Alternativamente, basta executar programa disponibilizado em
```bash
apoteose\src\exec.exe
```

#### Jogando

A inicialização do jogo resulta na imagem abaixo

![Boas vindas](./imgs/imageWelcome.png "Boas vindas")

Nela podem ser selecionados os valores 1, 2, 3, 4. Qualquer das opções selecionadas resulta na marcação conforme mostrado abaixo. Caso confirmado "s", somos direcionados à página relativa à opção

![Sobre Selecionado](./imgs/imageAbout0.png "Sobre Selecionado")

##### Sobre
A opção 4 nos leva à história do jogo. Para retornar ao menu inicial, basta digitar qualquer tecla conforme indicado no topo da tela

![Sobre](./imgs/imageAbout1.png "Sobre")

##### Regras
A opção 2 nos leva às regras do jogo. Para retornar ao menu inicial, basta digitar qualquer tecla conforme indicado no topo da tela

![Regras](./imgs/imageRules.png "Regras")

##### Dificuldades
A opção 3 nos leva às dificuldades do jogo. Nesta tela devemos selecionar 1, 2, 3 de acordo com a dificuldade desejada

![Dificuldades](./imgs/imageDificults.png "Dificuldades")

##### Jogar
Por fim a opção 1 nos leva ao início de fato do jogo. Após confirmação da opção 1 somos direcionados à pergunta da duração do jogo, conforme o topo da imagem abaixo. Nela informamos qual o tempo máximo (em segundos) que uma determinada pessoa pode esperar na fila. O jogo acaba quando esse número é atingido por alguma das pessoas

![Inicio](./imgs/imageDuration.png "Inicio")

Ao iniciar o jogo, algumas pessoas já terão sido geradas conforme imagem abaixo. Sua função é digitar, conforme o topo da tela, o nome da pessoa a ser atendida

![Jogo1](./imgs/imageGame1.png "Jogo1")

No exemplo abaixo inserimos o nome "Luan Augusto", que foi removido da lista ao apertarmos a tecla ENTER. A pontuação e o tempo são incrementados conforme calculado. Neste caso estamos na dificuldade 2, onde é necessário digitarmos "Nome" + " " + "Sobrenome". Como digitamos corretamente, a pessoa escolhida é removida da lista, uma nova é gerada e inserida ao final.

![Jogo2](./imgs/imageGame2.png "Jogo2")

Caso o nome não seja digitado corretamente, tal como a imagem abaixo, a pontuação é mantida e também uma nova pessoa é inserida na fila

![Jogo3](./imgs/imageGame3.png "Jogo3")

Ao final do jogo, as pontuações e métricas são exibidas na tela, conforme imagem abaixo

![Jogo4](./imgs/imageGame4.png "Jogo4")

Caso a dificuldade escolhida seja 3, após a digitação correta do nome e sobrenome, cada pessoa apresenta uma mensagem personalizada que deverá ser inserida no pedido

![Jogo5](./imgs/imageGame1Dif3.png "Jogo5")

Apenas com a inserção correta da frase é que a pessoa é removida da fila conforme imagens abaixo onde a pessoa escolhida foi "Madonna Tadeu"

![Jogo6](./imgs/imageGame2Dif3.png "Jogo6")

![Jogo7](./imgs/imageGame3Dif3.png "Jogo7")

E a final, mostramos também o resultado do jogo atual

![Jogo8](./imgs/imageGame4Dif3.png "Jogo8")
