# OSC implementation for Adafruit Feather + EtherWing
## What
[Adafruit Featehr M0](https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython)または[M4](https://learn.adafruit.com/adafruit-feather-m4-express-atsamd51)と[Ethernet FeatherWing](https://www.adafruit.com/product/3201)の組み合わせにより小型のOSCモジュールを作った。ArduOSCはArduino Feather環境では依存関係が崩れてしまいそのままだとコンパイルできなかったため自力で実装した。依存関係はEthernetおよびEthernetUDP。platformio.iniを参照のこと。
ArduinoUnoにEthernetモジュールを載せると大型になり、また割り込み関係でNeoPixelと共存させられないなどの課題が多い。MbedにEthernetモジュールをつけることも出来るが、基板上での配線が必要となり、さほど小さく出来ない。Featherシステムを使うとスタックさせるだけでモジュールとして完成するのでサイズがコンパクトに出来、インスタレーション等での応用がしやすいと思われる。
This is an OSC implementation for [Adafruit Featehr M0](https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython) + [Ethernet FeatherWing](https://www.adafruit.com/product/3201). It should also run on [M4](https://learn.adafruit.com/adafruit-feather-m4-express-atsamd51), although I haven't tested.
As we build digital interactive installations, we often use OSC as device communication. Normally you would use ArduOSC, but I could not compile directly due to corrupt dependency. So I thought it would be faster to write my own implementation. Please see dependencies on platformio.ini. On Platformio the libraries should be automatically downloaded.
To use OSC communication in small controllers like Arduino, you often see the combination of ArduinoUno + Ethernet top, however it tends to be bit bulky. Also you can't use Neopixel or other libraries using interruption. For this purpose you could also use the combination of Mbed + MbedEther module, but its footprint is still larger than Feather + EtherWing. Also you need to consider bit of wiring, while you can just stack modules when you use Feather combo.

## Dependency
* Arduino / Platformio
* [Ethernet](https://platformio.org/lib/show/872/Ethernet)
* [Adafruit NeoPixel](https://platformio.org/lib/show/28/Adafruit%20NeoPixel)

## About OSC
### The official OSC documentaiton
日本語ドキュメンテーション
http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0.html
特にこちらの文章がOSCを理解するうえで分かりやすい。
http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0_examples.html#OSCstrings

English Doc
http://opensoundcontrol.org/introduction-osc

## How to test
テストのためにTouchDesignerでOSCを送るファイルも添付する。TouchDesigner上ではRGBWで信号を送るようになっているが実際にFeatherM0上に実装されているのはRGBのNeoPixelなのでWを入力しても色は反映されない。
For the test purpose I attach a TouchDesigner project file. On the file there are four sliders, each R, G, B, W, however the NeoPixel dot on the Feather board is only RGB, so W will be ignored.
PC/Host IP address : 192.168.0.2, Port: 8889
Device IP address : 192.168.0.3, Port: 8888
テスト環境に応じてポート番号変更のこと。
Please modify setting.h according to your set up.

![Set up](https://github.com/1-10/featherM0_OSC/blob/master/IMG_1233e.jpg?raw=true)