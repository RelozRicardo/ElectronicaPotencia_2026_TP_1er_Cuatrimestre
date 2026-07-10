/*
 Proyecto: Control PWM mediante ADC
 Placa: ESP32-WROOM-32
 Arduino Core: Espressif 3.3.8
 Board: ESP32 Dev Module

GPIO34 -> Entrada ADC
GPIO25 -> Salida PWM 25 kHz

Soft Start:
    Duty inicial = 5%
    Incremento = 5% cada 50 ms
    Duty final = 58.3%
    Espera = 100 ms
*/

const int ADC_PIN = 34;
const int PWM_PIN = 25;

const int PWM_FREQ = 25000;      // 25 kHz
const int PWM_RES  = 10;         // 10 bits (0-1023)

const float DUTY_MIN = 0.47;
const float DUTY_MAX = 0.75;
const float DUTY_START = 0.583;
//////////////////////////////////////////////////////////////

void escribirPWM(float duty)
{
    // Limitar por seguridad
    if (duty < 0.0) duty = 0.0;
    if (duty > 1.0) duty = 1.0;

    uint16_t pwm = (uint16_t)(duty * 1023.0);

    ledcWrite(PWM_PIN, pwm);
}

//////////////////////////////////////////////////////////////

void softStart()
{
    float duty = 0.05;

    while (duty < DUTY_START)
    {
        escribirPWM(duty);

        Serial.print("SoftStart Duty: ");
        Serial.print(duty * 100.0, 1);
        Serial.println("%");

        delay(50);

        duty += 0.05;

        if (duty > DUTY_START)
            duty = DUTY_START;
    }

    escribirPWM(DUTY_START);

    Serial.println("Soft Start Finalizado");

    delay(100);
}

//////////////////////////////////////////////////////////////

void setup()
{
    Serial.begin(115200);
    analogReadResolution(12);
    // Configurar PWM
    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RES);
    // Arranque suave
    softStart();
    Serial.println("-----------------------------");
    Serial.println("Modo Normal");
    Serial.println("-----------------------------");
}

//////////////////////////////////////////////////////////////
void loop()
{
    // Promedio de 16 muestras para reducir ruido
    long suma = 0;
    for (int i = 0; i < 16; i++){
        suma += analogRead(ADC_PIN);
    }

    int adc = suma / 16;
    // Calcular duty lineal
    float duty = DUTY_MIN + (DUTY_MAX - DUTY_MIN) * ((float)adc / 4095.0);

    escribirPWM(duty);

    Serial.print("ADC: ");
    Serial.print(adc);

    Serial.print("    Duty: ");
    Serial.print(duty * 100.0, 2);
    Serial.println("%");

    delay(20);
}