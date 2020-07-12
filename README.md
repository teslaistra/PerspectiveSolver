<h2>Описание приложения</h2><br>
В приложении используются следующие библиотеки: ImGui, OpenCV, stb, OpenGL. Stb – необходима для загрузки изображения в текстуру изображения для дальнейшего отображения ее пользователю. OpenGL – используется как библиотека для рендеринга интерфейса ImGui. Выбор в ее пользу был сделан ввиду ее кроссплатформенности. <br>
При запуске приложения пользователь видит небольшое окно (рисунок 1), в котором ему предлагается ввести путь до изображения, которое следует обработать.<br> Такое метод выбора изображения также использован для соблюдения требований по кроссплатформенности, так как ImGui не имеет штатных средств для работы с файловой системой.<br> 
![Start](../master/pic/1.png)
Рисунок 1 - стартовое окно<br>
Если ввести некорректный путь до изображения, то будет выведено сообщение об ошибке (рисунок 2). <br>
 
Рисунок 2 - cсообщение об ошибке<br>
При вводе корректного пути до изображения откроется новое окно (рисунок 3), где будет возможно выбрать точки для коррекции перспективы. Там же будут кнопки для отката на стартовую страницу, сохранения и для выбора пути сохранения. Также выводится текущий путь, куда сохраняется обработанное изображение. <br>
 
Рисунок 3 - окно с открытым изображением<br>
Если нажать на кнопку сохранения до момента создания исправленного изображения, то выведется сообщение об ошибке (рисунок 4). <br>
 
Рисунок 4 - ошибка при сохранении<br>
Далее при выборе четырех разных углов на изображении будет выведено (рисунок 5) исправленное изображение. Порядок выбора углов не имеет значения. <br>
 
Рисунок 5 - исправленное изображение<br>
<h2>Инструкция по сборке </h2><br>

<h5>Для сборки необходимо добавить системные переменные, указывающие на OpenCV. Работа приложения проверена на OpenCV версии 4.20.</h5> <br>
1.	В переменные среды для пользователя в Path добавьте путь до bin-директории нужной сборки OpenCV. Некорректное указание этой переменной может приводить к ошибке при сборке с указанием на отсутствие отсутствия .dll файла OpenCV.  <br>
2.	Также добавьте системную и пользовательскую переменную OPENCV_DIR c путем до папки OpenCV. <br>
3.	Если для сборки используется не версия OpenCV 4.2, то в настройках проекта VS следует поменять название файла opencv_world420.lib на ту версию, которая будет использоваться, то есть, например, на opencv_world310.lib. Менять следует в Настройки проекта -> Компоновщик
 -> Ввод -> Дополнительные зависимости. <br>
4.	После выполнения этих действий проект можно запускать
