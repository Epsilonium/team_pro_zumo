const float PERPENDICULAR = 655;  //90 graders sving
const float ONE_ROTATION = 909.7;  //ca.11.8cm
const float ONE_METER = 7750;
bool btnAIsPressed = false;
bool btnBIsPressed = false;
bool btnCIsPressed = false;

const char BlueDanube [] PROGMEM =
  "!T84 L8 V8 ddf#aa4 O5 aa4f#f#4"
  " O4 ddf#aa4 O5 aa4gg4"
  " O4 c#c#ebb4 O5 bb4gg4"
  " O4 c#c#ebb4 O5 bb4f#f#4"
  " O4 ddf#a>d4 O6 dd4<a<a4"
  " O4 ddf#a>d4 O6 dd4<b<b4"
  " O4 eegbb2g#a  >f#2df#f#4eb4ad4dd4.";

const char ImperialMarch [] PROGMEM =
  "!T100 V8 O3 aaaf8.>c16af8.>c16a2"
  " O4 eeef8.c16 O3 a-f8.>c16a2"
  " O4 a<aa L16 a-8.gg-fg-"
  " L4 R<b-8e- L16 d8.d-c<bc"
  " L4 Rf8a-f8.>c16a f8.>c16a1";

const char GoT [] PROGMEM =
  "!T87 L8 V8 gce-16f16 gce-16f16 gce-16f16"
  " gce16f16gce16f16gce16f16gce16f16g4."
  " L4 g.c.e-16f16gce-16f16"
  " L8 d<g<b-16c16 d<g<b-16c16 d<g<b-16c16d4."
  " L4 f.<b-.e-16d16fb-e-16d16"
  " O3 L8 >cfa-16b-16 >cfa-16b-16 >cfa-16b-16>c4.";
  
void patternDriving() {
  btnAIsPressed = buttonA.getSingleDebouncedRelease();
  btnBIsPressed = buttonB.getSingleDebouncedRelease();;
  btnCIsPressed = buttonC.getSingleDebouncedRelease();

  if (btnAIsPressed == true) {
    delay(500);
    driveSquare();
  }
  if (btnBIsPressed == true) {
    delay(500);
    driveCircle();
  }
  if (btnCIsPressed == true) {
    delay(500);
    forwardAndBack();
  }
}

void driveSquare() {
  Serial.print("BlueDanube");
  buzzer.playFromProgramSpace(GoT);
  float left;
  float right;
  for (int n = 0; n < 4; n++) {
    motors.setSpeeds(maxSpeed, maxSpeed);
    delay(1000);
    motors.setSpeeds(0, 0);
    delay(200);

    float prevLeft = encoders.getCountsLeft();
    float prevRight = encoders.getCountsRight();

    motors.setSpeeds(maxSpeed, -maxSpeed);
    while ((left <= prevLeft + PERPENDICULAR) && (right <= prevRight + PERPENDICULAR)) {
      left = encoders.getCountsLeft();
      right = encoders.getCountsRight();
    }
    motors.setSpeeds(0, 0);
    delay(200);
  }
  while (buzzer.isPlaying());
}

void driveCircle() {
  Serial.print("circle");
  buzzer.playFromProgramSpace(BlueDanube);
  float right;
  float prevRight = encoders.getCountsRight();
  motors.setSpeeds(maxSpeed, maxSpeed / 3);
  while (right <= (prevRight + ONE_ROTATION * 3.09 )) {
    right = encoders.getCountsRight();
  }
  motors.setSpeeds(0, 0);
  delay(200);
  while (buzzer.isPlaying());
}

void forwardAndBack() {
  maxSpeed = 200;
  Serial.print("forwardAndBack");
  buzzer.playFromProgramSpace(ImperialMarch);
  float left;
  float right;
  motors.setSpeeds(maxSpeed, maxSpeed);
  unsigned long time_now = millis();
  Serial.println(time_now);
  while (millis() <= time_now + 1000) //kjører 1 sek fram før den snur
  Serial.println("time done");
  motors.setSpeeds(0, 0);
  delay(200);

  float prevLeft = encoders.getCountsLeft();
  float prevRight = encoders.getCountsRight();
  
  motors.setSpeeds(maxSpeed, -maxSpeed);
  Serial.println("turn");
  while ((left <= prevLeft + PERPENDICULAR * 2) && (right <= prevRight + PERPENDICULAR * 2)) { //Snur 180 grader
    left = encoders.getCountsLeft();
    right = encoders.getCountsRight();
  }
  motors.setSpeeds(0, 0);
  delay(200);
  
  motors.setSpeeds(maxSpeed, maxSpeed);
  Serial.println("back");
  time_now = millis();
  while (millis() <= time_now + 1000) //kjører 1 sek fram
  motors.setSpeeds(0, 0);
  Serial.println("done");
  while (buzzer.isPlaying());
  delay(200);
}
