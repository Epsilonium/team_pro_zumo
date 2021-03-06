/*
   Author: Torje
   Revidert: Line
   Denne koden inneholder funksjoner for Zumo.
   Disse funksjoner skal kjøres sammen som beskrevet. De funksjoner "spedoometer()" består av bør ikke kjøres individuelt.
   Funksjonen skal loope i main og kjøres i hver iterasjon. Hvis det mer mye delay blir målingene mindre presise.
   Denne funksjonen tar vare på:
    - Hastigheten, meter/sek. Oppdatert hvert sekund.
    - Høyeste hastighet.
    - Total distanse kjørt siden programmet startet Oppdateres hvert sekund.
    - gjennomsnittshastighet, oppdatert hvert 60. sekund
    - Distanse kjørt hvert 60. sekund
    - Antall sekunder bilen har kjørt over 70% av maks hastighet, utregnes hvert 60. sekund.
    - Batteri forbrukt
    - Batteri gjenstående
    - Antall ladesykluser på batteriet
  Programmet begrenser også batterikapasiteten utfra antall ladinger og overlading, slik som med et ekte batteri.

  Todo:
  Slå sammen med alle andre funksjoner for Zumo, og lag bibliotek

*/

//--------Global-----
const float SEVENTY_LIMIT = 0.462; //Grensen for 70% av bilens maxhastighet. (maks hastighet er 0.66 m/s)

float speedSixtyFinal;      //Verdien tas vare på for å printe til BLYNK, selv om en ny måling blir gjennomført
float distanceSixtyFinal;   //Verdien tas vare på for å printe til BLYNK, selv om en ny måling blir gjennomført
float distanceTotal = 0;    //Distansen kjørt, oppgitt i meter
float speedSixty;           //Mellomregningsmål for å beregne gjennomsnittshastighet
int numSixty = 0;           //Teller for hvor mange summer speedSixty består av. Brukes for å regne ut gjennomsnittet.
float avgSpeedSixty;        //Gjennomsnittshastighet hvert 60. sekund

float speedo;             //Utregnet meter/sekund. Faktiske speedometerverdien.
float measuredMaxSpeed = 0;       //Historisk mål for høyeste hastighet
float speedLeft;            //Encoderverdi
float speedRight;           //Encoderverdi
unsigned long movementTime;         //Tiden for en bevegelse. Brukes for å regne ut hastigheten.

bool newSpeedo = false;             //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool newDistanceTotal = false;      //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool newSpeedSixtyFinal = false;    //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool newDistanceSixtyFinal = false; //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool newMaxSpeed = false;           //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool newBatteryPercent = false;     //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool lowBatteryToESP = false;       //Brukes for å si om det finnes en ny verdi å skrive til ESP
bool toggleLED = false;             //Toggel for lavt batterivarsel-led

//float batteryChargeCycles = 0;  //Antall totale ladesykluser for batteriet. Brukes også til å begrense toppverdien for batteriet. Flere sykluser gir lavere batterikapasietet.
int batteryPercent;             //Batterinivå i prosent.

unsigned long currentMillis;
bool oneCalc = false; //For å sjekke om man er i en beregning av 1 sekunder
unsigned long oneCalcTime = 0; //Timestamp for start av en måling av 1 sekund


//-----------------------Speedometer------------------

void speedometer() {
  oneCheck();     //Oppdaterer hastighet hvert sekund
  seventyCheck(); //Sjekker om man kjører over 70% eller mer av max, og lagrer tiden
  sixtyCheck();   //Sjekker om der har gått 60 sekunder og regner ut gj.snitts hastighet og distanse kjørt på den tiden
  batteryCheck(); //Regner ut batteri brukt, eller oppladet
  batteryHealth(); //Gir beskjed om service eller bytte av batteri
  writeToESP();   //Skriver til ESP som deretter oppdaterer blynk
}


//-----------------------oneCheck--------------------

void oneCheck() {
  if (oneCalc == false) { //Sjekke om man er i en "sekundtelling" eller ikke. Starter en måling
    oneCalcTime = millis();  //Timestamp
    encoders.getCountsAndResetLeft(); //Resetter encoders i starten av en måling
    encoders.getCountsAndResetRight(); //Resetter encoders i starten av en måling
    oneCalc = true; //Markerer at man er i en måling
  }
  if (millis() - oneCalcTime >= 1000) { //Når det har gått ett sekund siden sist måling
    movementTime = millis() - oneCalcTime; //Regner ut tid for målingen av bevegelsen.
    speedLeft = encoders.getCountsLeft(); //Henter encoderverdiene.
    speedRight = encoders.getCountsRight(); //Henter encoderverdiene.

    speedCheck(); //Regner ut farten under sekundet, og oppdaterer evt. ny maks.hastighet
    oneCalc = false; //Ferdig med en hastighetsmåling.

    speedSixty += speedo; //Mellomregning for å regne ut gjennomsnittshastighet hvert 60. sekund. Tar med summen av alle fartsberegninger.
    numSixty++; //Teller for antall summer i speedSixty. Brukes for å senere regne ut gjennomsnittet av dem.

    distanceTotal += (speedo * (movementTime / 1000)); //Nullstilles aldri. Historisk mål for total distanse kjørt, målt i meter
    newDistanceTotal = true; //Markerer at en ny verdi er tilgjengelig for å printe til Blynk
  }
}


//-----------------------SpeedCheck------------------

void speedCheck() { //Brukes til å beregne hastigheten til gjennomsnittsforflyttningen til hjulene.
  const int SPEED_CONVERSION = 7750; // omregning fra dekodertellinger til m.
  float combinedSpeed = (speedLeft + speedRight) / 2;
  speedo = ((combinedSpeed / SPEED_CONVERSION) / movementTime) * 1000; // hastighet for bevegelsen, hensyntatt konvertering fra "dekodertellinger per millisekund" til "meter/sek"
  if (speedo <= 0) { //Hvis man snurrer om sin egen akse har man ikke fart, og det skal ikke påvirke snittfarta.
    speedo = 0;
    numSixty--;
  }
  if (speedo >  measuredMaxSpeed) { //Historisk mål for høyeste hastighet
    measuredMaxSpeed = speedo;
    newMaxSpeed = true; //Markerer at ny verdi er klar til å sendes til Blynk
  }
  newSpeedo = true; //Markerer at et ny speedometerverdi er oppdatert, så kan den sendes til ESP
}

//-----------------------sixtyCheck-----------------
unsigned long seventyTime; //Total tid over 70% av max
float distanceSixty; //Total distanse innen 60 sekunder
bool sixtyCalc = false; // For å sjekke om man er i en beregning av 60 sekunder
unsigned long sixtyCalcTime; //Timestamp for start av en måling av 1 sekund

void sixtyCheck() {
  if (sixtyCalc == false) { //Starter på en ny telling dersom en ikke eksisterer
    sixtyCalcTime = millis(); //Timestamp ved starten av en telling
    distanceSixty = distanceTotal; //Midlertidig verdi for beregning av total distanse over 60 sekunder
    sixtyCalc = true; //Bool for at timestamp ikke skal sjekke igjen
  }
  if (millis() - sixtyCalcTime >= 60000) { //Sjekker om det har gått 60 sekunder.
    distanceSixty = distanceTotal - distanceSixty; //Regner total distanse siden starten av 60-sekundersperioden, basert på differansen mellom den totale distansen over 60 sekunder.
    avgSpeedSixty = speedSixty / numSixty; //Gjennomsnittshastigheten over 60 sekunder.

    //Tar vare på verdier før man nullstiller og gjør klar for en ny måling
    speedSixtyFinal = avgSpeedSixty;
    distanceSixtyFinal = distanceSixty;

    //nullstiller tellere for å gjøre klart for en ny måling
    speedSixty = 0;
    distanceSixty = 0;
    seventyTime = 0;
    numSixty = 0;

    sixtyCalc = false; //Bool for at ny timestamp skal kunne gjennomføres (Starten av denne funksjonen)
    newSpeedSixtyFinal = true; //Markerer at nye verdier er klare for å sendes til BLYNK
    newDistanceSixtyFinal = true; //Markerer at nye verdier er klare for å sendes til BLYNK
    //EEPROM.write(0, batteryChargeCycles); //Lagrer antall batterisykluser i EEPROM en gang i minuttet. Gjør dette sjeldent for å ikke "slite ut" minnet, som her levetid på ca 100,000 skrive/slette-runder
  }
}

//----------------seventyCheck()-----------
// Funksjonen beregner tiden man er over 70% av max hastighet, etter en terskel man har lagt inn.
bool seventyCalc = false; //For å sjekke når bilen kjører over 70% av maxfart
unsigned long seventyCalcTime; //Timestamp for start av periode med kjøring over 70% av max

void seventyCheck() {
  if (speedo >= SEVENTY_LIMIT && seventyCalc == false) {
    seventyCalc = true;
    seventyCalcTime = millis();
  }
  if (speedo < SEVENTY_LIMIT && seventyCalc == true) {
    seventyCalc = false;
    seventyTime += millis() - seventyCalcTime;
  }
}


//--------------batteryCheck---------
float charged; //Teller hvor mye som lades
int batteryCapasity = BATTERY_MAX;
bool lowBattery = false;
bool batteryServiceNeeded = false;
bool prevServiceState = false;
bool batteryChangeNeeded = false;
bool prevChangeState = false;

void batteryCheck() {
  if (newSpeedo == true) {
    batteryLeft -= (speedo * (movementTime / 1000)); //Samme som distansen, men med egen teller. Oppgitt i meter.
    newBatteryPercent = true;
  }
  if (newCharge == true) {
    batteryLeft += charged;
    batteryChargedTotal += charged; //Teller hvor mye batteriet totalt har blitt oppladet. "Overlading" tærer også på batteriet, som i virkeligheten (Altså om man forsøker å lade over 100%)
    batteryChargeCycles = batteryChargedTotal / (BATTERY_MAX - (BATTERY_MAX * (ceil(batteryChargeCycles)) / 10)); //Sjekker hvor mye batteriet har blitt oppladet totalt siden programmet startet
    batteryLeft = constrain(batteryLeft, 0, BATTERY_MAX - (BATTERY_MAX * (ceil(batteryChargeCycles)) / 10));

    Serial.print("batteryChargeCycles: ");
    Serial.println(batteryChargeCycles);
    newCharge = false;
    newBatteryPercent = true;
  }
  batteryCapasity = constrain(batteryCapasity, 0, BATTERY_MAX - (BATTERY_MAX * (ceil(batteryChargeCycles)) / 10)); //Begrenser maks batterikapasitet med 10% for hver hele ladesyklus.
  batteryPercent = batteryLeft / batteryCapasity * 100;

  checkIfLowBattery();
}

void chargeBattery() {
  /*Kjøres når "charge"-knappen i blynk blir holdt inne, og lagrer antall sekunder knappen er holdt inne for å regne ut ladningsprosent*/
  charged = (ceil((millis() - chargeTime) / 1000)) * 10; //Runder opp til nærmeste antall sek kjørt. 1 sek = 10% batteri oppladet
  charged = constrain(charged, 0, BATTERY_MAX - (BATTERY_MAX * (ceil(batteryChargeCycles)) / 10));
  Serial.print("charged: ");
  Serial.println(charged);
  newCharge = true;
}

void checkIfLowBattery() {
  if (batteryPercent <= 95 && lowBattery == false) {
    lowBattery = true;
    lowBatteryToESP = true;
  }
  if (batteryPercent > 95 && lowBattery == true) {
    lowBattery = false;
    lowBatteryToESP = true;
  }
}

void batteryHealth() {
  if (batteryChargeCycles >= 2 && batteryChargeCycles < 3 && batteryServiceNeeded == false) {
    batteryServiceNeeded = true;
    Serial.println("SERVICE NEEDED");
  }
  if (batteryChargeCycles >= 3 && batteryChangeNeeded == false) {
    batteryChangeNeeded = true;
    Serial.println("CHANGE NEEDED");
  }
}
//-----------------writeToESP----------
//Skriver alle nye verdier til ESP
void writeToESP() {
  if (newSpeedo == true) {
    Serial1.write(1);
    delay(2);
    Serial1.print(float(speedo));


    Serial.print("BLYNK speedo: ");
    Serial.println(float(speedo));

    newSpeedo = false;
  }
  if (newDistanceTotal == true) {
    Serial1.write(2);
    delay(2);
    Serial1.print(float(distanceTotal));

    Serial.print("BLYNK distanceTotal: ");
    Serial.println(float(distanceTotal));

    newDistanceTotal = false;
  }
  if (newSpeedSixtyFinal == true) {
    Serial1.write(3);
    delay(2);
    Serial1.print(float(speedSixtyFinal));

    Serial.print("BLYNK speedSixtyFinal: ");
    Serial.println(speedSixtyFinal);

    newSpeedSixtyFinal = false;
  }
  if (newDistanceSixtyFinal == true) {
    Serial1.write(4);
    delay(2);
    Serial1.print(float(distanceSixtyFinal));

    Serial.print("BLYNK distanceSixtyFinal: ");
    Serial.println(float(distanceSixtyFinal));

    newDistanceSixtyFinal = false;
  }
  if (newMaxSpeed == true) {
    Serial1.write(5);
    delay(2);
    Serial1.write(int(measuredMaxSpeed));

    Serial.print("BLYNK MAXSPEED: ");
    Serial.println(measuredMaxSpeed);

    newMaxSpeed = false;
  }
  if (newBatteryPercent == true) {
    Serial1.write(6);
    delay(2);
    Serial1.write(batteryPercent);

    Serial.print("BLYNK batteryPercent: ");
    Serial.println(batteryPercent);

    newBatteryPercent = false;
  }
  if (lowBatteryToESP == true) {
    Serial1.write(7);
    delay(2);
    toggleLED = !toggleLED;
    Serial1.write(toggleLED);

    Serial.print("BLYNK update low battery LED: ");
    Serial.println(toggleLED);

    lowBatteryToESP = false;
  }
}
