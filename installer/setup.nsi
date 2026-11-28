;---------------------------------------------------------
; MonkeySounds Installer Script
; Supports Standard (recommended) and Portable modes
; Multi-language: auto-detects Windows UI language
;---------------------------------------------------------

!define APP_NAME       "MonkeySounds"
!define APP_EXE        "MonkeySounds.exe"
!define PUBLISHER      "NSTechBytes"
!define VERSION        "1.1.0.0"
!define REGKEY         "Software\MonkeySounds"
!define UNINSTALL_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\MonkeySounds"
!define WEBSITE        "https://github.com/NSTechBytes/MonkeySounds"

Name "${APP_NAME}"
OutFile "dist_output\MonkeySounds_Setup_v${VERSION}.exe"
Unicode True
SetCompressor /SOLID lzma

; Default install dir (overridden by portable mode at runtime)
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${REGKEY}" "Install_Dir"

RequestExecutionLevel admin

;---------------------------------------------------------
; Includes — StrFunc MUST be included and initialized
;             before any function that uses ${StrStr}
;---------------------------------------------------------
!include "StrFunc.nsh"
${StrStr}   ; initialize the StrStr function

!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "Sections.nsh"

;---------------------------------------------------------
; Interface
;---------------------------------------------------------
!define MUI_ICON                       "assets\installer.ico"
!define MUI_UNICON                     "assets\uninstaller.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP         "assets\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP   "assets\banner.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "assets\banner.bmp"

!define MUI_WELCOMEPAGE_TITLE "$(INST_WELCOME_TITLE)"
!define MUI_WELCOMEPAGE_TEXT  "$(INST_WELCOME_TEXT)"

!define MUI_FINISHPAGE_RUN          "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT     "$(INST_FINISH_LAUNCH)"
!define MUI_FINISHPAGE_LINK         "$(INST_FINISH_LINK)"
!define MUI_FINISHPAGE_LINK_LOCATION "${WEBSITE}"

;---------------------------------------------------------
; Pages
;---------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"

; Custom page: choose Standard or Portable
Page custom InstallModePageCreate InstallModePageLeave

; Directory page — hidden / overridden in portable mode
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.RemoveDataPageCreate un.RemoveDataPageLeave
!insertmacro MUI_UNPAGE_INSTFILES

;---------------------------------------------------------
; Languages — NSIS picks the best match for the Windows UI
; language at runtime.  English MUST be listed first so it
; becomes the fallback when no better match is found.
;---------------------------------------------------------
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"

;---------------------------------------------------------
; Custom localizable strings
; Each LangString defines a per-language value.
; Strings not defined for a language fall back to English.
;---------------------------------------------------------

; ---- Welcome page ----
LangString INST_WELCOME_TITLE ${LANG_ENGLISH}      "Welcome to ${APP_NAME} ${VERSION} Setup"
LangString INST_WELCOME_TITLE ${LANG_ARABIC}       "مرحباً بك في إعداد ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_GERMAN}       "Willkommen beim Setup von ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SPANISH}      "Bienvenido al instalador de ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_FRENCH}       "Bienvenue dans l'installation de ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_ITALIAN}      "Benvenuto nell'installazione di ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_JAPANESE}     "${APP_NAME} ${VERSION} セットアップへようこそ"
LangString INST_WELCOME_TITLE ${LANG_KOREAN}       "${APP_NAME} ${VERSION} 설치 프로그램에 오신 것을 환영합니다"
LangString INST_WELCOME_TITLE ${LANG_DUTCH}        "Welkom bij de installatie van ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_POLISH}       "Witamy w instalatorze ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_PORTUGUESEBR} "Bem-vindo ao instalador do ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_PORTUGUESE}   "Bem-vindo ao instalador do ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_RUSSIAN}      "Добро пожаловать в установщик ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SIMPCHINESE}  "欢迎安装 ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_TRADCHINESE}  "歡迎安裝 ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_TURKISH}      "${APP_NAME} ${VERSION} Kurulum Sihirbazına Hoşgeldiniz"
LangString INST_WELCOME_TITLE ${LANG_UKRAINIAN}    "Ласкаво просимо до встановлення ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_CZECH}        "Vítejte v instalačním průvodci ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SLOVAK}       "Vitajte v inštalátore ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_HUNGARIAN}    "Üdvözli a ${APP_NAME} ${VERSION} telepítője"
LangString INST_WELCOME_TITLE ${LANG_ROMANIAN}     "Bun venit la instalarea ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_BULGARIAN}    "Добре дошли в инсталатора на ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_CROATIAN}     "Dobrodošli u instalaciju ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_FINNISH}      "Tervetuloa ${APP_NAME} ${VERSION} -asennukseen"
LangString INST_WELCOME_TITLE ${LANG_GREEK}        "Καλώς ήρθατε στην εγκατάσταση του ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_HEBREW}       "ברוך הבא להתקנת ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_INDONESIAN}   "Selamat datang di Penginstal ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_VIETNAMESE}   "Chào mừng đến với trình cài đặt ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_THAI}         "ยินดีต้อนรับสู่ตัวติดตั้ง ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SWEDISH}      "Välkommen till installationen av ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_NORWEGIAN}    "Velkommen til installasjon av ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_DANISH}       "Velkommen til installation af ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_ESTONIAN}     "Tere tulemast ${APP_NAME} ${VERSION} paigaldamisse"
LangString INST_WELCOME_TITLE ${LANG_LATVIAN}      "Laipni lūdzam ${APP_NAME} ${VERSION} instalācijā"
LangString INST_WELCOME_TITLE ${LANG_LITHUANIAN}   "Sveiki atvykę į ${APP_NAME} ${VERSION} diegimą"
LangString INST_WELCOME_TITLE ${LANG_MALAY}        "Selamat datang ke Pemasangan ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SERBIAN}      "Добродошли у инсталацију ${APP_NAME} ${VERSION}"
LangString INST_WELCOME_TITLE ${LANG_SLOVENIAN}    "Dobrodošli v namestitev ${APP_NAME} ${VERSION}"

LangString INST_WELCOME_TEXT ${LANG_ENGLISH}      "This will install ${APP_NAME} on your computer.$\r$\n$\r$\nMonkeySounds adds satisfying keyboard and mouse sounds to every click and keystroke.$\r$\n$\r$\nClick Next to continue."
LangString INST_WELCOME_TEXT ${LANG_ARABIC}       "سيقوم هذا البرنامج بتثبيت ${APP_NAME} على جهازك.$\r$\n$\r$\nيضيف MonkeySounds أصواتاً ممتعة للوحة المفاتيح والفأرة.$\r$\n$\r$\nانقر التالي للمتابعة."
LangString INST_WELCOME_TEXT ${LANG_GERMAN}       "Dieses Programm installiert ${APP_NAME} auf Ihrem Computer.$\r$\n$\r$\nMonkeySounds fügt angenehme Tastatur- und Mausgeräusche hinzu.$\r$\n$\r$\nKlicken Sie auf Weiter, um fortzufahren."
LangString INST_WELCOME_TEXT ${LANG_SPANISH}      "Este programa instalará ${APP_NAME} en su equipo.$\r$\n$\r$\nMonkeySounds añade sonidos de teclado y ratón a cada clic.$\r$\n$\r$\nHaga clic en Siguiente para continuar."
LangString INST_WELCOME_TEXT ${LANG_FRENCH}       "Ce programme va installer ${APP_NAME} sur votre ordinateur.$\r$\n$\r$\nMonkeySounds ajoute des sons de clavier et de souris à chaque frappe.$\r$\n$\r$\nCliquez sur Suivant pour continuer."
LangString INST_WELCOME_TEXT ${LANG_ITALIAN}      "Questo programma installerà ${APP_NAME} sul tuo computer.$\r$\n$\r$\nMonkeySounds aggiunge suoni di tastiera e mouse a ogni clic.$\r$\n$\r$\nFai clic su Avanti per continuare."
LangString INST_WELCOME_TEXT ${LANG_JAPANESE}     "このプログラムはコンピューターに ${APP_NAME} をインストールします。$\r$\n$\r$\nMonkeySounds はキーボードとマウスの音を追加します。$\r$\n$\r$\n次へをクリックして続行してください。"
LangString INST_WELCOME_TEXT ${LANG_KOREAN}       "이 프로그램은 ${APP_NAME}을(를) 컴퓨터에 설치합니다.$\r$\n$\r$\nMonkeySounds는 모든 클릭 및 키 입력에 소리를 추가합니다.$\r$\n$\r$\n다음을 클릭하여 계속하십시오."
LangString INST_WELCOME_TEXT ${LANG_DUTCH}        "Dit programma installeert ${APP_NAME} op uw computer.$\r$\n$\r$\nMonkeySounds voegt toetsenbord- en muisgeluiden toe aan elke klik.$\r$\n$\r$\nKlik op Volgende om door te gaan."
LangString INST_WELCOME_TEXT ${LANG_POLISH}       "Ten program zainstaluje ${APP_NAME} na Twoim komputerze.$\r$\n$\r$\nMonkeySounds dodaje dźwięki klawiatury i myszy do każdego kliknięcia.$\r$\n$\r$\nKliknij Dalej, aby kontynuować."
LangString INST_WELCOME_TEXT ${LANG_PORTUGUESEBR} "Este programa instalará o ${APP_NAME} em seu computador.$\r$\n$\r$\nMonkeySounds adiciona sons de teclado e mouse a cada clique.$\r$\n$\r$\nClique em Avançar para continuar."
LangString INST_WELCOME_TEXT ${LANG_PORTUGUESE}   "Este programa instalará o ${APP_NAME} no seu computador.$\r$\n$\r$\nMonkeySounds adiciona sons de teclado e rato a cada clique.$\r$\n$\r$\nClique em Seguinte para continuar."
LangString INST_WELCOME_TEXT ${LANG_RUSSIAN}      "Этот установщик установит ${APP_NAME} на ваш компьютер.$\r$\n$\r$\nMonkeySounds добавляет звуки клавиатуры и мыши к каждому нажатию.$\r$\n$\r$\nНажмите Далее, чтобы продолжить."
LangString INST_WELCOME_TEXT ${LANG_SIMPCHINESE}  "此程序将在您的计算机上安装 ${APP_NAME}。$\r$\n$\r$\nMonkeySounds 为每次点击和按键添加悦耳的声音。$\r$\n$\r$\n点击“下一步”继续。"
LangString INST_WELCOME_TEXT ${LANG_TRADCHINESE}  "此程式將在您的電腦上安裝 ${APP_NAME}。$\r$\n$\r$\nMonkeySounds 為每次點擊和按鍵加入悅耳的聲音。$\r$\n$\r$\n按一下「下一步」繼續。"
LangString INST_WELCOME_TEXT ${LANG_TURKISH}      "Bu program ${APP_NAME} uygulamasını bilgisayarınıza kuracaktır.$\r$\n$\r$\nMonkeySounds her tuş ve tıklamaya klavye ve fare sesleri ekler.$\r$\n$\r$\nDevam etmek için İleri'ye tıklayın."
LangString INST_WELCOME_TEXT ${LANG_UKRAINIAN}    "Ця програма встановить ${APP_NAME} на ваш комп'ютер.$\r$\n$\r$\nMonkeySounds додає звуки клавіатури та миші до кожного натискання.$\r$\n$\r$\nНатисніть Далі, щоб продовжити."
LangString INST_WELCOME_TEXT ${LANG_CZECH}        "Tento program nainstaluje ${APP_NAME} na váš počítač.$\r$\n$\r$\nMonkeySounds přidává zvuky klávesnice a myši ke každému stisku.$\r$\n$\r$\nPokračujte kliknutím na Další."
LangString INST_WELCOME_TEXT ${LANG_HUNGARIAN}    "Ez a program telepíti a ${APP_NAME} alkalmazást a számítógépére.$\r$\n$\r$\nA MonkeySounds billentyűzet- és egérhangokat ad minden kattintáshoz.$\r$\n$\r$\nA folytatáshoz kattintson a Tovább gombra."
LangString INST_WELCOME_TEXT ${LANG_ROMANIAN}     "Acest program va instala ${APP_NAME} pe calculatorul dvs.$\r$\n$\r$\nMonkeySounds adaugă sunete de tastatură și mouse la fiecare apăsare.$\r$\n$\r$\nFaceți clic pe Următorul pentru a continua."
LangString INST_WELCOME_TEXT ${LANG_VIETNAMESE}   "Chương trình này sẽ cài đặt ${APP_NAME} trên máy tính của bạn.$\r$\n$\r$\nMonkeySounds thêm âm thanh bàn phím và chuột vào mỗi lần nhấp.$\r$\n$\r$\nNhấp Tiếp theo để tiếp tục."
LangString INST_WELCOME_TEXT ${LANG_INDONESIAN}   "Program ini akan memasang ${APP_NAME} di komputer Anda.$\r$\n$\r$\nMonkeySounds menambahkan suara keyboard dan mouse pada setiap klik.$\r$\n$\r$\nKlik Lanjut untuk melanjutkan."
LangString INST_WELCOME_TEXT ${LANG_THAI}         "โปรแกรมนี้จะติดตั้ง ${APP_NAME} บนคอมพิวเตอร์ของคุณ$\r$\n$\r$\nMonkeySounds เพิ่มเสียงคีย์บอร์ดและเมาส์ทุกครั้งที่กด$\r$\n$\r$\nคลิก ถัดไป เพื่อดำเนินการต่อ"
LangString INST_WELCOME_TEXT ${LANG_BULGARIAN}    "Тази програма ще инсталира ${APP_NAME} на вашия компютър.$\r$\n$\r$\nMonkeySounds добавя звуци на клавиатурата и мишката при всяко натискане.$\r$\n$\r$\nНатиснете Напред, за да продължите."
LangString INST_WELCOME_TEXT ${LANG_CROATIAN}     "Ovaj će program instalirati ${APP_NAME} na vaše računalo.$\r$\n$\r$\nMonkeySounds dodaje zvukove tipkovnice i miša svakom kliku.$\r$\n$\r$\nKliknite Dalje za nastavak."
LangString INST_WELCOME_TEXT ${LANG_DANISH}       "Dette program installerer ${APP_NAME} på din computer.$\r$\n$\r$\nMonkeySounds tilføjer tastatur- og muselyde til hvert klik.$\r$\n$\r$\nKlik på Næste for at fortsætte."
LangString INST_WELCOME_TEXT ${LANG_ESTONIAN}     "See programm paigaldab ${APP_NAME} teie arvutisse.$\r$\n$\r$\nMonkeySounds lisab igale klõpsule ja klahvivajutusele heli.$\r$\n$\r$\nJätkamiseks klõpsake Edasi."
LangString INST_WELCOME_TEXT ${LANG_FINNISH}      "Tämä ohjelma asentaa sovelluksen ${APP_NAME} tietokoneellesi.$\r$\n$\r$\nMonkeySounds lisää näppäimistö- ja hiiriäänet jokaiseen painallukseen.$\r$\n$\r$\nJatka napsauttamalla Seuraava."
LangString INST_WELCOME_TEXT ${LANG_GREEK}        "Αυτό το πρόγραμμα θα εγκαταστήσει το ${APP_NAME} στον υπολογιστή σας.$\r$\n$\r$\nΤο MonkeySounds προσθέτει ήχους πληκτρολογίου και ποντικιού σε κάθε κλικ.$\r$\n$\r$\nΚάντε κλικ στο Επόμενο για συνέχεια."
LangString INST_WELCOME_TEXT ${LANG_HEBREW}       "תוכנית זו תתקין את ${APP_NAME} במחשב שלך.$\r$\n$\r$\nMonkeySounds מוסיפה צלילי מקלדת ועכבר לכל לחיצה.$\r$\n$\r$\nלחץ על הבא כדי להמשיך."
LangString INST_WELCOME_TEXT ${LANG_LATVIAN}      "Šī programma instalēs ${APP_NAME} jūsu datorā.$\r$\n$\r$\nMonkeySounds pievieno tastatūras un peles skaņas katram klikšķim.$\r$\n$\r$\nNoklikšķiniet uz Tālāk, lai turpinātu."
LangString INST_WELCOME_TEXT ${LANG_LITHUANIAN}   "Ši programa įdiegs ${APP_NAME} jūsų kompiuteryje.$\r$\n$\r$\nMonkeySounds prideda klaviatūros ir pelės garsus kiekvienam paspaudimui.$\r$\n$\r$\nSpustelėkite Pirmyn, kad tęstumėte."
LangString INST_WELCOME_TEXT ${LANG_MALAY}        "Program ini akan memasang ${APP_NAME} pada komputer anda.$\r$\n$\r$\nMonkeySounds menambah bunyi papan kekunci dan tetikus pada setiap klik.$\r$\n$\r$\nKlik Seterusnya untuk meneruskan."
LangString INST_WELCOME_TEXT ${LANG_NORWEGIAN}    "Dette programmet vil installere ${APP_NAME} på datamaskinen din.$\r$\n$\r$\nMonkeySounds legger til tastatur- og muselyder til hvert klikk.$\r$\n$\r$\nKlikk Neste for å fortsette."
LangString INST_WELCOME_TEXT ${LANG_SERBIAN}      "Овај програм ће инсталирати ${APP_NAME} на ваш рачунар.$\r$\n$\r$\nMonkeySounds додаје звуке тастатуре и миша сваком клику.$\r$\n$\r$\nКликните Даље за наставак."
LangString INST_WELCOME_TEXT ${LANG_SLOVAK}       "Tento program nainštaluje ${APP_NAME} do vášho počítača.$\r$\n$\r$\nMonkeySounds pridáva zvuky klávesnice a myši ku každému stlačeniu.$\r$\n$\r$\nPokračujte kliknutím na Ďalej."
LangString INST_WELCOME_TEXT ${LANG_SLOVENIAN}    "Ta program bo namestil ${APP_NAME} v vaš računalnik.$\r$\n$\r$\nMonkeySounds doda zvoke tipkovnice in miške vsakemu kliku.$\r$\n$\r$\nKliknite Naprej za nadaljevanje."
LangString INST_WELCOME_TEXT ${LANG_SWEDISH}      "Detta program installerar ${APP_NAME} på din dator.$\r$\n$\r$\nMonkeySounds lägger till tangentbords- och musljud vid varje klick.$\r$\n$\r$\nKlicka på Nästa för att fortsätta."

; ---- Finish page ----
LangString INST_FINISH_LAUNCH ${LANG_ENGLISH}      "Launch ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_ARABIC}       "تشغيل ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_GERMAN}       "${APP_NAME} starten"
LangString INST_FINISH_LAUNCH ${LANG_SPANISH}      "Iniciar ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_FRENCH}       "Lancer ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_ITALIAN}      "Avvia ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_JAPANESE}     "${APP_NAME} を起動する"
LangString INST_FINISH_LAUNCH ${LANG_KOREAN}       "${APP_NAME} 실행"
LangString INST_FINISH_LAUNCH ${LANG_DUTCH}        "${APP_NAME} starten"
LangString INST_FINISH_LAUNCH ${LANG_POLISH}       "Uruchom ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_PORTUGUESEBR} "Iniciar ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_PORTUGUESE}   "Iniciar ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_RUSSIAN}      "Запустить ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_SIMPCHINESE}  "启动 ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_TRADCHINESE}  "啟動 ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_TURKISH}      "${APP_NAME} Başlat"
LangString INST_FINISH_LAUNCH ${LANG_UKRAINIAN}    "Запустити ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_CZECH}        "Spustit ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_HUNGARIAN}    "${APP_NAME} indítása"
LangString INST_FINISH_LAUNCH ${LANG_ROMANIAN}     "Lansați ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_VIETNAMESE}   "Khởi động ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_INDONESIAN}   "Jalankan ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_THAI}         "เปิด ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_SWEDISH}      "Starta ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_NORWEGIAN}    "Start ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_DANISH}       "Start ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_BULGARIAN}    "Стартирай ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_CROATIAN}     "Pokreni ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_ESTONIAN}     "Käivita ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_FINNISH}      "Käynnistä ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_GREEK}        "Εκκίνηση ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_HEBREW}       "הפעל את ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_LATVIAN}      "Palaist ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_LITHUANIAN}   "Paleisti ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_MALAY}        "Lancarkan ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_SERBIAN}      "Покрени ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_SLOVAK}       "Spustiť ${APP_NAME}"
LangString INST_FINISH_LAUNCH ${LANG_SLOVENIAN}    "Zaženi ${APP_NAME}"

LangString INST_FINISH_LINK ${LANG_ENGLISH}      "Visit GitHub"
LangString INST_FINISH_LINK ${LANG_ARABIC}       "زيارة GitHub"
LangString INST_FINISH_LINK ${LANG_GERMAN}       "GitHub besuchen"
LangString INST_FINISH_LINK ${LANG_SPANISH}      "Visitar GitHub"
LangString INST_FINISH_LINK ${LANG_FRENCH}       "Visiter GitHub"
LangString INST_FINISH_LINK ${LANG_ITALIAN}      "Visita GitHub"
LangString INST_FINISH_LINK ${LANG_JAPANESE}     "GitHubを訪問する"
LangString INST_FINISH_LINK ${LANG_KOREAN}       "GitHub 방문"
LangString INST_FINISH_LINK ${LANG_DUTCH}        "GitHub bezoeken"
LangString INST_FINISH_LINK ${LANG_POLISH}       "Odwiedź GitHub"
LangString INST_FINISH_LINK ${LANG_PORTUGUESEBR} "Visitar GitHub"
LangString INST_FINISH_LINK ${LANG_PORTUGUESE}   "Visitar GitHub"
LangString INST_FINISH_LINK ${LANG_RUSSIAN}      "Посетить GitHub"
LangString INST_FINISH_LINK ${LANG_SIMPCHINESE}  "访问 GitHub"
LangString INST_FINISH_LINK ${LANG_TRADCHINESE}  "造訪 GitHub"
LangString INST_FINISH_LINK ${LANG_TURKISH}      "GitHub'ı Ziyaret Et"
LangString INST_FINISH_LINK ${LANG_UKRAINIAN}    "Відвідати GitHub"
LangString INST_FINISH_LINK ${LANG_CZECH}        "Navštívit GitHub"
LangString INST_FINISH_LINK ${LANG_HUNGARIAN}    "GitHub megtekintése"
LangString INST_FINISH_LINK ${LANG_ROMANIAN}     "Vizitați GitHub"
LangString INST_FINISH_LINK ${LANG_VIETNAMESE}   "Truy cập GitHub"
LangString INST_FINISH_LINK ${LANG_INDONESIAN}   "Kunjungi GitHub"
LangString INST_FINISH_LINK ${LANG_THAI}         "เยี่ยมชม GitHub"
LangString INST_FINISH_LINK ${LANG_SWEDISH}      "Besök GitHub"
LangString INST_FINISH_LINK ${LANG_BULGARIAN}    "Посетете GitHub"
LangString INST_FINISH_LINK ${LANG_CROATIAN}     "Posjetite GitHub"
LangString INST_FINISH_LINK ${LANG_DANISH}       "Besøg GitHub"
LangString INST_FINISH_LINK ${LANG_ESTONIAN}     "Külasta GitHubi"
LangString INST_FINISH_LINK ${LANG_FINNISH}      "Käy GitHubissa"
LangString INST_FINISH_LINK ${LANG_GREEK}        "Επίσκεψη στο GitHub"
LangString INST_FINISH_LINK ${LANG_HEBREW}       "בקר ב-GitHub"
LangString INST_FINISH_LINK ${LANG_LATVIAN}      "Apmeklēt GitHub"
LangString INST_FINISH_LINK ${LANG_LITHUANIAN}   "Apsilankyti GitHub"
LangString INST_FINISH_LINK ${LANG_MALAY}        "Lawati GitHub"
LangString INST_FINISH_LINK ${LANG_NORWEGIAN}    "Besøk GitHub"
LangString INST_FINISH_LINK ${LANG_SERBIAN}      "Посетите GitHub"
LangString INST_FINISH_LINK ${LANG_SLOVAK}       "Navštíviť GitHub"
LangString INST_FINISH_LINK ${LANG_SLOVENIAN}    "Obiščite GitHub"

; ---- Install Mode page ----
LangString INST_MODE_HEADER ${LANG_ENGLISH}      "Choose Installation Mode"
LangString INST_MODE_HEADER ${LANG_ARABIC}       "اختر وضع التثبيت"
LangString INST_MODE_HEADER ${LANG_GERMAN}       "Installationsmodus wählen"
LangString INST_MODE_HEADER ${LANG_SPANISH}      "Elegir modo de instalación"
LangString INST_MODE_HEADER ${LANG_FRENCH}       "Choisir le mode d'installation"
LangString INST_MODE_HEADER ${LANG_ITALIAN}      "Scegli la modalità di installazione"
LangString INST_MODE_HEADER ${LANG_JAPANESE}     "インストールモードを選択"
LangString INST_MODE_HEADER ${LANG_KOREAN}       "설치 모드 선택"
LangString INST_MODE_HEADER ${LANG_DUTCH}        "Installatiemodus kiezen"
LangString INST_MODE_HEADER ${LANG_POLISH}       "Wybierz tryb instalacji"
LangString INST_MODE_HEADER ${LANG_PORTUGUESEBR} "Escolha o modo de instalação"
LangString INST_MODE_HEADER ${LANG_PORTUGUESE}   "Escolha o modo de instalação"
LangString INST_MODE_HEADER ${LANG_RUSSIAN}      "Выберите режим установки"
LangString INST_MODE_HEADER ${LANG_SIMPCHINESE}  "选择安装模式"
LangString INST_MODE_HEADER ${LANG_TRADCHINESE}  "選擇安裝模式"
LangString INST_MODE_HEADER ${LANG_TURKISH}      "Kurulum Modunu Seçin"
LangString INST_MODE_HEADER ${LANG_UKRAINIAN}    "Виберіть режим встановлення"
LangString INST_MODE_HEADER ${LANG_CZECH}        "Vyberte režim instalace"
LangString INST_MODE_HEADER ${LANG_HUNGARIAN}    "Válassza ki a telepítési módot"
LangString INST_MODE_HEADER ${LANG_ROMANIAN}     "Alegeți modul de instalare"
LangString INST_MODE_HEADER ${LANG_VIETNAMESE}   "Chọn chế độ cài đặt"
LangString INST_MODE_HEADER ${LANG_INDONESIAN}   "Pilih Mode Instalasi"
LangString INST_MODE_HEADER ${LANG_THAI}         "เลือกโหมดการติดตั้ง"
LangString INST_MODE_HEADER ${LANG_SWEDISH}      "Välj installationsläge"
LangString INST_MODE_HEADER ${LANG_NORWEGIAN}    "Velg installasjonsmodus"
LangString INST_MODE_HEADER ${LANG_DANISH}       "Vælg installationstilstand"
LangString INST_MODE_HEADER ${LANG_BULGARIAN}    "Изберете режим на инсталиране"
LangString INST_MODE_HEADER ${LANG_CROATIAN}     "Odaberite način instalacije"
LangString INST_MODE_HEADER ${LANG_ESTONIAN}     "Valige paigaldusrežiim"
LangString INST_MODE_HEADER ${LANG_FINNISH}      "Valitse asennustila"
LangString INST_MODE_HEADER ${LANG_GREEK}        "Επιλογή λειτουργίας εγκατάστασης"
LangString INST_MODE_HEADER ${LANG_HEBREW}       "בחר מצב התקנה"
LangString INST_MODE_HEADER ${LANG_LATVIAN}      "Izvēlieties instalēšanas režīmu"
LangString INST_MODE_HEADER ${LANG_LITHUANIAN}   "Pasirinkite diegimo režimą"
LangString INST_MODE_HEADER ${LANG_MALAY}        "Pilih Mod Pemasangan"
LangString INST_MODE_HEADER ${LANG_SERBIAN}      "Изаберите режим инсталације"
LangString INST_MODE_HEADER ${LANG_SLOVAK}       "Vyberte režim inštalácie"
LangString INST_MODE_HEADER ${LANG_SLOVENIAN}    "Izberite način namestitve"

LangString INST_MODE_LABEL ${LANG_ENGLISH}      "Choose how to install ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_ARABIC}       "اختر طريقة تثبيت ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_GERMAN}       "Wählen Sie, wie ${APP_NAME} installiert werden soll:"
LangString INST_MODE_LABEL ${LANG_SPANISH}      "Elija cómo instalar ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_FRENCH}       "Choisissez comment installer ${APP_NAME} :"
LangString INST_MODE_LABEL ${LANG_ITALIAN}      "Scegli come installare ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_JAPANESE}     "${APP_NAME} のインストール方法を選択してください："
LangString INST_MODE_LABEL ${LANG_KOREAN}       "${APP_NAME} 설치 방법을 선택하세요:"
LangString INST_MODE_LABEL ${LANG_DUTCH}        "Kies hoe ${APP_NAME} geïnstalleerd moet worden:"
LangString INST_MODE_LABEL ${LANG_POLISH}       "Wybierz sposób instalacji ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_PORTUGUESEBR} "Escolha como instalar o ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_PORTUGUESE}   "Escolha como instalar o ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_RUSSIAN}      "Выберите способ установки ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_SIMPCHINESE}  "选择安装 ${APP_NAME} 的方式："
LangString INST_MODE_LABEL ${LANG_TRADCHINESE}  "選擇安裝 ${APP_NAME} 的方式："
LangString INST_MODE_LABEL ${LANG_TURKISH}      "${APP_NAME} nasıl kurulsun:"
LangString INST_MODE_LABEL ${LANG_UKRAINIAN}    "Виберіть спосіб встановлення ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_CZECH}        "Vyberte, jak nainstalovat ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_HUNGARIAN}    "Válassza ki a ${APP_NAME} telepítési módját:"
LangString INST_MODE_LABEL ${LANG_ROMANIAN}     "Alegeți cum să instalați ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_VIETNAMESE}   "Chọn cách cài đặt ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_INDONESIAN}   "Pilih cara menginstal ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_THAI}         "เลือกวิธีติดตั้ง ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_SWEDISH}      "Välj hur ${APP_NAME} ska installeras:"
LangString INST_MODE_LABEL ${LANG_NORWEGIAN}    "Velg hvordan ${APP_NAME} skal installeres:"
LangString INST_MODE_LABEL ${LANG_DANISH}       "Vælg hvordan ${APP_NAME} skal installeres:"
LangString INST_MODE_LABEL ${LANG_BULGARIAN}    "Изберете как да инсталирате ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_CROATIAN}     "Odaberite kako instalirati ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_ESTONIAN}     "Valige, kuidas ${APP_NAME} paigaldada:"
LangString INST_MODE_LABEL ${LANG_FINNISH}      "Valitse, miten ${APP_NAME} asennetaan:"
LangString INST_MODE_LABEL ${LANG_GREEK}        "Επιλέξτε πώς να εγκατασταθεί το ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_HEBREW}       "בחר כיצד להתקין את ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_LATVIAN}      "Izvēlieties, kā instalēt ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_LITHUANIAN}   "Pasirinkite, kaip įdiegti ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_MALAY}        "Pilih cara memasang ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_SERBIAN}      "Изаберите начин инсталације ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_SLOVAK}       "Vyberte, ako nainštalovať ${APP_NAME}:"
LangString INST_MODE_LABEL ${LANG_SLOVENIAN}    "Izberite način namestitve ${APP_NAME}:"

LangString INST_MODE_STD ${LANG_ENGLISH}      "Standard (recommended) — install with uninstaller, registry entries and shortcuts"
LangString INST_MODE_STD ${LANG_ARABIC}       "قياسي (موصى به) — تثبيت مع برنامج إلغاء التثبيت وإدخالات التسجيل والاختصارات"
LangString INST_MODE_STD ${LANG_GERMAN}       "Standard (empfohlen) — Installation mit Deinstallationsprogramm, Registrierungseinträgen und Verknüpfungen"
LangString INST_MODE_STD ${LANG_SPANISH}      "Estándar (recomendado) — instalar con desinstalador, entradas de registro y accesos directos"
LangString INST_MODE_STD ${LANG_FRENCH}       "Standard (recommandé) — installer avec désinstallateur, entrées de registre et raccourcis"
LangString INST_MODE_STD ${LANG_ITALIAN}      "Standard (consigliato) — installa con disinstallatore, voci di registro e scorciatoie"
LangString INST_MODE_STD ${LANG_JAPANESE}     "標準（推奨）— アンインストーラー、レジストリエントリ、ショートカット付きでインストール"
LangString INST_MODE_STD ${LANG_KOREAN}       "표준 (권장) — 제거 프로그램, 레지스트리 항목 및 바로가기 포함 설치"
LangString INST_MODE_STD ${LANG_DUTCH}        "Standaard (aanbevolen) — installeren met verwijderprogramma, registeritems en snelkoppelingen"
LangString INST_MODE_STD ${LANG_POLISH}       "Standardowy (zalecany) — zainstaluj z dezinstalатором, wpisami rejestru i skrótami"
LangString INST_MODE_STD ${LANG_PORTUGUESEBR} "Padrão (recomendado) — instalar com desinstalador, entradas de registro e atalhos"
LangString INST_MODE_STD ${LANG_PORTUGUESE}   "Padrão (recomendado) — instalar com desinstalador, entradas de registo e atalhos"
LangString INST_MODE_STD ${LANG_RUSSIAN}      "Стандартный (рекомендуется) — установить с деинсталлятором, записями реестра и ярлыками"
LangString INST_MODE_STD ${LANG_SIMPCHINESE}  "标准（推荐）— 安装卸载程序、注册表项和快捷方式"
LangString INST_MODE_STD ${LANG_TRADCHINESE}  "標準（建議）— 安裝解除安裝程式、登錄機碼和捷徑"
LangString INST_MODE_STD ${LANG_TURKISH}      "Standart (önerilen) — kaldırıcı, kayıt defteri girişleri ve kısayollarla kur"
LangString INST_MODE_STD ${LANG_UKRAINIAN}    "Стандартний (рекомендовано) — встановити з деінсталятором, записами реєстру та ярликами"
LangString INST_MODE_STD ${LANG_CZECH}        "Standardní (doporučeno) — instalovat s odinstalátorem, záznamy registru a zástupci"
LangString INST_MODE_STD ${LANG_HUNGARIAN}    "Normál (ajánlott) — telepítés eltávolítóval, beállításjegyzék-bejegyzésekkel és parancsikonokkal"
LangString INST_MODE_STD ${LANG_ROMANIAN}     "Standard (recomandat) — instalare cu dezinstalator, intrări în registru și comenzi rapide"
LangString INST_MODE_STD ${LANG_VIETNAMESE}   "Tiêu chuẩn (khuyến nghị) — cài đặt với trình gỡ cài đặt, mục đăng ký và lối tắt"
LangString INST_MODE_STD ${LANG_INDONESIAN}   "Standar (disarankan) — instal dengan penghapus instalasi, entri registri, dan pintasan"
LangString INST_MODE_STD ${LANG_THAI}         "มาตรฐาน (แนะนำ) — ติดตั้งพร้อมตัวถอนการติดตั้ง รายการรีจิสทรี และทางลัด"
LangString INST_MODE_STD ${LANG_SWEDISH}      "Standard (rekommenderas) — installera med avinstallationsprogram, registerposter och genvägar"
LangString INST_MODE_STD ${LANG_NORWEGIAN}    "Standard (anbefalt) — installer med avinstalleringsprogram, registeroppføringer og snarveier"
LangString INST_MODE_STD ${LANG_DANISH}       "Standard (anbefalet) — installer med afinstallationsprogram, registerposter og genveje"
LangString INST_MODE_STD ${LANG_BULGARIAN}    "Стандартен (препоръчително) — инсталиране с деинсталатор, записи в регистъра и преки пътища"
LangString INST_MODE_STD ${LANG_CROATIAN}     "Standardno (preporučeno) — instalacija s programom za deinstalaciju, zapisima registra i prečacima"
LangString INST_MODE_STD ${LANG_ESTONIAN}     "Standardne (soovitatav) — paigalda desinstallija, registrikirjete ja otseteedega"
LangString INST_MODE_STD ${LANG_FINNISH}      "Vakio (suositeltu) — asenna poisto-ohjelman, rekisterimerkintöjen ja pikakuvakkeiden kanssa"
LangString INST_MODE_STD ${LANG_GREEK}        "Τυπική (συνιστάται) — εγκατάσταση με πρόγραμμα απεγκατάστασης, καταχωρίσεις μητρώου και συντομεύσεις"
LangString INST_MODE_STD ${LANG_HEBREW}       "רגיל (מומלץ) — התקן עם מסיר התקנה, רשומות רישום וקיצורי דרך"
LangString INST_MODE_STD ${LANG_LATVIAN}      "Standarta (ieteicams) — instalēt ar atinstalētāju, reģistra ierakstiem un saīsnēm"
LangString INST_MODE_STD ${LANG_LITHUANIAN}   "Standartinis (rekomenduojama) — įdiegti su šalinimo programa, registro įrašais ir nuorodomis"
LangString INST_MODE_STD ${LANG_MALAY}        "Standard (disyorkan) — pasang dengan penyahpasang, entri registri dan pintasan"
LangString INST_MODE_STD ${LANG_SERBIAN}      "Стандардно (препоручено) — инсталација са деинсталатором, уносима у регистар и пречицама"
LangString INST_MODE_STD ${LANG_SLOVAK}       "Štandardný (odporúčané) — inštalovať s odinštalátorom, záznamami registra a odkazmi"
LangString INST_MODE_STD ${LANG_SLOVENIAN}    "Standardno (priporočeno) — namesti z odstranjevalcem, vnosi v register in bližnjicami"

LangString INST_MODE_STD_DESC ${LANG_ENGLISH}      "Installs to Program Files. Sounds saved in %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_ARABIC}       "يثبت في ملفات البرنامج. يتم حفظ الأصوات في %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_GERMAN}       "Wird in Programme installiert. Sounds gespeichert in %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_SPANISH}      "Se instala en Archivos de programa. Sonidos guardados en %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_FRENCH}       "Installe dans Fichiers programme. Sons enregistrés dans %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_ITALIAN}      "Installa in Programmi. Suoni salvati in %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_JAPANESE}     "Program Files にインストール。サウンドは %APPDATA%\MonkeySounds\Sounds に保存されます。"
LangString INST_MODE_STD_DESC ${LANG_KOREAN}       "Program Files에 설치됩니다. 소리는 %APPDATA%\MonkeySounds\Sounds에 저장됩니다."
LangString INST_MODE_STD_DESC ${LANG_DUTCH}        "Installeert in Program Files. Geluiden opgeslagen in %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_POLISH}       "Instaluje w Programy. Dźwięki zapisane w %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_PORTUGUESEBR} "Instala em Arquivos de Programas. Sons salvos em %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_PORTUGUESE}   "Instala em Ficheiros de Programa. Sons guardados em %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_RUSSIAN}      "Устанавливается в Program Files. Звуки сохраняются в %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_SIMPCHINESE}  "安装到 Program Files。声音保存在 %APPDATA%\MonkeySounds\Sounds。"
LangString INST_MODE_STD_DESC ${LANG_TRADCHINESE}  "安裝到 Program Files。聲音儲存在 %APPDATA%\MonkeySounds\Sounds。"
LangString INST_MODE_STD_DESC ${LANG_TURKISH}      "Program Files'a yükler. Sesler %APPDATA%\MonkeySounds\Sounds içine kaydedilir."
LangString INST_MODE_STD_DESC ${LANG_UKRAINIAN}    "Встановлюється у Program Files. Звуки зберігаються в %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_CZECH}        "Instaluje do Program Files. Zvuky uloženy v %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_HUNGARIAN}    "A Program Files mappába települ. Hangok mentve itt: %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_ROMANIAN}     "Instalează în Program Files. Sunetele sunt salvate în %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_VIETNAMESE}   "Cài đặt vào Program Files. Âm thanh được lưu trong %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_INDONESIAN}   "Menginstal ke Program Files. Suara disimpan di %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_THAI}         "ติดตั้งใน Program Files เสียงถูกบันทึกใน %APPDATA%\MonkeySounds\Sounds"
LangString INST_MODE_STD_DESC ${LANG_SWEDISH}      "Installerar i Program Files. Ljud sparas i %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_NORWEGIAN}    "Installerer i Program Files. Lyder lagres i %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_DANISH}       "Installerer i Program Files. Lyde gemmes i %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_BULGARIAN}    "Инсталира в Program Files. Звуците се записват в %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_CROATIAN}     "Instalira u Program Files. Zvukovi se spremaju u %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_ESTONIAN}     "Paigaldab kausta Program Files. Helid salvestatakse kausta %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_FINNISH}      "Asentaa Program Files -kansioon. Äänet tallennetaan kansioon %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_GREEK}        "Εγκαθίσταται στο Program Files. Οι ήχοι αποθηκεύονται στο %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_HEBREW}       "מתקין ב-Program Files. הצלילים נשמרים ב-%APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_LATVIAN}      "Instalē mapē Program Files. Skaņas tiek saglabātas %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_LITHUANIAN}   "Įdiegia į Program Files. Garsai įrašomi į %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_MALAY}        "Memasang ke Program Files. Bunyi disimpan dalam %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_SERBIAN}      "Инсталира у Program Files. Звукови се чувају у %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_SLOVAK}       "Inštaluje do Program Files. Zvuky sú uložené v %APPDATA%\MonkeySounds\Sounds."
LangString INST_MODE_STD_DESC ${LANG_SLOVENIAN}    "Namesti v Program Files. Zvoki so shranjeni v %APPDATA%\MonkeySounds\Sounds."

LangString INST_MODE_PORT ${LANG_ENGLISH}      "Portable — no registry entries, no uninstaller, runs from any folder"
LangString INST_MODE_PORT ${LANG_ARABIC}       "محمول — بدون إدخالات تسجيل، بدون برنامج إلغاء تثبيت، يعمل من أي مجلد"
LangString INST_MODE_PORT ${LANG_GERMAN}       "Portabel — keine Registrierungseinträge, kein Deinstallationsprogramm, läuft aus jedem Ordner"
LangString INST_MODE_PORT ${LANG_SPANISH}      "Portátil — sin entradas de registro, sin desinstalador, funciona desde cualquier carpeta"
LangString INST_MODE_PORT ${LANG_FRENCH}       "Portable — aucune entrée de registre, pas de désinstallateur, fonctionne depuis n'importe quel dossier"
LangString INST_MODE_PORT ${LANG_ITALIAN}      "Portabile — nessuna voce di registro, nessun disinstallatore, funziona da qualsiasi cartella"
LangString INST_MODE_PORT ${LANG_JAPANESE}     "ポータブル — レジストリエントリなし、アンインストーラーなし、任意のフォルダーから実行可能"
LangString INST_MODE_PORT ${LANG_KOREAN}       "포터블 — 레지스트리 항목 없음, 제거 프로그램 없음, 임의 폴더에서 실행 가능"
LangString INST_MODE_PORT ${LANG_DUTCH}        "Portable — geen registeritems, geen verwijderprogramma, werkt vanuit elke map"
LangString INST_MODE_PORT ${LANG_POLISH}       "Przenośny — bez wpisów rejestru, bez dezinstalатора, działa z dowolnego folderu"
LangString INST_MODE_PORT ${LANG_PORTUGUESEBR} "Portátil — sem entradas de registro, sem desinstalador, funciona de qualquer pasta"
LangString INST_MODE_PORT ${LANG_PORTUGUESE}   "Portátil — sem entradas de registo, sem desinstalador, funciona de qualquer pasta"
LangString INST_MODE_PORT ${LANG_RUSSIAN}      "Портативный — без записей реестра, без деинсталлятора, запускается из любой папки"
LangString INST_MODE_PORT ${LANG_SIMPCHINESE}  "便携版 — 无注册表项，无卸载程序，可从任意文件夹运行"
LangString INST_MODE_PORT ${LANG_TRADCHINESE}  "便攜版 — 無登錄機碼，無解除安裝程式，可從任意資料夾執行"
LangString INST_MODE_PORT ${LANG_TURKISH}      "Taşınabilir — kayıt defteri girişi yok, kaldırıcı yok, herhangi bir klasörden çalışır"
LangString INST_MODE_PORT ${LANG_UKRAINIAN}    "Портативний — без записів реєстру, без деінсталятора, запускається з будь-якої папки"
LangString INST_MODE_PORT ${LANG_CZECH}        "Přenosný — žádné záznamy registru, žádný odinstalátor, spouští se z libovolné složky"
LangString INST_MODE_PORT ${LANG_HUNGARIAN}    "Hordozható — nincs beállításjegyzék-bejegyzés, nincs eltávolító, bármely mappából futtatható"
LangString INST_MODE_PORT ${LANG_ROMANIAN}     "Portabil — fără intrări în registru, fără dezinstalator, rulează din orice folder"
LangString INST_MODE_PORT ${LANG_VIETNAMESE}   "Di động — không có mục đăng ký, không có trình gỡ cài đặt, chạy từ bất kỳ thư mục nào"
LangString INST_MODE_PORT ${LANG_INDONESIAN}   "Portabel — tidak ada entri registri, tidak ada penghapus instalasi, dapat dijalankan dari folder mana saja"
LangString INST_MODE_PORT ${LANG_THAI}         "พกพาได้ — ไม่มีรายการรีจิสทรี ไม่มีตัวถอนการติดตั้ง ทำงานได้จากทุกโฟลเดอร์"
LangString INST_MODE_PORT ${LANG_SWEDISH}      "Portabel — inga registerposter, inget avinstallationsprogram, körs från valfri mapp"
LangString INST_MODE_PORT ${LANG_NORWEGIAN}    "Portabel — ingen registeroppføringer, ingen avinstalleringsprogram, kjører fra en hvilken som helst mappe"
LangString INST_MODE_PORT ${LANG_DANISH}       "Portabel — ingen registerposter, inget afinstallationsprogram, kører fra en vilkårlig mappe"
LangString INST_MODE_PORT ${LANG_BULGARIAN}    "Преносим — без записи в регистъра, без деинсталатор, работи от всяка папка"
LangString INST_MODE_PORT ${LANG_CROATIAN}     "Prijenosno — bez zapisa u registru, bez programa za deinstalaciju, radi iz bilo koje mape"
LangString INST_MODE_PORT ${LANG_ESTONIAN}     "Kaasaskantav — registrikirjeteta, desinstallijata, töötab igast kaustast"
LangString INST_MODE_PORT ${LANG_FINNISH}      "Kannettava — ei rekisterimerkintöjä, ei poisto-ohjelmaa, toimii mistä tahansa kansiosta"
LangString INST_MODE_PORT ${LANG_GREEK}        "Φορητή — χωρίς καταχωρίσεις μητρώου, χωρίς απεγκατάσταση, εκτελείται από οποιονδήποτε φάκελο"
LangString INST_MODE_PORT ${LANG_HEBREW}       "נייד — ללא רשומות רישום, ללא מסיר התקנה, פועל מכל תיקייה"
LangString INST_MODE_PORT ${LANG_LATVIAN}      "Pārnēsājams — bez reģistra ierakstiem, bez atinstalētāja, darbojas no jebkuras mapes"
LangString INST_MODE_PORT ${LANG_LITHUANIAN}   "Nešiojamas — be registro įrašų, be šalinimo programos, veikia iš bet kurio aplanko"
LangString INST_MODE_PORT ${LANG_MALAY}        "Mudah alih — tiada entri registri, tiada penyahpasang, berjalan dari mana-mana folder"
LangString INST_MODE_PORT ${LANG_SERBIAN}      "Преносиво — без уноса у регистар, без деинсталатора, ради из било које фасцикле"
LangString INST_MODE_PORT ${LANG_SLOVAK}       "Prenosný — žiadne záznamy registra, žiadny odinštalátor, spúšťa sa z ľubovoľného priečinka"
LangString INST_MODE_PORT ${LANG_SLOVENIAN}    "Prenosno — brez vnosov v register, brez odstranjevalca, deluje iz katere koli mape"

LangString INST_MODE_PORT_DESC ${LANG_ENGLISH}      "All files (EXE + Sounds) placed together. Move the folder freely."
LangString INST_MODE_PORT_DESC ${LANG_ARABIC}       "جميع الملفات (EXE + الأصوات) موضوعة معاً. انقل المجلد بحرية."
LangString INST_MODE_PORT_DESC ${LANG_GERMAN}       "Alle Dateien (EXE + Sounds) werden zusammen abgelegt. Verschieben Sie den Ordner frei."
LangString INST_MODE_PORT_DESC ${LANG_SPANISH}      "Todos los archivos (EXE + sonidos) colocados juntos. Mueva la carpeta libremente."
LangString INST_MODE_PORT_DESC ${LANG_FRENCH}       "Tous les fichiers (EXE + Sons) placés ensemble. Déplacez le dossier librement."
LangString INST_MODE_PORT_DESC ${LANG_ITALIAN}      "Tutti i file (EXE + Suoni) posizionati insieme. Sposta la cartella liberamente."
LangString INST_MODE_PORT_DESC ${LANG_JAPANESE}     "すべてのファイル（EXE + サウンド）を一緒に配置。フォルダーを自由に移動できます。"
LangString INST_MODE_PORT_DESC ${LANG_KOREAN}       "모든 파일 (EXE + 소리)이 함께 배치됩니다. 폴더를 자유롭게 이동하세요."
LangString INST_MODE_PORT_DESC ${LANG_DUTCH}        "Alle bestanden (EXE + Geluiden) samen geplaatst. Verplaats de map vrij."
LangString INST_MODE_PORT_DESC ${LANG_POLISH}       "Wszystkie pliki (EXE + Dźwięki) umieszczone razem. Przenieś folder dowolnie."
LangString INST_MODE_PORT_DESC ${LANG_PORTUGUESEBR} "Todos os arquivos (EXE + Sons) colocados juntos. Mova a pasta livremente."
LangString INST_MODE_PORT_DESC ${LANG_PORTUGUESE}   "Todos os ficheiros (EXE + Sons) colocados juntos. Mova a pasta livremente."
LangString INST_MODE_PORT_DESC ${LANG_RUSSIAN}      "Все файлы (EXE + Звуки) размещены вместе. Перемещайте папку свободно."
LangString INST_MODE_PORT_DESC ${LANG_SIMPCHINESE}  "所有文件（EXE + 声音）放在一起。可自由移动文件夹。"
LangString INST_MODE_PORT_DESC ${LANG_TRADCHINESE}  "所有檔案（EXE + 聲音）放在一起。可自由移動資料夾。"
LangString INST_MODE_PORT_DESC ${LANG_TURKISH}      "Tüm dosyalar (EXE + Sesler) bir arada. Klasörü serbestçe taşıyın."
LangString INST_MODE_PORT_DESC ${LANG_UKRAINIAN}    "Всі файли (EXE + Звуки) розміщені разом. Переміщайте папку вільно."
LangString INST_MODE_PORT_DESC ${LANG_CZECH}        "Všechny soubory (EXE + Zvuky) umístěny společně. Složku přesuňte kamkoliv."
LangString INST_MODE_PORT_DESC ${LANG_HUNGARIAN}    "Minden fájl (EXE + Hangok) együtt elhelyezve. A mappát szabadon áthelyezheti."
LangString INST_MODE_PORT_DESC ${LANG_ROMANIAN}     "Toate fișierele (EXE + Sunete) plasate împreună. Mutați folderul liber."
LangString INST_MODE_PORT_DESC ${LANG_VIETNAMESE}   "Tất cả tệp (EXE + Âm thanh) đặt cùng nhau. Di chuyển thư mục tự do."
LangString INST_MODE_PORT_DESC ${LANG_INDONESIAN}   "Semua file (EXE + Suara) ditempatkan bersama. Pindahkan folder dengan bebas."
LangString INST_MODE_PORT_DESC ${LANG_THAI}         "ไฟล์ทั้งหมด (EXE + เสียง) อยู่รวมกัน ย้ายโฟลเดอร์ได้อย่างอิสระ"
LangString INST_MODE_PORT_DESC ${LANG_SWEDISH}      "Alla filer (EXE + Ljud) placeras tillsammans. Flytta mappen fritt."
LangString INST_MODE_PORT_DESC ${LANG_NORWEGIAN}    "Alle filer (EXE + Lyder) plassert sammen. Flytt mappen fritt."
LangString INST_MODE_PORT_DESC ${LANG_DANISH}       "Alle filer (EXE + Lyde) placeret sammen. Flyt mappen frit."
LangString INST_MODE_PORT_DESC ${LANG_BULGARIAN}    "Всички файлове (EXE + Звуци) са поставени заедно. Местете папката свободно."
LangString INST_MODE_PORT_DESC ${LANG_CROATIAN}     "Sve datoteke (EXE + Zvukovi) smještene su zajedno. Slobodno premještajte mapu."
LangString INST_MODE_PORT_DESC ${LANG_ESTONIAN}     "Kõik failid (EXE + Helid) on koos. Liigutage kausta vabalt."
LangString INST_MODE_PORT_DESC ${LANG_FINNISH}      "Kaikki tiedostot (EXE + Äänet) sijoitetaan yhteen. Siirrä kansiota vapaasti."
LangString INST_MODE_PORT_DESC ${LANG_GREEK}        "Όλα τα αρχεία (EXE + Ήχοι) τοποθετούνται μαζί. Μετακινήστε τον φάκελο ελεύθερα."
LangString INST_MODE_PORT_DESC ${LANG_HEBREW}       "כל הקבצים (EXE + צלילים) ממוקמים יחד. העבר את התיקייה בחופשיות."
LangString INST_MODE_PORT_DESC ${LANG_LATVIAN}      "Visi faili (EXE + Skaņas) novietoti kopā. Pārvietojiet mapi brīvi."
LangString INST_MODE_PORT_DESC ${LANG_LITHUANIAN}   "Visi failai (EXE + Garsai) sudėti kartu. Laisvai perkelkite aplanką."
LangString INST_MODE_PORT_DESC ${LANG_MALAY}        "Semua fail (EXE + Bunyi) diletakkan bersama. Alihkan folder dengan bebas."
LangString INST_MODE_PORT_DESC ${LANG_SERBIAN}      "Све датотеке (EXE + Звукови) су заједно. Слободно премештајте фасциклу."
LangString INST_MODE_PORT_DESC ${LANG_SLOVAK}       "Všetky súbory (EXE + Zvuky) sú umiestnené spolu. Priečinok môžete voľne presúvať."
LangString INST_MODE_PORT_DESC ${LANG_SLOVENIAN}    "Vse datoteke (EXE + Zvoki) so skupaj. Mapo lahko prosto premikate."

; ---- Portable folder protection warning ----
LangString INST_PORT_SYSDIR_WARN ${LANG_ENGLISH}      "Portable mode cannot install into system-protected folders (Program Files / Windows).$\r$\nGo back and choose Standard mode, or pick a different folder."
LangString INST_PORT_SYSDIR_WARN ${LANG_ARABIC}       "لا يمكن للوضع المحمول التثبيت في المجلدات المحمية من النظام (ملفات البرنامج / Windows).$\r$\nارجع واختر الوضع القياسي أو اختر مجلداً مختلفاً."
LangString INST_PORT_SYSDIR_WARN ${LANG_GERMAN}       "Der portable Modus kann nicht in systemgeschützte Ordner installieren (Programme / Windows).$\r$\nGehen Sie zurück und wählen Sie den Standardmodus oder einen anderen Ordner."
LangString INST_PORT_SYSDIR_WARN ${LANG_SPANISH}      "El modo portátil no puede instalarse en carpetas protegidas del sistema (Archivos de programa / Windows).$\r$\nVuelva y elija el modo estándar o seleccione una carpeta diferente."
LangString INST_PORT_SYSDIR_WARN ${LANG_FRENCH}       "Le mode portable ne peut pas s'installer dans des dossiers protégés par le système (Fichiers programme / Windows).$\r$\nRevenez en arrière et choisissez le mode Standard ou un autre dossier."
LangString INST_PORT_SYSDIR_WARN ${LANG_ITALIAN}      "La modalità portabile non può essere installata in cartelle protette dal sistema (File di programma / Windows).$\r$\nTornare indietro e scegliere la modalità Standard o una cartella diversa."
LangString INST_PORT_SYSDIR_WARN ${LANG_JAPANESE}     "ポータブルモードはシステム保護フォルダー（Program Files / Windows）にインストールできません。$\r$\n戻って標準モードを選択するか、別のフォルダーを選んでください。"
LangString INST_PORT_SYSDIR_WARN ${LANG_KOREAN}       "포터블 모드는 시스템 보호 폴더 (Program Files / Windows)에 설치할 수 없습니다.$\r$\n뒤로 가서 표준 모드를 선택하거나 다른 폴더를 선택하세요."
LangString INST_PORT_SYSDIR_WARN ${LANG_DUTCH}        "De portable modus kan niet worden geïnstalleerd in systeembeveiligde mappen (Program Files / Windows).$\r$\nGa terug en kies de standaardmodus of kies een andere map."
LangString INST_PORT_SYSDIR_WARN ${LANG_POLISH}       "Tryb przenośny nie może być zainstalowany w folderach chronionych przez system (Program Files / Windows).$\r$\nWróć i wybierz tryb standardowy lub inny folder."
LangString INST_PORT_SYSDIR_WARN ${LANG_PORTUGUESEBR} "O modo portátil não pode ser instalado em pastas protegidas do sistema (Program Files / Windows).$\r$\nVolte e escolha o modo Padrão ou uma pasta diferente."
LangString INST_PORT_SYSDIR_WARN ${LANG_PORTUGUESE}   "O modo portátil não pode ser instalado em pastas protegidas pelo sistema (Program Files / Windows).$\r$\nVolte e escolha o modo Padrão ou uma pasta diferente."
LangString INST_PORT_SYSDIR_WARN ${LANG_RUSSIAN}      "Портативный режим не может быть установлен в папки, защищённые системой (Program Files / Windows).$\r$\nВернитесь и выберите стандартный режим или другую папку."
LangString INST_PORT_SYSDIR_WARN ${LANG_SIMPCHINESE}  "便携模式不能安装到受系统保护的文件夹（Program Files / Windows）。$\r$\n请返回选择标准模式或其他文件夹。"
LangString INST_PORT_SYSDIR_WARN ${LANG_TRADCHINESE}  "便攜模式無法安裝至受系統保護的資料夾（Program Files / Windows）。$\r$\n請返回選擇標準模式或其他資料夾。"
LangString INST_PORT_SYSDIR_WARN ${LANG_TURKISH}      "Taşınabilir mod, sistem korumalı klasörlere (Program Files / Windows) yüklenemez.$\r$\nGeri gidin ve Standart modu veya farklı bir klasör seçin."
LangString INST_PORT_SYSDIR_WARN ${LANG_UKRAINIAN}    "Портативний режим не може бути встановлений в системно-захищені папки (Program Files / Windows).$\r$\nПоверніться і виберіть стандартний режим або іншу папку."
LangString INST_PORT_SYSDIR_WARN ${LANG_CZECH}        "Přenosný režim nelze nainstalovat do systémem chráněných složek (Program Files / Windows).$\r$\nJděte zpět a zvolte standardní režim nebo jiná složku."
LangString INST_PORT_SYSDIR_WARN ${LANG_HUNGARIAN}    "A hordozható mód nem telepíthető rendszer által védett mappákba (Program Files / Windows).$\r$\nMenjen vissza, és válassza a Normál módot, vagy válasszon másik mappát."
LangString INST_PORT_SYSDIR_WARN ${LANG_ROMANIAN}     "Modul portabil nu se poate instala în foldere protejate de sistem (Program Files / Windows).$\r$\nMergeți înapoi și alegeți modul Standard sau alt folder."
LangString INST_PORT_SYSDIR_WARN ${LANG_VIETNAMESE}   "Chế độ di động không thể cài đặt vào các thư mục được hệ thống bảo vệ (Program Files / Windows).$\r$\nQuay lại và chọn chế độ Tiêu chuẩn hoặc chọn thư mục khác."
LangString INST_PORT_SYSDIR_WARN ${LANG_INDONESIAN}   "Mode portabel tidak dapat diinstal ke folder yang dilindungi sistem (Program Files / Windows).$\r$\nKembali dan pilih mode Standar, atau pilih folder lain."
LangString INST_PORT_SYSDIR_WARN ${LANG_THAI}         "โหมดพกพาไม่สามารถติดตั้งลงในโฟลเดอร์ที่ระบบป้องกัน (Program Files / Windows)$\r$\nกลับไปเลือกโหมดมาตรฐาน หรือเลือกโฟลเดอร์อื่น"
LangString INST_PORT_SYSDIR_WARN ${LANG_BULGARIAN}    "Преносимият режим не може да се инсталира в системни папки (Program Files / Windows).$\r$\nВърнете се и изберете стандартен режим или друга папка."
LangString INST_PORT_SYSDIR_WARN ${LANG_CROATIAN}     "Prijenosni način ne može se instalirati u sistemske mape (Program Files / Windows).$\r$\nVratite se i odaberite standardni način ili drugu mapu."
LangString INST_PORT_SYSDIR_WARN ${LANG_DANISH}       "Portabel tilstand kan ikke installeres i systembeskyttede mapper (Program Files / Windows).$\r$\nGå tilbage og vælg Standardtilstand, eller vælg en anden mappe."
LangString INST_PORT_SYSDIR_WARN ${LANG_ESTONIAN}     "Kaasaskantavat režiimi ei saa paigaldada süsteemikaustadesse (Program Files / Windows).$\r$\nMinge tagasi ja valige standardrežiim või muu kaust."
LangString INST_PORT_SYSDIR_WARN ${LANG_FINNISH}      "Kannettavaa tilaa ei voi asentaa järjestelmän suojaamiin kansioihin (Program Files / Windows).$\r$\nPalaa takaisin ja valitse vakiotila tai toinen kansio."
LangString INST_PORT_SYSDIR_WARN ${LANG_GREEK}        "Η φορητή λειτουργία δεν μπορεί να εγκατασταθεί σε φακέλους συστήματος (Program Files / Windows).$\r$\nΕπιστρέψτε και επιλέξτε τυπική λειτουργία ή άλλον φάκελο."
LangString INST_PORT_SYSDIR_WARN ${LANG_HEBREW}       "מצב נייד אינו יכול להתקין בתיקיות מוגנות מערכת (Program Files / Windows).$\r$\nחזור ובחר במצב רגיל או בחר תיקייה אחרת."
LangString INST_PORT_SYSDIR_WARN ${LANG_LATVIAN}      "Pārnēsājamo režīmu nevar instalēt sistēmas aizsargātās mapēs (Program Files / Windows).$\r$\nDodieties atpakaļ un izvēlieties standarta režīmu vai citu mapi."
LangString INST_PORT_SYSDIR_WARN ${LANG_LITHUANIAN}   "Nešiojamojo režimo negalima įdiegti į sistemos apsaugotus aplankus (Program Files / Windows).$\r$\nGrįžkite ir pasirinkite standartinį režimą arba kitą aplanką."
LangString INST_PORT_SYSDIR_WARN ${LANG_MALAY}        "Mod mudah alih tidak boleh dipasang ke folder dilindungi sistem (Program Files / Windows).$\r$\nKembali dan pilih mod Standard, atau pilih folder lain."
LangString INST_PORT_SYSDIR_WARN ${LANG_NORWEGIAN}    "Portabel modus kan ikke installeres i systembeskyttede mapper (Program Files / Windows).$\r$\nGå tilbake og velg standardmodus eller velg en annen mappe."
LangString INST_PORT_SYSDIR_WARN ${LANG_SERBIAN}      "Преносиви режим не може да се инсталира у системске фасцикле (Program Files / Windows).$\r$\nВратите се и изаберите стандардни режим или другу фасциклу."
LangString INST_PORT_SYSDIR_WARN ${LANG_SLOVAK}       "Prenosný režim nemožno nainštalovať do systémom chránených priečinkov (Program Files / Windows).$\r$\nVráťte sa a vyberte štandardný režim alebo iný priečinok."
LangString INST_PORT_SYSDIR_WARN ${LANG_SLOVENIAN}    "Prenosnega načina ni mogoče namestiti v sistemske mape (Program Files / Windows).$\r$\nVrnite se in izberite standardni način ali drugo mapo."
LangString INST_PORT_SYSDIR_WARN ${LANG_SWEDISH}      "Portabelt läge kan inte installeras i systemskyddade mappar (Program Files / Windows).$\r$\nGå tillbaka och välj standardläge eller välj en annan mapp."

; ---- Uninstall data page ----
LangString UNINST_OPTIONS_HEADER ${LANG_ENGLISH}      "Uninstall Options"
LangString UNINST_OPTIONS_HEADER ${LANG_ARABIC}       "خيارات إلغاء التثبيت"
LangString UNINST_OPTIONS_HEADER ${LANG_GERMAN}       "Deinstallationsoptionen"
LangString UNINST_OPTIONS_HEADER ${LANG_SPANISH}      "Opciones de desinstalación"
LangString UNINST_OPTIONS_HEADER ${LANG_FRENCH}       "Options de désinstallation"
LangString UNINST_OPTIONS_HEADER ${LANG_ITALIAN}      "Opzioni di disinstallazione"
LangString UNINST_OPTIONS_HEADER ${LANG_JAPANESE}     "アンインストールオプション"
LangString UNINST_OPTIONS_HEADER ${LANG_KOREAN}       "제거 옵션"
LangString UNINST_OPTIONS_HEADER ${LANG_DUTCH}        "Verwijderopties"
LangString UNINST_OPTIONS_HEADER ${LANG_POLISH}       "Opcje dezinstalacji"
LangString UNINST_OPTIONS_HEADER ${LANG_PORTUGUESEBR} "Opções de desinstalação"
LangString UNINST_OPTIONS_HEADER ${LANG_PORTUGUESE}   "Opções de desinstalação"
LangString UNINST_OPTIONS_HEADER ${LANG_RUSSIAN}      "Параметры удаления"
LangString UNINST_OPTIONS_HEADER ${LANG_SIMPCHINESE}  "卸载选项"
LangString UNINST_OPTIONS_HEADER ${LANG_TRADCHINESE}  "解除安裝選項"
LangString UNINST_OPTIONS_HEADER ${LANG_TURKISH}      "Kaldırma Seçenekleri"
LangString UNINST_OPTIONS_HEADER ${LANG_UKRAINIAN}    "Параметри видалення"
LangString UNINST_OPTIONS_HEADER ${LANG_CZECH}        "Možnosti odinstalování"
LangString UNINST_OPTIONS_HEADER ${LANG_HUNGARIAN}    "Eltávolítási beállítások"
LangString UNINST_OPTIONS_HEADER ${LANG_ROMANIAN}     "Opțiuni de dezinstalare"
LangString UNINST_OPTIONS_HEADER ${LANG_VIETNAMESE}   "Tùy chọn gỡ cài đặt"
LangString UNINST_OPTIONS_HEADER ${LANG_INDONESIAN}   "Opsi Penghapusan Instalasi"
LangString UNINST_OPTIONS_HEADER ${LANG_THAI}         "ตัวเลือกการถอนการติดตั้ง"
LangString UNINST_OPTIONS_HEADER ${LANG_SWEDISH}      "Avinstallationsalternativ"
LangString UNINST_OPTIONS_HEADER ${LANG_NORWEGIAN}    "Avinstalleringsalternativer"
LangString UNINST_OPTIONS_HEADER ${LANG_DANISH}       "Afinstallationsindstillinger"
LangString UNINST_OPTIONS_HEADER ${LANG_BULGARIAN}    "Опции за деинсталиране"
LangString UNINST_OPTIONS_HEADER ${LANG_CROATIAN}     "Mogućnosti deinstalacije"
LangString UNINST_OPTIONS_HEADER ${LANG_ESTONIAN}     "Desinstallimise suvandid"
LangString UNINST_OPTIONS_HEADER ${LANG_FINNISH}      "Asennuksen poiston asetukset"
LangString UNINST_OPTIONS_HEADER ${LANG_GREEK}        "Επιλογές απεγκατάστασης"
LangString UNINST_OPTIONS_HEADER ${LANG_HEBREW}       "אפשרויות הסרת התקנה"
LangString UNINST_OPTIONS_HEADER ${LANG_LATVIAN}      "Atinstalēšanas opcijas"
LangString UNINST_OPTIONS_HEADER ${LANG_LITHUANIAN}   "Šalinimo parinktys"
LangString UNINST_OPTIONS_HEADER ${LANG_MALAY}        "Pilihan Nyahpasang"
LangString UNINST_OPTIONS_HEADER ${LANG_SERBIAN}      "Опције деинсталације"
LangString UNINST_OPTIONS_HEADER ${LANG_SLOVAK}       "Možnosti odinštalovania"
LangString UNINST_OPTIONS_HEADER ${LANG_SLOVENIAN}    "Možnosti odstranitve"

LangString UNINST_REMOVE_DATA ${LANG_ENGLISH}      "Also remove all user data (settings, custom sound profiles) from %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_ARABIC}       "إزالة جميع بيانات المستخدم أيضاً (الإعدادات وملفات الصوت المخصصة) من %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_GERMAN}       "Auch alle Benutzerdaten (Einstellungen, benutzerdefinierte Soundprofile) aus %APPDATA%\MonkeySounds entfernen"
LangString UNINST_REMOVE_DATA ${LANG_SPANISH}      "También eliminar todos los datos de usuario (configuraciones, perfiles de sonido personalizados) de %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_FRENCH}       "Supprimer également toutes les données utilisateur (paramètres, profils audio personnalisés) de %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_ITALIAN}      "Rimuovere anche tutti i dati utente (impostazioni, profili audio personalizzati) da %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_JAPANESE}     "%APPDATA%\MonkeySounds からすべてのユーザーデータ（設定、カスタムサウンドプロファイル）も削除する"
LangString UNINST_REMOVE_DATA ${LANG_KOREAN}       "%APPDATA%\MonkeySounds에서 모든 사용자 데이터 (설정, 맞춤 사운드 프로필)도 제거"
LangString UNINST_REMOVE_DATA ${LANG_DUTCH}        "Verwijder ook alle gebruikersgegevens (instellingen, aangepaste geluidsprofielen) uit %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_POLISH}       "Usuń również wszystkie dane użytkownika (ustawienia, niestandardowe profile dźwiękowe) z %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_PORTUGUESEBR} "Também remover todos os dados do usuário (configurações, perfis de som personalizados) de %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_PORTUGUESE}   "Remover também todos os dados do utilizador (definições, perfis de som personalizados) de %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_RUSSIAN}      "Также удалить все пользовательские данные (настройки, пользовательские звуковые профили) из %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_SIMPCHINESE}  "同时从 %APPDATA%\MonkeySounds 删除所有用户数据（设置、自定义声音配置文件）"
LangString UNINST_REMOVE_DATA ${LANG_TRADCHINESE}  "同時從 %APPDATA%\MonkeySounds 刪除所有使用者資料（設定、自訂聲音設定檔）"
LangString UNINST_REMOVE_DATA ${LANG_TURKISH}      "%APPDATA%\MonkeySounds klasöründeki tüm kullanıcı verilerini de kaldır (ayarlar, özel ses profilleri)"
LangString UNINST_REMOVE_DATA ${LANG_UKRAINIAN}    "Також видалити всі дані користувача (налаштування, власні звукові профілі) з %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_CZECH}        "Také odstranit všechna uživatelská data (nastavení, vlastní zvukové profily) z %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_HUNGARIAN}    "Az összes felhasználói adat eltávolítása is (beállítások, egyéni hangprofilok) innen: %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_ROMANIAN}     "Eliminați și toate datele utilizatorului (setări, profiluri de sunet personalizate) din %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_VIETNAMESE}   "Cũng xóa tất cả dữ liệu người dùng (cài đặt, hồ sơ âm thanh tùy chỉnh) khỏi %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_INDONESIAN}   "Juga hapus semua data pengguna (pengaturan, profil suara kustom) dari %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_THAI}         "ลบข้อมูลผู้ใช้ทั้งหมด (การตั้งค่า โปรไฟล์เสียงที่กำหนดเอง) จาก %APPDATA%\MonkeySounds ด้วย"
LangString UNINST_REMOVE_DATA ${LANG_SWEDISH}      "Ta även bort alla användardata (inställningar, anpassade ljudprofiler) från %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_NORWEGIAN}    "Fjern også alle brukerdata (innstillinger, egendefinerte lydprofiler) fra %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_DANISH}       "Fjern også alle brugerdata (indstillinger, brugerdefinerede lydprofiler) fra %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_BULGARIAN}    "Премахнете също всички потребителски данни (настройки, звукови профили) от %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_CROATIAN}     "Uklonite i sve korisničke podatke (postavke, zvučne profile) iz %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_ESTONIAN}     "Eemalda ka kõik kasutajaandmed (sätted, heliprofiilid) kaustast %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_FINNISH}      "Poista myös kaikki käyttäjätiedot (asetukset, ääniprofiilit) kansiosta %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_GREEK}        "Κατάργηση και όλων των δεδομένων χρήστη (ρυθμίσεις, προφίλ ήχου) από το %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_HEBREW}       "הסר גם את כל נתוני המשתמש (הגדרות, פרופילי שמע) מ-%APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_LATVIAN}      "Noņemt arī visus lietotāja datus (iestatījumus, skaņas profilus) no %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_LITHUANIAN}   "Taip pat pašalinti visus naudotojo duomenis (nustatymus, garso profilus) iš %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_MALAY}        "Alih keluar juga semua data pengguna (tetapan, profil bunyi tersuai) daripada %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_SERBIAN}      "Уклоните и све корисничке податке (подешавања, звучне профиле) из %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_SLOVAK}       "Odstrániť aj všetky používateľské údaje (nastavenia, zvukové profily) z %APPDATA%\MonkeySounds"
LangString UNINST_REMOVE_DATA ${LANG_SLOVENIAN}    "Odstranite tudi vse uporabniške podatke (nastavitve, zvočne profile) iz %APPDATA%\MonkeySounds"

; ---- Detail print strings ----
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_ENGLISH}      "Portable mode: no registry entries, no uninstaller, no shortcuts."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_ARABIC}       "الوضع المحمول: بدون إدخالات تسجيل، بدون برنامج إلغاء تثبيت، بدون اختصارات."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_GERMAN}       "Portabler Modus: keine Registrierungseinträge, kein Deinstallationsprogramm, keine Verknüpfungen."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SPANISH}      "Modo portátil: sin entradas de registro, sin desinstalador, sin accesos directos."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_FRENCH}       "Mode portable: aucune entrée de registre, pas de désinstallateur, pas de raccourcis."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_ITALIAN}      "Modalità portabile: nessuna voce di registro, nessun disinstallatore, nessun collegamento."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_JAPANESE}     "ポータブルモード：レジストリエントリなし、アンインストーラーなし、ショートカットなし。"
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_KOREAN}       "포터블 모드: 레지스트리 항목 없음, 제거 프로그램 없음, 바로 가기 없음."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_DUTCH}        "Portable modus: geen registeritems, geen verwijderprogramma, geen snelkoppelingen."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_POLISH}       "Tryb przenośny: bez wpisów rejestru, bez dezinstalatora, bez skrótów."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_PORTUGUESEBR} "Modo portátil: sem entradas de registro, sem desinstalador, sem atalhos."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_PORTUGUESE}   "Modo portátil: sem entradas de registo, sem desinstalador, sem atalhos."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_RUSSIAN}      "Портативный режим: без записей реестра, без деинсталлятора, без ярлыков."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SIMPCHINESE}  "便携模式：无注册表项，无卸载程序，无快捷方式。"
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_TRADCHINESE}  "便攜模式：無登錄機碼，無解除安裝程式，無捷徑。"
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_TURKISH}      "Taşınabilir mod: kayıt defteri girişi yok, kaldırıcı yok, kısayol yok."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_UKRAINIAN}    "Портативний режим: без записів реєстру, без деінсталятора, без ярликів."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_CZECH}        "Přenosný režim: žádné záznamy registru, žádný odinstalátor, žádné zástupce."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_HUNGARIAN}    "Hordozható mód: nincs beállításjegyzék-bejegyzés, nincs eltávolító, nincs parancsikon."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_ROMANIAN}     "Mod portabil: fără intrări în registru, fără dezinstalator, fără comenzi rapide."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_VIETNAMESE}   "Chế độ di động: không có mục đăng ký, không có trình gỡ cài đặt, không có lối tắt."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_INDONESIAN}   "Mode portabel: tidak ada entri registri, tidak ada penghapus instalasi, tidak ada pintasan."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_THAI}         "โหมดพกพา: ไม่มีรายการรีจิสทรี ไม่มีตัวถอนการติดตั้ง ไม่มีทางลัด"
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SWEDISH}      "Portabelt läge: inga registerposter, inget avinstallationsprogram, inga genvägar."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_NORWEGIAN}    "Portabelt modus: ingen registeroppføringer, ingen avinstalleringsprogram, ingen snarveier."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_DANISH}       "Portabelt tilstand: ingen registerposter, inget afinstallationsprogram, ingen genveje."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_BULGARIAN}    "Преносим режим: без записи в регистъра, без деинсталатор, без преки пътища."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_CROATIAN}     "Prijenosni način: bez zapisa u registru, bez deinstalacije, bez prečaca."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_ESTONIAN}     "Kaasaskantav režiim: registrikirjeteta, desinstallijata, otseteedeta."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_FINNISH}      "Kannettava tila: ei rekisterimerkintöjä, ei poisto-ohjelmaa, ei pikakuvakkeita."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_GREEK}        "Φορητή λειτουργία: χωρίς καταχωρίσεις μητρώου, χωρίς απεγκατάσταση, χωρίς συντομεύσεις."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_HEBREW}       "מצב נייד: ללא רשומות רישום, ללא מסיר התקנה, ללא קיצורי דרך."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_LATVIAN}      "Pārnēsājams režīms: bez reģistra ierakstiem, bez atinstalētāja, bez saīsnēm."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_LITHUANIAN}   "Nešiojamas režimas: be registro įrašų, be šalinimo programos, be nuorodų."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_MALAY}        "Mod mudah alih: tiada entri registri, tiada penyahpasang, tiada pintasan."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SERBIAN}      "Преносиви режим: без уноса у регистар, без деинсталатора, без пречица."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SLOVAK}       "Prenosný režim: žiadne záznamy registra, žiadny odinštalátor, žiadne odkazy."
LangString DETAIL_PORTABLE_NO_SHORTCUTS ${LANG_SLOVENIAN}    "Prenosni način: brez vnosov v register, brez odstranjevalca, brez bližnjic."

LangString DETAIL_DATA_REMOVED ${LANG_ENGLISH}      "Removed user data: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_ARABIC}       "تم حذف بيانات المستخدم: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_GERMAN}       "Benutzerdaten entfernt: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SPANISH}      "Datos de usuario eliminados: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_FRENCH}       "Données utilisateur supprimées: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_ITALIAN}      "Dati utente rimossi: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_JAPANESE}     "ユーザーデータを削除しました: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_KOREAN}       "사용자 데이터 제거됨: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_RUSSIAN}      "Пользовательские данные удалены: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SIMPCHINESE}  "已删除用户数据: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_TRADCHINESE}  "已刪除使用者資料: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_TURKISH}      "Kullanıcı verileri kaldırıldı: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_UKRAINIAN}    "Дані користувача видалено: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_POLISH}       "Usunięto dane użytkownika: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_DUTCH}        "Gebruikersgegevens verwijderd: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_PORTUGUESEBR} "Dados do usuário removidos: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_PORTUGUESE}   "Dados do utilizador removidos: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_CZECH}        "Uživatelská data odstraněna: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SLOVAK}       "Používateľské údaje odstránené: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_HUNGARIAN}    "Felhasználói adatok eltávolítva: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_ROMANIAN}     "Datele utilizatorului au fost eliminate: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_BULGARIAN}    "Потребителските данни са премахнати: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_CROATIAN}     "Korisnički podaci uklonjeni: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_FINNISH}      "Käyttäjätiedot poistettu: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_GREEK}        "Τα δεδομένα χρήστη αφαιρέθηκαν: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_HEBREW}       "נתוני המשתמש הוסרו: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_INDONESIAN}   "Data pengguna dihapus: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_LITHUANIAN}   "Naudotojo duomenys pašalinti: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_LATVIAN}      "Lietotāja dati noņemti: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_MALAY}        "Data pengguna dialih keluar: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_NORWEGIAN}    "Brukerdata fjernet: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SWEDISH}      "Användardata borttagna: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_THAI}         "ลบข้อมูลผู้ใช้แล้ว: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_VIETNAMESE}   "Đã xóa dữ liệu người dùng: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_DANISH}       "Brugerdata fjernet: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_ESTONIAN}     "Kasutajaandmed eemaldatud: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SLOVENIAN}    "Uporabniški podatki odstranjeni: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_REMOVED ${LANG_SERBIAN}      "Кориснички подаци уклоњени: $APPDATA\${APP_NAME}"

LangString DETAIL_DATA_PRESERVED ${LANG_ENGLISH}      "User data preserved: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_ARABIC}       "تم الاحتفاظ ببيانات المستخدم: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_GERMAN}       "Benutzerdaten beibehalten: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SPANISH}      "Datos de usuario conservados: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_FRENCH}       "Données utilisateur préservées: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_ITALIAN}      "Dati utente preservati: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_JAPANESE}     "ユーザーデータを保持しました: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_KOREAN}       "사용자 데이터 보존됨: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_RUSSIAN}      "Пользовательские данные сохранены: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SIMPCHINESE}  "已保留用户数据: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_TRADCHINESE}  "已保留使用者資料: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_TURKISH}      "Kullanıcı verileri korundu: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_UKRAINIAN}    "Дані користувача збережено: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_POLISH}       "Dane użytkownika zachowane: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_DUTCH}        "Gebruikersgegevens behouden: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_PORTUGUESEBR} "Dados do usuário preservados: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_PORTUGUESE}   "Dados do utilizador preservados: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_CZECH}        "Uživatelská data zachována: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SLOVAK}       "Používateľské údaje zachované: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_HUNGARIAN}    "Felhasználói adatok megőrizve: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_ROMANIAN}     "Datele utilizatorului au fost păstrate: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_BULGARIAN}    "Потребителските данни са запазени: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_CROATIAN}     "Korisnički podaci sačuvani: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_FINNISH}      "Käyttäjätiedot säilytetty: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_GREEK}        "Τα δεδομένα χρήστη διατηρήθηκαν: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_HEBREW}       "נתוני המשתמש נשמרו: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_INDONESIAN}   "Data pengguna dipertahankan: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_LITHUANIAN}   "Naudotojo duomenys išsaugoti: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_LATVIAN}      "Lietotāja dati saglabāti: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_MALAY}        "Data pengguna dikekalkan: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_NORWEGIAN}    "Brukerdata bevart: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SWEDISH}      "Användardata bevarade: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_THAI}         "เก็บรักษาข้อมูลผู้ใช้ไว้แล้ว: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_VIETNAMESE}   "Đã giữ lại dữ liệu người dùng: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_DANISH}       "Brugerdata bevaret: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_ESTONIAN}     "Kasutajaandmed säilitatud: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SLOVENIAN}    "Uporabniški podatki ohranjeni: $APPDATA\${APP_NAME}"
LangString DETAIL_DATA_PRESERVED ${LANG_SERBIAN}      "Кориснички подаци сачувани: $APPDATA\${APP_NAME}"

;---------------------------------------------------------
; Variables
;---------------------------------------------------------
Var InstallMode      ; "standard" or "portable"
Var RadioStandard
Var RadioPortable
Var RemoveUserData       ; BST_CHECKED or BST_UNCHECKED
Var UnRemoveCheckbox

;---------------------------------------------------------
; .onInit  —  default to standard mode
;---------------------------------------------------------
Function .onInit
  StrCpy $InstallMode "standard"
FunctionEnd

;---------------------------------------------------------
; Custom page: Install Mode
;---------------------------------------------------------
Function InstallModePageCreate
  !insertmacro MUI_HEADER_TEXT "$(INST_MODE_HEADER)" ""
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 20u "$(INST_MODE_LABEL)"
  Pop $0

  ${NSD_CreateRadioButton} 0 26u 100% 14u "$(INST_MODE_STD)"
  Pop $RadioStandard

  ${NSD_CreateLabel} 12u 42u 100% 20u "$(INST_MODE_STD_DESC)"
  Pop $0

  ${NSD_CreateRadioButton} 0 66u 100% 14u "$(INST_MODE_PORT)"
  Pop $RadioPortable

  ${NSD_CreateLabel} 12u 82u 100% 20u "$(INST_MODE_PORT_DESC)"
  Pop $0

  ; Check the correct radio based on current mode
  ${If} $InstallMode == "portable"
    ${NSD_Check} $RadioPortable
  ${Else}
    ${NSD_Check} $RadioStandard
  ${EndIf}

  nsDialogs::Show
FunctionEnd

Function InstallModePageLeave
  ${NSD_GetState} $RadioPortable $0
  ${If} $0 == ${BST_CHECKED}
    StrCpy $InstallMode "portable"
    ; Default portable dir to the folder the setup EXE is running from
    StrCpy $INSTDIR "$EXEDIR\${APP_NAME}"
  ${Else}
    StrCpy $InstallMode "standard"
    StrCpy $INSTDIR "$PROGRAMFILES64\${APP_NAME}"
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Directory page leave — block portable installs inside
; system-protected folders (Program Files / Windows)
;---------------------------------------------------------
Function DirectoryPageLeave
  ${If} $InstallMode == "portable"
    StrCpy $0 "0"

    ${StrStr} $1 "$INSTDIR" "$PROGRAMFILES64"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${StrStr} $1 "$INSTDIR" "$PROGRAMFILES"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${StrStr} $1 "$INSTDIR" "$WINDIR"
    ${If} $1 != ""
      StrCpy $0 "1"
    ${EndIf}

    ${If} $0 == "1"
      MessageBox MB_ICONEXCLAMATION|MB_OK "$(INST_PORT_SYSDIR_WARN)"
      Abort
    ${EndIf}
  ${EndIf}
FunctionEnd

;---------------------------------------------------------
; Install Section
;---------------------------------------------------------
Section "MonkeySounds" SecMain
  SectionIn RO

  SetRegView 64
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
  ${EndIf}

  ; Kill any running instance before overwriting files
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  ; --- Files common to both modes ---
  SetOutPath "$INSTDIR"
  File "..\dist\${APP_EXE}"

  ; Locale JSON files (always installed next to EXE)
  SetOutPath "$INSTDIR\locales"
  File /r "..\dist\locales\*.*"
  SetOutPath "$INSTDIR"

  ${If} $InstallMode == "standard"
    ;------------------------------------------------------
    ; STANDARD MODE
    ;------------------------------------------------------

    ; Sound profiles go to %APPDATA%\MonkeySounds\Sounds
    ; so multiple users on the same machine share one copy
    SetOutPath "$APPDATA\${APP_NAME}\Sounds"
    File /r "..\dist\Sounds\*.*"
    SetOutPath "$INSTDIR"

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Registry — install dir
    WriteRegStr HKLM "${REGKEY}" "Install_Dir" "$INSTDIR"

    ; Add / Remove Programs
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayName"     "${APP_NAME} ${VERSION}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayIcon"     "$INSTDIR\${APP_EXE}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "Publisher"       "${PUBLISHER}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayVersion"  "${VERSION}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "URLInfoAbout"    "${WEBSITE}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "HelpLink"        "${WEBSITE}"
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair"        1

    ; Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
                   "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" \
                   "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0

    ; Desktop shortcut
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" \
                   "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

  ${Else}
    ;------------------------------------------------------
    ; PORTABLE MODE
    ;------------------------------------------------------

    ; Sound profiles sit next to the EXE
    SetOutPath "$INSTDIR\Sounds"
    File /r "..\dist\Sounds\*.*"
    SetOutPath "$INSTDIR"

    ; Write a settings.json stub so the app detects portable mode
    ; (MonkeySounds checks for settings.json next to the EXE)
    FileOpen  $0 "$INSTDIR\settings.json" w
    FileWrite $0 "{}"
    FileClose $0

    DetailPrint "$(DETAIL_PORTABLE_NO_SHORTCUTS)"

  ${EndIf}

SectionEnd

;---------------------------------------------------------
; Uninstaller custom page — ask about user data removal
;---------------------------------------------------------
Function un.RemoveDataPageCreate
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 24u "$(UNINST_OPTIONS_HEADER)"
  Pop $0

  ${NSD_CreateCheckbox} 0 30u 100% 14u "$(UNINST_REMOVE_DATA)"
  Pop $UnRemoveCheckbox

  ; Default: unchecked — preserve user data by default
  ${NSD_Uncheck} $UnRemoveCheckbox
  StrCpy $RemoveUserData ${BST_UNCHECKED}

  nsDialogs::Show
FunctionEnd

Function un.RemoveDataPageLeave
  ${NSD_GetState} $UnRemoveCheckbox $RemoveUserData
FunctionEnd

;---------------------------------------------------------
; Uninstaller  (standard mode only — portable has none)
;---------------------------------------------------------
Section "Uninstall"

  SetRegView 64
  ReadRegStr $INSTDIR HKLM "${REGKEY}" "Install_Dir"

  ; Kill running instance
  nsExec::ExecToStack 'taskkill /F /IM "${APP_EXE}"'
  Sleep 500

  ; Remove EXE and uninstaller
  Delete "$INSTDIR\${APP_EXE}"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove locales
  RMDir /r "$INSTDIR\locales"

  ; Remove install dir if now empty
  RMDir "$INSTDIR"

  ; Remove shortcuts
  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${APP_NAME}"

  ; Remove registry keys
  DeleteRegKey HKLM "${UNINSTALL_KEY}"
  DeleteRegKey HKLM "${REGKEY}"

  ; Remove auto-start entry if the user had it enabled
  DeleteRegValue HKCU \
    "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"

  ; Conditionally remove %APPDATA%\MonkeySounds based on the user's choice
  ${If} $RemoveUserData == ${BST_CHECKED}
    RMDir /r "$APPDATA\${APP_NAME}"
    DetailPrint "$(DETAIL_DATA_REMOVED)"
  ${Else}
    DetailPrint "$(DETAIL_DATA_PRESERVED)"
  ${EndIf}

SectionEnd
