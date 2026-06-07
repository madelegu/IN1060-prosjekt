#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// grønn - norge
// 
// 


//KOMPONENTER
//breakbeam
const int breakbeam = 9; // mottaker fototransistor(BREAKBEAM)
//knapper
int startknapp = 2;//pin på brettet
int stoppknapp = 3;// pin på brettet 
int gjentaknapp = 4;// pin på brettet
bool forrigeKnappTilstand = HIGH;
bool forrigeStoppTilstand = HIGH;

//lyd
static const uint8_t PIN_MP3_TX = 11; //Kobler seg til df spillerens RX SJEKKE HER
static const uint8_t PIN_MP3_RX = 10;// Kobller seg til df spillerens TX

//reed switch
const int reed_1 = 5;//pin på brettet
const int reed_2 = 6;//pin på brettet
const int reed_3 = 7;//pin på brettet
int reedStatus1;
int reedStatus2;
int reedStatus3;

//VARIABLER
//breakbeam
bool movementdetected = false; 
//lyd
SoftwareSerial mp3Serial(PIN_MP3_RX, PIN_MP3_TX);//LYD
DFRobotDFPlayerMini avspiller;//LYD lager avspillingsobjekt

//tema og teller for spørsmål
int valgtTema = 0;
int spmteller = 1;



void setup(){
  Serial.begin(9600);
  mp3Serial.begin(9600);
  pinMode(breakbeam, INPUT); // breakbeam
    
  pinMode(startknapp, INPUT_PULLUP); // setter opp knappene
  pinMode(stoppknapp, INPUT_PULLUP);
  pinMode(gjentaknapp, INPUT_PULLUP);
  
	pinMode(reed_1, INPUT_PULLUP); // setter opp reed
  pinMode(reed_2, INPUT_PULLUP);
  pinMode(reed_3, INPUT_PULLUP);
  
  // Starter kommunikasjon med DFPlayer
  if (avspiller.begin(mp3Serial)) {
   	Serial.println("DFPlayer OK");
    // Set volume to maximum (0 to 30).
    avspiller.volume(30);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  
  Serial.println("Venter på at startknapp trykkes...");
  
  while (digitalRead(startknapp) == HIGH || valgtTema == 0) {
    // ikke gjør noe, start kun når startknapp blir trykket
    velgTema(); // leser reed-switches kontinuerlig til startknapp trykkes
    
    Serial.print("valgtTema: ");
    Serial.println(valgtTema);
    Serial.print("startknapp: ");
    Serial.println(digitalRead(startknapp));
    /*
    if (valgtTema == 0) {
      Serial.println("Velg tema før start!");
    }
    */
  }
  
  Serial.println("Spill startet!");
  // avspiller.playFolder(4, 1); // Når vi har stopplyd

  avspiller.playFolder(4, 1);
  delay(7000);
  
  // Spiller av tema-filen
  avspiller.playFolder(valgtTema, spmteller);
  spmteller++;
}

void loop() {

  while (movementdetected != true) {
    detekterBevegelse();
    sjekkGjentaknapp();
    sjekkStoppknapp();
  }
  
  // bevegelse detektert
  lesSpm();

  while (digitalRead(breakbeam) == HIGH) {
    // venter til low
  }

  movementdetected = false; // nullstiller til neste ball  
}

void detekterBevegelse(){
  int status = digitalRead(breakbeam); //leser signaler fra photoresistorens pin
  if (status == HIGH && !movementdetected) {
    movementdetected = true;
    Serial.println("Breakbeam blokkert");
  } else {
    movementdetected = false;
    Serial.println("Breakbeam ikke blokkert");
  }
}


void velgTema() {
  reedStatus1 = digitalRead(reed_1);
  reedStatus2 = digitalRead(reed_2);
  reedStatus3 = digitalRead(reed_3);
  
  
	if (reedStatus1 == LOW) {  // LOW = magnet detektert med pullup
    valgtTema = 1;
    Serial.println("Valgt tema: Norge");
  } else if (reedStatus2 == LOW) { // sjekker for alle temaene, feilmelding hvis ingen er dektektert
    valgtTema = 2;
    Serial.println("Valgt tema: Reise");
  } else if (reedStatus3 == LOW) {
    valgtTema = 3;
    Serial.println("Valgt tema: Blandet");
  } // else {
   // Serial.println("Klarte ikke lese tema"); // kan kommenteres ut etter testing
 // }
}

void lesSpm(){
  if (spmteller <= 6) {
    avspiller.playFolder(valgtTema, spmteller);
  	spmteller++;
  }
  else {
    avspiller.playFolder(4, 2); // tomt for spørsmål, takk for nå
  }
}

void gjenta(){
  if (spmteller > 1) { // kan ikke spille av spor 0 - finnes ikke 
    avspiller.playFolder(valgtTema, spmteller-1);
  }
}

void sjekkGjentaknapp() {
  bool gjentaTilstandNaa = digitalRead(gjentaknapp);

  if (forrigeKnappTilstand == HIGH && gjentaTilstandNaa == LOW) {
    delay(50); // debounce
    if (digitalRead(gjentaknapp) == LOW) {
      gjenta();
      delay(3000); // venter 3 sek etter spørsmål har blitt stilt, for at ikke trykk blir registrert flere ganger
    }
  }

  forrigeKnappTilstand = gjentaTilstandNaa;
}

// for å resette hele greia
void(*resetFunc)(void) = 0; // Peker til adresse 0

void stopp() {
  avspiller.playFolder(4, 3); // Når vi har stopplyd
  delay(6000); // arduino venter i 6 sek mens DFPlayer spiller
  resetFunc(); // reset
}

void sjekkStoppknapp() {
  bool stoppTilstandNaa = digitalRead(stoppknapp);

  if (forrigeStoppTilstand == HIGH && stoppTilstandNaa == LOW) {
    delay(50); // debounce
    if (digitalRead(stoppknapp) == LOW) {
      stopp();
    }
  }

  forrigeStoppTilstand = stoppTilstandNaa;
}






