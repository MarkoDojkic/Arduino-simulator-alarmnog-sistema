#include <LiquidCrystal.h> //Biblioteka za rad sa LCD displejem
#include <Adafruit_NeoPixel.h> //Biblioteka za rad sa NeoPixel prstenom
#include <Keypad.h> //Biblioteka za rad numeričkom tastaturom
#include <string.h> //Biblioteka za rad sa string-ovima, koristi se za strcmp funkcijum

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = { //Mapiranje tastera u matricu
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'D' },
  { '7', '8', '9', 'S' },
  { 'H', '0', 'E', 'X' }
};

uint8_t colPins[COLS] = { 49, 51, 53, 45 }; // Pinovi kolona redom: C1, C2, C3, C4
uint8_t rowPins[ROWS] = { 48, 50, 52, 44 }; // Pinovi redova redom: R1, R2, R3, R4
bool isIntruderDetected = false; //Flag za proveru da li je provalnik detektovan od strane alarma
char inputPassword[7]; //Niz za smeštanje unete lozinke za alarm od strane korisnika
char password[7] = { '2','0','1','6','8','2','\0' }; //Smeštena ispravna lozinka za proveru
int counter = 0; //Brojač koji se koristi prilikom unosa lozinke
bool isArmed = false;  //Flag za proveru da li je uključen alarm
bool isPasswordInput = false; //Flag za proveru da li se na LCD displeju pokazuje unos lozinke

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //Inicijalizacija numeričke tastature, parametri su odgovarajuće vrednosti redova i kolona
LiquidCrystal lcd(23, 25, 27, 29, 31, 33);  //Incijalizacija LCD displeja, data pinovi su uneti kao parametri konstruktora
Adafruit_NeoPixel strip(60, 47, NEO_GRB + NEO_KHZ800); //Inicijalizacija NeoPixel prstena, parametri su broj dioda, data in pin i frekvencija


void colorWipe(uint32_t color, int wait) {  //Funkcija za bojenje dioda NeoPixel prstena u jednu boju
  for(int i=0; i<strip.numPixels(); i++) {  //Prolazak kroz diode
    strip.setPixelColor(i, color);         // Postavljanje boje pixela u RAM memoriju
    strip.show();                          // Ažuriranje podatka na prstenu
    delay(wait);                           // Kratka pauza između bojenja
  }
}

void theaterChase(uint32_t color, int wait) { //Funkcija za bojenje dioda NeoPixel prstena u niz boja
  for(int a=0; a<10; a++) {
    for(int b=0; b<3; b++) {
      strip.clear(); //Isključivanje svih dioda na prstenu
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); //Bojenje svake 3će diode
      }
      strip.show();
      delay(wait);
    }
  }
}

int get_distance() { //Funkcija koja vraća dužinu izmerenu ultrazvučnim senzorom (u centimetrima)
  static int distance;
  uint16_t duration = 0;
  uint32_t interval = 0;

  digitalWrite(40, LOW);
  delayMicroseconds(5);
  digitalWrite(40, HIGH);
  delayMicroseconds(10);
  digitalWrite(40, LOW);

  duration = pulseIn(42, HIGH);

  distance = (duration / 2) / 29;

  return distance;
}

void produceSiren() { //Funkcija koja pušta sirenu
for(int i=3;i<=11;i+=2)
  for(int hz = 440; hz < 1000; hz++){
    tone(12, hz, 50);
    delay(5);
  }
  for(int hz = 1000; hz > 440; hz--){
    tone(12, hz, 50);
    delay(5);
  }
}

void arm(){ //Funkcija za aktivaciju alarma
  if(strcmp(inputPassword,password) == 0){
    isArmed = true;
    isPasswordInput = false;
    memset(inputPassword, 0, sizeof inputPassword); //Brisanje predhodno unete lozinke iz niza
    counter = 0;
  } else {
    Serial.println("Failed to Arm. Incorrect password.");
  }
}

void disArm(){ //Funkcija za deaktivaciju alarma
  if(strcmp(inputPassword,password) == 0){
    isArmed = false;
    isPasswordInput = false;
    memset(inputPassword, 0, sizeof inputPassword);
    counter = 0;
  } else {
    Serial.println("Failed to disarm. Incorrect password.");
  }
}

void stopAlarm(){ //Funkcija za prekid alarma nakon detektovanja provalnika
  if(strcmp(inputPassword,password) == 0){
    isIntruderDetected = false;
    isPasswordInput = false;
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Alarm stopped");
    memset(inputPassword, 0, sizeof inputPassword);
    counter = 0;
    theaterChase(strip.Color(0,   127,   0), 100);
  } else {
    Serial.println("Failed to stop alarm. Incorrect password.");
  }
}

void setArmed(){ //Funkcija za postavku kada je alarm aktivan
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Status:");
  lcd.setCursor(6,1);
  lcd.print("ARMED");
  colorWipe(strip.Color(255, 0, 0), 50);
}

void setDisArmed(){ //Funkcija za postavku kada je alarm neaktivan
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Status:");
  lcd.setCursor(5,1);
  lcd.print("DISARMED");
  colorWipe(strip.Color(  0, 255,   0), 50);
}

void reprintLCDPassword(){ //Funkcija za ispis poruke za unos lozinke na LCD-u
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ENTER  PASSWORD:");
  for(int i = 0; i < 6; i++){
    lcd.setCursor(5+i,1);
    if(inputPassword[i] == 0) lcd.print('_');
    else lcd.print(inputPassword[i]);
  }
}

void reprintLCDIntruderDetected(){ //Funkcija za ispis poruke kada je provalnik detektovan
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("INTRUDER");
  lcd.setCursor(4,1);
  lcd.print("DETECTED!");
}

void setup() {
  lcd.begin(16, 2); //Aktivacija LCD displeja veličine 16x2
  Serial.begin(9600); //Baud rate postavljen na 9600, podrazumevan za Arduino
  strip.begin(); //Aktivatija NeoPixel prstena

  inputPassword[7] = '\0'; //Inicijalno dodavanje završnog karaktera na niz unete lozinke
  
  pinMode(LED_BUILTIN, OUTPUT); //Postavljanje pina za diodu kao izlazni
  pinMode(40, OUTPUT); //Postavljanje TRIG pina ultrazvučnog senzora kao izlazni
  pinMode(42, INPUT); //Postavljanje ECHO pina ultrazvučnog senzora kao ulazni
  pinMode(12, OUTPUT); //Postavljanje pina buzzera kao izlazni

  setDisArmed(); //Inicjalno postavljanje alarma kao isključenog
}

void loop() {
  char key = keypad.getKey(); //Čitanje pritisnutog tastera
  if(key != NO_KEY){ //Provera da li je neki taster pritisnut
    switch(key){
      case 'A': if(!isArmed) arm(); break; //Pritiskom na A aktivirati alarm, ukoliko on već nije
      case 'D': if(isArmed) disArm(); break; //Pritiskom na D deaktivirati alarm, ukoliko je on aktiviran
      case 'S': if(isIntruderDetected) stopAlarm(); break; //Pritiskom na S prekinuti alarm, ukoliko je detektovan provalnik
      case 'H': Serial.print("© 2021 Marko Dojkić 2018/201682\nFor more info visit github (github.com/markodojkic)"); break; //Pritiskom na H se ispisuje informaciona poruka na serijskoj vezi
      case 'E': isPasswordInput = true; reprintLCDPassword(); break; //Pritiskom na E (tj. enter) taster postavlja se LCD u mod za unos lozinike alarma
      case 'X': //Pritiskom na X (tj. backspace) taster se briše predhodno uneti karakter lozinke
          if(counter > -1){
            inputPassword[counter-1] = 0;
            reprintLCDPassword();
            if(counter != 0) counter--;
          }
          break;
      default: //Pritiskom preostalih tastera unosi se karakter koji je pritisnut
        if(counter < 7){
          inputPassword[counter] = key;
          reprintLCDPassword();
          if(counter != 6) counter++;
        }
        break;
    }
  }

  if(!isArmed && digitalRead(LED_BUILTIN) == HIGH && !isPasswordInput){
    digitalWrite(LED_BUILTIN, LOW);
    setDisArmed();
  }

  if(isArmed && digitalRead(LED_BUILTIN) == LOW && !isPasswordInput){
    digitalWrite(LED_BUILTIN, HIGH);
    setArmed();
  }

  if(isArmed && get_distance() < 200 && !isIntruderDetected){
    reprintLCDIntruderDetected();
    theaterChase(strip.Color(127,   0,   0), 100);
    isIntruderDetected = true;
    produceSiren();
  }
}