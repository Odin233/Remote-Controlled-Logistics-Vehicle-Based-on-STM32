## 目录

  - 嵌入式开发相关经验
  - 模块/片外外设介绍
    - 关于接线
    - 电调
    - 电机驱动板/电驱板
      - TB6612使用经验
    - 电机、舵机
      - 编码器
  - 控制算法记录
    - 四轮移动算法
      - 一般车轮
      - 麦克纳姆轮
  - STM32外设介绍与使用
    - ST-LINK
  - STM32开发
    - STM32开发模式
    - STM32CubeMX+keil+ST-LINK Utility方案环境搭建
    - STM32CubeMX使用
      - STM32CubeMX创建LED灯测试工程完整流程（STM32f103ZET6）
  - 基于STM32CubeMX和HAL库的PS2手柄遥控运输小车项目
    - PWM控制电机转速与编码器读取
      - 关于控制电机的正反转
    - PWM控制舵机与PCA9685舵机驱动板
      - 适用HAL库的第三方stm32_pca9685库的使用
    - 使用破解PS2手柄（原理、接线、SPI协议参数配置）
      - 移植PS2库（标准库的PS2库）（不使用SPI协议，使用普通引脚）
      - 使用SPI协议获得PS2手柄按键输入（移植标准库的PS2库，并通过HAL库使用SPI协议）
      - 移植库文件（编写自己的.h和.c文件）（适用于任何keil项目）
      - 移植库文件（仅适用于STM32CubeMX生成的keil项目）
    - 电脑端配置串口（通过串口助手软件显示打印的信息）
      - 安装CH340/CH341时出现"驱动预安装成功"而不是"驱动安装成功"
    - 在STM32程序中打印信息
      - printf语句使得程序不执行问题
      - SWD调试方法对printf函数的重定向和相关设置（自行选择关闭半主机模式或使用微库）（常用方法）
      - SWV调试方法对printf函数的重定向和相关设置（自行选择关闭半主机模式或使用微库）
      - HAL库的HAL_UART_Transmit()函数打印信息（自行选择关闭半主机模式或使用微库）
      - 串口直接发送（未验证）
    - 整合-PS2手柄控制小车（电机和舵机）

## 嵌入式开发相关经验

- 无论是哪一种MCU/外设/模块/被控对象，芯片手册（datasheet）和参考手册（reference manual）都是最重要的，大部分问题（包括部分概念，例程，接线）都可以通过芯片和参考手册解决（所有博客教程都是芯片和参考手册的二级产物）。

![76a4682ffb4910c64c6db1f6b31c0a9f.png](/_resources/76a4682ffb4910c64c6db1f6b31c0a9f.png)

- 一个电池为多个设备供电，最清晰的方法是通过面包板进行并联（实际上就是正极接正极，负极接负极）。电子元件的供电方式和学习到的电路是一样的，可以通过面包板的或者直接的并联供电。
- XT60接口的锂电池（LiPo电池），商品上标的是平均电压，按每节3.7V算，几S就是几节锂电池串联，因此平均电压均为3.7V的倍数，但是参考意义不大，直接看满电电压即可。3S满电电压为12.6V，4S满电电压为16.8V。但一般并不会冲到满电，不同S数的锂电池有一个较为适宜的长期储存电压。
- 关于锂电池的放电额定值，"#C"（例如25C）代表的是锂电池输出的额定最大电流为容量（毫安时的毫安）的#倍。也就是说，一个#C的锂电池，在最大输出功率下，放电时间为1/#个小时。由此可得锂电池的最大功率，从而根据用电器的额定功率计算得出所需的锂电池的参数。
- 嵌入式开发，本职工作实际上是用户手册之间的"连接"的工作。因为元器件都是第三方厂商生产的，是无法总结出通用的核心法则的，只能掌握底层原理基本知识框架，而实践则必须建立在第三方厂商的datasheet手册之间的"连接"上，而"连接"则需要个人理解和实践相结合。
- 移植例程和前人成果是学习单片机的根本，得到成功的第一例是开发的基础，经验、代码学习和相关原理知识是移植和得到第一例的润滑剂。
- 调试：真正能引起自我怀疑的考验，迷茫，是即使重新选择方向努力了仍无法得到任何结果。此时需要的是耐心和坚韧，在原本方向的继续努力，和寻找并尝试更多新方向并努力，是缺一不可的。只要有一条路走通，就能突破现在的困境。

## 模块/片外外设介绍

### 关于接线

- 分清输入还是输出（从STM32到外设还是从外设到STM32）
- 分清是否只有高低电平之分（只需要设置最普通的GPIO）
- 查询外设的用户手册。
- 查询外设的引脚定义与相应的片内外设的引脚定义。

### 电调

电调的全称是电子调速器（Electronic Speed Control，ESC），针对电机不同，可分为有刷电调和无刷电调；有刷电调就是简单的直流输出；无刷电调就是把直流电转换成为3相交流电。

电调有"电流控制"作用。电调内部电路有一套MOSFET管（功率管）。电流输入电调，电调的内部电路接收来自接收机的信号，根据信号，对电流作出合适的"控制"，然后把"控制"后的电流输出到马达，从而控制电机的启停及转速。

无刷电机没有无刷电调是无法正常工作的，因为无刷电机的输入端为PWM三相交流电，如果直接用电池的正负极接，电池只有正负极，而无刷电机有三相，强行连接可能会导致烧坏电机。

狭义的电调指的是无刷电调，一般额定电流很大（大于10A）。电驱板的作用可以等效于电调，但是常见的电驱板都是有刷电调，电流很小。

### 电机驱动板/电驱板

电机驱动板是专门为控制特定电机而设计的。例如，对于有刷电机和无刷电机，电驱板发挥电调的作用（通过类似的电路或直接搭载电调）。因此，电驱板就是将"特定类型电机"所需要的"对应功能的驱动元器件"集合起来的电路板，是一个广泛的概念。

电机驱动器是用来控制和给电机供电的一个器件，他是一个密闭的立方体，其包括电机控制板和电机驱动板两个部分（均在驱动器内部），电机控制板上面包括主控芯片，pwm输出电路，滤波电路，加密芯片等部分，电机驱动板部分包括逆变器部分（即ipm或者igbt和二极管的组合），电源转换部分（包括整流部分，电源保护部分，放电部分等），电流采样部分（数字采样芯片或者电阻采样等方式），通信部分，等等。我们俗称的电机控制器一般指软件算法层面，不是指硬件层面，非要从硬件层面去说，其实就是那一个主控芯片。当然，有的人喜欢把电机驱动器这个整体叫做电机控制器，也有人喜欢把电机控制板叫做电机驱动器。

同时，电机驱动板也可以提高单片机输入的电流（单片机能够承受的最大电流一般很小）和电压，以满足驱动电机的需要。

#### TB6612使用经验

- 电驱板的AO1和AO2（BO1和BO2）引脚是输出引脚，对电机输出电平信号。用于对电机进行供电和控制。
- 电驱板的AIN1和AIN2（BIN1和BIN2）引脚是输入引脚，从MCU接收电平信号。接收到的电平信号决定了AO1和AO2（BO1和BO2）引脚输出的电平信号。
- 一般电驱板只能接收和输出高低电平信号，某些电驱板可以接收和输出PWM信号（能够对电机进行调速）。
- 某些电驱板带有编码器接口，E1A和E1B（E2A和E2B）引脚各有两组：一组为输入引脚，接收电机的编码器A相和编码器B相的电平信号。一组为输出引脚，对MCU输出另一组引脚接收到的电平信号。一般编码器引脚旁边的5V和GND是给电机的编码器供电的，与电机本身无关。

- 不支持PWM的电驱板的控制表：

![714c34594a6d18401148530c93e9f728.png](/_resources/714c34594a6d18401148530c93e9f728.png)

- 支持PWM的电驱板的控制表（TB6612、AT8236）：

![812808d16aa4d32209db44ea36ccfa89.png](/_resources/812808d16aa4d32209db44ea36ccfa89.png)

### 电机、舵机

电机是输入电能输出扭矩的设备。

舵机是特殊的电机，只在一定角度范围输出扭矩。

同等扭矩下，电机的功率越大，转速就越大，额定电流就越大。

舵机一般频率为50Hz和60Hz。即使有供电，没信号的话舵机是不会固定前一个角度的。

#### 编码器

假如有A,B两相编码器信号，A相此时比B相提前90度。在下图第三种情况下：若相对信号电平信号为B相信号，此时B相电平为高电平，此时A相信号下降沿，那么计数器向上计数一次；此时B相电平为低电平，此时A相信号上升沿，那么计数器向上计数一次。若相对信号电平信号为A相信号，此时A相电平为高电平，此时B相信号上升沿，那么计数器向上计数一次；此时A相电平为低电平，此时B相信号下降沿，那么计数器向上计数一次。所以计数器一共会计数4次，提高了计数精度。在后面的程序中除以4就是因为编码器模式采用了这种4倍频。

单相计数和双相计数图：

![af51f6d73213d3f1de70eacaf9a63040.png](/_resources/af51f6d73213d3f1de70eacaf9a63040.png)

输入捕获中断法测速，当电机转速很高时，STM32会很频繁的进入中断，单片机花费在中断处理的时间会很长，占用资源多。如果使用编码器模式，则可以占用更少资源。

## 控制算法记录

### 四轮移动算法

#### 一般车轮

普通轮子：

前进和后退很简单，左右两边的方向都朝前或朝后，速度一致；

原地顺时针旋转时，左边轮子前进，右边轮子后退，速度一致；
原地逆时针旋转时，左边轮子后退，右边轮子前进，速度一致；

偏左前进时，左右两边的方向都朝前，左轮速度比右轮速度慢一点；
偏右前进时，左右两边的方向都朝前，左轮速度比右轮速度快一点；

偏左后退时，左右两边的方向都朝后，左轮速度比右轮速度慢一点；
偏右后退时，左右两边的方向都朝后，左轮速度比右轮速度快一点；

#### 麦克纳姆轮

如果是麦克纳姆轮，则可以有更多的操作：

![af2e1eb220178a0f6575a208212faed6.png](/_resources/af2e1eb220178a0f6575a208212faed6.png)

## STM32外设介绍与使用

### ST-LINK

ST-LINK种类：

- 官方的ST-LINK V2（盒子状，且有两排排针），需要使用特定的连接带。

![5dd8ff85b282ce0444af053de74d1ce0.png](/_resources/5dd8ff85b282ce0444af053de74d1ce0.png)

- 非官方的ST LINK V2（U盘状），可同时对STM32板子供电。SWD（Serial Wire Debug）调试方法仅需要使用杜邦线连接3.3V，GND，SWDIO（TMS）引脚，SWCLK（TCK）引脚。SWV（Serial Wire Viewer）调试方法则还需要连接SWO（Serial Wire Output）（SWIM/TDO）引脚，只有ST LINK V2-1才有SWO接口。

![1f11f0a305ab7d7b5343179a431bb272.png](/_resources/1f11f0a305ab7d7b5343179a431bb272.png)

![2a4bf89d3eb3c84c653344fbcfdd90c3.png](/_resources/2a4bf89d3eb3c84c653344fbcfdd90c3.png)

- 一般使用SWD调试方式和通过XCOM软件查看MCU的串口输出。

ST-LINK指示灯：

- 红灯常亮：正常。

- 红蓝闪烁：与ST-LINK Utility成功连接，可以烧录文件。

单片机的烧录模式：

- ICP：使用SWD接口进行烧录，如J-Link烧录器和J-Flash软件配合使用。

- ISP：使用引导程序（Bootload）加上外围UART/USB等接口进行烧录。

- IAP：软件自身实现在线电擦除和编程的方法，不使用任何工具。程序通常分成两块，分别为引导程序和应用程序。

## STM32开发

### STM32开发模式

STM32三种开发模式：

- 方式一：基于寄存器开发。
- 方式二：基于st官方的固件库/标准库和/或HAL库进行开发。
- 方式三：基于STM32cubeMX/STM32cubeIDE（集成STM32cubeMX的官方IDE）进行开发。因为同一芯片型号的片内外设、引出的引脚、各类参数是固定的，STM32cubeMX记录了每种芯片型号的信息。因此STM32cubeMX在设置某个片内外设时，会自动设置对应功能的引脚，而不需要自己查看芯片手册找到对应的引脚一个一个进行设置。STM32CubeMX还有一个好处就是可以直接生成整个keil工程文件（并且自动生成需要的HAL库的库文件，和部分对HAL库的再次封装的函数）。但是要注意生成的keil代码中，不同注释区域对代码的执行是有影响的（例如初始化），而且STM32CubeMX再次在同一项目生成代码，不会删除标注为用户代码的注释区域内的代码，而其他注释区域的代码会被删除，生成配置相关代码的注释区域则会被删除后被新的配置代码覆盖。

方法三-方案一：STM32CubeMX+keil+ST-LINK Utility，STM32CubeMX仅能生成keil工程和代码合导入HAL库文件，keil仅能编辑代码和编译生成hex文件（通过设置可以烧录到板子里，但是和Utility烧录的执行方式不同，有可能无法运行），Utility附有ST-LINK驱动且仅能将hex文件烧录到板子里。

方法三-方案二：STM32CubeIDE，STM32CubeIDE能生成代码和编译调试，包含了STM32CubeMX所有功能。

### STM32CubeMX+keil+ST-LINK Utility方案环境搭建

1.安装STM32CubeMX（官网）：

安装STM32CubeMX需要JRE，也就是JAVA环境，官网下载链接：https://www.java.com/en/download/manual.jsp

2.安装STM32CubeMX库（HAL库）：

在线安装库，则在STM32CubeMX中，Help-Manage embedded software package找到对应芯片型号的包，勾选然后Install Now。

离线安装STM32CubeMX库，在官网上下载对应的基础包和补丁包（压缩文件）。然后直接将基础包解压后的文件夹粘贴到C:\Users\11270\STM32Cube\Repository。将补丁包解压后的文件夹的名字（和基础包名字一样）的版本号更改为对应的补丁版本号，再粘贴到C:\Users\11270\STM32Cube\Repository。重启后应该能看到库已经安装上了。

3.安装ST-LINK驱动：

官网下载驱动并安装：https://www.st.com/en/development-tools/stsw-link009.html

如果在keil中无法检测到ST-LINK，可能是驱动没有安装成功，直接安装ST-LINK Utility（自带ST-LINK驱动）即可。

4.安装keil：

随便安装一个破解版。

安装对应芯片型号的官方库，点击：

![de61ab37efaed7eed5d5a5b67563f086.png](/_resources/de61ab37efaed7eed5d5a5b67563f086.png)

搜索到对应的芯片型号的（例如STM32F1xx_DFP）并下载安装（下载超时可以使用VPN）。如果发现下载键是灰色的，说明keil正在自动安装其默认安装的库，等待即可。

注意确定芯片选择正确，且选择了Debug-Use对应的调试方式（例如ST-Link Debugger），再点击Use右边的settings选择与开发板匹配的flash。

5.安装ST-LINK Utility：

在STM32官网下载并安装。

在ST-Link不连接MCU（也就是不连接板子），直接连接电脑时在ST-LINK Utility中ST-LINK-Firmware update-Device Connect-Yes升级ST-Link的固件。

不然keil会出现多个错误，或者检测不到连接的ST-Link。

### STM32CubeMX使用

#### STM32CubeMX创建LED灯测试工程完整流程（STM32f103ZET6）

- STM32f103ZET6本身的供电需要5V。
- STM32f103ZET6的JTAG/SWD接口一一对照（功能对应）：

![7302a65866ea62dc89b92ab2fe924170.png](/_resources/7302a65866ea62dc89b92ab2fe924170.png)

需要选择对应的芯片型号（包括封装方式）来新建工程。

在Pinout & Configuration-System Core中开启System Core-RCC-HSE-Crystal/Ceramic Resonator，以外部晶振作时钟源。

如果使用ST-Link进行调试，则在Pinout & Configuration-System Core中开启System Core-SYS-Debug-Serial Wire来开启串口调试，不然会出现第二次之后的下载就找不到设备的BUG。

配置GPIO：

![fd2cc57d78b0a87e4fa107aa056d8cca.png](/_resources/fd2cc57d78b0a87e4fa107aa056d8cca.png)

![c9d76811757bdda9d495525f09ca524c.png](/_resources/c9d76811757bdda9d495525f09ca524c.png)

F1系列芯片最大系统时钟为72MHzs，可以选用：1.高速内部时钟HSI作为系统时钟。2.高速外部时钟HSE作为系统时钟。3.通过锁相环时钟PLL，选择HSI或HSE其中一个，对其进行分频（除法）和倍频（乘法）操作后的结果作为系统时钟。一般来说，通过PLL将系统时钟设置成最大即可。因此如图所示配置时钟树（时钟树的配置其实是根据所需的总线频率来进行整体的配置）：

![9b46da236f0c6acac7dca983b64550a8.png](/_resources/9b46da236f0c6acac7dca983b64550a8.png)

在Project Manager-Project中编辑Project Name、Project Location、Toolchaim/IDE（使用keil则选择MDK-ARM）。在Project Manager-Code Generator中勾选Gernerate peripheral initialization as a pair of '.c/.h' files per peripheral一项。

点击GENERATE CODE，生成代码后点击Open Project使用keil打开。

在keil中，Application/User/Core-main.c，可以编辑代码。在int main(void)代码块中的while(1)代码块中编写代码（STM32板子自带的调试用LED在低电平时亮）：

```
HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
//将PE5置高电平，LED1灭
// GPIOE为PE系列0-15个针脚，后面的PIN_5表示PE系列中的PE5针脚。
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//将PB5置低电平，LED0亮
HAL_Delay(1000);
//延时1s

HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
//将PB5置高电平，LED0灭
HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
//将PE5置低电平，LED1亮
HAL_Delay(1000);
//延时1s
```

点击![6ea1c77b445956b1fc1b7997d413577e.png](/_resources/6ea1c77b445956b1fc1b7997d413577e.png)勾选Output-Create HEX File一项，点击![577e617d2d7a4abdab878de146451ceb.png](/_resources/577e617d2d7a4abdab878de146451ceb.png)无报错后点击![cb2e28152803c0335341e4f63f507896.png](/_resources/cb2e28152803c0335341e4f63f507896.png)将会在项目目录生成hex文件。

打开ST-Link utility，Target-connect，打开一个窗口，然后从文件管理器将keil生成的Hex文件拖动到此窗口，然后Target-Program，出现Verigication...OK即下载/烧录成功。

下载成功后，只要STM32板子通电，就会持续按照生成hex文件的代码工作。

## 基于STM32CubeMX和HAL库的PS2手柄遥控运输小车项目

### PWM控制电机转速与编码器读取

1、芯片选择。根据自己的板子来进行选择。

<img width="712" height="457" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_92dd272d322c4acca.png"/>

2、设置RCC。设置高速外部时钟HSE的晶振：

<img width="712" height="401" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_67ec7e73a51e406d9.png"/>

3、LED1配置。配置LED目的是通过观察小灯的亮灭判断是否进入定时器中断。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_08e38885365a45819.png"/>

4、USART1配置（通用同步异步收发器）。异步收发，波特率默认：115200 Bit/s。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_abba2cf2bbd3407ba.png"/>

5、PWM配置。使用定时器1通道1和通道4（TIM1_CH1和TIM1_CH4），频率72MHz。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_1f867ebe5ec4467ab.png"/>

6、定时器中断配置。使用定时器2（TIM2），定时器周期（Counter Period）设为10ms（100），即10ms进一次定时器中断，且使能自动重装载（auto-reload preload），其对应着TIMx 控制寄存器 1（TIMx_CR1）的位7——ARPE，也就是自动重装载预装载允许位 (Auto-reload preload enable，ARPE)：

0： TIMx_ARR寄存器没有缓冲；

1： TIMx_ARR寄存器被装入缓冲器。

auto-reload preload=Disable：自动重装载寄存器写入新值后，计数器立即产生计数溢出，然后开始新的计数周期。

auto-reload preload=Enable：自动重装载寄存器写入新值后，计数器完成当前旧的计数后，再开始新的计数周期。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_ee3905adbfb94263a.png"/>

打开更新定时器中断：

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_a1658d8de2a249d2b.png"/>

7、编码器配置。STM32自带编码器配置，使用定时器4（TIM4_CH1和TIM4_CH2），打开更新定时器中断。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_645a43e58093465fa.png"/>

8、中断优先级配置。因为编码器中断要发生在定时10ms中断内，故编码器中断的抢占优先级（主优先级）要大于定时10ms（值越大优先级越低）。嵌套向量中断控制器（NVIC）控制着整个STM32芯片中断相关的功能，配置中断优先级。中断是由某个事件触发的，对于串口，可以是电平的变化等（自定义设置）。对于定时器，则是计数/计时达到要求时触发中断。具有高抢占式优先级的中断可以在具有低抢占式优先级的中断服务程序执行过程中被响应，即中断嵌套，或者说高抢占式优先级的中断可以抢占低抢占式优先级的中断的执行。在抢占式优先级相同的情况下，有几个子优先级不同的中断同时到来，那么高子优先级的中断优先被响应。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_8186d50ad5c8428b9.png"/>

9、配置时钟。F1系列芯片系统时钟为72MHzs。

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_16e356e7d9c143cd9.png"/>

10、项目创建最后步骤

- 设置项目名称
- 选择所用IDE

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_3c5cb129cb2e499f8.png"/>

11、输出文件

- ②处：复制所用文件的.c和.h
- ③处：每个功能生产独立的.c和.h文件

<img width="712" height="456" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_4ae56639f95c443e9.png"/>

12、创建工程文件。点击GENERATE CODE 创建工程。

13、配置下载工具。这里我们需要勾选上下载后直接运行，然后进行一次编译。

<img width="712" height="382" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_aeaea78ac7c94af78.png"/>

14、下载工具内编辑代码。例程代码如下（在注释内的代码在重新生成代码时不会被删除）：

```
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
unsigned int MotorSpeed;  // Motor Current Speed by Encoder
int MotorOutput;		  // Motor Output
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *p)
{
	while(!(USART1->SR & (1<<7))); // 相当于while((USART1->SR&0X40)==0)，判断USART1的SR状态寄存器的第7位是0还是1。一位一位地循环发送，直到发送完毕。
	USART1->DR = ch; // 将USART1的DR变量赋值。
	
	return ch;
}
/* USER CODE END 0 */

/* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);	    // TIM1_CH1(pwm)
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1); // Open Encoder A
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2); // Open Encoder B
  HAL_TIM_Base_Start_IT(&htim2);                // Enable Timer2 Stop
  /* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static unsigned char i = 0;
    if (htim == (&htim2)) // Tim2 Stop Happen.
    {
        //1.Get Motor Speed
        MotorSpeed = (short)(__HAL_TIM_GET_COUNTER(&htim4)/18);   
        // TIM4 get motor inpulse,the inpulse get by 10ms / 18 is the real rpm
        __HAL_TIM_SET_COUNTER(&htim4,0);  // Timer return 0
        
      
        //2.Input the Duty Cycle
        MotorOutput=5000; // 3600 mean 50% duty cycle
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, MotorOutput);
        MotorOutput=MotorOutput*100/7200; 
        // dutu cycle print
        i++;
        if(i>100) // 注意：某些代码的执行需要一定的间隔，如果每次循环都执行（但是很奇怪的是，每两次循环进行一次是可以的），会导致程序直接卡住。
        {
          // Oberserve the LED to judge if enter the timer stop
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
          // print Timer4 value,short (-32768 - 32767), but must be commented if use ST-LINK Utility
          // printf("Encoder = %d moto = %d \r\n",MotorSpeed,MotorOutput);	
          i=0;
        }
    }
}
/* USER CODE END 4 */
```

STM32CUBEMX配置中，可以设置一个计时器TIM编码器模式的同时，设置其他通道为PWM输出，但是实际测试中使用了编码器，PWM就不能用了。stm32中，同一个定时器4个通道，同时用作PWM输出和编码器模式是不能实现的。因此使用编码器模式进行电机测速，stm32f103zet6的定时器不够（仅有5个定时器能够用来输出PWM或者使用编码器模式）。

因此可以使用外部中断法进行测速，但是占用资源。

#### 关于控制电机的正反转

>尝试着正转和反转，但是遇到了大坑。想通过关闭TIM来关闭PWM输出，结果是关闭了TIM之后PWM输出端口就自动高电平无法拉低！.
原因：复用推挽输出下IO口无法控制高低电平，因此拉低只能使用配置占空比的方法。
所以之前配多路TIM是没太大意义的。。关闭PWM只要把占空比调成零就好了！

### PWM控制舵机与PCA9685舵机驱动板

>STM32的I2C通信有两种方式，一种是硬件I2C，还有一种是模拟I2C，这两种的区别大概就是硬件I2C直接使用库函数进行操作，模拟I2C根据I2C的工作时序，自己写相应的函数，操作单片机，包括起始信号，停止信号，应答信号等，跟串口相同的地方是I2C同样有中断和DMA，但是在这里仅仅使用阻塞方式就行，I2C的具体原理已经有很多的资料可以参考了，HAL库的I2C使用硬件I2C更加方便，因为HAL库已经提供了比较完整的库函数，不需要自己写函数操作单片机，更加便捷。

在STM32CubeMX中，配置Connectivity-ISC2，直接选择I2C（一般使用I2C2，I2C1会被其他常用功能占用），保持默认设置即可。两个GPIO分别为SCL和SDA。PCA9865的OE接口可以不接（不接或接地），V+接口是PCA9865给舵机供电用的，可以不接（5V左右）。接了外接电源口，就不需要接V+接口了。

PCA9685的VCC（给PCA9685本身供电）电压范围可接受3.3V和5V电平。

通过焊接A5~A0确定PCA9685模块的i2c地址，如果不做任何焊接，默认地址是0x40。PCA9685地址位和很多描述的不一样，根据芯片手册，地址位的寄存器一共8位，其中最高位固定是1，A0-A5这六位是用户可更改的，而其中最关键的一位是R/W位，这一位主要是决定了读还是写，置1时为读，置0时为写，所以我们在写程序的时候，PCA9685的地址应把R/W位加上，是0x80，而不是0x40，在写的时候，发送地址位是0x80，在读的时候，发送的地址位是0x81。

#### 适用HAL库的第三方stm32_pca9685库的使用

```
// stm32_pca9685.h
#ifndef __STM32PCA9685_H
#define __STM32PCA9685_H	

//#include "stm32f10x.h"
#include "stm32f1xx_hal.h"

#define pca_adrr 0x80

#define pca_mode1 0x0
#define pca_pre 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define jdMIN  115 // minimum
#define jdMAX  590 // maximum
#define jd000  130 //0度对应4096的脉宽计数值
#define jd180  520 //180度对应4096的脉宽计算值


void pca_write(uint8_t adrr,uint8_t data);
uint8_t pca_read(uint8_t adrr);
void PCA_MG9XX_Init(float hz,uint8_t angle);
void pca_setfreq(float freq);
void pca_setpwm(uint8_t num, uint32_t on, uint32_t off);
void PCA_MG9XX(uint8_t num,uint8_t end_angle);

#endif

```

```
// stm32_pca9685.c
#include "stm32_pca9685.h"
#include "math.h"
#include "i2c.h"

uint8_t pca_read(uint8_t startAddress) {
    //Send address to start reading from.
    uint8_t tx[1];
    uint8_t buffer[1];
    tx[0]=startAddress;

    HAL_I2C_Master_Transmit(&hi2c2,pca_adrr, tx,1,10000); // adjust &hi2c# to I2C# your set
    HAL_I2C_Master_Receive(&hi2c2,pca_adrr,buffer,1,10000); // adjust &hi2c# to I2C# your set
    
    /*
    HAL库I2C阻塞方式下的库函数：
    I2C写数据的函数：
    HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    I2C_HandleTypeDef *hi2c ：也就是你所设置的那个实例，比如I2C1 &hi2c1
    uint16_t DevAddress ： 你要写入数据的地址，比如0xA0
    uint8_t *pData :存放你要写的数据
    uint16_t Size ：数据的大小
    uint32_t Timeout ：最大的传输时间
    例如HAL_I2C_Master_Transmit(&hi2c1,0xA1,(uint8_t*)TxData,2,1000)
     I2C读数据的函数：
     HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    I2C_HandleTypeDef *hi2c ：也就是你所设置的那个实例，比如I2C1 &hi2c1
    uint16_t DevAddress ： 你要读入数据的地址，比如0xA0
    uint8_t *pData :存放你要读的数据
    uint16_t Size ：数据的大小
    uint32_t Timeout ：最大的传输时间
     */

    return buffer[0];
}

void pca_write(uint8_t startAddress, uint8_t buffer) {
    //Send address to start reading from.
    uint8_t tx[2];
    tx[0]=startAddress;
    tx[1]=buffer;
    HAL_I2C_Master_Transmit(&hi2c2,pca_adrr, tx,2,10000); // adjust &hi2c# to I2C# your set

}

void pca_setfreq(float freq)//设置PWM频率
{
		uint8_t prescale,oldmode,newmode;
		double prescaleval;
		freq *= 0.915; 
		prescaleval = 25000000;
		prescaleval /= 4096;
		prescaleval /= freq;
		prescaleval -= 1;
		prescale =floor(prescaleval + 0.5f);

		oldmode = pca_read(pca_mode1);
	
		newmode = (oldmode&0x7F) | 0x10; // sleep
	
		pca_write(pca_mode1, newmode); // go to sleep
	
		pca_write(pca_pre, prescale); // set the prescaler
	
		pca_write(pca_mode1, oldmode);
		HAL_Delay(2);
	
		pca_write(pca_mode1, oldmode | 0xa1); 
}

void pca_setpwm(uint8_t num, uint32_t on, uint32_t off)
{
		pca_write(LED0_ON_L+4*num,on);
		pca_write(LED0_ON_H+4*num,on>>8);
		pca_write(LED0_OFF_L+4*num,off);
		pca_write(LED0_OFF_H+4*num,off>>8);
}
/*num:舵机PWM输出引脚0~15，on:PWM上升计数值0~4096,off:PWM下降计数值0~4096
一个PWM周期分成4096份，由0开始+1计数，计到on时跳变为高电平，继续计数到off时
跳变为低电平，直到计满4096重新开始。所以当on不等于0时可作延时,当on等于0时，
off/4096的值就是PWM的占空比。*/

/*
	函数作用：初始化舵机驱动板
	参数：1.PWM频率
		  2.初始化舵机角度
*/
void PCA_MG9XX_Init(float hz,uint8_t angle)
{
	uint32_t off=0;
//	IIC_Init();
	pca_write(pca_mode1,0x0);
	pca_setfreq(hz);//设置PWM频率
	off=(uint32_t)(145+angle*2.4);
	pca_setpwm(0,0,off);pca_setpwm(1,0,off);pca_setpwm(2,0,off);pca_setpwm(3,0,off);
	pca_setpwm(4,0,off);pca_setpwm(5,0,off);pca_setpwm(6,0,off);pca_setpwm(7,0,off);
	pca_setpwm(8,0,off);pca_setpwm(9,0,off);pca_setpwm(10,0,off);pca_setpwm(11,0,off);
	pca_setpwm(12,0,off);pca_setpwm(13,0,off);pca_setpwm(14,0,off);pca_setpwm(15,0,off);
	HAL_Delay(50);
}

/*
	函数作用：控制舵机转动；
	参数：1.输出端口，可选0~15；
		  2.起始角度，可选0~180；
		  3.结束角度，可选0~180；
		  4.模式选择，0 表示函数内无延时，调用时需要在函数后另外加延时函数，且不可调速，第五个参数可填任意值；
					  1 表示函数内有延时，调用时不需要在函数后另外加延时函数，且不可调速，第五个参数可填任意值；
					  2 表示速度可调，第五个参数表示速度值；
		  5.速度，可填大于 0 的任意值，填 1 时速度最快，数值越大，速度越小；
	注意事项：模式 0和1 的速度比模式 2 的最大速度大；
*/
void PCA_MG9XX(uint8_t num,uint8_t end_angle)
{
	uint32_t off=0;
	
	off=(uint32_t)(158+end_angle*2.2);
	pca_setpwm(num,0,off);
}

```

```
// main.c
/* USER CODE BEGIN Includes */
#include "stm32_pca9685.h"
/* USER CODE END Includes */

/* Initialize all configured peripherals */
PCA_MG9XX_Init(60,90); // 对一般舵机，50hz和60hz都可以选择，60hz的精度更高。初始化为90度。
// 初始化函数的位置必须在这里（虽然放在其他地方也可以编译成功），即使重新生成工程时将会被删除，不然会不执行此函数，使得PCA9685板子根本就没启动和初始化。且此句放置位置不对，有可能使得整个程序卡住（在/* USER CODE BEGIN 2 */-/* USER CODE END 2 */段内）。
// 成功初始化一次之后，只要STM32没有断过电，即使断开SCL和SDA接口，连接到舵机驱动板的舵机都会持续执行初始化（固定在初始化角度）。
// 即使成功初始化，且已经开始根据SCL和SDA接口的信号控制舵机，如果此时断开SCL或者SDA接口的接线超过1秒左右，那么即使重新接上断开的接口也无法恢复通过SCL和SDA接口的信号控制舵机，只会持续进行初始化。也就是卡住不动的情况。
// 有可能会发生PCA9865卡住不动的情况（例如在中途断开SCL或者SDA接口的接线后重新接上），此时应该对STM32进行断电后重新供电重启，更方便的方法是按下"RST"按钮进行重启。
/* USER CODE BEGIN 2 */

// 程序，注意位置在while(1){}代码块中。
/* USER CODE BEGIN WHILE */
  while (1)
  {
   PCA_MG9XX(0,0); //通道0舵机 从0度转到180度
   PCA_MG9XX(1,0); //通道1舵机 从0度转到180度
   HAL_Delay(50);
   PCA_MG9XX(0,180); //通道0舵机 从180度转到0度
   PCA_MG9XX(1,180); //通道1舵机 从180度转到0度
   HAL_Delay(50);
   }
/* USER CODE END WHILE */
```

### 使用破解PS2手柄（原理、接线、SPI协议参数配置）

根据PS2硬件资料，SPI参数配置需要注意的地方如下

- 全双工模式，主机
- 不使能硬件nss，自己多设置一个输出引脚当CS就好
- LSB先输出
- CPOL设置为High
- CPHA设置为第一个边沿
- 64分频（非常重要，PS2支持的通信频率只有250khz，要是你单片机通信频率过高会造成PS2只返回给你0xff，就是一直给你拉高电平）

在STM32CubeMX中，设置SPI1，波特率（baud rate）为64分频（prescaler，PSC）。PS2支持的通信频率只有250khz，要是单片机通信频率过高会造成PS2只返回0xff（255），就是一直拉高电平。时钟极性（clock polarity）代表平时的时钟极性是高还是低的。时钟相位（clock phase）设置为1，说明在下降沿发送。CRC Caculation选择Disabled。NSS Signal Type选择software。将会自动配置SPI1_SCK（CLK接口），SPI1_MISO（DAT接口），SPI1_MOSI（CMD接口）三个引脚，然后再随意选择一个引脚设置为GPIO_Output（设置初始电平为高），用于连接CS接口。

![079e81461fa0c1b9d7ea7a2c2e0ae9f9.png](/_resources/079e81461fa0c1b9d7ea7a2c2e0ae9f9.png)

时序，CLK，DI，DO，CS：

![05c152fc73b99b2456156104bd24658f.png](/_resources/05c152fc73b99b2456156104bd24658f.png)

CLK：时钟不断地上升沿和下降沿。

下降沿：数字电平从高电平（1）变为低电平（0）的那一瞬间叫作下降沿。

DO、DI：在在CLK时钟的下降沿完成数据的发送和读取。

CS：在通讯期间拉低，通信过程中CS信号线在一串数据（9个字节，每个字节为8位）发送完毕后才会拉高，而不是每个字节发送完拉高。

CLK的每个周期为12us。若在某个时刻，CLK处于下降沿，若此时DO为高电平则取“1”，低电平则取“0”。连续读8次则得到一个字节byte的数据，连续读9个字节就能得到一次传输周期所需要的数据。DI也是一样的，发送和传输同时进行。

每个CLK的下降沿读一个bit，每读八个bit（即一个Byte）CLK拉高一小段时间（完成一次读取），一共读九组bit（九个Byte），DO和DI的九个Byte的内容如下图所示（Byte的内容和按键的对应可能根据商家有所更改）：

![0cd5b9a18dabf5a007965f2af07a2080.png](/_resources/0cd5b9a18dabf5a007965f2af07a2080.png)

对DO（主机到手柄，也就是单片机到手柄），第1个byte是STM32发给接收器命令“0X01” ，是一个开始的命令。第2个byte是STM32发给PS2一个0x42请求数据。PS2手柄会在第二个byte回复它的ID（0x41=绿灯模式，0x73=红灯模式）。红灯模式时 ：左右摇杆发送模拟值，0x00~0xFF 之间，且摇杆按下的键值 L3 、 R3 有效。绿灯模式：左右摇杆模拟值为无效，推到极限时，对应发送 UP、RIGHT、DOWN、 LEFT、△、○、╳、□，按键 L3 、 R3 无效。

第3个byte时，对DI（手柄到主机，也就是手柄到单片机），PS2会给主机发送 “0x5A” 告诉STM32数据来了。

从第4个byte开始到第9个byte全是接收器（无线接收器，将手柄数据传输给单片机）给主机发送的数据，每个byte定义如上图。当有按键按下，其所属的byte的对应位置的bit的值就会为“0 ”，例如当键“SELECT”被按下时， `Data[3]=11111110`，因此可以是`00000000-11111111`。"0x"代表十六进制，例如"0x11"转换为十进制数就等于$16^1 * 1+16^0 * 1=17$。

![1a19f20cc7b694e54aa1f216ac48e16d.png](/_resources/1a19f20cc7b694e54aa1f216ac48e16d.png)

手柄需要两节7号1.5V的电池供电，接收器和单片机共用一个电源，电源范围为3~5V，不能接反，不能超压，过压和反接，都会使接收器烧坏。手柄上有个电源开关，0N开/OFF关，将手柄开关打到0N上，在未搜索到接收器的状况下，手柄的灯会不停的闪，在一定时间内，还未搜索到接收器，手柄将进入待机模式，手柄的灯将灭掉，这时，只有通过“START”键，唤醒手柄。接收器供电，在未配对的情况下，绿灯闪。手柄打开，接收器供电，手柄和接收器会自动配对，这时灯常亮，手柄配对成功。

#### 移植PS2库（标准库的PS2库）（不使用SPI协议，使用普通引脚）

需要配置普通GPIO引脚，以下代码设置的引脚为PD0、PD1、PD2、PD3。

```
// ps2.c
#include "stm32f1xx_hal.h" // 根据芯片选择HAL库。
#include "misc.h"
#include "ps2.h"
#include "gpio.h"

#define DELAY_TIME  delay_us(5); 

volatile int PS2_LX,PS2_LY,PS2_RX,PS2_RY,PS2_KEY;     //
uint16_t Handkey;
uint8_t Comd[2]={ 
   0x01,0x42};	//开始命令。请求数据
uint8_t Data[9]={ 
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//数据存储数组
uint16_t MASK[]={ 
   
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
	};	//按键值与按键明

	
	//向手柄发送命令
void PS2_Cmd(uint8_t CMD)
{ 
   
	volatile uint16_t ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{ 
   
		if(ref&CMD)
		{ 
   
			DO_H;                   //输出一位控制位
		}
		else DO_L;

		CLK_H;                        //时钟拉高
		DELAY_TIME;
		CLK_L;
		DELAY_TIME;
		CLK_H;
		if(DI)
			Data[1] = ref|Data[1];
	}
	delay_us(16);
}

//判断是否为红灯模式 0x41=模拟绿灯 0x73=模拟红灯
//返回值；0，红灯模式
// 其他，其他模式
uint8_t PS2_RedLight(void)
{ 
   
	CS_L;
	PS2_Cmd(Comd[0]);  //开始命令
	PS2_Cmd(Comd[1]);  //请求数据
	CS_H;
	if( Data[1] == 0X73)   return 0 ;
	else return 1;

}

//读取手柄数据
void PS2_ReadData(void)
{ 
   
	volatile uint8_t byte=0;
	volatile uint16_t ref=0x01;
	CS_L;
	PS2_Cmd(Comd[0]);  //开始命令
	PS2_Cmd(Comd[1]);  //请求数据
	for(byte=2;byte<9;byte++)          //开始接受数据
	{ 
   
		for(ref=0x01;ref<0x100;ref<<=1)
		{ 
   
			CLK_H;
			DELAY_TIME;
			CLK_L;
			DELAY_TIME;
			CLK_H;
		      if(DI)
		      Data[byte] = ref|Data[byte];
		}
        delay_us(16);
	}
	CS_H;
}

//对读出来的PS2的数据进行处理 只处理了按键部分 默认数据是红灯模式 只有一个按键按下时
//按下为0， 未按下为1
uint8_t PS2_DataKey()
{ 
   
	uint8_t index;

	PS2_ClearData();
	PS2_ReadData();

	Handkey=(Data[4]<<8)|Data[3];     //这是16个按键 按下为0， 未按下为1
	for(index=0;index<16;index++)
	{ 
   	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
		return index+1;
	}
	return 0;          //没有任何按键按下
}

//得到一个摇杆的模拟量 范围0~256
uint8_t PS2_AnologData(uint8_t button)
{ 
   
	return Data[button];
}

//清除数据缓冲区
void PS2_ClearData()
{ 
   
	uint8_t a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}

//void delay_init(uint8_t SYSCLK)
//{ 
   
// SysTick->CTRL&=0xfffffffb;//bit2清空,选择外部时钟 HCLK/8
// fac_us=SYSCLK/8;
//}

//void delay_us(uint32_t nus)
//{ 
// uint32_t temp; 
// SysTick->LOAD=nus*fac_us; //时间加载 
// SysTick->VAL=0x00; //清空计数器
// SysTick->CTRL=0x01 ; //开始倒数 
// do
// { 
   
// temp=SysTick->CTRL;
// }
// while(temp&0x01&&!(temp&(1<<16)));//等待时间到达 
// SysTick->CTRL=0x00; //关闭计数器
// SysTick->VAL =0X00; //清空计数器 
//}

//short poll
void PS2_ShortPoll(void)
{ 
   
	CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x42);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x00);
	CS_H;
	delay_us(16);	
}

//进入设置
void PS2_EnterConfing(void)
{ 
   
    CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01);
	PS2_Cmd(0x00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);
}

//发送模式设置
void PS2_TurnOnAnalogMode(void)
{ 
   
	CS_L;
	PS2_Cmd(0x01);  
	PS2_Cmd(0x44);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01); //analog=0x01;digital=0x00 软件设置发送模式
	PS2_Cmd(0x03); //Ox03锁存设置，即不可通过按键“MODE”设置模式。
				   //0xEE不锁存软件设置，可通过按键“MODE”设置模式。
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);
}

//振动设置
void PS2_VibrationMode(void)
{ 
   
	CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x4D);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0X01);
	CS_H;
	delay_us(16);	
}


//完成并保存配置
void PS2_ExitConfing(void)
{ 
   
  CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	CS_H;
	delay_us(16);
}

//手柄配置初始化
void PS2_SetInit(void)
{ 
   
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_EnterConfing();		//进入配置模式
	PS2_TurnOnAnalogMode();	//“红绿灯”配置模式，并选择是否保存
	//PS2_VibrationMode(); //开启震动模式
	PS2_ExitConfing();		//完成并保存配置
}

/****************************************************** Function: void PS2_Vibration(u8 motor1, u8 motor2) Description: 手柄震动函数， Calls: void PS2_Cmd(u8 CMD); Input: motor1:右侧小震动电机 0x00关，其他开 motor2:左侧大震动电机 0x40~0xFF 电机开，值越大 震动越大 ******************************************************/
void PS2_Vibration(uint8_t motor1, uint8_t motor2)
{ 
   
	CS_L;
	delay_us(16);
    PS2_Cmd(0x01);  //开始命令
	PS2_Cmd(0x42);  //请求数据
	PS2_Cmd(0X00);
	PS2_Cmd(motor1);
	PS2_Cmd(motor2);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);  
}

//读取手柄信息
void PS2_Receive (void)
{ 
   
		PS2_LX=PS2_AnologData(PSS_LX);
		PS2_LY=PS2_AnologData(PSS_LY);
		PS2_RX=PS2_AnologData(PSS_RX);
		PS2_RY=PS2_AnologData(PSS_RY);
		PS2_KEY=PS2_DataKey();
}

```

```
// ps2.h
#ifndef __PS2_H__
#define __PS2_H__

#include "stm32f1xx_hal.h" // 根据芯片选择HAL库。
#include "sys.h"

#define DI PDin(0)

#define DO_H PDout(1)=1
#define DO_L PDout(1)=0

#define CS_H PDout(2)=1 
#define CS_L PDout(2)=0

#define CLK_H PDout(3)=1 
#define CLK_L PDout(3)=0 


//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2         9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16
#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      26

//These are stick values
#define PSS_RX 5                //右摇杆X轴数据
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

extern uint8_t Data[9];
extern uint16_t MASK[16];
extern uint16_t Handkey;

void PS2_Init(void);
uint8_t PS2_RedLight(void);//判断是否为红灯模式
void PS2_ReadData(void);
void PS2_Cmd(uint8_t CMD);		  //
uint8_t PS2_DataKey(void);		  //键值读取
uint8_t PS2_AnologData(uint8_t button); //得到一个摇杆的模拟量
void PS2_ClearData(void);	  //清除数据缓冲区
void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);

void PS2_ShortPoll(void);//short poll
void PS2_EnterConfing(void);//进入设置
void PS2_TurnOnAnalogMode(void);//保存并完成设置
void PS2_VibrationMode(void);
void PS2_ExitConfing(void);//保存并完成设置
void PS2_SetInit(void);//手柄设置初始化
void PS2_Vibration(uint8_t motor1 ,uint8_t motor2);

#endif
;

```

```
// sys.c
#include "sys.h"  
// 

//********************************************************************************
//修改说明
//无
// 


//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI 
__asm void WFI_SET(void)
{ 
   
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{ 
   
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{ 
   
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(uint32_t	addr) 
{ 
   
	MSR MSP, r0 			//set Main Stack value
	BX r14
}

```

```
// sys.h
#ifndef __SYS_H
#define __SYS_H	 
#include "stm32f1xx.h" // 根据芯片选择HAL库。 
#include "delay.h"
#include "ps2.h"
#define SYSTEM_SUPPORT_OS		0		//定义系统文件夹是否支持UCOS
extern volatile int PS2_LX,PS2_LY,PS2_RX,PS2_RY,PS2_KEY;

//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).M4同M3类似,只是寄存器地址变了.
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14 
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014 
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414 
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814 
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14 
#define GPIOI_ODR_Addr    (GPIOI_BASE+20) //0x40022014 

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10 
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010 
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410 
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810 
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10 
#define GPIOI_IDR_Addr    (GPIOI_BASE+16) //0x40022010 
// 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //输出 
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  //输入

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //输出 
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  //输入

//以下为汇编函数
void WFI_SET(void);		//执行WFI指令
void INTX_DISABLE(void);//关闭所有中断
void INTX_ENABLE(void);	//开启所有中断
void MSR_MSP(uint32_t addr);	//设置堆栈地址 
#include <string.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#endif

```

```
// misc.c
#include "stm32f1xx_hal.h" // 根据芯片选择HAL库。
#include "misc.h"

/* Cortex M3 Delay functions */

static __IO uint32_t TimingDelay = 0;

void Delay(__IO uint32_t nTime)
{ 
   
    TimingDelay = nTime;

    while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{ 
   
    if (TimingDelay != 0x00) { 
   
        TimingDelay--;
    }
}

/* STM32F10x TIM helper functions */

TIM_Direction TIM_ReadDirection(TIM_TypeDef* TIMx)
{ 
   
    return (TIMx->CR1 & TIM_CR1_DIR);
}

void SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource)
{ 
   
  /* Check the parameters */
  assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));
  if (SysTick_CLKSource == SysTick_CLKSource_HCLK)
  { 
   
    SysTick->CTRL |= SysTick_CLKSource_HCLK;
  }
  else
  { 
   
    SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;
  }

}

```

```
// misc.h
#ifndef __STM32F10X_MISC_H__
#define __STM32F10X_MISC_H__

#include "stm32f1xx_hal.h" // 根据芯片选择HAL库。

/* Cortex M3 bit banding macros */

#define BITBAND_SRAM_REF   0x20000000
#define BITBAND_SRAM_BASE  0x22000000
#define BITBAND_SRAM(ptr,n) ((volatile uint32_t*)((BITBAND_SRAM_BASE + \
                                (((uint32_t)ptr)-BITBAND_SRAM_REF)*32 + (n*4))))
#define BITBAND_PERI_REF   0x40000000
#define BITBAND_PERI_BASE  0x42000000
#define BITBAND_PERI(ptr,n) ((volatile uint32_t*)((BITBAND_PERI_BASE + \
                                (((uint32_t)ptr)-BITBAND_PERI_REF)*32 + (n*4))))
          
#define SysTick_CLKSource_HCLK_Div8    ((uint32_t)0xFFFFFFFB)
#define SysTick_CLKSource_HCLK         ((uint32_t)0x00000004)

/* Cortex M3 Delay functions */
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);
void SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource);

/* STM32F10x GPIO helper functions */

#define GPIO_ToggleBit(port, pin) GPIO_WriteBit(port, pin, \
                            (BitAction) (1-GPIO_ReadOutputDataBit(port, pin)))

/* STM32F10x TIM helper functions */

typedef enum { 
   UP = 0, DOWN = 1} TIM_Direction;

TIM_Direction TIM_ReadDirection(TIM_TypeDef* TIMx);

#endif

```

```
// delay.c
#include "delay.h"
#include "misc.h"
#include "sys.h"
// 
//如果使用OS,则包括下面的头文件（以ucos为例）即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//支持OS时，使用 
#endif

// 

static uint8_t  fac_us=0;							//us延时倍乘数 
static uint16_t fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数
	
#if SYSTEM_SUPPORT_OS							//如果SYSTEM_SUPPORT_OS定义了,说明要支持OS了(不限于UCOS).

#ifdef 	OS_CRITICAL_METHOD						//OS_CRITICAL_METHOD定义了,说明要支持UCOSII 
#define delay_osrunning		OSRunning			//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	OS_TICKS_PER_SEC	//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	OSIntNesting		//中断嵌套级别,即中断嵌套次数
#endif

//支持UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD					//CPU_CFG_CRITICAL_METHOD定义了,说明要支持UCOSIII 
#define delay_osrunning		OSRunning			//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	OSCfg_TickRate_Hz	//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	OSIntNestingCtr		//中断嵌套级别,即中断嵌套次数
#endif


//us级延时时,关闭任务调度(防止打断us级延迟)
void delay_osschedlock(void)
{ 
   
#ifdef CPU_CFG_CRITICAL_METHOD   			//使用UCOSIII
	OS_ERR err; 
	OSSchedLock(&err);						//UCOSIII的方式,禁止调度，防止打断us延时
#else										//否则UCOSII
	OSSchedLock();							//UCOSII的方式,禁止调度，防止打断us延时
#endif
}

//us级延时时,恢复任务调度
void delay_osschedunlock(void)
{ 
   	
#ifdef CPU_CFG_CRITICAL_METHOD   			//使用UCOSIII
	OS_ERR err; 
	OSSchedUnlock(&err);					//UCOSIII的方式,恢复调度
#else										//否则UCOSII
	OSSchedUnlock();						//UCOSII的方式,恢复调度
#endif
}

//调用OS自带的延时函数延时
//ticks:延时的节拍数
void delay_ostimedly(u32 ticks)
{ 
   
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err);//UCOSIII延时采用周期模式
#else
	OSTimeDly(ticks);						//UCOSII延时
#endif 
}
 
//systick中断服务函数,使用OS时用到
void SysTick_Handler(void)
{ 
   	
	if(delay_osrunning==1)					//OS开始跑了,才执行正常的调度处理
	{ 
   
		OSIntEnter();						//进入中断
		OSTimeTick();       				//调用ucos的时钟服务程序 
		OSIntExit();       	 				//触发任务切换软中断
	}
}
#endif
			   
//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为AHB时钟的1/8
//SYSCLK:系统时钟频率
void delay_init(uint8_t SYSCLK)
{ 
   
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	u32 reload;
#endif
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
	fac_us=SYSCLK/8;						//不论是否使用OS,fac_us都需要使用
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	reload=SYSCLK/8;						//每秒钟的计数次数 单位为M 
	reload*=1000000/delay_ostickspersec;	//根据delay_ostickspersec设定溢出时间
											//reload为24位寄存器,最大值:16777216,在168M下,约合0.7989s左右 
	fac_ms=1000/delay_ostickspersec;		//代表OS可以延时的最少单位 
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;   	//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/delay_ostickspersec秒中断一次 
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; 	//开启SYSTICK 
#else
	fac_ms=(uint16_t)fac_us*1000;				//非OS下,代表每个ms需要的systick时钟数 
#endif
}								    

#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
//延时nus
//nus:要延时的us数. 
//nus:0~204522252(最大值即2^32/fac_us@fac_us=21) 
void delay_us(u32 nus)
{ 
   		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值 
	ticks=nus*fac_us; 						//需要的节拍数 
	delay_osschedlock();					//阻止OS调度，防止打断us延时
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{ 
   
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{ 
   	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};
	delay_osschedunlock();					//恢复OS调度 
}  
//延时nms
//nms:要延时的ms数
//nms:0~65535
void delay_ms(u16 nms)
{ 
   	
	if(delay_osrunning&&delay_osintnesting==0)//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度) 
	{ 
   		 
		if(nms>=fac_ms)						//延时的时间大于OS的最少时间周期 
		{ 
    
   			delay_ostimedly(nms/fac_ms);	//OS延时
		}
		nms%=fac_ms;						//OS已经无法提供这么小的延时了,采用普通方式延时 
	}
	delay_us((u32)(nms*1000));				//普通方式延时
}
#else  //不用ucos时
//延时nus
//nus为要延时的us数. 
//注意:nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
void delay_us(uint32_t nus)
{ 
   		
	uint32_t temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//时间加载 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; //开始倒数 
	do
	{ 
   
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达 
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL =0X00;       				//清空计数器 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对168M条件下,nms<=798ms 
void delay_xms(uint16_t nms)
{ 
   	 		  	  
	uint32_t temp;		   
	SysTick->LOAD=(uint32_t)nms*fac_ms;			//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           			//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数 
	do
	{ 
   
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达 
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;     		  		//清空计数器 
} 
//延时nms 
//nms:0~65535
void delay_ms(uint16_t nms)
{ 
   	 	 
	uint8_t repeat=nms/540;						//这里用540,是考虑到某些客户可能超频使用,
											//比如超频到248M的时候,delay_xms最大只能延时541ms左右了
	uint16_t remain=nms%540;
	while(repeat)
	{ 
   
		delay_xms(540);
		repeat--;
	}
	if(remain)delay_xms(remain);
} 
#endif

```

```
// delay.h
#ifndef __DELAY_H
#define __DELAY_H 		
#include "misc.h"
// 
void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#endif

```

main.c的代码基于标准库，也就是不使用SPI协议，使用的引脚也不同：

```
// main.c
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ps2.h"
#include "sys.h"
#include "delay.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
	delay_init(168);
	PS2_SetInit();
	delay_ms(500);
/* USER CODE END 2 */

while (1)
  { 
   
    /* USER CODE END WHILE */
		  PS2_LX=PS2_AnologData(PSS_LX);    
			PS2_LY=PS2_AnologData(PSS_LY);
			PS2_RX=PS2_AnologData(PSS_RX);
			PS2_RY=PS2_AnologData(PSS_RY);
			PS2_KEY=PS2_DataKey();	
			printf("PS2_LX:%d ",PS2_LX);
			printf("PS2_LY:%d ",PS2_LY);
		  printf("PS2_RX:%d ",PS2_RX);
			printf("PS2_RY:%d ",PS2_RY);
			printf("PS2_KEY:%d \r\n",PS2_KEY);
			delay_ms(100);
    /* USER CODE BEGIN 3 */	
	}
```

#### 使用SPI协议获得PS2手柄按键输入（移植标准库的PS2库，并通过HAL库使用SPI协议）

需要对PS2手柄初始化（可以通过移植标准库的PS2库的初始化函数进行）。需要设置一个GPIO引脚连接CS接口（下面代码以PC4为例），此引脚初始电平为高。使用HAL库使用SPI协议来获得来自手柄的控制信号（与移植标准库的PS2库无关）：

```
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
#include "ps2.h"
#include "sys.h"
#include "delay.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
  delay_init(168);
  PS2_SetInit();
  delay_ms(500);
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
uint8_t cmd[3] = {0x01,0x42,0x00};  // 请求接受数据
uint8_t PS2data[9] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};   //存储手柄返回数据。

void PS2_Get(void)    //接受ps2数据。注意：此代码的执行需要一定的间隔，如果每次循环都执行（但是很奇怪的是，每两次循环进行一次是可以的），会导致程序直接卡住。
{
	uint8_t i = 0;
	
	HAL_GPIO_WritePin(ENABLE_GPIO_Port,ENABLE_Pin,GPIO_PIN_RESET);  //拉低，开始通讯
	// HAL_GPIO_WritePin填写作为CS口的GPIO的port和号码。
		
	HAL_SPI_TransmitReceive(&hspi1,&cmd[0],&PS2data[0],1,0xffff); // 发送0x01，请求接受数据
	delay_us(10);
	HAL_SPI_TransmitReceive(&hspi1,&cmd[1],&PS2data[1],1,0xffff); // 发送0x42，接受0x01（PS2表示开始通信）
	delay_us(10);
	HAL_SPI_TransmitReceive(&hspi1,&cmd[2],&PS2data[2],1,0xffff); // 发送0x00，接受ID（红绿灯模式）
	delay_us(10);
	for(i = 3;i <9;i++)
	{
		HAL_SPI_TransmitReceive(&hspi1,&cmd[2],&PS2data[i],1,0xffff); // 接受数据
		delay_us(10);
		
	}

for(i=0;i<9;i++)
	{
	  printf("\r\n PS2:%d \r\n",PS2data[i]);// 输出存储的手柄按键输入结果。
	  // 0-8位的默认值分别为：255，65，90，255，255，255，255，255，255。
	  // 第0位 固定为255
	  // 第1位 65为绿灯模式，115为红灯模式
	  // 第2位 固定为90
	  // 第3位 绿灯模式下（上下左右、select、start） 红灯模式下，二进制0位1位2位3位4位5位6位7位（上下左右、摇杆键2个、select、start）
	  // 第4位 绿灯模式下（三角方框圆圈叉叉、L2R2、L3R3） 红灯模式下，二进制0位1位2位3位4位5位6位7位（三角方框圆圈叉叉、L2R2、L3R3）
	  // 第5位 红灯模式下，右摇杆左极限为0
	  // 第6位 红灯模式下，右摇杆前极限为0
	  // 第7位 红灯模式下，左摇杆左极限为0
	  // 第8位 红灯模式下，左摇杆前极限为0
	  printf("PS2:%d \r\n",dectobin((int)PS2data[j])); // 输出二进制值。dectobin()函数是自定义的十进制转二进制的函数。
	  char str[8];
	  sprintf(str,"%d",dectobin((int)PS2data[4]));
	  // 输出二进制值字符串。sprintf是c语言标准库自带的将整数转换为字符串的函数。
	  // 此句在中断中执行效率很低，有概率会卡住整个中断，原因不知为何。但是在中断函数需要执行的操作比较少时仍可以使用。
      printf("PS2:%s \r\n",str);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,GPIO_PIN_SET);  //拉高，准备下次通讯
    // HAL_GPIO_WritePin填写作为CS口的GPIO的port和号码。
	
}
/* USER CODE END 4 */
```

#### 移植库文件（编写自己的.h和.c文件）（适用于任何keil项目）

1.新建一个存放自己程序的文件夹–MY

红色

![在这里插入图片描述](/_resources/watermark_type_ZmFuZ3poZW5naGVpd_b39cea7721114678b.png)
选中工程名，右键
<img width="712" height="446" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_495f9145420a44eba.png"/>
<img width="712" height="519" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_c5dc5d5208934e3f8.png"/>

<img width="712" height="556" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_be598718006a48aca.png"/>

2.在自己的工程文件夹中也新建一个MY文件夹

<img width="712" height="344" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_5f4eac8da5ba411cb.png"/>

3.在MY文件夹中新建功能所对应的文件夹名称（如：KEY、LED、MPU6050、PWM）<img width="712" height="253" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_307653733d124bc48.png"/>

4.新建key.c和key.h文件，保存到没MY文件夹下的KEY中

<img width="712" height="536" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_c1c63411ba3648e58.png"/>

5.将.c文件加入到工程

<img width="712" height="308" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_f11868fdbbbd41318.png"/>
![在这里插入图片描述](/_resources/watermark_type_ZmFuZ3poZW5naGVpd_e351f16a45854b6c8.png)

6.key.c和key.h

![在这里插入图片描述](/_resources/watermark_type_ZmFuZ3poZW5naGVpd_359d7bfc3e62456d8.png)
![在这里插入图片描述](/_resources/watermark_type_ZmFuZ3poZW5naGVpd_ea4992b081a8459ba.png)

7.将.h文件路径加载到工程

<img width="712" height="450" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_c88e5f6886714006a.png"/>
<img width="712" height="292" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_33c3a188f7da4e598.png"/>

8.编译一下

<img width="712" height="210" src="/_resources/watermark_type_ZmFuZ3poZW5naGVpd_a15fd7d8f178414f8.png"/>

9.看一下MY文件夹

![在这里插入图片描述](/_resources/watermark_type_ZmFuZ3poZW5naGVpd_2a697b66f69944cd8.png)

#### 移植库文件（仅适用于STM32CubeMX生成的keil项目）

对STM32CubeMX生成的工程来说，更简便的移植方法：

将.c文件放置在cube生成的任意文件夹内（这个路径其实是随意的，可以在Core下新建文件夹单独放）。

将.h文件放置在cube生成的Core-Inc文件夹，或与.c文件放置在一起。

在keil中，新建一个文件夹（文件夹名任取，但最好与.c文件放置的文件夹名相同以方便辨认）-Add Existing files to Group to "xxx"，将.c文件添加。然后点击![21aff4dfa54ed58bea69a7df03a9caee.png](/_resources/21aff4dfa54ed58bea69a7df03a9caee.png)-C/C++-Include Paths中添加.c文件和.h文件所在的所有路径，注意有多个可能有重叠的函数的移植库时，需要调整路径的优先顺序（上下拖动，越靠前越优先），并且先把其他库的路径删除（即使不在Project窗口导入也会自动编译添加了的路径的库）。也不要导入其他移植的库并include，也不要include已经在一个库里include过的库。

注意添加.c文件和.h文件所在的所有路径时，包含了项目名称的绝对路径好像是不会变的（一般是相对路径，但无法判断keil魔法棒中的路径是否相对路径），因此直接复制整个项目出来并对文件夹进行改名后，进行编译会找不到库，疑似因为项目的文件夹名称被更改了。X-Cube-AI包导入模型时模型的路径好像是相对路径，也影响到相关的库在keil中的检测，因此导入的模型最好包含在整个项目文件夹中，避免绝对路径。需要在改名后的第一次重新使用stm32cubeMX生成工程前就把整个项目的文件夹名改回项目名同名，并且X-Cube-AI包导入模型需要重新设置为处于整个项目的文件夹内的相对路径的模型而不是绝对路径的模型，如果第一次没有这样操作，那么使用stm32cubeMX生成工程后还是会生成带有黄色感叹号的库，使得编译不成功。如果还是不行（明明文件夹里有但编译就是找不到库），就把带有黄色感叹号的库移除，并在keil中重新Add Existing files to Group to "xxx"。

对于是否使用STM32CubeMX，都可以移植基于标准库/HAL库编写的库。但是需要修改导入的芯片型号对应的标准库和HAL库，和其他的各种定义。包括一些函数故意挖空，通过报错来提醒移植者填入自己的参数。

STM32CubeIDE工程文件，也是有main.c文件，也是能够进行基于标准库/HAL库编写的库的移植的。

### 电脑端配置串口（通过串口助手软件显示打印的信息）

因为没有ST-LINK V2-1，没有SWO接口，无法采用SWV调试方式，因此无法直接通过ST-LINK Utility直接显示打印的信息。

串口助手都只有监视串口信息的功能，且只能监视电脑自己能够检测出的串口的信息，因此所有串口助手的作用都是相同的，随便选用一个即可。使用较为传统的XCOM串口助手。

想要使得单片机打印信息输出到电脑上，首先需要同时使用ST-LINK（烧录hex文件，如果已烧录可以不使用ST-LINK连接电脑）和USB-232线（将单片机开发板上集成的CH340与电脑进行连接，注意只有特定的梯形接口与CH340是连接的，要在连接后设备管理器中没有黄色感叹号才是连接到CH340）将单片机和电脑连接。然后安装CH340/CH341驱动。

如果使用USB-232线将单片机和电脑连接，那么串口助手中的可选端口应该会多出一个com，这个就是捕获打印的信息的串口。

将波特率设置得与单片机程序设置相同（其他设置也必须与单片机程序设置相同，但一般不需要修改）。

点击"开启串口"，即可开始监视。

#### 安装CH340/CH341时出现"驱动预安装成功"而不是"驱动安装成功"

不需要通过ST-LINK连接，需要通过USB-232线连接（也就是买板子送的一边梯形头一边USB的调试用的线），而且注意板子可能只有其中一个梯形头和内部集成的CH340模块是连接的，连接正确，然后安装CH340/CH341驱动才能提示"驱动安装成功"。

### 在STM32程序中打印信息

SWV调试方法通过ST-LINK Utility查看打印的信息。

配置好串口后，将打印的信息导向电脑的串口助手软件：

1.对printf函数进行重定向（通过重定义fputc函数实现，printf函数依赖于fputc函数，重新实现fputc函数内部从串口发送数据即可间接地实现printf打印输出数据到串口）且使用微库（在keil中勾选Use MicroLIB）；

2.对printf函数进行重定向且使用标准库。

3.直接使用HAL库的HAL_UART_Transmit()函数。

注意对fputc重定义时，需要将数据通过USART串口传输到电脑串口，因此需要提前在STM32CubeMX中配置USART串口。

#### printf语句使得程序不执行问题

在使用CubeMX初始化代码时，生成的工程默认是使用Microlib的，正常情况下，在STM32CubeMX通过成的.s文件里可以看到一个__main函数，这个就是microlib的入口地址，他会完成创建栈空间，创建堆空间，初始化用户可能用到的系统库等初始化动作，最后跳转到我们熟悉的main，当使用Microlib时__main链接的是Microlib，当不使用Microlib时__main链接的是标准库的C/C++；一旦在程序中调用printf等函数时，会让MCU进入半主机模式，从而程序卡死。

要避免卡死，可以选择：

- 使用微库。

- 关闭标准库下的半主机模式。

无论选择哪种方法，都需要将printf函数重定向到UART（需要编写UART驱动）或ITM（SWV调试）。

关闭半主机模式（仍需要重定向printf函数），在代码开头添加：

```
/* 告知连接器不从C库链接使用半主机的函数 */
 
#pragma import(__use_no_semihosting)
 
/* 定义 _sys_exit() 以避免使用半主机模式 */
 
void _sys_exit(int x)
 
{
 
    x = x;
 
}
 
/* 标准库需要的支持类型 */
 
struct __FILE
 
{
 
    int handle;
 
};
 
FILE __stdout;
```

使用微库（仍需要重定向printf函数），只需要：

在keil中生成hex文件前，Project-Option for Target-勾选Use MicroLIB（微库）。

#### SWD调试方法对printf函数的重定向和相关设置（自行选择关闭半主机模式或使用微库）（常用方法）

在STM32CubeMX生成代码中，添加以下代码：

```
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *p)
{
	while((USART1->SR&0X40)==0);
	USART1->DR = ch;
	
	return ch;
} // fputc有可能在某些库里已经定义了，此时就不需要我们手动重定向了。
/* USER CODE END 0 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    printf("\r\n TEST 233 \r\n");
	   // while(1)中的printf()语句不会报错。
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  printf("\r\n TEST 233 \r\n");
  // USER CODE 3 代码块内的printf()语句不会报错。
  /* USER CODE END 3 */

/* USER CODE BEGIN 4 */
// 中断函数中的printf()语句不会报错。直接写的printf()语句是无法编译成功的。
/* USER CODE END 4 */
```

即可在串口助手中查看打印的信息。

#### SWV调试方法对printf函数的重定向和相关设置（自行选择关闭半主机模式或使用微库）

STM32f103ZET6的SWO（TDO）口为PB3（通过STM32CubeMX可以自动设置）。但是旧版本的ST-LINK上没有SWO接口（实际上只有ST-LINK V2-1才有）。

仪器跟踪单元（Instrumentation Trace Macrocell，ITM）主要用途是调试信息的输出。使用SWV调试方式，将printf函数重定向到ITM：

```
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f)
{
  ITM_SendChar(ch);
  return ch;
}
/* USER CODE END 0 */
```

在keil中生成hex文件前，![6ea1c77b445956b1fc1b7997d413577e.png](/_resources/6ea1c77b445956b1fc1b7997d413577e.png)-Debug-Use:"xxx"Settings-Trace-设置Core为单片机主频+勾选Trace Enable。

SWV调试方法不通过串口助手就可以查看单片机打印的信息：

- ![0280e85cd7f164fd69a5ff75cd676d98.png](/_resources/0280e85cd7f164fd69a5ff75cd676d98.png)进入调试模式，View-Serial Windows-Debug(printf) Viewer，然后全速运行，即可看到串口打印的信息。

- 在ST-LINK Utility中，将hex文件烧录完成后，ST-LINK -Printf via SWO viewer-设置System clock为单片机主频+设置端口-START，即可看到串口打印的信息。

#### HAL库的HAL_UART_Transmit()函数打印信息（自行选择关闭半主机模式或使用微库）

需要配置SYS和USART1，参数默认。

在STM32CubeMX生成代码中，添加以下例程：

```
  /* USER CODE BEGIN 1 */
  uint8_t tx_buff[]="USART Test\r\n";
  /* USER CODE END 1 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		HAL_UART_Transmit(&huart1,"Hello!\r\n",8,10);
		HAL_Delay(1000);
		HAL_UART_Transmit(&huart1,tx_buff,12,10);
		HAL_Delay(1000);
   }
  /* USER CODE END WHILE */
```

即可在串口助手中查看打印的信息。

#### 串口直接发送（未验证）

```
// 发送单个字节
void Serial_SendByte(USART_TypeDef* USARTx,uint8_t Byte)
{
	USART_SendData(USARTx,Byte);
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);//标志位TXE等于0就说明寄存器中还有数据未发送出去，等待
}
// 发送字符串,字符串自带一个结束标志位，所以不需要传递长度参数
void Serial_SendString(USART_TypeDef* USARTx,char *String)
{
	uint8_t i;
	for(i=0;String[i]!='\0';i++)
	{
		Serial_SendByte(USARTx,String[i]);
	}
}
```

### 整合-PS2手柄控制小车（电机和舵机）

调试，最重要的还是"知道哪里错了"，或者说"到哪一步错了"，而不是无序地排除错误。

每次通过STM32CubeMX生成工程和代码时，记得添加以下代码（HAL库的PCA9865库的初始化函数，必须放置在这个注释区域才能生效，因此每次重新生成工程和代码时需要重新添加此条代码）：

```
/* Initialize all configured peripherals */
PCA_MG9XX_Init(60,90);
/* USER CODE BEGIN 2 */
```

注意：某些代码的执行需要一定的间隔，可能是执行时间较长。如果在while(1)代码块中执行，由于中断的存在，将会导致无法完整执行。而如果在中断函数中执行，且每次中断都执行，会导致程序直接卡住（每几次中断执行一次是可以的），这意味着这些代码只能通过设定每几次循环才执行一次来保证正常执行（因此在中断函数中运行更合适，因为可以通过中断的次数作为循环数来进行判断）。

根据中断发生的方式，while(1)代码块和中断代码块执行的频率是不一致的。相同时间内，假设以10ms发生一次的定时器中断，中断代码块中的代码的执行次数，较明显地大于while(1)代码块中的代码的执行次数。

STM32执行的主程序，各种初始化和设置执行完后，便会一直执行while(1)代码块中的内容。

通过调用`HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *发生中断的定时器形参名)`函数，可以指定当特定定时器发生中断时执行的代码。中断会发生在while(1)代码块执行的过程中，因此在while(1)代码块中是可以调用中断时执行的代码涉及到（例如修改）的变量的。

注意：PS2的按键的信号输入是连续的，也就是说，主控会不断地受到信号，例如舵机，如果代码中有延时0.1秒，而持续按下对应的键位，则舵机一直不动（刚开始执行就触发下一次执行），而如果没有等待时间，且舵机只有两个状态来回切换，持续按下则会使得舵机不断执行两个状态来回切换的动作，且执行时间极短，因此会在原来的位置不断抖动。因此采用PS2手柄每隔1个中断采集一次信号，舵机程序每判断200~600次信号执行程序（实际需要根据主程序和中断函数分别的程序执行时间来测试，目标达到的效果为按下按键半秒后舵机转动，然后松开，刚好使得舵机不会直接执行下一次转动）。

注意：必须保持供电充足（稳压模块有时候会自行关闭输出，很坑），还有注意接线稳定，不然舵机驱动板和PS2接收器均会乱发射信号（发射的信号是未知的乱值）。

注意：中断函数和主程序while(1)中均不能有过多的printf函数。中断函数中包含过多printf函数使得中断函数的执行时间变长，有脱离一个中断后直接进入下一个中断的风险，此时就等于一直无法执行while(1)中的主程序。主程序while(1)中包含过多printf函数使得主程序的无效程序的执行时间变长，因为中断的存在，主程序的有效程序的执行频率就会减少（使用PCA9685操控舵机程序时，主程序while(1)不能含有printf函数，不然会出现大约秒级的极长的等待时间，未知原因）。综上所述，一般选择将少量printf函数放置在中断函数中（为防止一直执行中断函数，中断函数一般仅保留简单的少量操作，其他操作将尽可能放置在主程序while(1)中，此时printf函数对中断函数的影响更小）。同时操作舵机和电机时，要特别注意，对舵机，舵机的程序包含延时，不能在中断函数中执行，只能在主程序中执行，而对电机，对电机输出PWM信号的程序则必须在中断中执行。

>千万不要在中断函数中做多余的事！只能做最基本的操作，比如置标志位，让一个数加一等，与延时有关的操作千万不要做（包括执行包含了延时操作的函数）！10ms进一次中断，在里面又扫描按键，又显示液晶，这里面花的时间远远地超过10ms，它肯定死在中断里出不来啊，因为你不关闭中断的话它一直10ms进去（定时器的中断触发是独立于程序的），最好在中断里设置几个标志位，进中断就让标志位置1，然后出中断，在主循环里判断标志位，如果标志位置1了，就清零标志位，加上判断语句使得进入中断但不执行中断函数的内容，然后继续执行其他的动作。

注意：中断中的printf函数，有时候需要一个特定的数量，少了一条就整个程序（包括主程序）都无法运行，未知原因。

注意：电机有可能出现接收到PWM信号后只会嗡嗡响，无法启动，需要外力推动一下才能正常转动。越低的占空比，其启动的难度越高。这种情况可以选择更高的占空比（经实验，如果7200为最大占空比，则大概5600才可以达到勉强启动成功。也就是说，其实电机启动需要的占空比是一个较大的占空比，电机成功启动后才有最低速度，所以应该以成功启动的占空比为基础速度，避免以0占空比为基础速度）输出。

注意：经过实验，由基础速度开始逐渐升高（因为是逐渐升高，因此基础速度可以从0开始）的占空比的不断输入，才更容易使电机启动成功。

注意：电机的电压影响速度，电流影响力矩。电机难以启动可能是电流不够。

注意：原理问题。电机是双通道PWM操控的，电机双路pwm的信号会互相抵消。因此在输出一个方向的PWM的同时，需要把另一个方向的PWM清零。

注意：PS2手柄接收器最好用5V供电而不是3.3V供电，更不容易断连。

注意：电机进行前后转动，在相同的占空比下速度不一致，未知原因。

注意：通过将十进制转换为二进制时，如果第一位为0，则将会直接被省略（最后一位为0可以保留，但第一位是最大的一位，为0等于不存在）。

注意：发现原来正常工作的电子元件不工作了，不要慌张，根据对其状态进行改变的历史操作和经验逐步进行排查。

注意：舵机控制板乱发信号，舵机速度不一致，舵机抽搐等问题，都是供电不够导致的。不要用18650锂电池，直接使用航模锂电池。

注意：尽量一个电池供一个元器件（包括MCU）的电，同时对多个元器件供电调电压不方便，而且很大可能会造成两边都供电不足。电机和舵机都是有高功率的供电要求的，需要使用航模锂电池进行供电。MCU在连接受控对象和模块较少的情况下，使用普通18650锂电池供电即可。

注意：任何可导电的物质直接接触：pcb引脚、pcb上裸露的电路、芯片、芯片的封装，都有可能导致短路。表现为直接断电（重启也只能工作一瞬间，然后又需要重启），或者表现出和供电不足相同的表现。

注意：电机和舵机，基本都有个体误差，在发送相同值的pwm信号的条件下，转速和角度一般都会有明显的不同，基本都需要使用PID进行调整。

整体硬件接线（初版）：

![859803C8158BA3C058D6948489B4AF49.png](/_resources/859803C8158BA3C058D6948489B4AF49.png)