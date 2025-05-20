#define PINO_CLOCK 3
#define PINO_DADOS 4
#define PINO_CTS 5
#define PINO_RTS 6

#define BAUD_RATE 25

#include "Temporizador.h"

char dado;
bool paridade;
bool enviando = false;
int bitIndex = 0;
byte frame[9]; // 8 bits + 1 bit de paridade
bool clock = HIGH;

// Calcula bit de paridade - Par
bool bitParidade(char d){
  int ct = 0;
  for (int i = 0; i < 8; i++) {
    if (d & (1<<i)) ct++;
  }
  return (ct % 2 == 1); // Virar par
}

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect){
  if (!enviando) return;

  // Alternar o clock
  clock = !clock;
  digitalWrite(PINO_CLOCK, clock);

  if (clock == LOW) {
    // Quando o clock está descendo, então vou escrever no pino 
    // O receptor não pode ler aqui
    if (bitIndex < 9) {
      // Mandar o bit
      // Serial.print("Mandando o bit: ");
      // Serial.println(frame[bitIndex]);

      digitalWrite(PINO_DADOS, frame[bitIndex]);
      bitIndex++;
    } else {
      // Acabei, vou setar o RTS para 0
      digitalWrite(PINO_RTS, LOW);
      enviando = false;
      paraTemporizador();
    }
  }
}

// Executada uma vez quando o Arduino reseta
void setup(){
  //desabilita interrupcoes
  noInterrupts();
  
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);
  
  // Inicializa portas
  pinMode(PINO_DADOS, OUTPUT);
  pinMode(PINO_CLOCK, OUTPUT);
  pinMode(PINO_RTS, OUTPUT);
  pinMode(PINO_CTS, INPUT);

  // Inicializa dados
  digitalWrite(PINO_DADOS, LOW);
  digitalWrite(PINO_CLOCK, LOW);
  digitalWrite(PINO_RTS, LOW);

  // Configura timer
  configuraTemporizador(BAUD_RATE);

  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop () {
  // Pegar input do USB
  if (!enviando && Serial.available()) {
    // Tem dados. Então calcular bitparidade
    dado = Serial.read();
    if (dado < 32 || dado > 126) return;

    // Coloca no frame a mensagem a ser enviada
    for (int i = 0; i < 8; i++)
      frame[i] = ((1<<i) & dado) != 0;
    frame[8] = bitParidade(dado);

    // Printar o frame que estou mandando
    char resultado = 0;
    for (int i = 0; i < 8; i++) {
      resultado |= (frame[i] & 0x01) << i;
    }
    Serial.print("Dado a mandar: ");
    Serial.println(resultado);

    // Começa o handshake
    // Emissor seta RTS para HIGH, e espera que o receptor sete o CTS  
    digitalWrite(PINO_RTS, HIGH);
    while(digitalRead(PINO_CTS) == LOW) {}
    Serial.println("CTS recebido. Começando transmissão...");

    // Inicia a transmissão com o clock
    bitIndex = 0;
    enviando = true;
    clock = HIGH;
    iniciaTemporizador();
  }
}
