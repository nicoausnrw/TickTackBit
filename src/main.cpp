#include <Arduino.h>

// CONFIG ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define A1 5                // D1 Elektromagnet Pin 1
#define A2 4                // D2 Elektromagnet Pin 2
#define BUTTON 14           // D5 Button zum Vorspuhlen
#define BUTTON_Calibrate 13 // D7 Sekunden auf Null setzen
#define BUTTON_OFF 12       // D6 Schaltet die Uhr in Standby, Zähler läuft weiter aber Uhr Tickt nicht.
const int RelayOnTime = 350;  // wieviel ms, soll der Magnet eingeschaltet werden? Sollte es Schaltprobleme geben sollte dieser Wert erhöht werden

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long previousMillis = 0; // Zeitstempel für den letzten Schaltvorgang. Wichtig um die Minunten sauber durch zu zählen.
unsigned long lastTickStart = 0;  // Wann ist der Letzte Tick gestartet?
unsigned long now = 0;            // zwischenspeicher für aktuelle Millsec, damit alle funktionen hinterander mit der selben Zeit starten.
const int jumpInterval = 60000;   // wie oft soll die uhr weiter gehen Default ist eine Minute
int tickPosition = 0;             // Welcher der beiden Magneten ist gerade dran
int restTicks = 0;                // wieviele Minuten soll noch vorgespult werden bzw gewartet werden soll
int statusTick = 0;               // Status der Funktion wird angageben 0=startet, 1=läuft, 2=ende






// hier kann ich anfragen ob ich einen Tick starten darf, dazu gebe ich die Wunsch Startzeit von meinem Tick. Dies ist in der Regel  die aktuellen millsec.
void runTick(int anFrageStarttime = 0)
{

  // wenn letzter run zuende dann kann ich die neue Anfrage plazieren.
  // Alternativ wird weiter unten erstmal geprüft ob noch ein alter run läuft.
  if (statusTick == 2)
  {
      //Serial.println("run runTick, nichts zu tun");

    lastTickStart = anFrageStarttime;
  }

  // ist die Startzeit+Indervall dauer noch in der Zukunft, akak größer als jetziger Zeitpunkt, dann führe aus.
  if (lastTickStart + RelayOnTime >= now)
  {
    // Ein Tick ist gerade noch in Betrieb


    Serial.println("Relay ist gerade an.");

    // ein "einmaliger" Eingriff je Relay an Zeit
    // wir sind ja aktuell in einer Bedienung die bedeutet das auf jeden Fall das Relay geschaltet wird. Daher kann es hier nur zu zwei zuständen kommen, starter oder läuft
    if (statusTick == 2)
    {
      statusTick = 0; // Status für starten
      Serial.println("NEUE RUNDE");

      // Welcher Magnet muss angesteuert werden?
      if(tickPosition == 0){
        tickPosition = 1;
      }else{
        tickPosition = 0;
      }


    }
    else
    {
      statusTick = 1; // Status für läuft
    }

    // Simpler TickTack helfer um abwechseln die Relays zu schalten. 
    if (tickPosition == 0)
    {
      analogWrite(A1, 255);
      analogWrite(A2, 0);
    }
    else if (tickPosition == 1)
    {
      analogWrite(A1, 0);
      analogWrite(A2, 255);
    }

  } // der letzte Tick ist fertig, daher wird der spaß jetzt gestopt
  else
  {
    statusTick = 2;

    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);

  }

}





// ################################################################################################################################
// ################################################################################################################################
// ################################################################################################################################




void setup()
{

  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_Calibrate, INPUT_PULLUP);
  pinMode(BUTTON_OFF, INPUT_PULLUP);

  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);

  Serial.begin(9600);
  Serial.println("  Start");

}

void loop()
{

  // Speichert die aktuellen milliseckunden, da dieser Wert mehrfach gebraucht wird und da es eventuell beim wachsen des Programm am ende schon eine neue millisekunde sein kann was unerwartet Fehler verursachen kann. 
  now = millis(); 
  //Serial.println(now);

  // mit der Taste werden die Sec auf 0 gesetzt, spricht in 60sec ab hier kommt der nächste Schaltvorgang
  if(digitalRead(BUTTON_Calibrate) == LOW){
      previousMillis = now;
      
      Serial.println("Sec kalibriert");
  }

  // Wenn der Set Button gehalten wird dann tickt die Uhr einen Schritt weiter.
  if (digitalRead(BUTTON) == LOW)
  {
    Serial.println("Taster gedrückt!");
    runTick(now);
  }
  
  // Ich rechne aus wie lang her der letzte Schaltvorgang war, 
  // sollte der länger als eine Minute her sein, dann geht es in die nächste Runde. 
  if (now - previousMillis >= jumpInterval)
  {
    previousMillis = now;

    Serial.println("Minute rum, Schaltungvorgang gestartet");

    // Wenn der Schalter auf an ist, darf die Uhr jede Minute Schalten, ansosnten läuft das ganze Lautlos im Hintergrund weiter.
    if(digitalRead(BUTTON_OFF) != LOW){
      runTick(now);
    }

  }

  // in jeder Runde ausführen.  Eigentlich kann ich das setzenm von einem tick in eine getrennte funktion packen da der Status ja globald ist und ob ich eine Tick setzen darf bestimem ich dann mit eine extra funktion. so wie es aktuell ist eien Funktion für zwei aufgaben dran, einmal das sie jede runde läuft und zum anderne neue Zeit setzen
  runTick();
    
  }

