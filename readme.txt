[Install]
1. Push AndroidLogger.dll under "plugins" directory of Notepad++
2. Push AndroidLogger.xml under "plugins\Config" directory of Notepad++

[Features]
1. Support lexer fot APP & RADIO Log, and cutomizable
2. Support catching log on device: APP, RADIO
3. Support Shell CMD on device. 
->(1) The shell cmd line must start with '>' and at the top of doc.
->(2) Start with '#' is comment
->(3) Empty line is permitted
->(4) Freely use logcat, top & grep, tcpdump
4. Support capture device screenshot, now just save at d:\device.bmp

[NOTE]
Compatible with Notepad++ 6.5 later version

V1.1.0
1) adb cmd will not timeout!
2) thread concurrent optimization

V1.1.1
1) Fix rgb issue, now device screenshot is ok!
2) Log & Shell concurrent optimization!

V1.1.2
1) Fix when open style dialog, the fold margin is shown, should hide it!

V1.2.0
1) Support custom keyword, your keywords your color!
2) auto lexer for shell logcat cmd
3) optimization adb shell output efficiencily!

V1.2.1
1) Fix lexer to compatible with leading or trailing spaces!