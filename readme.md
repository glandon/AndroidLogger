
# AndroidLogger

## Introduction
AndroidLogger was initially designed as an logcat lexer plugin of Notepad++, has evolved into a comprehensive toolkit with magic functionalities.
Even if you are not an Android developer, it is very useful for you！

Currently it supports 7.8.x ~ latest version of Notepad++.

NOTE: It is recommended that you upgrade to a newer Npp version, taking 7.x as an example, the plugin toolbar is not displayed.

# Usage
## Quick Search
![quicksearch](help/quicksearch.gif)
Select text, then click the **[fast cmd]** to fire search quickly!

**1. Supports several options to fire search depend on your use.**
* /i : ignore case
* /o : search opend files
* /f : search folder
* /r : recursive search folder and sub-folders
* /s : detect src folder then search recursively
* /m : mark word highlight
* /w : match word not use regex


**2. You can config search at [Settings] & [Common Cmds]**

> Method1, config at settings
![searchsettings](help/searchsettings.png)

> Method2, config at common cmds
![searchatcommoncmds](help/searchcmds.png)


## Execute CMD
The [Common Cmds] support kinds of functions.

1. adbcmd: adb, adbout
```xml
<adbcmd>
    <desc>run adb shell getprop and out to NPP</desc>
    <cmd>getprop</cmd>
    <param></param>
    <type>adbout</type>
</adbcmd>
```
2. oscmd: cmd, cmdout
```xml
<oscmd>
    <desc>start calculator</desc>
    <cmd>calc</cmd>
    <param></param>
    <type>cmd</type>
</oscmd>
<oscmd>
    <desc>start logcat and out to NPP</desc>
    <cmd>cmd /c</cmd>
    <param>adb logcat</param>
    <type>cmdout</type>
</oscmd>
```
3. nppcmd: search
```xml
<nppcmd>
    <desc>Find [the] in Current</desc>
    <cmd>search /i /r /s *.h *cpp</cmd>
    <param></param>
    <type>search</type>
</nppcmd>
```

## Logcat Lexer
Essentially, it's a lexer for Android Logcat, you can custom color through NPP (Notepad++) preferences.
![loglexer](help/loglexer.png)


## Android Tools
Supports capture screenshot and logs with filters.


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