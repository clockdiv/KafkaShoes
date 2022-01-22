#pragma once

class tb6612fng
{
  private:
    const int MOTOR_A_SPEED = D8;
    const int MOTOR_A_DIR2 = D7;
    const int MOTOR_A_DIR1 = D6;
    const int STBY = D5;
    const int MOTOR_B_DIR1 = D3;
    const int MOTOR_B_DIR2 = D2;
    const int MOTOR_B_SPEED = D1;
    
  public:
    tb6612fng() {};
    ~tb6612fng() {};

    void initPins()
    {
      //analogWriteFreq(40);
      pinMode(MOTOR_A_SPEED, OUTPUT);
      pinMode(MOTOR_A_DIR2, OUTPUT);
      pinMode(MOTOR_A_DIR1, OUTPUT);
      pinMode(STBY, OUTPUT);
      pinMode(MOTOR_B_DIR1, OUTPUT);
      pinMode(MOTOR_B_DIR2, OUTPUT);
      pinMode(MOTOR_B_SPEED, OUTPUT);
      pinMode(LED_BUILTIN, OUTPUT);

      digitalWrite(STBY, HIGH);
      
      stop();
    }

    void drive(int motor1, int motor2) {
      //Serial.print("TB6612FNG - drive()");
      

      bool m1Direction = motor1 > 0 ? HIGH : LOW;
      bool m2Direction = motor2 > 0 ? HIGH : LOW;
      digitalWrite(MOTOR_A_DIR1, m1Direction);
      digitalWrite(MOTOR_A_DIR2, !m1Direction);
      digitalWrite(MOTOR_B_DIR1, !m2Direction);
      digitalWrite(MOTOR_B_DIR2, m2Direction);

      motor1 = abs(motor1);
      motor2 = abs(motor2);
      int m1Speed = map(motor1, 0, 127, 0, 1023);     // note: NodeMCU has 10-Bit AnalogWrite resolution,
      int m2Speed = map(motor2, 0, 127, 0, 1023);     // so we map to 1023 and not 255
      m1Speed = constrain(m1Speed * 2, 0, 1023);    // multiply by 2 to give them more power and speed. 
      m2Speed = constrain(m2Speed * 2, 0, 1023);
      
      analogWrite(MOTOR_A_SPEED, m1Speed);
      analogWrite(MOTOR_B_SPEED, m2Speed);
      
      digitalWrite(LED_MESSAGE_RECEIVED, LOW);
}

    void stop()
    {
      //Serial.println("TB6612FNG - stop()");
      digitalWrite(MOTOR_A_DIR1, 0);
      digitalWrite(MOTOR_A_DIR2, 0);
      digitalWrite(MOTOR_B_DIR1, 0);
      digitalWrite(MOTOR_B_DIR2, 0);

      analogWrite(MOTOR_A_SPEED, 0);
      analogWrite(MOTOR_B_SPEED, 0);

     // digitalWrite(STBY, LOW);
    }
    // ---------------------------------------------------------


};
