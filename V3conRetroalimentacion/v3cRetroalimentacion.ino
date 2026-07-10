/*
====================================================
        BOOST CONVERTER CONTROL - ESP32
        Arduino Core 3.3.8
        Board: ESP32 Dev Module
====================================================
GPIO34 -> Potenciómetro (Setpoint)
GPIO35 -> Vout * 0.128
GPIO25 -> PWM 25 kHz
====================================================
*/

//====================== PINES ======================

const int PIN_ADC_SETPOINT = 34;
const int PIN_ADC_VOUT     = 35;
const int PIN_PWM          = 25;

//======================= ADC =======================

const int   ADC_MAX        = 4095;
const float ADC_VREF       = 3.30;

//======================= PWM =======================

const int PWM_FREQ = 25000;
const int PWM_RES  = 10;
const int PWM_MAX  = 1023;

//=================== CONTROL =======================

float dutyActual = 0.583;

const float DUTY_MIN = 0.47;
const float DUTY_MAX = 0.64;

//============== SOFT START =========================

const float DUTY_SOFT_INICIO = 0.05;
const float DUTY_SOFT_FINAL  = 0.47;
const float DUTY_SOFT_PASO   = 0.05;

const int TIEMPO_SOFT = 50;      // ms
const int ESPERA_FINAL = 100;    // ms

//================== TENSIONES ======================

const float VREF_MIN = 10.0;
const float VREF_MAX = 14.0;
const float DIVISOR = 0.128;

//================ REGULACION =======================

const float TOLERANCIA = 0.02;      // ±2 %
const float PASO_DUTY = 0.002;      // 0.2 %
const int TIEMPO_CONTROL = 5;       // ms

//===================================================

void escribirPWM(float duty){
    duty = constrain(duty, DUTY_MIN, DUTY_MAX);
    uint16_t pwm = duty * PWM_MAX;
    ledcWrite(PIN_PWM, pwm);
}

//===================================================

float leerSetpoint(){
    long suma = 0;
    for(int i=0;i<16;i++)
        suma += analogRead(PIN_ADC_SETPOINT);

    int adc = suma / 16;
    return VREF_MIN + ((VREF_MAX - VREF_MIN) * ((float)adc / ADC_MAX));
}

//===================================================

float leerVout(){
    long suma = 0;
    for(int i=0;i<16;i++)
        suma += analogRead(PIN_ADC_VOUT);

    int adc = suma / 16;
    float vPin = adc * ADC_VREF / ADC_MAX;
    return vPin / DIVISOR;
}

//===================================================

void softStart(){
    Serial.println("Soft Start");
    float duty = DUTY_SOFT_INICIO;
    while(duty < DUTY_SOFT_FINAL)
    {
        ledcWrite(PIN_PWM, duty * PWM_MAX);

        Serial.print("Duty: ");
        Serial.print(duty * 100.0,1);
        Serial.println("%");

        delay(TIEMPO_SOFT);

        duty += DUTY_SOFT_PASO;

        if(duty > DUTY_SOFT_FINAL)
            duty = DUTY_SOFT_FINAL;
    }

    dutyActual = DUTY_SOFT_FINAL;
    escribirPWM(dutyActual);
    delay(ESPERA_FINAL);
    Serial.println("Soft Start OK");
}

//===================================================

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);

    ledcAttach(PIN_PWM, PWM_FREQ, PWM_RES);

    softStart();
}

//===================================================

void loop()
{

    //---------------- Leer referencias ----------------

    float Vref = leerSetpoint();
    float Vout = leerVout();

    //---------------- Tolerancia -----------------------

    float margen = Vref * TOLERANCIA;
    //---------------- Regulación -----------------------

    if(Vout < (Vref - margen)){
        dutyActual += PASO_DUTY;
    }
    else if(Vout > (Vref + margen)){
        dutyActual -= PASO_DUTY;
    }

    dutyActual = constrain(dutyActual, DUTY_MIN, DUTY_MAX);

    escribirPWM(dutyActual);

    //---------------- Monitor Serie -------------------

    Serial.print("Vref: ");
    Serial.print(Vref,2);

    Serial.print(" V");

    Serial.print("    Vout: ");
    Serial.print(Vout,2);

    Serial.print(" V");

    Serial.print("    Duty: ");
    Serial.print(dutyActual*100.0,2);

    Serial.println("%");

    delay(TIEMPO_CONTROL);
}