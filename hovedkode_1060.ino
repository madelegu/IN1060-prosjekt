#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h" // Importert bibliotek for å kunne bruke kode knyttet til DF-spilleren

//BILJARDBUDDY FULLSTENDIG KODE:)

//________________________________KOMPONENTER PÅ BRETTET____________________________________

//Breakbeam(balldeteksjon)
const int breakbeam = 9; // mottaker fototransistor(BREAKBEAM)

//Knapper(start, stopp, gjenta)
int startknapp = 2;//pin på brettet
int stoppknapp = 3;// pin på brettet 
int gjentaknapp = 4;// pin på brettet

//Lyd
static const uint8_t PIN_MP3_TX = 11; //Kobler seg til DF spillerens RX
static const uint8_t PIN_MP3_RX = 10;// Kobler seg til DF spillerens TX

//Reed switch(tema-deteksjon)
const int reed_1 = 5;//pin på brettet
const int reed_2 = 6;//pin på brettet
const int reed_3 = 7;//pin på brettet

//______________________________________VARIABLER____________________________________________

//Breakbeam(balldeteksjon)
bool movementdetected = false;

//Knapper 
bool forrigeKnappTilstand = HIGH; // 'KOMMENTER HER!!! 
bool forrigeStoppTilstand = HIGH; // 

//Lyd
SoftwareSerial mp3Serial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini avspiller;//LYD lager avspillingsobjekt

//Reed switch
int reedStatus1; //Variabel som senere lagrer om reed gir HIGH eller LOW 
int reedStatus2;
int reedStatus3;

//Valgt tema og teller for spørsmål
int valgtTema = 0; // Valgt tema blir lagret som en int i variabelen valgtTema
int spmteller = 1; // Teller for hvilket spørsmål man er på, slik at man kan gå videre til neste spørsmål. Baseres på filstruktur i SD kort.
int ballTilSporsmaal = random(1,4); // tilfeldig tall så det ikke kommer spørsmål hver gang ball går ned (1, 2 eller 3)
int ballTeller = 0; // Teller for å holde kontroll på når spørsmål skal stilles


//_________________________________________SETUP_____________________________________________

void setup(){
Serial.begin(9600);
mp3Serial.begin(9600);

//Breakbeam
pinMode(breakbeam, INPUT); // Setter opp breakbeam som input

//Knapper
pinMode(startknapp, INPUT_PULLUP); // Setter opp knappene som input
pinMode(stoppknapp, INPUT_PULLUP);
pinMode(gjentaknapp, INPUT_PULLUP);

//Reed switcher
pinMode(reed_1, INPUT_PULLUP); // Setter opp reedswtitcher som input
pinMode(reed_2, INPUT_PULLUP);
pinMode(reed_3, INPUT_PULLUP);

//Starter kommunikasjon med DFPlayer, og sjekker om den starter korrekt. 
if (avspiller.begin(mp3Serial)) {
Serial.println("DFPlayer OK");
// Setter volum til maks (0 til 30).
avspiller.volume(30);
} else {
Serial.println("Tilkobling til DFPlayer Mini failet!");
}

Serial.println("Venter på at startknapp trykkes..."); 


// Kjører mens startknapp ikke er trykket eller trekantsylinderen ikke er satt på plass
while (digitalRead(startknapp) == HIGH || valgtTema == 0) // 
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

avspiller.playFolder(4, 1); // "Velkommen til spillet" leses opp
delay(7000);

// Spiller av tema-filen
avspiller.playFolder(valgtTema, spmteller); //KOMMENTER HER!!!!
spmteller++;
}


//_______________________________________LOOP______________________________________________
void loop() {

while (movementdetected != true) { // så lenge en ball ikke har gått ned i hullet...
detekterBevegelse(); //sjekkes det om en ball har gått ned i hullet
sjekkGjentaknapp(); //sjekker om gjentaknappen har blitt trykket på 
sjekkStoppknapp(); // eller om stoppknappen har blitt trykket på 
}

// Hvis det brytes ut av whileløkken, altså, bevegelse er detektert:
sjekkBall(); // sjekker om ballen skal trigge spørsmål, leser opp hvis ja, nullstiller movementdetected til false
}



//______________________________________METODER____________________________________________

//Detekterer om en ball har gått ned i hullet

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

//Velger tema for runden 
void velgTema() {
reedStatus1 = digitalRead(reed_1); //Leser verdiene til reedswitchen og lagrer det i variabler. 
reedStatus2 = digitalRead(reed_2);
reedStatus3 = digitalRead(reed_3);


if (reedStatus1 == LOW) {  // LOW = magnet detektert med pullup
valgtTema = 1;
Serial.println("Valgt tema: Norge");
} else if (reedStatus2 == LOW) { // Sjekker for hver av temaene, feilmelding hvis ingen er dektektert.
valgtTema = 2;
Serial.println("Valgt tema: Reise");
} else if (reedStatus3 == LOW) {
valgtTema = 3;
Serial.println("Valgt tema: Blandet");
} // else {
// Serial.println("Klarte ikke lese tema"); // kan kommenteres ut etter testing
// }
}

//Leser opp spørsmål ut ifra tema som har blitt valgt
void lesSpm(){
if (spmteller <= 6) { // På SDkortet er det 6 filer i hver av temamappene
avspiller.playFolder(valgtTema, spmteller); //Spiller av spørsmål basert på tema
spmteller++; //Legger til på telleren over spørsmål slik at den spiller av korrekt spørsmål neste gang
}
else {
avspiller.playFolder(4, 2); // tomt for spørsmål, takk for nå
}
}

//Gjentar spørsmålet som har bitt stilt
void gjenta(){
if (spmteller > 1) { // kan ikke spille av spor 0 - finnes ikke 
avspiller.playFolder(valgtTema, spmteller-1); // Spiller av forrige spørsmål uten å endre telleren
}
}

//Sjekker om gjentaknappen har blitt trykket underveis i spillet. 
void sjekkGjentaknapp() {
bool gjentaTilstandNaa = digitalRead(gjentaknapp); // lagrer gjenstandknappens tilstand i en variabel

if (forrigeKnappTilstand == HIGH && gjentaTilstandNaa == LOW) { //dersom 
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


//
void stopp() {
avspiller.playFolder(4, 3); // Når vi har stopplyd
delay(6000); // arduino venter i 6 sek mens DFPlayer spiller
resetFunc(); // reset
}

//Sjekker om stoppknappen har blitt trykket underveis i spillet. 
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

// Sjekker om ballen skal aktivere spørsmål eller ikke
void sjekkBall() {
ballTeller++;

// hvis telleren er lik som random verdi
if (ballTeller >= ballTilSporsmaal) {
lesSpm(); //Spørsmål fra valgt tema leses opp
ballTeller = 0; // nullstiller teller 
ballTilSporsmaal = random(1,4); // velger nytt tilfeldig tall
}

while (digitalRead(breakbeam) == HIGH) {
// venter til ballen har passert, så den ikke registreres flere ganger
}

movementdetected = false; // nullstiller til neste ball 
}






