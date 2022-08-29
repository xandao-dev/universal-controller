/**********************CONTROLE REMOTO PROGRAMAVEL****************************
*
* Observacoes: As teclas A, B, C e D serão reservadas, as outras teclas serão programáveis.
* O botao 'A' será usado para gravar(ou apenas ler) novos codigos, enquanto estiver gravando o led estara aceso e BT enviando sinais.
* O botão 'B' será usado como jammer(bloqueia sinais infravermelhos), o led piscara lentamente e o BT envia sinal no comeco e no final
* O botão 'C' será usado como loopOff(um loop que tenta desligar uma grande variedade de aparelhos), o led piscara rapidamente e o BT envia sinal no comeco e no final
* O botão 'D' será usado para finalizar operacoes como gravacao, jammer e o loopOff.
* 
*
* Não estamos utilizando o formato RAW no momento por limitações de memória dinâmica.
*
*->Alguns Códigos são enviados em sequencia ou repetido algumas vezes, habilitar essa possibilidade(BIN,DEC,HEX)
******************************************************************************/
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "src\Keypad_Matrix\Keypad_Matrix.h"
#include "src\IRremote\IRremote.h"

#define qtdLinhas  4
#define qtdColunas  4
#define qtdPinosProgramaveis  12

#define RECV_PIN  2
#define BT_RX  19
#define BT_TX  18
#define Rainbow_PIN  8

const byte PinosLinhas[qtdLinhas] = {17, 16, 15, 14};
const byte PinosColunas[qtdColunas] = {12, 11, 10,9};


const unsigned short EEPROM_CodeAddresses[qtdPinosProgramaveis]={0,8,16,24,32,40,48,56,64,72,80,88};//0-95
const unsigned short EEPROM_CodeLenAddresses[qtdPinosProgramaveis]={96,97,98,99,100,101,102,103,104,105,106,107};//96-107
const unsigned short EEPROM_CodTypeAddresses[qtdPinosProgramaveis]={108,109,110,111,112,113,114,115,116,117,118,119};//108-119
//const unsigned short EEPROM_RAWAddress[RAWBUF] = {120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254, 256, 258, 260, 262, 264, 266, 268, 270, 272, 274, 276, 278, 280, 282, 284, 286, 288, 290, 292, 294, 296, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378, 380, 382, 384, 386, 388, 390, 392, 394, 396, 398, 400, 402, 404, 406, 408, 410, 412, 414, 416, 418, 420, 422, 424, 426, 428, 430, 432, 434, 436, 438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 460, 462, 464, 466, 468, 470, 472, 474, 476, 478, 480, 482, 484, 486, 488, 490, 492, 494, 496, 498, 500, 502, 504, 506, 508, 510, 512, 514, 516, 518, 520, 522, 524, 526, 528, 530, 532, 534, 536, 538, 540, 542, 544, 546, 548, 550, 552, 554, 556, 558, 560, 562, 564, 566, 568, 570, 572, 574, 576, 578, 580, 582, 584, 586, 588, 590, 592, 594, 596, 598, 600, 602, 604, 606, 608, 610, 612, 614, 616, 618, 620, 622, 624, 626, 628};

const char matriz_teclas[qtdLinhas][qtdColunas] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

/*-----------------------------CONSTRUTORES---------------------------------*/
SoftwareSerial mySerial(BT_TX,BT_RX);//-> BT
Keypad_Matrix teclado = Keypad_Matrix( makeKeymap(matriz_teclas), PinosLinhas, PinosColunas, qtdLinhas, qtdColunas); //-> Teclado
IRrecv irrecv(RECV_PIN); //-> TSOP4838
IRsend irsend;//Pin 3 -> Led IR
decode_results results; //-> Leitura do TSOP4838 
/*--------------------------------------------------------------------------*/

/*------------------------------VARIAVEIS----------------------------------*/
signed char codeType = -1;
unsigned long codeValue; 
short codeLen; 
//unsigned int rawCodes[RAWBUF];

bool isWritingNewCodeSENSOR = false;
bool isWritingNewCodeBT = false;
unsigned long codigosGravados[qtdPinosProgramaveis];
unsigned char codeLenGravados[qtdPinosProgramaveis];
char codeTypeGravados[qtdPinosProgramaveis];
/*--------------------------------------------------------------------------*/

void setup() 
{
  mySerial.begin(9600);
  pinMode(Rainbow_PIN,OUTPUT);

  for(int i = 0;i<12;i++)
  {
    EEPROM.get(EEPROM_CodeAddresses[i], codigosGravados[i]);
    EEPROM.get(EEPROM_CodeLenAddresses[i], codeLenGravados[i]);
    EEPROM.get(EEPROM_CodTypeAddresses[i], codeTypeGravados[i]);
  }
  irrecv.enableIRIn(); 
  teclado.begin ();
  teclado.setKeyDownHandler (keyDown);
  //kpd.setKeyUpHandler   (keyUp);
}

void loop() 
{
  teclado.scan ();

  if(isWritingNewCodeSENSOR && irrecv.decode(&results))
  {
    storeCode(&results);
  }

  if(isWritingNewCodeBT)
  {
    if(mySerial.available())
    {
      codeValue = mySerial.read();
      mySerial.println(codeValue);
    }
  }
}

void storeCode(decode_results *results) 
{
  codeType = results->decode_type;
  if (codeType == UNKNOWN) 
  {
    mySerial.println("Recebido um protocolo desconhecido, atualmente nao podemos salva-lo por limitacoes de memoria.");
    /*mySerial.println("Recebido protocolo desconhecido, salvando como RAW.");
    mySerial.println("OBS: Atualmente so pode salvar 1 RAW, por causa do tamanho.");
    codeLen = results->rawlen - 1;
    for (int i = 1; i <= codeLen; i++) 
    {
      if (i % 2) 
      {
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
      } 
      else 
      {
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
      }
      mySerial.print(rawCodes[i - 1], HEX);
    }
    mySerial.println("");*/
  }
  else 
  {
    if (codeType == NEC) 
    {
      mySerial.print("Protocolo NEC Recebido: ");
      /*if (results->value == REPEAT) 
      {
        mySerial.println("Codigo repetido, ignorando.");
        return;
      }*/
    } 
    else if (codeType == SONY)
      mySerial.print("Protocolo SONY Recebido: "); 
    else if (codeType == PANASONIC)
      mySerial.print("Protocolo PANASONIC Recebido: ");
    else if (codeType == JVC)
      mySerial.print("Protocolo JVC Recebido: ");
    else if (codeType == RC5)
      mySerial.print("Protocolo RC5 Recebido: ");
    else if (codeType == RC6)
      mySerial.print("Protocolo RC6 Recebido: ");
    else 
    {
      mySerial.print("Protocolo nao reconhecido.");
    }
    mySerial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
  } 
  irrecv.resume(); 
  delay(100);
}

void sendCode(int tecla_pressionada) 
{
  if (codeTypeGravados[tecla_pressionada] == NEC) 
  {
    irsend.sendNEC(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo NEC enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  } 
  else if (codeTypeGravados[tecla_pressionada] == SONY) 
  {
    irsend.sendSony(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo SONY enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  } 
  else if (codeTypeGravados[tecla_pressionada] == PANASONIC) 
  {
    irsend.sendPanasonic(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo PANASONIC enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if (codeTypeGravados[tecla_pressionada] == JVC) 
  {
    irsend.sendJVC(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada], false);
    mySerial.print("Protocolo JVC enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if (codeTypeGravados[tecla_pressionada] == RC5) 
  {
    mySerial.print("Protocolo RC5 enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
    irsend.sendRC5(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
  } 
  else if(codeTypeGravados[tecla_pressionada] == RC6)
  {
    irsend.sendRC6(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo RC6 enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if(codeTypeGravados[tecla_pressionada] == SAMSUNG)
  {
    irsend.sendSAMSUNG(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo SAMSUNG enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if(codeTypeGravados[tecla_pressionada] == WHYNTER)
  {
    irsend.sendWhynter(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo WHYNTER enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if(codeTypeGravados[tecla_pressionada] == LG)
  {
    irsend.sendLG(codigosGravados[tecla_pressionada], codeLenGravados[tecla_pressionada]);
    mySerial.print("Protocolo LG enviado: ");
    mySerial.println(codigosGravados[tecla_pressionada], HEX);
  }
  else if (codeTypeGravados[tecla_pressionada] == UNKNOWN) 
  {
    mySerial.println("Por enquanto nao podemos enviar formatos RAW.");
    //irsend.sendRaw(rawCodes, codeLenGravados[tecla_pressionada], 38);
    //mySerial.print("Protocolo RAW enviado.");
  }
}

void saveEEPROMandPrint(int endereco)
{
  if(codeType != UNKNOWN)
  {
    EEPROM.put(EEPROM_CodeAddresses[endereco], codeValue);
    EEPROM.put(EEPROM_CodeLenAddresses[endereco], codeLen);
    EEPROM.put(EEPROM_CodTypeAddresses[endereco], codeType);

    EEPROM.get(EEPROM_CodeAddresses[endereco], codigosGravados[endereco]);
    EEPROM.get(EEPROM_CodeLenAddresses[endereco], codeLenGravados[endereco]);
    EEPROM.get(EEPROM_CodTypeAddresses[endereco], codeTypeGravados[endereco]);

    mySerial.print("Gravado na Memoria: ");
    mySerial.println(codigosGravados[endereco],HEX);
  }
}

void blinkRainbowLed(int delayTime)
{
  digitalWrite(Rainbow_PIN,HIGH);
  delay(delayTime);
  digitalWrite(Rainbow_PIN,LOW);
}

void keyDown (const char tecla_pressionada)
{
  Serial.print (F("Tecla pressionada: "));
  Serial.println (tecla_pressionada);
  teclaPressionadaAction(tecla_pressionada);
}

/*void keyUp (const char tecla_pressionada)
{
  Serial.print (F("Tecla liberada: "));
  Serial.println (tecla_pressionada);
}*/

void teclaPressionadaAction(char tecla_pressionada)
{
  if (tecla_pressionada >= '0' && tecla_pressionada <= '9')
  {
    if(!isWritingNewCodeSENSOR && !isWritingNewCodeBT)
    {
      sendCode(tecla_pressionada - '0');
      blinkRainbowLed(100);
    }
    else if(isWritingNewCodeSENSOR && !isWritingNewCodeBT)
    {
      saveEEPROMandPrint(tecla_pressionada - '0');
      finalizar();
    }
    else if(!isWritingNewCodeSENSOR && isWritingNewCodeBT)
    {

    }
  }

  switch(tecla_pressionada)
  {
    case '*':
      if(!isWritingNewCodeSENSOR && !isWritingNewCodeBT)
      {
        sendCode(10);
        blinkRainbowLed(100);
      }
      else if(isWritingNewCodeSENSOR && !isWritingNewCodeBT)
      {
        saveEEPROMandPrint(10);
        finalizar();
      }
      else if(!isWritingNewCodeSENSOR && isWritingNewCodeBT)
      {

      }
    break;
    case '#':
      if(!isWritingNewCodeSENSOR && !isWritingNewCodeBT)
      {
        sendCode(11);
        blinkRainbowLed(100);
      }
      else if(isWritingNewCodeSENSOR && !isWritingNewCodeBT)
      {
        saveEEPROMandPrint(11);
        finalizar();
      }
      else if(!isWritingNewCodeSENSOR && isWritingNewCodeBT)
      {
        
      }
    break;
    case 'A':
      digitalWrite(Rainbow_PIN,HIGH);
    break;
    case 'B':
      if(!isWritingNewCodeBT)
      {
        mySerial.println("Recebendo dados pelo sensor!");
        irrecv.enableIRIn();
        isWritingNewCodeSENSOR = true;
        digitalWrite(Rainbow_PIN,HIGH);
      }
      else
        finalizar();
    break;
    case 'C':
      if(!isWritingNewCodeSENSOR)
      {
        mySerial.println("Recebendo dados pelo bluetooth!");
        isWritingNewCodeBT = true;
        digitalWrite(Rainbow_PIN,HIGH);
      }
      else
        finalizar();
    break;
    case 'D':
      finalizar();
    break;
  }  
}

void finalizar()
{
  mySerial.println("Finalizado!");
  isWritingNewCodeBT = false;
  isWritingNewCodeSENSOR = false;
  digitalWrite(Rainbow_PIN,LOW);
}