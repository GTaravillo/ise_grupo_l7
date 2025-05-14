#include "server_CGI.h"
/* ARM */
#include "cmsis_os2.h"
#include "rl_net.h"
/* std */
#include <stdio.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED
#include PATH_LCD
#include PATH_RTC
#include PATH_SERVER

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

// Local variables.
static uint8_t mSelectedLeds;
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];

void HandleLedData(const char *data);
void GetLcdInput(const char *data);
static void HandleDateData(const char *data);

// static uint32_t HandleNetworkScript(const char *env, char *buf);
// static uint32_t HandleLedScript(const char *env, char *buf);
// static uint32_t HandleTcpScript(const char *env, char *buf, uint32_t buflen, uint32_t *pcgi);
// static uint32_t HandleSystemScript(const char *env, char *buf);
// static uint32_t HandleLanguageScript(const char *env, char *buf);
static uint32_t HandleLcdScript(const char *env, char *buf);
static uint32_t HandleAdcScript(const char *env, char *buf, uint32_t *adv);
static uint32_t HandleDateInScript(const char *env, char *buf);
static uint32_t HandleTimeOutScript(const char *env, char *buf);
static uint32_t HandleDateOutScript(const char *env, char *buf);
static uint32_t HandleAdcOutputScript(const char *env, char *buf, uint32_t *adv);
// static uint32_t HandleButtonStateScript(const char *env, char *buf);

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

// Process query string received by GET request.
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char         var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, (sizeof(var) - 1));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) 
{
  char var[40];
  char passw[12] = {0};

  if (code != 0) 
  {
    // Ignore all other codes
    return;
  }

  mSelectedLeds = 0;

  if (len == 0) 
  {
    // No data or all items (radio, checkbox) are off
    ledMessage_t ledMsg = {
    .mode        = LED_OFF,
    .ledsOn.leds = LD1 | LD2 | LD3
	  };
	
    osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);

    return;
  }

  passw[0] = 1;

  do 
  {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    printf("[server_CGI::%s] var [%s]\n", __func__, var);
    
    if (strcmp (var, "pg=led") == 0) 
    {
      HandleLedData(data);
    }
    else if (strcmp (var, "pg=lcd") == 0) 
    {
      GetLcdInput(data);
    }
    else if ((strncmp (var, "pw0=", 4) == 0) ||
             (strncmp (var, "pw2=", 4) == 0)) 
    {
      // Change password, retyped password
      if (netHTTPs_LoginActive())
      {
        if (passw[0] == 1) 
        {
          strcpy (passw, var+4);
        }
        else if (strcmp (passw, var+4) == 0) 
        {
          // Both strings are equal, change the password
          netHTTPs_SetPassword (passw);
        }
      }
    }
    else if (strcmp (var, "pg=date") == 0)
    {
      HandleDateData(data);
    }
  } while (data);
}

void HandleLedData(const char *data) {
  char var[40];
	
  do {
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    printf("[server_CGI::%s] var [%s]\n", __func__, var);
    
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strcmp (var, "LD1=on") == 0) {
        mSelectedLeds |= LD1;
      }
      else if (strcmp (var, "LD2=on") == 0) {
        mSelectedLeds |= LD2;
      }
      else if (strcmp (var, "LD3=on") == 0) {
        mSelectedLeds |= LD3;
      }
      else if (strcmp (var, "ctrl=Browser") == 0) {
      //   LEDrun = false;
      }
    }
  } while (data);
  
	ledMessage_t ledMsg = {
    .mode        = LED_ON,
    .ledsOn.leds = mSelectedLeds
  };
	
  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 2, 0);
	
	ledMsg.mode         = LED_OFF;
	ledMsg.ledsOff.leds = ~mSelectedLeds;

  osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 2, 0);
}

void GetLcdInput(const char *data) {
  char var[40];
  lcdMessage_t lcdMsg;
  
  do 
  {
    data = netCGI_GetEnvVar (data, var, sizeof(var));
		
    if (data == NULL) { break; }
    printf("[server_CGI::%s] var [%s]\n", __func__, var);
    
		
    if (var[0] == NULL) { continue; }
		printf("[server_CGI::%s] var[0] = %c\n", __func__, var[0]);
    
		
		const bool isLine1 = (strncmp(var, "lcd1=", 5) == 0);
		const bool isLine2 = (strncmp(var, "lcd2=", 5) == 0);
		
		lcdMsg.mode = PRINT_NORMAL;
		char stringToPrint[LCD_STR_MAX_LEN];
		
		if (isLine1)
		{
			char* lcdInputLine1 = var+5;
			lcdMsg.printMsg.printLine = PRINT_LINE_1;
			
			strcpy(stringToPrint, lcdInputLine1);
			strncpy(lcdMsg.printMsg.msg, stringToPrint, LCD_STR_MAX_LEN - 1);
			
			osStatus_t osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
		}
		else if (isLine2)
		{
			const char* lcdInputLine2 = var+5;
			lcdMsg.printMsg.printLine = PRINT_LINE_2;
			
			strcpy(stringToPrint, lcdInputLine2);
			strncpy(lcdMsg.printMsg.msg, stringToPrint, LCD_STR_MAX_LEN - 1);
			
			osStatus_t osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
		}
  }
  while (data);
}

static void HandleDateData(const char *data)
{
 char var[40];
 ERtcFlag flag;
	
 do 
 {
   data = netCGI_GetEnvVar(data, var, sizeof (var));
   printf("[server_CGI::%s] var [%s]\n", __func__, var);
   if (var[0] != 0) 
   {
     if (strncmp (var, "timeIn=", 7) == 0) 
     {
       int hh, mm, ss;
			 char time[9];
			 
			 
			 
       // flag = TIME_SET_WEB;
       // Time input text
       strcpy (time, var+7);
			 printf("Time inserted: [%s]\n", time);
       // strcpy (lcd_text[0], var+5);
       int values = sscanf(time, "%02d:%02d:%02d", &hh, &mm, &ss);
       // Obtengo los valores a partir del string
       if (values == 3) 
       {
         SetRtcTime(hh, mm, ss, FORMATO_24H);
       } 
       else 
       {
         printf("[server_CGI::%s] Hora introducida con formato incorrecto\n", __func__);
       }
     }
     else if (strncmp (var, "dateIn=", 7) == 0) 
     {
       int dd, mm, yy;
			 char date[9];
       // flag = DATE_SET_WEB;
       // LCD Module line 2 text
       strcpy (date, var+7);
			 printf("Date inserted: [%s]", date);
       // strcpy (lcd_text[0], var+5);
       int values = sscanf(date, "%d/%d/%d", &dd, &mm, &yy);
       mm = RTC_ByteToBcd2(mm);
       // Obtengo los valores a partir del string
       if (values == 3) 
       {
         SetRtcDate(dd, (EMes)mm, yy, LUNES); // TODO: desplegable para selección de día
       } 
       else 
       {
         printf("[server_CGI::%s] Fecha introducida con formato incorrecto\n", __func__);
       }
     }
   }
 } while (data);
}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
	/*
		Ejemplo:
		lcd.cgi -> c f 1 <td><input type=text name=lcd1 size=20 maxlength=21 value="%s"></td></tr>
		*env    ->   f 1 <td><input type=text name=lcd1 size=20 maxlength=21 value="%s"></td></tr>
	*/
	static uint32_t adv;
	
  uint32_t len = 0U;

	EScriptId scriptId = env[0];

  switch (scriptId) {
    // Analyze a 'c' script line starting position 2
    case SCRIPT_NETWORK:
      // Network parameters from 'network.cgi'
      //len = HandleNetworkScript(env, buf);
      break;

    case SCRIPT_LED:
      // LED control from 'led.cgi'
      //len = HandleLedScript(env, buf);
      break;

    case SCRIPT_TCP:
      // TCP status from 'tcp.cgi'
      //len = HandleTcpScript(env, buf, buflen, pcgi);
      break;

    case SCRIPT_SYSTEM:
      // System password from 'system.cgi'
      //len = HandleSystemScript(env, buf);
      break;

    case SCRIPT_LANGUAGE:
      // Browser Language from 'language.cgi'
      //len = HandleLanguageScript(env, buf);
      break;

    case SCRIPT_LCD:
			// LCD Module control from 'lcd.cgi'
			len = HandleLcdScript(env, buf);
      break;
      
    case SCRIPT_ADC:
      // AD Input from 'ad.cgi'
      len = HandleAdcScript(env, buf, &adv);
      break;
    
    case SCRIPT_DATE_IN:
      // Date Module control from 'date.cgi'
      len = HandleDateInScript(env, buf);
      break;

    case SCRIPT_TIME_OUT:
      // Date Module control from 'date.cgi'
      len = HandleTimeOutScript(env, buf);
      break;
    
    case SCRIPT_DATE_OUT:
      // Date Module control from 'date.cgi'
      len = HandleDateOutScript(env, buf);
      break;

    case SCRIPT_ADC_OUT:
      // AD Input from 'ad.cgx'
      len = HandleAdcOutputScript(env, buf, &adv);
      break;

    case SCRIPT_BUTTON_STATE:
      // Button state from 'button.cgx'
      //len = HandleButtonStateScript(env, buf);
      break;
  }
  return len;
}

// static uint32_t HandleNetworkScript(const char *env, char *buf)
// {
//   int16_t      typ     = 0;
// 	bool         correct = true;
//   netIF_Option opt     = netIF_OptionMAC_Address;
	
//   switch (env[3]) {
// 		case '4': typ = NET_ADDR_IP4; break;
// 		case '6': typ = NET_ADDR_IP6; break;

// 		default: return 0;
// 	}
	
// 	switch (env[2]) {
// 		case 'l':
// 			// Link-local address
// 			if (env[3] == '4') { return 0;                             }
// 			else               { opt = netIF_OptionIP6_LinkLocalAddress; }
// 			break;

// 		case 'i':
// 			// Write local IP address (IPv4 or IPv6)
// 			if (env[3] == '4') { opt = netIF_OptionIP4_Address;       }
// 			else               { opt = netIF_OptionIP6_StaticAddress; }
// 			break;

// 		case 'm':
// 			// Write local network mask
// 			if (env[3] == '4') { opt = netIF_OptionIP4_SubnetMask; }
// 			else               { return (0);                       }
// 			break;

// 		case 'g':
// 			// Write default gateway IP address
// 			if (env[3] == '4') { opt = netIF_OptionIP4_DefaultGateway; }
// 			else               { opt = netIF_OptionIP6_DefaultGateway; }
// 			break;

// 		case 'p':
// 			// Write primary DNS server IP address
// 			if (env[3] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
// 			else               { opt = netIF_OptionIP6_PrimaryDNS; }
// 			break;

// 		case 's':
// 			// Write secondary DNS server IP address
// 			if (env[3] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
// 			else               { opt = netIF_OptionIP6_SecondaryDNS; }
// 			break;
// 	}

// 	netIF_GetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
// 	netIP_ntoa (typ, ip_addr, ip_string, sizeof(ip_string));
// 	return (uint32_t)sprintf (buf, &env[5], ip_string);
// }

// static uint32_t HandleLedScript(const char *env, char *buf)
// {
// 	uint8_t id;
	
//   if (env[2] == 'c') {
// 		// Select Control
// 		// return (uint32_t)sprintf (buf, &env[4], LEDrun ?     ""     : "selected",
// 		//                                        LEDrun ? "selected" :    ""     );
// 	}
// 	// LED CheckBoxes
// 	id = env[2] - '0';
// 	if (id > 0x04) {
// 		id = 0;
// 	}
// 	id = (uint8_t)(1U << id);
// 	return (uint32_t)sprintf (buf, &env[4], (mSelectedLeds & id) ? "checked" : "");
// }

// static uint32_t HandleTcpScript(const char *env, char *buf, uint32_t buflen, uint32_t *pcgi)
// {
//   int32_t socket;
//   netTCP_State state;
//   NET_ADDR r_client;

//   uint32_t len = 0U;

//   while ((len + 150) < buflen)
//   {
//     socket = ++MYBUF(pcgi)->idx;
//     state  = netTCP_GetState(socket);

//     if (state == netTCP_StateINVALID) 
//     {
//       /* Invalid socket, we are done */
//       return len;
//     }

//     // Start table row
//     len += (uint32_t)sprintf(buf + len, "<tr align=\"center\">");

//     if (state <= netTCP_StateCLOSED) 
//     {
//       len += (uint32_t)sprintf(buf + len,
//              "<td>%d</td><td>%d</td><td>-</td><td>-</td><td>-</td><td>-</td></tr>\r\n",
//              socket, netTCP_StateCLOSED);
//     }
//     else if (state == netTCP_StateLISTEN) 
//     {
//       len += (uint32_t)sprintf(buf + len,
//              "<td>%d</td><td>%d</td><td>%d</td><td>-</td><td>-</td><td>-</td></tr>\r\n",
//              socket, netTCP_StateLISTEN, netTCP_GetLocalPort(socket));
//     }
//     else 
//     {
//       netTCP_GetPeer(socket, &r_client, sizeof(r_client));
//       netIP_ntoa(r_client.addr_type, r_client.addr, ip_string, sizeof(ip_string));

//       len += (uint32_t)sprintf(buf + len,
//              "<td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
//              socket, state, netTCP_GetLocalPort(socket), netTCP_GetTimer(socket), ip_string, r_client.port);
//     }
//   }

// 	/* More sockets to go, set a repeat flag */
// 	return len | (1u << 31);
// }

// static uint32_t HandleSystemScript(const char *env, char *buf)
// {
// 	uint32_t len    = 0U;
	
//   switch (env[2])
//   {
//     case '1':
//       len = (uint32_t)sprintf (buf, &env[4], netHTTPs_LoginActive() ? "Enabled" : "Disabled");
// 		  break;

//     case '2':
//       len = (uint32_t)sprintf (buf, &env[4], netHTTPs_GetPassword());
// 		  break;
		
// 		default:
// 			break;
//   }
// 	return len;
// }

// static uint32_t HandleLanguageScript(const char *env, char *buf)
// {
// 	const char *lang;
//   lang = netHTTPs_GetLanguage();
// 	if      (strncmp (lang, "en", 2) == 0) {
// 		lang = "English";
// 	}
// 	else if (strncmp (lang, "de", 2) == 0) {
// 		lang = "German";
// 	}
// 	else if (strncmp (lang, "fr", 2) == 0) {
// 		lang = "French";
// 	}
// 	else if (strncmp (lang, "sl", 2) == 0) {
// 		lang = "Slovene";
// 	}
// 	else {
// 		lang = "Unknown";
// 	}
// 	return (uint32_t)sprintf (buf, &env[2], lang, netHTTPs_GetLanguage());
// }

static uint32_t HandleLcdScript(const char *env, char *buf)
{
	uint32_t len    = 0U;
  const char line = env[2];
	
	switch (line) {
		case '1': {
			const char* string = GetLcdTextInput(LCD_LINE_1);
			len = (uint32_t)sprintf(buf, &env[4], string);
			break;
		}
			
		case '2': {
			const char* string = GetLcdTextInput(LCD_LINE_2);
			len = (uint32_t)sprintf (buf, &env[4], string);
			break;
		}
		
		default:
			break;
	}
	
	return len;
}

static uint32_t HandleAdcScript(const char *env, char *buf, uint32_t *adv)
{
	uint32_t len    = 0U;
	
  switch (env[2]) {
		case '1':
			*adv = ADC_in (0);
		  printf("[server_CGI] 1 adv [%d]\n", *adv);
			len = (uint32_t)sprintf (buf, &env[4], (*adv));
		  break;
		
		case '2':
			len = (uint32_t)sprintf (buf, &env[4], (double)((float)(*adv)*3.3f)/4096);
			printf("[server_CGI] 2 adv [%f]\n", (double)((float)(*adv)*3.3f)/4096);
		  break;
		
		case '3':
			*adv = ((*adv) * 100) / 4096;
		  printf("[server_CGI] 3 adv [%d]\n", *adv);
			len = (uint32_t)sprintf (buf, &env[4], (*adv));
		  break;
		
		default:
			break;
	}
	
	return len;
}

static uint32_t HandleDateInScript(const char *env, char *buf)
{
	uint32_t len = 0U;
	
 switch (env[2]) 
	{
		case '1': 
		  len = (uint32_t)sprintf(buf, &env[4], GetRtcTime());
			break;

		case '2':
			len = (uint32_t)sprintf(buf, &env[4], GetRtcDate());
			break;

		case '3':
			// Hora que aparece por defecto en el recuadro de cambio de hora
			len = (uint32_t)sprintf (buf, &env[4], "20:34:00");
			break;

		case '4':
			// Fecha que aparece por defecto en el recuadro de cambio de fecha
			len = (uint32_t)sprintf (buf, &env[4], "26/03/25");
			break;
		
		default:
			break;
	}
	return len;
}

static uint32_t HandleTimeOutScript(const char *env, char *buf)
{
  return (uint32_t)sprintf (buf, &env[1], GetRtcTime());
}

static uint32_t HandleDateOutScript(const char *env, char *buf)
{
  return (uint32_t)sprintf (buf, &env[1], GetRtcDate());
}

static uint32_t HandleAdcOutputScript(const char *env, char *buf, uint32_t *adv)
{
  *adv = ADC_in (0);
  printf("[server_CGI] 4 env: %s\n", env);
  printf("[server_CGI] 4 adv [%d]\n", *adv);
  return (uint32_t)sprintf (buf, &env[1], *adv);
}

// static uint32_t HandleButtonStateScript(const char *env, char *buf)
// {
//   return (uint32_t)sprintf(buf, "<checkbox><id>button%c</id><on>%s</on></checkbox>",
//                            env[1], (get_button () & (1 << (env[1]-'0'))) ? "true" : "false");
// }

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
