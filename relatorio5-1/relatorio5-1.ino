#include <Servo.h>

// Definindo os pinos dos servos
const int servoPins[4] = {2, 3, 4, 5};
const char* servoNames[4] = {"Ombro", "Base", "Cotovelo", "Garra"};

// Limites dos servos
const int servoMins[4] = {100, 0, 90, 90};
const int servoMaxs[4] = {180, 180, 150, 120};
//const int servoMins[4] = {0, 0, 0, 0};
//const int servoMaxs[4] = {180, 180, 180, 180};

// Criando objetos Servo
Servo servos[4];

// Variáveis para armazenar as posições dos servos
int servoPositions[4] = {180, 0, 90, 120}; // Posições iniciais dentro dos limites

// Estados gravados (até 5 estados com 4 servos cada)
int recordedStates[5][4];
int recordedStateCount = 0;

// Estados do sistema
enum Mode {INPUT_DATA, RECORD, EXECUTE};
Mode currentMode = INPUT_DATA;

void setup() {
  // Inicializando os servos com as posições iniciais
  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoPositions[i]); // Move o servo para a posição inicial
  }

  // Inicializando a comunicação serial
  Serial.begin(9600);
  Serial.println("Sistema iniciado. Escolha uma opção: ");
  Serial.println("1 - Alterar posição de um motor");
  Serial.println("2 - Gravar estado atual");
  Serial.println("3 - Executar movimento");
  Serial.println("4 - Executar movimento de demonstração");
  Serial.println("5 - Executar movimento de pegar e soltar a bolinha");
  Serial.println("6 - Calibrar limites dos servos");
}

void loop() {
  if (Serial.available()) {
    char choice = Serial.read();

    switch (choice) {
      case '1':
        currentMode = INPUT_DATA;
        handleInputData();
        break;
      case '2':
        if (recordedStateCount < 5) {
          currentMode = RECORD;
          recordState();
        } else {
          Serial.println("Limite de estados gravados atingido!");
        }
        break;
      case '3':
        currentMode = EXECUTE;
        executeMovement();
        break;
      case '4':
        demoMovement();
        Serial.println("Movimento de demonstração concluído.");
        Serial.println("Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar");
        break;
      case '5':
        pickAndPlaceBall();
        Serial.println("Movimento de pegar e soltar a bolinha concluído.");
        Serial.println("Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar");
        break;
      case '6':
        calibrateServoLimits();
        Serial.println("Calibração concluída.");
        Serial.println("Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar, 6-Calibrar");
        break;
      default:
        Serial.println("Escolha inválida. Tente novamente.");
        break;
    }
  }
}

void calibrateServoLimits() {
  Serial.println("Iniciando calibração dos servos.");
  
  // Arrays para armazenar os limites mínimos e máximos de cada servo
  int servoMinsCalib[4] = {0, 0, 0, 0};
  int servoMaxsCalib[4] = {180, 180, 180, 180};

  for (int i = 0; i < 4; i++) {
    Serial.print("Calibrando Servo ");
    Serial.println(servoNames[i]);
    Serial.println("Use 'a' para diminuir, 'd' para aumentar, 's' para salvar e ir para o próximo servo.");
    int position = servoPositions[i];
    servos[i].write(position);

    while (true) {
      if (Serial.available()) {
        char command = Serial.read();

        if (command == 'a') {
          if (position > 0) {
            position -= 10;
            if (position < 0) position = 0;
            servos[i].write(position);
            servoPositions[i] = position;
            Serial.print("Posição atual: ");
            Serial.println(position);
          } else {
            Serial.println("Alcançou a posição mínima.");
          }
        } else if (command == 'd') {
          if (position < 180) {
            position += 10;
            if (position > 180) position = 180;
            servos[i].write(position);
            servoPositions[i] = position;
            Serial.print("Posição atual: ");
            Serial.println(position);
          } else {
            Serial.println("Alcançou a posição máxima.");
          }
        } else if (command == 's') {
          // Registrar os limites encontrados
          servoMinsCalib[i] = position < servoPositions[i] ? position : servoPositions[i];
          servoMaxsCalib[i] = position > servoPositions[i] ? position : servoPositions[i];
          Serial.print("Limites para o Servo ");
          Serial.print(servoNames[i]);
          Serial.print(": Mínimo = ");
          Serial.print(servoMinsCalib[i]);
          Serial.print(", Máximo = ");
          Serial.println(servoMaxsCalib[i]);
          break;
        }
      }
    }
    // Pequena pausa antes de passar para o próximo servo
    delay(500);
  }

  Serial.println("Calibração completa. Limites encontrados:");
  for (int i = 0; i < 4; i++) {
    Serial.print("Servo ");
    Serial.print(servoNames[i]);
    Serial.print(": Mínimo = ");
    Serial.print(servoMinsCalib[i]);
    Serial.print(", Máximo = ");
    Serial.println(servoMaxsCalib[i]);
  }

  // Opcional: atualizar os limites no código ou salvar em EEPROM
}

void handleInputData() {
  // Exibir os nomes dos servos para seleção
  Serial.println("Selecione o motor:");
  for (int i = 0; i < 4; i++) {
    Serial.print(i + 1);
    Serial.print(" - ");
    Serial.println(servoNames[i]);
  }

  // Limpa qualquer entrada anterior
  while (Serial.available() > 0) {
    Serial.read();
  }

  // Aguarda a nova entrada
  while (!Serial.available());

  // Lê a entrada até a nova linha
  String input = Serial.readStringUntil('\n');
  int motorIndex = input.toInt() - 1; // Índice de 0 a 3

  if (motorIndex >= 0 && motorIndex < 4) {
    Serial.print("Escolha a nova posição para o motor ");
    Serial.print(servoNames[motorIndex]);
    Serial.print(" (");
    Serial.print(servoMins[motorIndex]);
    Serial.print(" a ");
    Serial.print(servoMaxs[motorIndex]);
    Serial.println("): ");

    // Limpa qualquer entrada anterior
    while (Serial.available() > 0) {
      Serial.read();
    }

    // Aguarda a nova entrada
    while (!Serial.available());

    String posInput = Serial.readStringUntil('\n');
    int position = posInput.toInt();

    if (position >= servoMins[motorIndex] && position <= servoMaxs[motorIndex]) {
      moveServoSlowly(motorIndex, position);
      Serial.print("Motor ");
      Serial.print(servoNames[motorIndex]);
      Serial.print(" movido para a posição ");
      Serial.println(position);
    } else {
      Serial.println("Posição inválida. Tente novamente.");
    }
  } else {
    Serial.println("Motor inválido. Tente novamente.");
  }

  Serial.println("Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar");
}

void recordState() {
  Serial.print("Gravando estado ");
  Serial.println(recordedStateCount + 1);

  for (int i = 0; i < 4; i++) {
    recordedStates[recordedStateCount][i] = servoPositions[i];
  }

  Serial.print("Estado ");
  Serial.print(recordedStateCount + 1);
  Serial.print(": ");
  for (int i = 0; i < 4; i++) {
    Serial.print(servoNames[i]);
    Serial.print("=");
    Serial.print(recordedStates[recordedStateCount][i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println(" gravado.");

  recordedStateCount++;
  Serial.println("Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar");
}

void executeMovement() {
  if (recordedStateCount == 0) {
    Serial.println("Nenhum estado gravado. Gravar primeiro!");
    return;
  }

  Serial.println("Executando sequência gravada...");
  for (int i = 0; i < recordedStateCount; i++) {
    Serial.print("Executando estado ");
    Serial.println(i + 1);
    
    int servosToMove[4] = {0, 1, 2, 3};
    int targetPositions[4];
    for (int j = 0; j < 4; j++) {
      targetPositions[j] = recordedStates[i][j];
    }

    // Exibir as posições que serão movidas
    Serial.print("Movendo para posições: ");
    for (int j = 0; j < 4; j++) {
      Serial.print(servoNames[j]);
      Serial.print("=");
      Serial.print(targetPositions[j]);
      if (j < 3) Serial.print(", ");
    }
    Serial.println();

    moveServosSimultaneously(servosToMove, targetPositions, 4);
    
    delay(1000); // Espera 1 segundo entre estados
  }

  Serial.println("Sequência concluída. Escolha uma nova opção: 1-Alterar, 2-Gravar, 3-Executar, 4-Demo, 5-Pegar/Soltar");
}

void moveServoSlowly(int servoIndex, int targetPos) {
  // Constranger a posição alvo aos limites do servo
  targetPos = constrain(targetPos, servoMins[servoIndex], servoMaxs[servoIndex]);

  int currentPos = servoPositions[servoIndex];

  // Log da posição atual e da próxima posição
  Serial.print("Movendo servo ");
  Serial.print(servoNames[servoIndex]);
  Serial.print(" de ");
  Serial.print(currentPos);
  Serial.print(" para ");
  Serial.println(targetPos);

  if (currentPos < targetPos) {
    for (int pos = currentPos; pos <= targetPos; pos++) {
      servos[servoIndex].write(pos);
      
      Serial.print(pos);
      Serial.print(" - ");
      delay(100); // Ajuste este valor para controlar a velocidade
    }
  } else {
    for (int pos = currentPos; pos >= targetPos; pos--) {
      servos[servoIndex].write(pos);
      Serial.print(pos);
      Serial.print(" - ");
      delay(100); // Ajuste este valor para controlar a velocidade
    }
  }
  servoPositions[servoIndex] = targetPos;
}

// void moveServosSimultaneously(int servoIndices[], int targetPositions[], int numServos) {
//   // Arrays para armazenar as posições atuais, posições alvo, incrementos e posições intermediárias
//   float currentPositions[numServos];
//   float targetPositionsF[numServos]; // Posições alvo após serem limitadas
//   float increments[numServos];
//   float positions[numServos]; // Posições durante o movimento

//   // Obter as posições atuais e limitar as posições alvo
//   for (int i = 0; i < numServos; i++) {
//     int servoIndex = servoIndices[i];
//     // Obter a posição atual do servo
//     currentPositions[i] = servoPositions[servoIndex];
//     // Limitar a posição alvo aos limites do servo
//     int constrainedTargetPos = constrain(targetPositions[i], servoMins[servoIndex], servoMaxs[servoIndex]);
//     targetPositionsF[i] = constrainedTargetPos;
//   }

//   // Calcular o número máximo de passos necessários
//   int maxSteps = 0;
//   for (int i = 0; i < numServos; i++) {
//     int steps = abs((int)(targetPositionsF[i] - currentPositions[i]));
//     if (steps > maxSteps) {
//       maxSteps = steps;
//     }
//   }

//   if (maxSteps == 0) {
//     // Já estamos nas posições alvo
//     return;
//   }

//   // Calcular incrementos para cada servo
//   for (int i = 0; i < numServos; i++) {
//     float delta = targetPositionsF[i] - currentPositions[i];
//     increments[i] = delta / maxSteps;
//     positions[i] = currentPositions[i]; // Iniciar da posição atual
//   }

//   // Mover os servos em incrementos
//   for (int step = 1; step <= maxSteps; step++) {
//     for (int i = 0; i < numServos; i++) {
//       int servoIndex = servoIndices[i];
//       positions[i] += increments[i];
//       int newPos = round(positions[i]);

//       // Apenas enviar comando se a posição mudou
//       if (newPos != servoPositions[servoIndex]) {
//         servos[servoIndex].write(newPos);
//         servoPositions[servoIndex] = newPos; // Atualizar a posição do servo
//       }
//     }
//     delay(20); // Ajuste este valor para controlar a velocidade geral
//   }

//   // Garantir que os servos alcancem exatamente as posições alvo
//   for (int i = 0; i < numServos; i++) {
//     int servoIndex = servoIndices[i];
//     int finalPos = (int)targetPositionsF[i];
//     if (servoPositions[servoIndex] != finalPos) {
//       servos[servoIndex].write(finalPos);
//       servoPositions[servoIndex] = finalPos;
//     }
//   }
// }

void moveServosSimultaneously(int servoIndices[], int targetPositions[], int numServos) {
  for (int i = 0; i < numServos; i++) {
    int servoIndex = servoIndices[i];
    int targetPos = constrain(targetPositions[i], servoMins[servoIndex], servoMaxs[servoIndex]);
    // moveServoSlowly(servoIndex, targetPos);
    
    servos[servoIndex].write(targetPos);
    delay(500); // Pequena pausa entre os movimentos dos servos (opcional)
  }
}


void demoMovement() {
  // Movimento de demonstração dos servos

  // Exemplo: mover a base de um lado para o outro
  moveServoSlowly(3, 0);    // Base para a posição inicial
  delay(1000);
  moveServoSlowly(3, 180);  // Base para a posição oposta
  delay(1000);
  moveServoSlowly(3, 90);   // Base para a posição central
  delay(5000);

  // moveServoSlowly(1, 0);    // Base para a posição inicial
  // delay(1000);
  // moveServoSlowly(1, 180);  // Base para a posição oposta
  // delay(1000);
  // moveServoSlowly(1, 90);   // Base para a posição central
  // delay(5000);

  // moveServoSlowly(2, 0);    // Base para a posição inicial
  // delay(1000);
  // moveServoSlowly(2, 180);  // Base para a posição oposta
  // delay(1000);
  // moveServoSlowly(2, 90);   // Base para a posição central
  // delay(5000);

  // moveServoSlowly(3, 0);    // Base para a posição inicial
  // delay(1000);
  // moveServoSlowly(3, 180);  // Base para a posição oposta
  // delay(1000);
  // moveServoSlowly(3, 90);   // Base para a posição central
  // delay(5000);


  // Outros movimentos podem ser adicionados conforme desejado
}

void pickAndPlaceBall() {
  // Fluxo 1 - Estado inicial
  Serial.println("Iniciando estado inicial");
  servos[0].write(180);
  delay(1000);
  servos[1].write(0);
  delay(1000);
  servos[2].write(90);
  delay(1000);
  servos[3].write(80);
  Serial.println("Finalizando estado inicial");
  delay(5000);
  
  Serial.println("Iniciando Indo pegar a bolinha");
  servos[1].write(90);
  delay(1000);
  servos[0].write(180);
  delay(1000);
  servos[0].write(90);
  delay(1000);
  servos[2].write(90);
  Serial.println("Finalizando Indo pegar a bolinha");
  delay(5000);
  
  Serial.println("Iniciando Pegar a bola");
  servos[3].write(120);
  Serial.println("Finalizando Pegar a bola");
  delay(5000);

  Serial.println("Iniciando Levar a bola");
  
  servos[0].write(180);
  delay(1000);
  servos[2].write(90);
  delay(1000);
  servos[1].write(0);
  Serial.println("Finalizando Levar a bola");
  delay(5000);

  Serial.println("Iniciando Soltar a bolinha");
  servos[3].write(80);
  delay(1000);
  Serial.println("Finalizando Soltar a bolinha");
}

