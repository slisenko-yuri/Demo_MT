# Проект для демонстрации кооперативной многозадачности на микроконтроллере AVR.



## Введение

Проект разработан для демонстрации кооперативной многозадачности в
микроконтроллерах с ограниченными ресурсами, в частности для микроконтроллера
ATmega328P. Для сборки проекта использована интегрированная среда разработки
Atmel Studio 6.2 (build 1563).


## Описание

Для демонстрации проекта было использовано устройство, состоящее из платы
Arduino NANO и двухосевого джойстика KY-023, которые должны быть соединены
друг с другом в соответствии со следующей таблицей:

|Arduino NANO	|	Джойстик KY-023 |
|---------------|-------------------|
|GND			|GND				|
|5V				|+5V				|
|A7				|VRx				|
|A6				|VRy				|
|A5				|SW					|

Схема устройства также представлена в файле Demo_MT.pdf.
Модуль двухосевого джойстика состоит из двух потенциометров на 10 кОм,
определяющих положение осей X и Y. Изменение сопротивления осуществляется
рычагом. Средний вывод каждого потенциометра выведен на контакты VRx и VRy,
а два других вывода подключены к +5V и GND. Дополнительно установлена
тактовая кнопка, состояние которой снимается с контакта SW.

Для считывания данных с контактов VRx (ось X) и VRy (ось Y) используются
аналоговые порты микроконтроллера (диапазон изменения сигналов от 0 до 5В), а
для считывания данных с контакта SW используется цифровой порт (значения 0 и 1).

Для загрузки HEX-файла в микроконтроллер можно использовать загрузчик,
записанный в Arduino NANO или программатор, например AVRISP mkII.

Для организации работы программы в режиме кооперативной многозадачности в
проекте использована свободная библиотека макросов Protothreads автора Adam
Dunkels для работы с протопотоками, а также вспомогательная библиотека функций
и макросов Multitasking собственной разработки, предназначенная для работы
совместно с библиотекой Protothreads и обеспечивающей дополнительный сервис:

- Организация последовательного выполнения задач.
- Реализация общесистемного таймера.
- Реализация мьютексов для синхронизации доступа к ресурсам.
- Реализация таймаутов для отслеживания различных временных интервалов.
- Поддержка внеочередного выполнения приоритетной задачи.
- Поддержка драйверов периферийных устройств.

файл Config.h представляет собой конфигурационный файл программы.
Содержит объявления констант и макросов, используемых программными модулями,
входящими в проект.

В программе использовано 6 задач. Все задачи реализованы с помощью
протопотоков. В функции main() реализован общеизвестный бесконечный цикл,
который является аналогом планировщика ОС. В этом цикле с помощью макроса
MT_DISPATCH() в каждом проходе производится поочередный вызов активных задач,
реализованных в виде протопотоков и  зарегистрированных с помощью функции
MT_TaskInit(). Синхронизация доступа к АЦП реализована с помощью мьютекса. Для
обеспечения совместного доступа со стороны нескольких задач по выводу текстовых
строк с помощью UART реализован драйвер.

Задачи Task_X и Task_Y производят измерения положения двухосевого джойстка
соответственно по осям X и Y с помощью АЦП, встроенного в микроконтроллер, и,
если положение джойстика изменилось, преобразуют результат измерения в
текстовый вид и отправляют его в UART. Для измерений обе задачи вызывают
функцию-протопоток ReadADC, в которой для синхронизации доступа к АЦП со
стороны нескольких задач используется мьютекс (функция MT_MutexWait для захвата
мьютекса и функция MT_MutexFree для его освобождения).

Задача Task_SW находится в пассивном состоянии и не вызывается планировщиком,
пока не будет нажата кнопка на джойстике. При нажатии/отпускании кнопки
вызывается обработчик прерывания, в котором при помощи функции MT_TaskSetActive
задача Task_SW переводится в активное состояние. Поскольку задаче Task_SW
присвоен статус приоритетной (при помощи вызова MT_TaskSetPriority  в функции
main), то после перехода ее в активное состояние планировщик, как только
получит управление, запустит задачу Task_SW вне очереди. В начале своего
выполнения задача Task_SW приостанавливает свою работу с помощью вызова
MT_SleepMs на время, необходимое для успокоения дребезга контакта кнопки, затем
считывает состояние порта PC5, к которому подключена кнопка (контакт SW),
формирует текстовую строку с состоянием кнопки и отправляет ее в UART. После
этого с помощью вызова MT_TaskSetPassive задача Task_SW переводит сама себя в
пассивное состояние. Во время приостановки своей работы с помощью вызова
MT_SleepMs другие задачи продолжают вызываться планировщиком.

Задача Task_SEC находится в пассивном состоянии, пока не будет переведена в
активное из функции CalcSec, вызываемой из обработчика прерывания системного
таймера. Функция CalcSec производит подсчет тактов системного таймера и при
накоплении одной секунды переводит задачу Task_SEC в активное состояние при
помощи вызова MT_TaskSetActive. После вызова планировщиком задача Task_SEC
преобразует счетчик секунд в строковое представление, выводит полученную строку
в UART, а затем переводит сама себя в пассивное состояние с помощью вызова
MT_TaskSetPassive.

Задача Task_ECHO находится в состоянии ожидания байтов, принятых по UART. Для
ожидания приема используется макрос PT_WAIT_UNTIL(Context, UART0_ReadRx(&rx)).
В состоянии ожидания задача Task_ECHO не вызывается планировщиком. Если принят
байт, то драйвер UART переводит задачу в активное состояние с помощью вызова
MT_DrvTaskRunIrq(DRV_UART_RX). После вызова планировщиком задача Task_ECHO
преобразует полученный байт в строковое представление и выводит полученную
строку в UART, после чего с помощью вызова
PT_WAIT_UNTIL(Context, UART0_ReadRx(&rx)) снова переходит в состояние ожидания
байтов, принятых по UART.

Задача Task_BLINK в бесконечном цикле моргает светодиодом. Для формирования
пауз между переключениями светодиода используется вызов MT_SleepMs, который
на интервал времени, уазанный параметром TimeMs переводит задачу в состояние
ожидания, при котором задача не вызывается планировщиком. По истечении
указанного интервала времени обработчик системного таймера переводит задачу в
активное состояние и она продолжает свое выполнение. Кроме этого задача Task_BLINK
c интервалом пять секунд изменяет скорость переключения светодиода. Для
отслеживания пятисекундных интервалов используются макросы MT_TimeoutSet и
MT_TimeoutGet, предназначенные для работы с таймаутами.





## Компоненты, используемые в проекте

### 1. Свободная библиотека макросов Protothreads для работы с функциями-протопотоками.

Содержит включаемые Файлы с описанием макросов и констант для поддержки
протопотоков. Библиотека предназначена для многозадачного программирования с
малыми накладными расходами. Протопотоки позволяют разделить выполнение
программы на несколько независимых задач. В данном проекте протопотоки
используются для реализации кооперативной многозадачности, не требующей больших
накладных расходов. Основным свойством функций-протопотоков является
возможность продолжения исполнения кода с того места, в котором остановилось
выполнение при предыдущем вызове.

Ссылки для ознакомления с библиотекой Protothreads:
- http://dunkels.com/adam/pt/
- http://bsvi.ru/protopotoki-protothreads/
- https://habr.com/ru/company/embox/blog/244361/
- https://habr.com/ru/post/143318/

#### Сервисы библиотеки Protothreads, используемые в проекте
---


- PT_THREAD(Thread)
  
  Макрос для объявления протопотока.  
  Превращается препроцессором в обычную си-функцию, возвращающую char.
  Локальные переменные, которые должны сохранять свои значения между вызовами
  протопотока должны быть объявлены как static, либо переданы в функцию по
  ссылке. Все задачи в проекте реализованы как протопотоки с одним единственным
  параметром, представляющим собой указатель на контекст протопотока.

  ПРИМЕР:
  ```
  PT_THREAD(Thread(struct pt *Context))
  {
    PT_BEGIN(Context); // Начало протопотока

    // Код протопотока
	// ...

    PT_END(Context); // Завершение протопотока
  }
  ```


- PT_INIT(struct pt *Context);
  
  Инициализация структуры с контекстом протопотока.  
  Переменной Context->lc присваивается значение NULL. Контекст всегда должен
  быть инициализирован до вызова протопотока. В переменной контекста хранится
  адрес, с которого продолжится выполнение протопотока при его следующем
  вызове.

  ПРИМЕР:
  ```
  //...

  struct pt Context; // Объявление переменной для контекста протопотока

  PT_INIT(&Context); // Инициализация контекста

  Thread(&Context); // Вызов протопотока

  //...
  ```


- PT_BEGIN(struct pt *Context);
  
  Объявление начала протопотока внутри функции, реализующей протопоток.  
  Этот макрос осуществляет переход в то место протопотока, адрес которого
  хранится в переменной контекста, а точнее в поле Context->lc. Если
  Context->lc равен NULL, то переход не производится, а выполняется код,
  расположенный непосредственно после макроса PT_BEGIN. Иными словами макрос
  PT_BEGIN осуществляет переход к тому месту протопотока, в которым произошел
  выход в предыдущем вызове.


- PT_END(struct pt *Context);
  
  Объявление окончания протопотока внутри функции, реализующей протопоток.  
  Этот макрос обнуляет переменную контекста (Context->lc = NULL) и осуществляет
  возврат из протопотока значения PT_ENDED (число 3), означющего, что
  протопоток выполнился до конца. Код, расположенный после макроса PT_END не
  выполняется.


- PT_WAIT_UNTIL(struct pt *Context, Condition);
  
  Блокирует протопоток до тех пор, пока условие не станет истинным.  
  Этот макрос сохраняет текущий адрес в переменной контекста, а затем проверяет
  условие Condition. Если условие истинно, то выполнение протопотока
  продолжится после макроса PT_WAIT_UNTIL, иначе произойдет выход из
  протопотока с возвратом значения PT_WAITING (число 0), означающего, что
  протопоток не завершился. При следующем вызове протопотока макрос PT_BEGIN на
  основании переменной контекста осуществит передачу управления в то место
  протопотока, из которого произошел возврат при предыдущем вызове.

  ПРИМЕР:
  ```
  PT_THREAD(Thread(struct pt *Context))
  {
    PT_BEGIN(Context); // Начало протопотока

	//...

    //Блокировка протопотока, пока по интерфейсу UART0 не будет принят байт
    //данных, т.е. пока функция UART0_ReadRx не вернет значение, отличное от 0.
    PT_WAIT_UNTIL(Context, UART0_ReadRx(&rx));

    //...

    PT_END(Context); // Завершение протопотока
  }
  ```


- PT_WAIT_WHILE(struct pt *Context, Condition);
  
  Блокирует протопоток до тех пор, пока условие истинно.  
  Этот макрос сохраняет текущий адрес в переменной контекста, а затем проверяет
  условие Condition. Если условие истинно, то происходит выход из протопотока с
  возвратом значения PT_WAITING (число 0), означающего, что протопоток не
  завершился. В противном случае выполнение кода продолжится после макроса
  PT_WAIT_WHILE. Если в результате использования этого макроса произошел выход
  из протопотока, то при следующем его вызове макрос PT_BEGIN на основании
  переменной контекста осуществит передачу управления в то место протопотока,
  из которого произошел возврат при предыдущем вызове, в данном случае на
  строку с макросом PT_WAIT_WHILE.

  ПРИМЕР:
  ```
  PT_THREAD(Thread(struct pt *Context))
  {
    PT_BEGIN(Context); // Начало протопотока

    //...

    //Блокировка протопотока, пока АЦП не завершит преобразование, т.е. пока
	//бит старта преобразования ADSC в регистре ADCSRA не станет равным 0.
    PT_WAIT_WHILE(Context, ADCSRA & (1<<ADSC));

    //...

    PT_END(Context); // Завершение протопотока
  }
  ```


- PT_SPAWN(struct pt *ContextParent, struct pt *ContextChild, ThreadChild);
  
  Вызывает дочерний протопоток.  
  Блокирует родительский протопоток, пока дочерний не завершится. В качестве
  параметра ThreadChild необходимо передать имя функции, реализующей дочерний
  протопоток. Позволяет одним протопотоком (родительским) запустить другой
  (дочерний) протопоток с самого начала и дождаться его завершения. Дочерний
  протопоток считается завершенным, если он вернул значение PT_EXITED (число 2)
  или PT_ENDED (число 3). Этот макрос инициализирует переменную контекста
  дочернего протопотока ContextChild, сохраняет текущий адрес в переменной
  контекста ContextParent и вызывает дочерний протопоток. Если вызываемый
  дочерний протопоток не завершился (вернул PT_WAITING (число 0) или PT_YIELDED
  (число 1)), то осуществляет выход с возвратом значения PT_WAITING. В этом
  случае при последующем вызове родительского протопотока он продолжится с
  этого же места, при этом инициализация контекста дочернего протопотока в этом
  случае не выполняется. Если же вызываемый дочерний протопоток завершился,
  т.е. вернул PT_EXITED (число 2) или PT_ENDED (число 3), то выполнение
  родительского протопотока продолжится после макроса PT_SPAWN.

  ПРИМЕР:
  ```
  // Дочерний протопоток
  PT_THREAD(ThreadChild(struct pt *Context))
  {
    PT_BEGIN(Context);

    //Код дочернего протопотока
	//...

    PT_END(Context);
  }

  // Родительский протопоток
  PT_THREAD(ThreadParent(struct pt *ContextParent))
  {
    static struct pt ContextChild; // Контекст для дочернего протопотока
	
    PT_BEGIN(ContextParent);

	//...

    // Вызов дочернего протопотока
    PT_SPAWN(ContextParent, &ContextChild, ThreadChild(&ContextChild));

	//...

    PT_END(ContextParent);
  }
  ```


- PT_YIELD(struct pt *Context);
  
  Осуществляет выход из протопотока с возвратом значения PT_YIELDED (число 1)
  (протопоток не завершен) с возможностью продолжения выполнения кода,
  расположенного после макроса PT_YIELD при последующем вызове протопотока.

  ПРИМЕР:
  ```
  PT_THREAD(Thread(struct pt *Context))
  {
    PT_BEGIN(Context);

    //...

    PT_YIELD(Context); // Выход из протопотока.
                       // При следующем вызове протопотока выполнение кода
                       // продолжится с оператора, следующего после макроса
		               // PT_YIELD.
    //...

    PT_END(Context);
  }
  ```

В состав библиотеки Protothreads входят следующие файлы:

- Файл lc.h:  
Содержит макросы для работы с локальными продолжениями (Local continuations).
Локальные продолжения позволяют запомнить состояние выполнения программного
потока (функции), и вернуться к этому месту в дальнейшем.

- Файл lc-addrlabels.h:  
Содержит макросы для реализации локальных продолжений (Local continuations) с
помощью меток-переменных (Labels as values).

- Файл lc-switch.h:  
Содержит макросы для реализации локальных продолжений (Local continuations) с
помощью оператора switch языка программирования СИ.

- Файл pt.h:  
Содержит константы и макросы, используемые в протопотоках (инициализация,
блокировка, ожидание выполнения дочерних протопотоков)

- Файл pt-sem.h:  
Содержит макросы для использования семафоров в протопотоках.
ПРИМЕЧАНИЕ: В данном проекте макросы из этого файла не используются.

Преимущества библиотеки Protothreads:

- Протопотоки – очень удобное средство структурирования программы.
- Протопотоки позволяют разделить выполнение программы на несколько независимых
  задач.
- Протопотоки требуют очень мало ресурсов.




### 2. Вспомогательная библиотека MultiTasking.

Разработана для использования совместно с библиотекой Protothreads.
Предоставляет дополнительный сервис, используемый для кооперативной
многозадачности.  
Библиотека Multitasking состоит из файлов MT_Cfg.h, MT.h, MT.c.


#### Сервисы, предоставляемые библиотекой Multitasking
-------------------------------------------------------------------------------

- MT_DISPATCH();

  Используется только в функции main в теле бесконечного цикла, совместно с
  которым выполняет роль планировщика задач. В каждом проходе цикла запускает
  на выполнение очередную задачу, если она активна (т.е. если в переменной
  __MT_TaskActiveFlags установлен бит, соответствующий текущей задаче). Адреса
  функций (протопотоков), используемых в качестве задач хранятся в массиве
  __MT_Task[], а соответствующие этим задачам контексты в массиве
  __MT_Context[]. Номер текущей задачи, который увеличивается на единицу в
  каждом проходе цикла содержится в переменной __MT_TaskCur. Если активна
  приоритетная задача, то она запускается вне очереди.  
  Осуществляет обработку таймаутов, если константа MT_TIMEOUT_COUNT,
  объявленная в файле Mt_Cfg.h больше 0.

  ПРИМЕР:
  ```
  // Задача 1
  PT_THREAD(Task1(struct pt *Context))
  {
    PT_BEGIN(Context);

    // Код задачи 1
	// ...

	PT_END(Context);
  }

  // Задача 2
  PT_THREAD(Task2(struct pt *Context))
  {
    PT_BEGIN(Context);

	// Код задачи 2
    // ...

	PT_END(Context);
  }

  int main(void)
  {
    //...

    MT_Init(); // Инициализация модуля MT

    MT_TaskInit(Task1, TASK_ACTIVE); // Инициализация задачи 1
    MT_TaskInit(Task2, TASK_ACTIVE); // Инициализация задачи 2

	// Бесконечный цикл с макросом MT_DISPATCH() выполняет роль планировщика
	// задач.
	while (1)
	{
		MT_DISPATCH(); // Один проход планировщика 
	}
  }
  ```


- MT_SYSTIMER_CALLBACK();

  В макросе MT_SYSTIMER_CALLBACK можно разместить код, который будет
  выполняться по каждому прерыванию системного таймера.  
  Функции, переменные и константы, используемые в теле данного макроса
  должны быть видимы в файле Mt_Cfg.h. В данном проекте макрос
  MT_SYSTIMER_CALLBACK используется для вызова пользовательской функции
  CalcSec, прототип которой объявлен в файле Config.h. Файл Config.h в свою
  очередь подключен с помощью директивы #include в файле Mt_Cfg.h.  

  ПРИМЕР:
  ```
  #define MT_SYSTIMER_CALLBACK()\
  do {\
	CalcSec(); /*Вызов пользовательской функции*/\
  } while (0)
  ```


- void MT_Init(void);

  Инициализация менеджера задач.  
  Производит начальную инициализацию переменных, используемых для работы с
  задачами, мьютексами, таймаутами, драйверами.
  Настраивает таймер, используемый в качестве системного. Номер таймера
  определяется в файле MT_Cfg.h с помощью идентификатора MT_SYSTIMER, который
  может принимать значения от 0 до 5. Следует иметь в виду, что таймер, номер
  которого указывается с помощью идентификатора MT_SYSTIMER должен
  присутствовать в используемом микроконтроллере. Например, для
  микроконтроллера ATmega328P в качестве системного можно использовать любой из
  имеющихся таймеров с номерами от 0 до 2. 


- uint8_t MT_TaskInit(MT_TASK_TYPE Task, uint8_t fActive);

  Инициализация задачи.  
  Производит инициализацию контекста задачи и сохраняет адрес задачи в таблице
  задач. Контексты задач хранятся в массиве __MT_Context[], а в качестве таблицы
  задач используется массив __MT_Task[]. Размерность этих массивов определяется
  константой MT_TASK_COUNT в файле Mt_Cfg.h. Параметр fActive указывает
  начальное состояние задачи (активная/пассивная). Если параметр fActive равен
  TASK_ACTIVE, то в переменной __MT_TaskActiveFlags устанавливается бит
  активности для данной задачи.

  ПРИМЕР:
  ```
  uint8_t idTask2; // Переменная для хранения ID задачи Task2

  // Инициализация задачи 1 с пассивным начальным состоянием
  MT_TaskInit(Task1, TASK_PASSIVE);

  // Инициализацияs задачи 2 с активным начальным состоянием и с сохранением
  // идентификатора задачи в переменной idTask2.
  idTask2 = MT_TaskInit(Task2, TASK_ACTIVE);
  ```


- uint32_t MT_GetSysTimer(void);

  Возвращает количество тактов системного таймера, прошедших с момента
  инициализации менеджера задач с помощью функции MT_Init.  
  Возвращаемое значение можно преобразовать в миллисекунды с помощью макроса
  SYSTICK_TO_MS.  
  Для использования этой функций в проекте необходимо в файле Mt_Cfg.h
  раскомментировать следующую строку:  
  `#define MT_USE_GETSYSTIMER`

  ПРИМЕР:
  ```
  // Функция инвертирует вывод порта PIN1 один раз в секунду, а вывод
  // порта PIN5 один раз в пять секунд

  void TogglePins(void)
  {
    static uint32_t CntTicks1 = 0;
	static uint32_t CntTicks5 = 0;
	uint32_t SysTimer;

	// Получаем текущее количество тактов системного таймера
	SysTimer = MT_GetSysTimer();

	if (SysTimer - CntTicks1) >= MS_TO_SYSTICK(1000))
	{
	  CntTicks1 += MS_TO_SYSTICK(1000);
	  TOGGLE(PIN1); // Инверсия порта PIN1
	}

	if (SysTimer - CntTicks5) >= MS_TO_SYSTICK(5000))
	{
	  CntTicks5 += MS_TO_SYSTICK(5000);
	  TOGGLE(PIN5); // Инверсия порта PIN5
	}
  }
  ```


- MT_SleepMs(Context, TimeMs);

  Приостановка выполнения текущей задачи на заданное количество миллисекунд.  
  После завершения времени ожидания задача продолжит свое выполнение с
  оператора, следующего сразу после макроса MT_SleepMs.  
  Переводит задачу в ждущее состояние на промежуток времени, указанный
  параметром TimeMs. Время указывается в миллисекундах. В течение этого
  промежутка времени задача не будет вызываться планировщиком, не потребляя
  таким образом ресурсы процессора. Для использования в проекте необходимо в
  файле Mt_Cfg.h раскомментировать следующую строку:  
  `#define MT_USE_TASK_SLEEP`

  ПРИМЕР:
  ```
  PT_THREAD(TaskN(struct pt *Context))
  {
    PT_BEGIN(Context);
    ...
	// Задержка выполнения задачи на 500мс
	MT_SleepMs(Context, 500);
	...
	PT_END(Context);
  }
  ```


- MT_TimeoutSet(idTimeout, Takts);
- MT_TimeoutSetIrq(idTimeout, Takts);

  Установка таймаута.  
  Первым параметром передается ID таймаута, а параметром Takts время,
  выраженное в тактах системного таймера. Для указания времени в
  миллисекундах необходимо использовать макрос MS_TO_SYSTICK. Счетчики для
  таймаутов располагаются в массиве __MT_Timeout[] в файле Mt.c, количество
  элементов которого определяется константой MT_TIMEOUT_COUNT.  
  Обработка таймаутов производится планировщиком задач в теле макроса
  MT_DISPATCH и заключается в вычитании из счетчиков количества тактов
  системного таймера, пройденных с момента их последней обработки.
  Функция MT_TimeoutSetIrq используется только при запрещенных прерываниях.
  Для работы с большими интервалами времени необходимо в файле Mt_Cfg.h
  раскомментировать следующую строку:  
  `#define MT_USE_LONG_TIMEOUT`  
  В этом случае для таймаутов будут использованы 32-разрядные переменные вместо
  16-разрядных.

  ПРИМЕР:
  ```
  #define TIMEOUT_SW  0  // ID таймаута по нажатию кнопки SW

  //...

  // Включение светодиода HL на время 1000 мс, если нажата кнопка SW
  if (MT_TimeoutGet(TIMEOUT_SW) == 0) // Если таймаут закончился
  {
    if (ACTIVE(SW)) // Если нажата кнопка SW
	{
	  // Установка таймаута 1000 мс
	  MT_TimeoutSet(TIMEOUT_SW, MS_TO_SYSTICK(1000));

	  ON(HL); // Включение светодиода
	}
	else
	{
	  OFF(HL); // Выключение светодиода
	}
  }
  ```

- MT_TimeoutMs(idTimeout, Ms);
- MT_TimeoutMsIrq(idTimeout, Ms);

  Установка таймаута.  
  Первым параметром передается ID таймаута, а параметром Ms время в
  миллисекундах.

  ПРИМЕР:
  ```
  // Установка таймаута 1000 мс
  MT_TimeoutMs(TIMEOUT_SW, 1000);
  ```

- MT_TimeoutGet(idTimeout);

  Чтение тайаута.  
  Возвращает количество тактов системного таймера, оставшихся до окончания
  таймаута с идентификатором idTimeout. Если возвращаемое значение равно 0, то
  это означает, что таймаут закончился.


- MT_MutexWait(Context, Mutex);

  Захват мьютекса.  
  Служит для синхронизации доступа к общему ресурсу.
  Если мьютекс свободен, то задача захватывает его и продолжает свое
  выполнение. Если же мьютекс захвачен другой задачей, то текущая задача
  переводится в ждущее состояние и управление будет передано в планировщик.
  После освобождения мьютекса задачей, захватившей его, ждущая задача будет
  снова переведена в активное состояние и планировщик запустит ее на выполнение
  с того места, в котором была попытка захвата мьютекса с помощью вызова
  MT_MutexWait.  
  Задача может завладевать одним и тем же мьютексом несколько раз, и при этом
  не будет блокироваться даже в тех случаях, когда уже владеет им. Поскольку в
  мьютекс встроен счетчик задача должна освободить мьютекс столько же раз,
  сколько она его захватывала.  
  Для использования мьютексов в программе необходимо в файле Mt_Cfg.h была
  определена константа MT_MUTEX_COUNT в соответствии с требуемым количеством
  мьютексов. Используется только в протопотоках.

  ПРИМЕР:
  ```
  // Захват мьютекса.
  MT_MutexWait(Context, MUTEX_ADC);

  //...

  // Освобождение мьютекса.
  // С помощью этой функции все задачи, которые находятся в состоянии
  // ожидания освобождения этого мьютекса перейдут в активное состояние
  // и планировщик сможет их запустить на выполнение.
  MT_MutexFree(MUTEX_ADC);
  ```


- MT_MutexFree(uint8_t Mutex);

  Освобождение мьютекса.  
  Если задача ранее только один раз захватывала мьютекс с помощью вызова
  MT_MutexWait, то после вызова MT_MutexFree мьютекс освобождается и все
  задачи, ждущие освобождения данного мьютекса переводятся в активное
  состояние. Одна из них, которую раньше других вызовет планировщик сможет
  захватить данный мьютекс. Если же задача в ходе своего выполнения несколько
  раз осуществляла вызов MT_MutexWait, то после вызова MT_MutexFree мьютекс не
  освобождается, а осуществляется декремент счетчика, встроенного в мьютекс, и,
  только если счетчик станет равным 0 мьютекс будет освобожден.


- MT_TaskSetPriority(idTask);

  Присваивает задаче статус приоритетной.  
  Если активна приоритетная задача, то планировщик всегда запускает ее вне
  очереди. 

  ПРИМЕР:
  ```
  idTask_SW = MT_TaskInit(Task_SW, TASK_PASSIVE); // Инициализация задачи

  MT_TaskSetPriority(idTask_SW); // Присваеваем задаче статус приоритетной
  ```


- MT_TaskPriorityClr();

  Сбрасывает статус приоритета для текущей задачи.


- MT_TaskSetActive(uint8_t idTask);
- MT_TaskSetActiveIrq(uint8_t idTask);

  Перевод задачи в активное состояние.  
  После перевода задачи в активное состояние планировщик запустит ее на
  выполнение как только до нее дойдет очередь. Функция MT_TaskSetActiveIrq
  используется только при запрещенных прерываниях.

  ПРИМЕР:
  ```
  // Обработчик прерывания по изменению состояния порта
  ISR(PCINT1_vect)
  {
    //...
									
	// Перевод задачи, которая обслуживает кнопку в активное состояние
	MT_TaskSetActiveIrq(idTask_SW);
  }
  ```


- uint8_t MT_TaskSetPassive(void);

  Перевод задачи в пассивное состояние.  
  После перевода задачи в пассивное состояние планировщик не будет запускать ее
  на выполнение.


- MT_DrvTaskWait(idDrv);
- MT_DrvTaskWaitIrq(idDrv);

  Перевод задачи драйвером в ждущее состояние.  
  Если драйвер по какой-либо причине не может выполнить запрос на обслуживание,
  то с помощью этой функции драйвер переводит текущую задачу в ждущее состояние
  с сохранением ID текущей задачи (в виде битовой маски) в переменной
  __MT_DrvWaitFlags[idDrv]. В дальнейшем, когда драйвер будет готов выполнить
  запрос на обслуживание, то он с помощью функции MT_DrvTaskRun(idDrv)
  переведет все задачи, ожидающие освобождения драйвера, в активное состояние
  (т.е. задачи, битовые маски которых хранятся в переменной
  __MT_DrvWaitFlags[idDrv]).  
  Функция MT_DrvTaskWaitIrq используется только при запрещенных прерываниях.

  ПРИМЕР:
  ```
  // Функция драйвера по приему байта из UART0
  uint8_t UART0_ReadRx(char *Data)
  {
	if (UART0_CountRx == 0) // Если буфер приема пуст
	{
		//Перевод текущей задачи в неактивное состояние
		MT_DrvTaskWait(DRV_UART_RX);
		return 0;
	}

	//...

  }
  ```


- MT_DrvTaskRun(idDrv);
- MT_DrvTaskRunIrq(idDrv);

  Перевод задач, ждущих освобождения драйвера в активное состояние.  
  Функция MT_DrvTaskRunIrq используется только при запрещенных прерываниях.

  ПРИМЕР:
  ```
  // Обработчик прерывания по приему из UART
  ISR(USART_RX_vect)
  {
    WrRx(UDR0); // Сохраняем принятый байт в буфере приема

	// Активируем задачи, которые ждут освобождения данного драйвера
	MT_DrvTaskRunIrq(DRV_UART_RX);
  }


- MS_TO_SYSTICK(ms);

  Макрос для перевода миллисекунд в такты системного таймера.


- SYSTICK_TO_MS(tick);

  Макрос для перевода тактов системного таймера в миллисекунды.


#### Конфигурирование библиотеки Multitasking
-------------------------------------------------------------------------------

Конфигурирование библиотеки Multitasking производится с помощью директив
препроцессора, расположенных в файле MT_Cfg.h:

- `#define MT_TASK_COUNT n`, где n может принимать значения от 1 до 32.  
 Определяет количество задач в программе.

- `#define MT_TIMEOUT_COUNT n`, где n может принимать значения от 0 до 255.  
 Определяет количество таймаутов.

- `#define MT_MUTEX_COUNT n`, где n может принимать значения от 0 до 255.  
 Определяет количество мьютексов.  

- `#define MT_DRV_COUNT n`, где n может принимать значения от 0 до 255.  
 Определяет количество драйверов.  

- `#define MT_SYSTIMER n`, где n равен номеру одного из таймеров (начиная с 0),
 имеющегося в составе используемого микроконтроллера.  
 Определяет номер таймера, который будет использован в качестве системного.  

- `#define MT_SYSTIMER_PERIOD ms`, где параметр ms равен периоду срабатывания
 системного таймера в миллисекундах и должен быть в диапазоне от 1 до 20.  
 Настройка таймера на требуемый период срабатывания производится при вызове
 функции MT_Init.

- `#define MT_USE_TASK_SLEEP`  
 Определяется для использования программных задержек с помощью MT_SleepMs().  
 Если же вызов MT_SleepMs не используется, то для сокращения размера прошивки
 данную директиву следует закомментировать.

- `#define MT_USE_LONG_TIMEOUT`  
 Определяется для использования больших таймаутов.  
 Если определена данная директива, то для таймаутов будут использованы
 32-разрядные переменные. Если для таймаутов достаточно 16-разрядных
 переменных, то для сокращения размера прошивки и увеличения производительности
 данную директиву следует закомментировать.

- `#define MT_USE_GETSYSTIMER`  
 Определяется для использования функции MT_GetSysTimer().  
 Если же вызов MT_GetSysTimer не используется, то для сокращения размера
 прошивки эту директиву следует закомментировать.  
 _ПРИМЕЧАНИЕ_: Если константа MT_TIMEOUT_COUNT больше 0, то определение символа
 MT_USE_GETSYSTIMER не оказывает влияния на размер прошивки, поскольку функция
 MT_GetSysTimer используется при работе с таймаутами.

- `#define MT_USE_TASK_RUN_STOP`  
 Определяется для использования функций MT_TaskSetActive(),
 MT_TaskSetActiveIrq() и MT_TaskSetPassive().  
 Если же данные функции не используются, то для сокращения размера прошивки
 данную директиву следует закомментировать.

- `#define MT_USE_HIGH_PRIORITY_TASK`  
 Определяется для использования макросов MT_TaskSetPriority() и
 MT_TaskPriorityClr().  
 Если в программе не используется приоритетная задача, то для сокращения
 размера прошивки данную директиву следует закомментировать.

- `#define MT_USE_GPIOR`  
 Определяется для того, чтобы в качестве некоторых переменных модуля MT
 использовались регистры микроконтроллера GPIOR0...GPIOR2 вместо ячеек ОЗУ.  
 Такое использование регистров GPIOR0...GPIOR2 вместо ОЗУ дает сокращение
 размера прошивки.  
 _ПРИМЕЧАНИЕ_: Регистры GPIOR0...GPIOR2 не будут использоваться, если количество
 задач больше 8, даже если определен MT_USE_GPIOR, поскольку разрядности этих
 регистров хватает для хранения битовых масок не более, чем для 8-ми задач.




### 3. Библиотека для работы с АЦП.

Используется при измерении перемещений двухосевого джойстика.  
Состоит из файлов ADC_Cfg.h, ADC.h, ADC.c.  
В файле ADC.h с помощью директив препроцессора производится вычисление
константы ADCSRA_DIV, которую при инициализации АЦП необходимо записать в
разряды ADPS2...ADPS0 регистра ADCSRA для установки требуемой частоты
преобразования АЦП. Константа ADCSRA_DIV вычисляется на основании константы
ADC_FREQ_MIN или ADC_FREQ_MAX, одна из которых должна быть объявлена в файле
ADC_Cfg.h, а также константы F_CPU, объявленной в файле Config.h. Константа
ADC_FREQ_MIN определяет минимальную частоту преобразования АЦП (не ниже), а
ADC_FREQ_MAX максимальную (не выше).




### 4. Библиотека для работы с последовательным интерфейсом UART.

Используется для приема и передачи информации по UART.
Состоит из файлов UART.h, UART.c.
Функции передачи и приема с помощью UART выполняют роль драйверов. Это означает
то, что при вызове функции передачи или приема они могут перевести задачу в
состояние ожидания, если в данный момент не могут выполнить запрос, а затем
(обычно в обработчике прерывания) перевести все задачи, ждущие обслуживания
данным драйвером в активное состояние.


#### Сервисы, предоставляемые библиотекой для работы с UART
-------------------------------------------------------------------------------

- void UART0_Init(void)

  Инициализация UART.  
  Настраивает UART для работы в асинхронном режиме с одним стоп-битом и длиной
  посылки 8-бит без контроля ошибок паритета. Для установки скорости передачи
  использует значения, которые вычисляются препроцессором на основании констант
  F_CPU и BAUD, объявленных в файле Config.h.


- uint8_t UART0_SendStr(char *Str)

  Посылает строку в UART.  
  Вызов этой функции должен быть использован как аргумент макроса
  PT_WAIT_UNTIL.  
  Если в буфере для передачи (массив UART0_BufTx[]) недостаточно места для
  размещения передаваемой строки, то эта функция с помощью вызова
  MT_DrvTaskWait(DRV_UART_TX) сохраняет битовую маску текущей задачи в
  элементе массива __MT_DrvWaitFlags[DRV_UART_TX] и очищает бит активности этой
  задачи в переменной __MT_TaskActiveFlags, переводя таким образом текущую
  задачу в ждущее состояние. В этом случае функция возвращает 0, а макрос
  PT_WAIT_UNTIL на основании возвращенного значения обеспечивает выход из
  протопотока с сохранением своего адреса в переменной контекста с целью
  возврата к этому адресу при последующем вызове протопотока.  
  Если же в буфере для передачи имеется свободное место для размещения
  передаваемой строки, то функция UART0_SendStr копирует эту строку в буфер,
  разрешает прерывание по передаче и возвращает 1. Выполнение протопотока в
  этом случае продолжится после макроса PT_WAIT_UNTIL. Далее передача байтов из
  буфера в UART производится с помощью обработчика прерываний. По завершению
  передачи содержимого буфера все задачи, которые ждут обслуживания переводятся
  в активное состояние. Для этого в обработчике прерывания после передачи
  последнего байта используется вызов MT_DrvTaskRunIrq(DRV_UART_TX), который
  копирует битовые маски ждущих задач, сохраненных в переменной
  __MT_DrvWaitFlags[DRV_UART_TX] драйвера в переменную __MT_TaskActiveFlags, в
  результате чего планировщик запустит эти задачи на выполнение и они получат
  возможность снова вызвать функцию UART0_SendStr.  
  ПРИМЕЧАНИЕ: Строка, адрес которой указывается в качестве аргумента функции
  UART0_SendStr должна быть объявлена с использованием ключевого слова static,
  если она объявлена в области локальных переменных протопотока, т.к.
  содержимое строки не должно теряться между вызовами протопотока.


  ПРИМЕР:
  ```
  static char str[20];

  // Формирование текстовой строки для вывода в UART
  sprintf(str, "Task_SEC: %d sec\r", sec);
	
  // Вывод строки в UART
  PT_WAIT_UNTIL(Context, UART0_SendStr(str));
  ```


  - uint8_t UART0_SendStr_P(const char *Str)

  Выводит в UART0 строку, расположенную во FLASH.  
  Работает аналогично функции UART0_SendStr, за тем исключением, что
  предназначена для строк, расположенных во FLASH-памяти микроконтроллера.  

  ПРИМЕР:
  ```
  const char Msg[] PROGMEM = "Text1";

  PT_WAIT_UNTIL(pt, UART0_SendStr_P(Msg));

  PT_WAIT_UNTIL(pt, UART0_SendStr_P(PSTR("Text2")));
  ```


  - uint8_t UART0_SendChar(char Data)

  Посылает символ в UART0.  
  Работает аналогично функции UART0_SendStr, но только для одного символа.

  ПРИМЕР:
  ```
  PT_WAIT_UNTIL(pt, UART0_SendChar('#'));
  ```


  - uint8_t UART0_ReadRx(char *Data)

  Чтение байта из приемного буфера.  
  Вызов этой функции должен быть использован как аргумент макроса
  PT_WAIT_UNTIL.  
  Если буфер приема пуст (массив UART0_BufRx[UART0_SIZE_RX]), то эта функция с
  помощью вызова MT_DrvTaskWait(DRV_UART_RX) сохраняет битовую маску текущей
  задачи в элементе массива __MT_DrvWaitFlags[DRV_UART_RX] и очищает бит
  активности этой задачи в переменной __MT_TaskActiveFlags, переводя таким
  образом текущую задачу в ждущее состояние. В этом случае функция возвращает
  0, а макрос PT_WAIT_UNTIL на основании возвращенного значения обеспечивает
  выход из протопотока с сохранением своего адреса в переменной контекста с
  целью возврата к этому адресу при последующем вызове протопотока.  
  Если же в приемном буфере имеются байты, то функция UART0_ReadRx копирует
  байт из буфера по адресу, переданному в функцию в качестве параметра и
  возвращает 1. Выполнение протопотока в этом случае продолжится после макроса
  PT_WAIT_UNTIL. Прием байтов из UART в приемный буфер производится с помощью
  обработчика прерываний, в котором по приему байта задача, ожидающая приема
  переводится в активное состояние с помощью вызова
  MT_DrvTaskRunIrq(DRV_UART_RX), который копирует битовую маску ждущей задачи,
  сохраненную в переменной __MT_DrvWaitFlags[DRV_UART_RX] драйвера в переменную
  __MT_TaskActiveFlags, в результате чего планировщик запустит эту задачу на
  выполнение и она сможет обработать принятый байт.

  ПРИМЕР:
  ```
  // Задача считывает байт, принятый из UART, преобразует его в текстовую форму
  // и отправляет обратно в UART.
  PT_THREAD(Task_ECHO(struct pt *Context))
  {
    static char rx;
    static char str[20];

    PT_BEGIN(Context); // Начало протопотока

    // Ожидание приема байта из UART.
    // Если имеется принятый байт, то выполнение задачи продолжится, иначе
    // произойдет выход из протопотока и передача управления в планировщик.
    PT_WAIT_UNTIL(Context, UART0_ReadRx(&rx));
	
    // Формирование текстовой строки для вывода в UART
    sprintf(str, "Task_ECHO: Code=%02x\r\n", rx);

    // Вывод строки в UART.
    PT_WAIT_UNTIL(Context, UART0_SendStr(str));

    PT_END(Context); // Завершение протопотока
  }
  ```


- uint8_t UART0_CntRx(void)

  Возвращает количество байтов, находящихся в буфере приема.




### 5. Библиотека макросов, объявленных в файле pin_macros.h.

Используется для управления портами микроконтроллера.  
Например, в файле Config.h для управления выводом порта микроконтроллера в
качестве выхода, который управляет светодиодом сделано следующее объявление:

`#define HL		B,5,H`

Здесь символ "B" означает принадлежность вывода к порту B микроконтроллера,
цифра "5" обозначает номер разряда порта, а символ "H" обозначает активный
логический уровень (лог.1) сигнала HL.  
После этого объявления управлять выводом порта можно с помощью
следующих макросов:  
```
DRIVER(HL, OUT); // Настройка вывода порта HL в качестве выхода
ON(HL); // Установить активный уровень сигнала HL (т.е. уровень, определенный
        // символом H при объявлении сигнала HL)
OFF(HL); // Установить пассивный уровень сигнала HL (т.е. уровень,
         // противоположный активному)
CLR(HL); // Установить сигнал HL в состояние лог.0
SET(HL); // Установить сигнал HL в состояние лог.1
CPL(HL); // Инвертировать сигнал HL
TOGGLE(HL); // Смотри CPL(HL)
```

Настройка порта в качестве входа производится следующим образом:
```
#define SW		C,5,L // Порт C, Разряд 5, активный уровень - лог.0
DRIVER(SW, IN);  // Настройка вывода порта SW в качестве входа
```
После этого работать с портом можно с помощью следующих макросов:  
```
if (ACTIVE(SW)) {...} // Если на входе SW активный уровень сигнала, то
                      // выполнить код в фигурных скобках
if (PIN_H(SW)) {...} // Если на входе SW высокий уровень сигнала, то
                     // выполнить код в фигурных скобках
if (PIN_L(SW)) {...} // Если на входе SW низкий уровень сигнала, то
                     // выполнить код в фигурных скобках
```
