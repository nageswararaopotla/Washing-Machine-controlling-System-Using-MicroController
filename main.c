// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)      // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include"main.h"
#pragma config WDTE=OFF 
unsigned char reset_mode,program_no=0,water_level_index=0;
int wash_time,rinse_time,spin_time;
char *washing_prog[]={"Daily","Heavy","Delicates","Whites","Stainwash","EcoCottons","Wollens","Bedsheets","Rinse+Dry","Dry only","Wash only","Aqua Store"};
char *water_level_arr[]={"Auto","Low","Medium","High","Max"};
unsigned char min,sec;
unsigned char operation_mode;
static void init_config(void)
{
    init_clcd();
    init_digital_keypad();
    init_timer2();
    PEIE=1;
    BUZZER_DDR=0;//buzzer as output
    FAN_DDR=0;//fan as output
    GIE=1;//setting Global Interrupt Enable
    ADCON1=0x06;//Turn of analog to digital convertors
    DC_MOTOR_DDR=0;//motor as output
    PORTA=OFF;//Initially motor is kept off
}
void main(void)
{
    unsigned char key;
    operation_mode = WASHING_PROGRAM_DISPLAY;
    init_config();
    clcd_print(" Press Key5 To ",LINE1(0));
    clcd_print("    Power ON    ",LINE2(0));
    clcd_print(" Washing Machine ",LINE3(0));
    while(read_digital_keypad(STATE) != SW5)
    {
        for(int j=0;j<3000;j++);
    }
    clear_screen();// erase all the previous message on screen
    power_on_screen(); //Calling power on screen
    reset_mode = WASHING_PROGRAM_DISPLAY_RESET;
    while(1)
    {
      key=read_digital_keypad(STATE);
      for(int j=0;j<3000;j++);
      if(key == LSW4 && operation_mode == WASHING_PROGRAM_DISPLAY)
      {
          operation_mode=WATER_LEVEL;
          reset_mode=WATER_LEVEL_RESET;
      }
      else if(key==LSW4 && operation_mode==WATER_LEVEL)
      {
          set_time_for_program();
          clear_screen();// erase all the previous message on screen
          clcd_print(" Press Switch:",LINE1(0));
          clcd_print(" SW5: START ",LINE2(0));
          clcd_print(" SW6: STOP ",LINE3(0));
          operation_mode=START_STOP_SCREEN;
      }
      else if(key == SW5 && operation_mode ==PAUSE)
      {
          TMR2ON = ON;//.timer is on
          FAN = ON;//turning on fan
          DC_MOTOR=ON;//turning on motor
          operation_mode=START_PROGRAM;
      }
      switch(operation_mode)
      {
          case WASHING_PROGRAM_DISPLAY:
              washing_program_display(key);
              break;
          case WATER_LEVEL:
              water_level_display(key);
              break;
          case START_STOP_SCREEN:
              if(key==SW5)
              {
                  operation_mode=START_PROGRAM;
                  reset_mode=START_PROGRAM_RESET;
                  continue;
              }
              else if(key==SW6)
              {
                  clear_screen();//erases all the previous content on screen
                  clcd_print("Returning To ",LINE1(2));
                  clcd_print("Main Screen...",LINE2(2));
                  __delay_ms(1000);
                  operation_mode=WASHING_PROGRAM_DISPLAY;
                  reset_mode=WASHING_PROGRAM_DISPLAY_RESET;
                  continue;
              }
              break;
          case START_PROGRAM:
              run_program(key);
              break;
      }
      reset_mode=RESET_NOTHING;
    }
}
void power_on_screen(void)
{
    for(unsigned char i=0;i<16;i++)
    {
        clcd_putch(BLOCK,LINE1(i));
    }
    clcd_print("  Powering ON   ",LINE2(0));
    clcd_print(" Washing Machine ",LINE3(0));
    for(unsigned char i=0;i<16;i++)
    {
        clcd_putch(BLOCK,LINE4(i));
    }
    __delay_ms(1000);
}
void clear_screen(void)//function that helps to clear the screen
{
    clcd_write(CLEAR_DISP_SCREEN,INST_MODE);
    __delay_us(100);
}
void washing_program_display(unsigned char key)
{
    if(reset_mode == WASHING_PROGRAM_DISPLAY_RESET)
    {
        clear_screen();// erase all the previous message on screen
        program_no=0;
    }
    if(key==SW4)
    {
        program_no++;
        if(program_no==12)
        {
            program_no=0;
        }
        clear_screen();// erase all the previous message on screen
    }
    clcd_print("Washing Programs",LINE1(0));
    clcd_putch('*',LINE2(0));
    if(program_no<=9)
    {
    clcd_print(washing_prog[program_no],LINE2(2));
    clcd_print(washing_prog[program_no + 1],LINE3(2));
    clcd_print(washing_prog[program_no + 2],LINE4(2));
    }
    else if(program_no==10)
    {
    clcd_print(washing_prog[program_no],LINE2(2));
    clcd_print(washing_prog[program_no + 1],LINE3(2));
    clcd_print(washing_prog[0],LINE4(2));
    }
    else if(program_no==11)
    {
    clcd_print(washing_prog[program_no],LINE2(2));
    clcd_print(washing_prog[0],LINE3(2));
    clcd_print(washing_prog[1],LINE4(2));
    }
}
void water_level_display(unsigned char key)
{
    if(reset_mode==WATER_LEVEL_RESET)
    {
        water_level_index=0;
        clear_screen();// erase all the previous message on screen
    }
    if(key == SW4)
    {
        water_level_index++;
        if(water_level_index==5)
        {
            water_level_index=0;
        }
        clear_screen();// erase all the previous message on screen
    }
    clcd_print("Water Level:",LINE1(0));
    clcd_putch('*',LINE2(0));
    if(water_level_index <= 2)
    {
        clcd_print(water_level_arr[water_level_index],LINE2(2));
        clcd_print(water_level_arr[water_level_index + 1],LINE3(2));
        clcd_print(water_level_arr[water_level_index + 2],LINE4(2));
    }
    else if(water_level_index == 3)
    {
        clcd_print(water_level_arr[water_level_index],LINE2(2));
        clcd_print(water_level_arr[water_level_index + 1],LINE3(2));
        clcd_print(water_level_arr[0],LINE4(2));        
    }
    else if(water_level_index == 4)
    {
        clcd_print(water_level_arr[water_level_index],LINE2(2));
        clcd_print(water_level_arr[0],LINE3(2));
        clcd_print(water_level_arr[1],LINE4(2));
    }
}
void set_time_for_program(void)
{
    switch(program_no)
    {
        case 0:  //Daily
            switch(water_level_index)
            {
                case 1:
                    sec=33;
                    min=0;
                    break;
                case 0:
                case 2:
                    sec=41;
                    min=0;
                    break;
                case 3:
                case 4:
                    sec=45;
                    min=0;
            }
            break;
        case 1:  //Heavy
            switch(water_level_index)
            {
                case 1:
                    sec=43;
                    min=0;
                case 0:
                case 2:
                    sec=50;
                    min=0;
                    break;
                case 3:
                case 4:
                    sec=57;
                    min=0;
            }
            break;
        case 2:  //Delicates
            switch(water_level_index)
            {
                case 1:
                case 0:
                case 2:
                    sec=26;
                    min=0;
                    break ;
                case 3:
                case 4:
                    sec=31;
                    min=0;
            }
            break;
        case 3:  //Whites
            sec=16;
            min=1;
            break;
        case 4:  //Stain_wash
            sec=36;
            min=1;
        case 5:  //Eco_cottons
            sec=28;
            min=0;
            break;
        case 6:  //Wollens
            sec=29;
            break;
        case 7:  //Bed_sheets
            switch(water_level_index)
            {
                case 1:
                    sec=46;
                    min=0;
                case 0:
                case 2:
                    sec=53;
                    min=0;
                    break;
                case 3:
                case 4:
                    sec=60;
                    min=0;
            }
            break;
        case 8:
            switch(water_level_index)
            {
                case 1:
                    sec=18;
                    min=0;
                case 0:
                case 2:
                case 3:
                case 4:
                    sec=20;
                    min=0;
            }
            break;
        case 9:  //Dry_Wash
            sec=6;
            break;
        case 10:  //Wash_only
        case 11:  //Aqua store
            switch(water_level_index)
            {
                case 1:
                    sec=16;
                    min=0;
                case 0:
                case 2:
                    sec=21;
                    min=0;
                    break;
                case 3:
                case 4:
                    sec=26;
                    min=0;
            }
            break;
    }
}
void door_status_check(void)
{
   if(RB0 == 0)//Door open
   {
       BUZZER=ON;//Turn on th Buzzer
       FAN=OFF;//turning off fan
       DC_MOTOR=OFF;//turning off motor
       TMR2ON=OFF;//timer is off
       clear_screen();// erase all the previous message on screen
       clcd_print("DOOR : OPEN",LINE1(0));
       clcd_print("Please CLose",LINE2(0));
       while(RB0==0)
       {
           ;
       }
       clear_screen();// erase all the previous message on screen
        clcd_print("Function -",LINE1(0));
        clcd_print("Time= ",LINE2(0));
        clcd_print("5-START  6-PAUSE",LINE4(0));
       FAN=ON;//turning on fan
       DC_MOTOR=ON;
       TMR2ON=ON;
       BUZZER=OFF;
   }
}
void run_program(unsigned char key)
{
    static int total_time,time;
    if(reset_mode==START_PROGRAM_RESET)
    {
        clear_screen();// erase all the previous message on screen
        clcd_print("Prog: ",LINE1(0));
        clcd_print(washing_prog[program_no],LINE1(5));
        clcd_print("Time: ",LINE2(0));
        clcd_putch((min/10)+'0',LINE2(6));
        clcd_putch((min%10)+'0',LINE2(7));
        clcd_putch(':',LINE2(8));
        clcd_putch((sec/10)+'0',LINE2(9));
        clcd_putch((sec%10)+'0',LINE2(10));
        clcd_print("(MM:SS)",LINE3(5));
        __delay_ms(3000);
        clear_screen();// erase all the previous message on screen
        clcd_print("Function -",LINE1(0));
        clcd_print("Time = ",LINE2(0));
        clcd_print("5-START  6-PAUSE",LINE4(0));
        time=total_time=(min*60+sec);
        wash_time= (int)(total_time * 0.46);
        rinse_time=(int)(total_time * 0.12);
        spin_time=(total_time - wash_time - rinse_time);
        TMR2ON = 1;//Starting the timer
        FAN=ON;//Starting Fan
        DC_MOTOR=ON;//turning on motor
    }
    door_status_check();//checking the status of the door(SW1)
    if(key==SW6)//pause the process
    {
        TMR2ON=OFF;//stops the timer
        FAN=OFF;//turning off fan
        operation_mode=PAUSE;
        DC_MOTOR=OFF;//turning off motor
    }
    total_time=(min * 60 + sec);
    if(program_no<7)
    {
    if(total_time >=( time - wash_time))
    {
        clcd_print("Wash",LINE1(11));//wash mode
    }
    else if(total_time >= (time - wash_time - rinse_time))
    {
        clcd_print("Rinse",LINE1(11));//rinse mode
    }
    else
    clcd_print(" Spin",LINE1(11));//spin mode
    }
    else if(program_no==8)
    {
        if(total_time>=(time - (int)(0.40*time)))
        {
            clcd_print("Rinse",LINE1(11));//rinse mode
        }
        else
            clcd_print(" Spin",LINE1(11));//spin mode
    }
    else if(program_no==9)
    {
        clcd_print("Rinse",LINE1(11));
    }
    else
    {
         clcd_print("Wash",LINE1(11));
    }
    clcd_putch((min/10)+'0',LINE2(6));
    clcd_putch((min%10)+'0',LINE2(7));
    clcd_putch(':',LINE2(8));
    clcd_putch((sec/10)+'0',LINE2(9));
    clcd_putch((sec%10)+'0',LINE2(10));
    if(sec==0 && min==0) //when time up
    { 
        TMR2ON = OFF;//stop the timer
        FAN = OFF;//fan is off
        DC_MOTOR=OFF;//turning off the motor
        BUZZER = ON;//alert to user using buzzer
        clear_screen();// erase all the previous message on screen
        clcd_print("Prog Completed",LINE1(0));
        clcd_print("Remove Clothes",LINE2(0));
        __delay_ms(1000);
        BUZZER=OFF;//turning off the buzzer
        operation_mode=WASHING_PROGRAM_DISPLAY;
        reset_mode=WASHING_PROGRAM_DISPLAY_RESET;
        clear_screen();// erase all the previous message on screen
    }
}

