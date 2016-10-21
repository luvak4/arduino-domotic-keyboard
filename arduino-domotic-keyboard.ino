#include <IRremote.h> 
#include <VirtualWire.h>
////////////////////////////////
// pins
////////////////////////////////
const int pin_rx  = 11;
const int pin_tx  = 12;
const int pin_ir  =  2; // ir pin
////////////////////////////////
// indirizzi radio TX
////////////////////////////////
#define MASTRdisplay  100 // info to display
#define MASTRa 101 // get luce/temp/rele <--(CANTIa)
#define MASTRb 102 // !- set temp (soglia) up   +10
#define MASTRc 103 // !- set temp (soglia) down -10
#define MASTRd 104 // !- set luce (soglia a) up +5
#define MASTRe 105 // !- set luce (soglia a) dn -5
#define MASTRf 106 // !- set luce (soglia b) up +50
#define MASTRg 107 // !- set luce (soglia b) dn -50
#define MASTRh 108 // !---> get soglie   <--(CANTIb)
#define MASTRi 109 // !- set AGC delay up +100
#define MASTRj 110 // !- set AGC delay dn -100
#define MASTRk 111 // !---> AGC delay    <--(CANTIc)
#define MASTRl 112 // >>> salva  EEPROM  <--(CANTIok)
#define MASTRm 113 // >>> carica EEPROM  <--(CANTIok)
#define MASTRn 114 // >>> carica DEFAULT <--(CANTIok)
#define MASTRo 115 // get temp/luce STATO/tempo
////////////////////////////////
// indirizzi radio RX
////////////////////////////////
#define CANTIa   1000 // get value luce/temp/rele
#define CANTIb   1001 // get soglie luce/temp
#define CANTIc   1002 // get AGC 
#define CANTId   1003 // get temp/luce STATO/tempo
#define CANTIokA 1004 // get ok salva eprom
#define CANTIokB 1005 // get ok carica eprom
#define CANTIokC 1006 // get ok carica default
////////////////////////////////
// trasmissione radio a display
////////////////////////////////
#define DISPLAYindirizzoLSB    0
#define DISPLAYindirizzoMSB    1
#define DISPLAYcolonna         2
#define DISPLAYriga            3
#define DISPLAYnCaratteri      4
#define DISPLAYinizioTesto     5
#define VELOCITAhi          2000
//
byte    BYTEradioindirDISPLAY[VW_MAX_MESSAGE_LEN];
String  CARATTERI;
////////////////////////////////
// LCM
////////////////////////////////
// caratteri personalizzati
#define SIMBluce  0
#define SIMBtermo 1
#define SIMBlivB  2
#define SIMBlivC  3
#define SIMBlivD  4
#define SIMBlivE  5
#define SIMBlivF  6
#define SIMBgiu   7
// caratteri interni
#define SIMBsu    B01011110
#define SIMBlivA  B01011111
#define SIMBon    B01101111
#define SIMBoff   B10100101
////////////////////////////////
// STATI
////////////////////////////////
#define SALITA              1
#define DISCESA             0
#define PORTACHIUSA         0
#define PORTAAPERTA         1
#define LUCECORRIDOIOACCESA 2
////////////////////////////////
// comunicazione radio principale
////////////////////////////////
#define VELOCITAstd   500
#define MESSnum         0
#define DATOa           1
#define DATOb           2
#define DATOc           3
#define BYTEStoTX       8
int     INTERIlocali[4]={0,0,0,0};
byte    BYTEradio[BYTEStoTX];
uint8_t buflen = BYTEStoTX; //for rx
////////////////////////////////
// ricezione via infrarossi
////////////////////////////////
#define KEY_1     -983386969
#define KEY_2    -1519975698
#define KEY_3     1007641363
#define KEY_4       11835754
#define KEY_5     -374420087
#define KEY_6     1316801890
#define KEY_7    -1829136225
#define KEY_8    -1395652082
#define KEY_9      497848941
#define KEY_0    -1975719008
#define KEY_UP          9999
#define KEY_DN          9998
#define KEY_OK   -1812574087
#define KEY_CLEAR -477592334
IRrecv irrecv(pin_ir); // ir initialize library
decode_results irX;    // ir variable
////////////////////////////////
// varie
////////////////////////////////
byte CIFR[]={223,205,228,240,43,146,241,//
	     87,213,48,235,131,6,81,26,//
	     70,34,74,224,27,111,150,22,//
	     138,239,200,179,222,231,212};
#define mask 0x00FF
int NUMcomp=0;
bool IRricevuto=false;
byte cinqueSec=0;
unsigned long tempo;
byte decimi;
byte secondi;
byte minuti;
////////////////////////////////
// setup
////////////////////////////////
void setup()
{
  vw_set_tx_pin(pin_tx); 
  vw_set_rx_pin(pin_rx);  
  vw_setup(VELOCITAstd);       
  vw_rx_start();               
  irrecv.enableIRIn();
  Serial.begin(9600);
}
////////////////////////////////
// loop
////////////////////////////////
void loop(){
  char buf[4]
  ////////////////////////////////
  // tieni il tempo
  ////////////////////////////////
  if ((abs(millis()-tempo))>100){
    tempo=millis();
    decimi++;
    ////begin ogni decimo///////////
    ////end   ogni decimo///////////    
    if (decimi>9){
      ////begin ogni secondo//////////
      //
      //--dopo 5 sec azzera composizione--
      if (IRricevuto){
	cinqueSec++;
	if (cinqueSec>4){
	  IRricevuto=false;
	  cinqueSec=0;
	  NUMcomp=0;
	  stampaNc();
	}
      }
      ////end   ogni secondo//////////          
      decimi=0;
      secondi++;
      if (secondi>59){
	////begin ogni minuto //////////
	////end   ogni minuto //////////          	
	secondi=0;
	minuti++;
	if (minuti>250){
	  minuti=0;
	}
      }
    }
    ////////////////////////////////
    // IR
    ////////////////////////////////
    chechForIR();
    ////////////////////////////////
    // radio rx
    ////////////////////////////////
    if (vw_get_message(BYTEradio, &buflen)){
      vw_rx_stop();
      decodeMessage();
      switch (INTERIlocali[MESSnum]){
      case CANTIokA:
	break;
      case CANTIokB:
	break;
      case CANTIokC:
	break;	
      case CANTIa:
	////////////////////////////////
	// --------------------
	//                     
	// 1000 100 20 600 
	// ^  13  _  10       
	// L 1001   T 2129 x     <<<<<<<
	// --------------------
	////////////////////////////////
	sprintf(buf, "%4d",INTERIlocali[DATOb]);
	CARATTERI  = char(SIMBluce)  + " " + buf + "   ";
	sprintf(buf, "%4d",INTERIlocali[DATOa]);  
	CARATTERI += char(SIMBtermo) + " " + buf;	
	if (INTERIlocali[DATOc]>0){
	  CARATTERI += " " + char(SIMBon); 
	} else {
	  CARATTERI += " " + char(SIMBoff); 	  
	}	 
	txDISPLAY(0,3);
	break;
      case CANTIb:
	// --------------------
	//                     
	// XXXX 100 20 600    <<<<<<<
	// ^  13  _  10              
	// L 1001   T2129
	// --------------------
	////////////////////////////////
	sprintf(buf, "%3d",INTERIlocali[DATOa]);
	CARATTERI   = buf + " ";
	sprintf(buf, "%2d",INTERIlocali[DATOb]);
	CARATTERI  += buf + " ";
	sprintf(buf, "%3d",INTERIlocali[DATOc]);
	CARATTERI  += buf;
	txDISPLAY(5,1);
	break;
      case CANTIc:
	/////agc delay//////////////////
	// --------------------
	//                     
	// 1000 XXX XX XXX     <<<<     
	// ^  13  _  10       
	// L 1001   T2129
	// --------------------
	////////////////////////////////
	sprintf(buf, "%4d",INTERIlocali[DATOa]);
	CARATTERI  = buf;
	txDISPLAY(0,1);
	break;
      case CANTId:
	////stato tempo di luce e temp//
	// --------------------
	//                     
	// 1000 100 20 600 
	// ^  13  _  10       <<<<<<<
	// L 1001   T2129
	// --------------------
	////////////////////////////////
	byte stTemp;
	byte stLuce;
	INTtoBYTE(INTERIlocali[DATOa],stTemp,stLuce);
	//
	sprintf(buf, "%4d",INTERIlocali[DATOb]);  
	switch(stTemp){
	case SALITA:
	  CARATTERI  = char(SIMBsu);
	  break;
	case DISCESA:
	  CARATTERI  = char(SIMBgiu); 
	  break;
	}
	CARATTERI += buf + "   ";
	//
	sprintf(buf, "%4d",INTERIlocali[DATOc]);  
	switch(stLuce){
	case PORTACHIUSA:
	  caratteri+= char(SIMBlivA);
	  break;
	case PORTAAPERTA:
	  caratteri+= char(SIMBlivD);	  
	  break;
	case LUCECORRIDOIOACCESA:
	  caratteri+= char(SIMBlivF);	  
	  break;	  
	}
	CARATTERI += buf;
	txDISPLAY(0,2);
	break;
      }
      vw_rx_start();
    } 
  }
}
////////////////////////////////
// RADIO -> locale
////////////////////////////////
void decodeMessage(){
  byte m=0;
  cipher();
  for (int n=0; n<4;n++){
    INTERIlocali[n]=BYTEradio[m+1];
    INTERIlocali[n]=INTERIlocali[n] << 8;
    INTERIlocali[n]=INTERIlocali[n]+BYTEradio[m];
    m+=2;
  }
}
////////////////////////////////
// locale -> RADIO
////////////////////////////////
void encodeMessage(){
  byte m=0;
  for (int n=0; n<4;n++){
    BYTEradio[m]=INTERIlocali[n] & mask;
    INTERIlocali[n]=INTERIlocali[n] >> 8;
    BYTEradio[m+1]=INTERIlocali[n] & mask;
    m+=2;
  }
  cipher();
}
////////////////////////////////
// cifratura XOR del messaggio
////////////////////////////////
void cipher(){
  for (byte n=0;n<8;n++){
    BYTEradio[n]=BYTEradio[n]^CIFR[n];
  }
}

////////////////////////////////
// decodifica ir
////////////////////////////////
long ir_decode(decode_results *irX){
  long keyLongNumber = irX->value;
  return keyLongNumber;
}

////////////////////////////////
// controlla se segnale IR
////////////////////////////////
void chechForIR(){
  ///////start check for IR///////
  if (irrecv.decode(&irX)) {
    long key=ir_decode(&irX);
    ////////start switch////////////
    switch (key){
    case KEY_OK:
      ////////////////////////////////
      // invia il numero composto
      ////////////////////////////////
      stampaNc();
      INTERIlocali[MESSnum]=MASTRa;
      INTERIlocali[DATOa]=NUMcomp;
      INTERIlocali[DATOb]=0;
      INTERIlocali[DATOc]=0;
      encodeMessage();
      vw_rx_stop();
      vw_send((uint8_t *)BYTEradio,BYTEStoTX);
      vw_wait_tx();
      vw_rx_start();
      break;
    case KEY_1: scorriNumero(1);break;
    case KEY_2: scorriNumero(2);break;
    case KEY_3: scorriNumero(3);break;
    case KEY_4: scorriNumero(4);break;
    case KEY_5: scorriNumero(5);break;
    case KEY_6: scorriNumero(6);break;
    case KEY_7: scorriNumero(7);break;
    case KEY_8: scorriNumero(8);break;
    case KEY_9: scorriNumero(9);break;
    case KEY_0: scorriNumero(0);break;
    case KEY_CLEAR: NUMcomp=0; stampaNc(); break;
    case KEY_UP: break;
    case KEY_DN: break;      
    }
    ////////end switch////////////////    
    IRricevuto=true;
    delay(100);
    irrecv.resume();
  }
  /////end check for IR///////////
}
////////////////////////////////
// trasmette al display via radio
// occorre riempire anche il testo
////////////////////////////////
void txDISPLAY(byte colonna, byte riga){
  // pulisce bytes-radio
  for (byte n=0;n<20;n++){
    BYTEradioindirDISPLAY[n]=0;
  }
  // indirizzo display
  int g=MASTRdisplay;
  // caricamento int su bytes
  BYTEradioindirDISPLAY[0]=g & mask;
  g=g >> 8;
  BYTEradioindirDISPLAY[1]=g & mask;
  // caricamento riga e colonna su bytes
  BYTEradioindirDISPLAY[2]=colonna;
  BYTEradioindirDISPLAY[3]=riga;
  // caricamento lunghezza testo su bytes
  BYTEradioindirDISPLAY[4]=CARATTERI.length();
  // la lunghezza deve essere non superiore a 20
  if (BYTEradioindirDISPLAY[4]>20){
    BYTEradioindirDISPLAY[4]=20;
  }
  // caricamento messaggio su bytes
  for (byte n=5;n<BYTEradioindirDISPLAY[4]+5;n++){
    BYTEradioindirDISPLAY[n]=CARATTERI[n-5];
  }
  // cifratura
  for (byte n=0; n<27;n++){
    BYTEradioindirDISPLAY[n]=BYTEradioindirDISPLAY[n] ^ CIFR[n];
  }
  // tx via radio
  vw_rx_stop();
  // cambio di velocita
  vw_setup(VELOCITAhi);
  vw_send((uint8_t *)BYTEradioindirDISPLAY,VW_MAX_PAYLOAD);
  vw_wait_tx();
  // ripristino velocita
  vw_setup(VELOCITAstd);
  vw_rx_start();
}
////////////////////////////////
// stampa numero ricevuto via IR
////////////////////////////////
void stampaNc(){
  char buf[5];
  sprintf(buf, "%5d",NUMcomp);  
  CARATTERI=buf;
  txDISPLAY(0,3);//---->
}
////////////////////////////////
// viene composto il numero
////////////////////////////////
void scorriNumero(byte aggiungi){
  NUMcomp=NUMcomp*10+aggiungi;
  if (NUMcomp<0){
    NUMcomp=0;
  }  
  stampaNc();
}
