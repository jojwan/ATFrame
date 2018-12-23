目的:
本工程指在提供一个通用的，处理AT指令的框架，框架文件是ATFrame/User/NB-IOT/ATHandleFrame.c(.h)。但是我提供了一个完整的可供下载编译测试的工程。下面详细介绍相关测试平台。

平台：
Windows 7 64bit, MDK-ARM 5.15.0, STM32F103RC, SIM7020,

几个重要文件的介绍：

ATFrame/User/NB-IOT/ATHandleFrame.c AT命令处理框架(AT Cmd Handle Frame - ACHF)
  AT_Cmd_Task() AT命令流程处理函数，用状态机的方式，依次设置AT命令。
  AT_Cmd_String_Process() AT命令回复的字符串处理函数，当收到某个字符串，相应的标志位会被置位。

ATFrame/User/NB-IOT/NB-IOT.c 用ACHF连接NB-IOT平台，程序流程会监测插入的是电信卡还是移动卡，如果是电信卡会连接电信云平台，移动卡的话直连服务器。
