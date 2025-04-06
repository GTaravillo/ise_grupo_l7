#ifndef SERVER_CGI_H
#define SERVER_CGI_H

typedef enum {
  SCRIPT_NETWORK      = 'a',
  SCRIPT_LED          = 'b',
  SCRIPT_TCP          = 'c',
  SCRIPT_SYSTEM       = 'd',
  SCRIPT_LANGUAGE     = 'e',
  SCRIPT_LCD          = 'f',
  SCRIPT_ADC          = 'g',
  SCRIPT_DATE_IN      = 'h',
  SCRIPT_TIME_OUT     = 'v',
  SCRIPT_DATE_OUT     = 'w',
  SCRIPT_ADC_OUT      = 'x',
  SCRIPT_BUTTON_STATE = 'y'
} EScriptId;

#endif
