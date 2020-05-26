# Adafruit Feather OSC
## What
[Adafruit Featehr M0](https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython)または[M4](https://learn.adafruit.com/adafruit-feather-m4-express-atsamd51)と[Ethernet FeatherWing](https://www.adafruit.com/product/3201)の組み合わせにより小型のOSCモジュールを作ったコード。ArduOSCはArduino Feather環境では依存関係が崩れてしまいそのままだとコンパイルできなかったため自力で実装した。依存関係はEthernetおよびEthernetUDP。platformio.iniを参照のこと。
ArduinoUnoにEthernetモジュールを載せると大型になり、また割り込み関係でNeoPixelと共存させられないなどの課題が多い。MbedにEthernetモジュールをつけることも出来るが、基板上での配線が必要となり、さほど小さく出来ない。Featherシステムを使うとスタックさせるだけでモジュールとして完成するのでサイズがコンパクトに出来、インスタレーション等での応用がしやすいと思われる。

## Env
* Arduino / Platformio
* [Ethernet](https://platformio.org/lib/show/872/Ethernet)
* [Adafruit NeoPixel](https://platformio.org/lib/show/28/Adafruit%20NeoPixel)
## OSC
### The official OSC documentaiton
http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0.html
特にこちらの文章がOSCを理解するうえで分かりやすい。
http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0_examples.html#OSCstrings

## How to test
テストのためにTouchDesignerでOSCを送るファイルも添付する。
TouchDesigner上ではRGBWで信号を送るようになっているが実際にFeatherM0上に実装されているのはRGBのNeoPixelなのでWを入力しても色は反映されない。
PC側のアドレスは192.168.0.2, 受信ポートは8889
デバイス側のアドレスは192.168.0.3, ポート:8888
テスト環境に応じてポート番号変更のこと。

![Set up](https://github.com/1-10/featherM0_OSC/IMG_1233e.jpg)