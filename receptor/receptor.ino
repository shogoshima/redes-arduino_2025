#define PINO_DADOS 4
#define PINO_CLOCK 5
#define PINO_RTS 6
#define PINO_CTS 7

bool recebendo = false;
byte frame[9];
int bitIndex = 0;
int contagemParidade = 0;

// Função que será chamada quando o clock subir
void lerBit(){
  if (!recebendo) return;

  if (bitIndex < 9) {
    // Ler dado
    frame[bitIndex] = digitalRead(PINO_DADOS);
    if (bitIndex < 8 && frame[bitIndex] == HIGH) 
      contagemParidade++; 
    bitIndex++;
  } 
}

// Executada uma vez quando o Arduino reseta
void setup(){
  //desabilita interrupcoes
  noInterrupts();
  
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);

  // Inicializa portas
  pinMode(PINO_DADOS, INPUT);
  pinMode(PINO_CLOCK, INPUT);
  pinMode(PINO_RTS, INPUT);
  pinMode(PINO_CTS, OUTPUT);

  // Inicializa porta
  digitalWrite(PINO_CTS, LOW);

  // Quando o pino de clock estiver subindo, então chamo a função
  attachInterrupt(digitalPinToInterrupt(PINO_CLOCK), lerBit, RISING);
  
  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop ( ) {
  if (!recebendo && digitalRead(PINO_RTS) == HIGH) {
    // Setar as variáveis iniciais
    recebendo = true;
    bitIndex = 0;
    contagemParidade = 0;

    // Setar o CTS para 1
    // Inicializar temporizador
    digitalWrite(PINO_CTS, HIGH);
  }

  if (recebendo && digitalRead(PINO_RTS) == LOW) {
    // Checar bit de paridade e tals
    byte paridade = frame[8];
    if (contagemParidade % 2 == paridade) {
      // Está correto. Vou printar
      char resultado = 0;
      for (int i = 0; i < 8; i++) {
        // shift do bit para a posição correta
        resultado |= (frame[i] & 0x01) << i;
      }

      Serial.print("Recebido: ");
      Serial.println(resultado);
    } else {
      // Incorreto
      Serial.println("Bit de paridade incorreto");
    }

    // terminar
    digitalWrite(PINO_CTS, LOW);
    recebendo = false;
  }
}
