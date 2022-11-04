#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define HELO "helo"  // Saludo al servidor SMTP
#define MAIL "mail from"  
#define RCPT "rcpt to" 
#define DATA "data" 

#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion

// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_INIT 0
#define S_HELO 1
#define S_MAIL 2
#define S_DATA 3
#define S_QUIT 4


//PUERTO DEL SERVICIO
#define SMTP_SERVICE_PORT	25

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"
#define PASSWORD	"123456"