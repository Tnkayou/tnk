#include "SerialCtrl.h"
#include "math.h"
#include "mbed.h"

// LED, モータ関係の宣言
//足回り
DigitalOut Mt1(PB_0); //左
DigitalOut Mt2(PC_8); //右
// 3はわけあってないです
DigitalOut Mt4(PB_1); //多分ロジャーアーム
DigitalOut Mt5(PC_9); //アーム（つかむとこ）
// led宣言
DigitalOut led1(PB_6); // led
DigitalOut led2(PC_4); // led
DigitalOut led3(PA_7); // led
DigitalOut blue(PC_6);//
DigitalOut red(PA_6);//
DigitalOut green(PB_4);//
DigitalIn sw1(PB_7);
// pwm宣言
//足回り
PwmOut pwm1(PA_0); //右
PwmOut pwm2(PB_8); //左
// 3はわけあってないです
PwmOut pwm4(PA_1); //多分ロジャーアーム
PwmOut pwm5(PB_9); //アーム


//シリアル通信(PS3)
Serial PS3(PC_12, PD_2); // PS3のシリアル(tx:送信,rx:受信)
Timer stim;              //シリアルのタイマー設定
SerialCtrl wire(
    &PS3,
    &stim); //クラスの初期化(読みたいコントローラーのアドレス,タイマーのアドレス)

int main() {
  //シリアル通信(PS3)
sw1.mode(PullUp);
PS3.baud(38400);        //ボードレートは38400
stim.reset();           //タイマーリセット
stim.start();           //タイマースタート
wire.param(0xAF, 0xED); //先頭アドレスと終端アドレスの設定

int fail = 0; //ループ内のカウンターの初期化
float motor_out[4];
int Q = 0; //ロジャーアーム
int n = 0; //アーム用の変数
int Z = 0; //LED切り替え用(テープ)
int k = 1;//速度
int stop = 1;

  // PWM出力の周期を設定
  pwm1.period_us(83); //モータの周期の設定
  pwm2.period_us(83); //モータの周期の設定
  pwm4.period_us(83); //モータの周期の設定
  pwm5.period_us(83); //モータの周期の設定

  while (1) 
  {

    if (wire.get()) {
      fail = 0;
      led1 = 1; //通信成功



if (sw1 == 0 && stop != 0)
{
    if (wire.data[B2] & START)
{
       stop = 0;
    wait_ms(1000);
   
}
    led2 = 1;
    led3 = 0;
      //足回り
      //右設定（コントローラーからのデータを１～－１に変換）
      if (wire.data[1] >= 122 && wire.data[1] <= 132) {
        motor_out[0] = 0; // ニュートラルの設定
      } else {
        motor_out[0] =
            (float(wire.data[1]) - 128) / 128 * (1); // 回転速度の設定
      }

      //左（コントローラーからのデータを１～－１に変換）
      if (wire.data[3] >= 122 && wire.data[3] <= 132) { // ニュートラルの設定
        motor_out[1] = 0;
      } else {
        motor_out[1] =
            (float(wire.data[3]) - 128) * (-1) / 128; // 回転速度の設定
      }
    
    if (wire.data[B2] & PS && wire.data[B1] & CIRCLE)
    {
        k = 0;
    }
    if (wire.data[B2] & PS && wire.data[B1] & SQUARE) 
    {
        k = 1;
    }
    switch (k) {
    case 0:
    //足回り回す
    Mt1 = motor_out[0] > 0 ? 0 : 1; // motor_out[0]が0より大きいなら0を返しそうでなければ1を返す
    pwm1 = (motor_out[0] > 0 ? 0 : 1) + motor_out[0] / 2; //なんとなくはわかるのだが説明はできない
    Mt2 = motor_out[1] > 0 ? 0 : 1; // motor_out[1]が0より大きいなら0を返しそうでなければ1を返す
    pwm2 = (motor_out[1] > 0 ? 0 : 1) + motor_out[1] / 2; //なんとなくはわかるのだが説明はできない
    break;
    case 1:
    Mt1 = motor_out[0] > 0 ? 0 : 1; // motor_out[0]が0より大きいなら0を返しそうでなければ1を返す
    pwm1 = (motor_out[0] > 0 ? 0 : 1) + motor_out[0] / 4; //なんとなくはわかるのだが説明はできない
    Mt2 = motor_out[1] > 0 ? 0 : 1; // motor_out[1]が0より大きいなら0を返しそうでなければ1を返す
    pwm2 = (motor_out[1] > 0 ? 0 : 1) + motor_out[1] / 4; //なんとなくはわかるのだが説明はできない
    break;
    }

      //ロジャーアーム
      if (wire.data[B1] & TRIANGLE)
        Q = 1;
      if (wire.data[B1] & CROSS)
        Q = 2;
      if ((!(wire.data[B1] & TRIANGLE)) && (!(wire.data[B1] & CROSS)))
        Q = 0;
      switch (Q) {
      case 0:
        Mt4 = pwm4 = 0;
        break;
      case 1:
        Mt4 = 0;
        pwm4 = 1;
        break;
      case 2:
        Mt4 = 1;
        pwm4 = 0;
        break;
      }

      // L1ボタン（アーム）
      if (wire.data[B2] & L1)
        n = 1;
      if (wire.data[B2] & L2)
        n = 2;
      if ((!(wire.data[B2] & L1)) && (!(wire.data[B2] & L2)))
        n = 0;
      switch (n) 
      {
      case 0:
        Mt5 = pwm5 = 0;
        break;
      case 1:
        Mt5 = 0;
        pwm5 = 1;
        break;
      case 2:
        Mt5 = 1;
        pwm5 = 0;
        break;
      }
    
    //テープLED
        if (wire.data[B1] & UP && wire.data[B2] & PS)
        {
            Z = 1;
        }
        if (wire.data[B1] & DOWN && wire.data[B2] & PS)
        {
            Z = 2;
        }
        if (wire.data[B1] & LEFT && wire.data[B2] & PS)
        {
            Z = 0;
        }
        switch (Z)
        {
        case 0:
        red = green = blue = 1;
        break;
        case 1:
        blue = 1;
        red = 0;
        break;
        case 2:
        red = 1;
        blue = 0;
        break;
        }
    }
else
{
    led2 = 0;
    led3 = 1;

   if (wire.data[B2] & START)
{
       stop = 1;
    wait_ms(1000);
}else{
    red = 1;
    wait_ms(1000);
    red = 0;
    wait_ms(1000);
}
}
    }
    else
    {
      fail++;
      //カウンターが5を超えたら通信失敗と表示
      if (fail > 10) {

        printf("connection false\n");

        // 通信失敗時は絶対に動かないようにしておく
        Mt1 = pwm1 = Mt2 = pwm2 = Mt4 = pwm4 = Mt5 = pwm5 = 0;
      }
    }
  }

}