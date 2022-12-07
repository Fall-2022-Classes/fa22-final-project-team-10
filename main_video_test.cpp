/*****************************************************************//**
 * @file main_video_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "vga_core.h"
#include "sseg_core.h"
//////
#include "sseg_core.h"
#include "ps2_core.h"
#include "ddfs_core.h"
#include "adsr_core.h"
#include "spi_core.h"
//////



////////////////Final Project///////////////////
void ps2_move(Ps2Core *ps2_p, SpriteCore *ghost_p) {
   int id;
   int lbtn, rbtn, xmov, ymov;
   id = ps2_p->init();
   uart.disp("\n\rPS2 device (1-keyboard / 2-mouse): ");
   if (ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov)) {
               uart.disp("[");
               uart.disp(lbtn);
               uart.disp(", ");
               uart.disp(rbtn);
               uart.disp(", ");
               uart.disp(xmov);
               uart.disp(", ");
               uart.disp(ymov);
               uart.disp("] \r\n");

            }   // end get_mouse_activitiy()
   uart.disp("\n\r");

   ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov);
   uart.disp("\n\rExit PS2 test \n\r");
   uart.disp(lbtn);

   static int x = 100, y = 100;
   ghost_p->move_xy(x, y);

}

void ghost_init(SpriteCore *ghost_p){
	ghost_p->wr_ctrl(0x04);//animated
}

void ps2_check(Ps2Core *ps2_p, SpriteCore *ghost_p) {
   int id;
   int lbtn, rbtn, xmov, ymov;
   char ch;
   unsigned long last;


   id = ps2_p->init();

   last = now_ms();
   do {
      if (id == 2) {  // mouse
         if (ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov)) {

            last = now_ms();
            static int x=100, y=100;
            x+=xmov;
            y-=ymov;
            ghost_p->move_xy(x, y);
            uart.disp(x);
            uart.disp("\r\n");
         }   // end get_mouse_activitiy()
      } else {
         if (ps2_p->get_kb_ch(&ch)) {
            uart.disp(ch);
            uart.disp(" ");
            last = now_ms();
         } // end get_kb_ch()
      }  // end id==2
   } while (now_ms() - last < 5000);
   uart.disp("\n\rExit PS2 test \n\r");

}

void adsr_check(AdsrCore *adsr_p, GpoCore *led_p, GpiCore *sw_p) {
   const int melody[] = { 0, 2, 4, 5, 7, 9, 11 };
   int i, oct;

   adsr_p->init();
   // no adsr envelop and  play one octave
   adsr_p->bypass();
   for (i = 0; i < 7; i++) {
      led_p->write(bit(i));
      adsr_p->play_note(melody[i], 3, 500);
      sleep_ms(500);
   }
   adsr_p->abort();
   sleep_ms(1000);
   // set and enable adsr envelop
   // play 4 octaves
   adsr_p->select_env(sw_p->read());
   for (oct = 3; oct < 6; oct++) {
      for (i = 0; i < 7; i++) {
         led_p->write(bit(i));
         adsr_p->play_note(melody[i], oct, 500);
         sleep_ms(500);
      }
   }
   led_p->write(0);
   // test duration
   sleep_ms(1000);
   for (i = 0; i < 4; i++) {
      adsr_p->play_note(0, 4, 500 * i);
      sleep_ms(500 * i + 1000);
   }
}

void osd_fill(OsdCore *osd_p) {
   osd_p->set_color(0x08D, 0x001); // dark gray/green
   osd_p->bypass(0);
   osd_p->clr_screen();
   //80 x 29-30
   for (int i = 0; i < 79; i++) {
	   //osd_p->wr_char(1, 0, char, reverse);
//      osd_p->wr_char(8+i, 22, 64, 1);
//      osd_p->wr_char(0+i, 21+i, 64, 1);
	   osd_p->wr_char(i, 24, 64, 1);
      //sleep_ms(100);
   }
   sleep_ms(3000);
}

void screen_fill(OsdCore *osd_p, int color) {
   osd_p->set_color(color, 0x001); // light blue
   osd_p->bypass(0);

   //80 x 29-30
   for(int x = 0; x < 79; x++) {
	   for (int y = 0; y < 29; y++){
		   osd_p->wr_char(x, y, 64, 1);
	   }
   }
}

////////////////////////////////////////////////

// external core instantiation
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
FrameCore frame(FRAME_BASE);
GpvCore bar(get_sprite_addr(BRIDGE_BASE, V7_BAR));
GpvCore gray(get_sprite_addr(BRIDGE_BASE, V6_GRAY));
SpriteCore ghost(get_sprite_addr(BRIDGE_BASE, V3_GHOST), 1024);
SpriteCore mouse(get_sprite_addr(BRIDGE_BASE, V1_MOUSE), 1024);
OsdCore osd(get_sprite_addr(BRIDGE_BASE, V2_OSD));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));

//int main() {
//   while (1) {
//      test_start(&led);
//      // bypass all cores
//      frame.bypass(1);
//      bar.bypass(1);
//      gray.bypass(1);
//      ghost.bypass(1);
//      osd.bypass(1);
//      mouse.bypass(1);
//      sleep_ms(3000);
//
//      // enable cores one by one
//      frame_check(&frame);
//      bar_check(&bar);
//      gray_check(&gray);
//      ghost_check(&ghost);
//      osd_check(&osd);
//      mouse_check(&mouse);
//      while (sw.read(0)) {
//         // test composition with different overlays if sw(0) is 1
//         mouse.bypass(sw.read(1));
//         osd.bypass(sw.read(2));
//         ghost.bypass(sw.read(3));
//         gray.bypass(sw.read(6));
//         bar.bypass(sw.read(7));
//         frame.bypass(sw.read(8));
//         // set osd background color to transparent
//         osd.set_color(0x0f0, sw.read(9));
//         // set color/animation of ghost sprite
//         ghost.wr_ctrl(sw.read() >> 11);
//      } //while
//   } // while
//} //main

SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
DdfsCore ddfs(get_slot_addr(BRIDGE_BASE, S12_DDFS));
AdsrCore adsr(get_slot_addr(BRIDGE_BASE, S13_ADSR), &ddfs);

int test_main(){
	int time = now_ms();
	uart.disp(time);
	int random = rand();
	ghost_init(&ghost);
	time = now_us();
	uart.disp(time);
	while(1){
//		bar_check(&bar);
//		ps2_move(&ps2, &ghost);
//		osd_check(&osd);
//		ps2_check(&ps2, &ghost);
//		frame_check(&frame);
//		sleep_ms(1000);
//		time = now_us();
//		uart.disp(time);
//		uart.disp("\r\n");
//		bar_check(&bar);
		// 0x08D light blue, E12 red, 1D4 green, takes ~2332us
		screen_fill(&osd, 0x08D);
		sleep_ms(2000);
		int t_start = now_us();
		screen_fill(&osd, 0xE12);
		int t_end = now_us() - t_start;
		uart.disp("\r\n");
		uart.disp(t_end);
		sleep_ms(2000);
		screen_fill(&osd, 0x1D4);
		sleep_ms(2000);
//		adsr_check(&adsr, &led, &sw);
	}
}

void displaySeg(SsegCore *sseg, int n){
	//initial values
	//sseg->write_1ptn(0x7F, 2);
	sseg->set_dp(0);

	int d1,d2,d3,d4;
	d1 = sseg->h2s(n%10);
	d2 = sseg->h2s(n/10%10);
	d3 = sseg->h2s(n/100%10);
	d4 = sseg->h2s(n/1000);
	sseg->write_1ptn(d1,0);
	sseg->write_1ptn(d2,1);
	sseg->write_1ptn(d3,2);
	sseg->write_1ptn(d4,3);

}

int tapMap(double diff) {
	int int_diff = (int)(diff * 10);
	int num = 1;

	for(int i = 0; i < int_diff; i++){
		num*=2;
	}
	return num - 1;
}

int accMap_new(double x, double y, double z){
	double total = abs(x) + abs(y) + abs(z);
	static double prev_total;
	double diff;

	if(total - prev_total > 0.1){
//		uart.disp("\n\r");
		diff = total - prev_total;
//		uart.disp(diff);
		prev_total = total;
		return tapMap(diff);
	}

	prev_total = total;
	//(2^n)-1 to generate 1, 11, 111, ...
//	uart.disp("\n\r");
//	uart.disp(0);
	return tapMap(0);// >> 8;
}

double tap_sensor(SpiCore *spi_p, GpoCore *led) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t PART_ID_REG = 0x02;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   float x, y, z;
   int id;

   spi_p->set_freq(400000);
   spi_p->set_mode(0, 0);
   // check part id
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(PART_ID_REG);  // part id address
   id = (int) spi_p->transfer(0x00);
   spi_p->deassert_ss(0);

   // read 8-bit x/y/z g values once
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //
   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   x = (float) xraw / raw_max;
   y = (float) yraw / raw_max;
   z = (float) zraw / raw_max;

   double acc = accMap_new(x, y, z);
   //sleep_ms(25);
   led->write(acc);

   return acc;
}

int tap_react(SpiCore *spi_p, GpoCore *led_p){
	int time1, total_time = 0, round = 0;
	int state = 0;
	while(state!=4){

		if(state == 0){
			//waiting state
			uart.disp("waiting for input");
			if(tap_sensor(spi_p, led_p)>0.01){
				state = 1;
			}
		}
		else if(state == 1){
			//ready state
			screen_fill(&osd, 0x08D); //blue
			sleep_ms(2500);
			state = 2;
		}
		else if(state == 2){
			screen_fill(&osd, 0xE12); //red
			int random = (rand() % 4000) + 2000; //sleeps for rand num between 2 - 5 seconds
			sleep_ms(random);
			screen_fill(&osd, 0x1D4); //green
			state = 3;
			time1 = now_ms();
		}
		else if(state == 3){
			if(tap_sensor(spi_p, led_p)>0.01){
				int react_time = now_ms() - time1;
				uart.disp("reaction time: ");
				uart.disp(react_time);
				uart.disp("ms\r\n");
//				displaySeg(&sseg, react_time);
				total_time += react_time;

				uart.disp("round: ");
				uart.disp(round);
				uart.disp("\r\n");
				round += 1;
				if(round == 5){
					int avg_time = total_time/5;
					uart.disp("average reaction time: ");
					uart.disp(avg_time);
					uart.disp("ms\r\n");
//					displaySeg(&sseg, react_time);
					screen_fill(&osd, 0x000); // black
					state = 4;
					//done = 1;
					return 0;
				}
				else{
					state = 1;
				}
			}
		}
	}
}

int mouse_react(Ps2Core *ps2_p){
	int lbtn, rbtn, xmov, ymov;
	int time1, total_time = 0, round = 0;
	int done = 0;
	int state = 0;
	while(state!=4){
		ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov);
		if(state == 0){
			//waiting state
			//sleep_ms(500);
			uart.disp("waiting for input");
			if(lbtn == 1){
				state = 1;
			}
		}
		else if(state == 1){
			//ready state
			screen_fill(&osd, 0x08D); //blue
			sleep_ms(2500);
			state = 2;
		}
		else if(state == 2){
			screen_fill(&osd, 0xE12); //red
			int random = (rand() % 4000) + 2000; //sleeps for rand num between 2 - 5 seconds
			sleep_ms(random);
			screen_fill(&osd, 0x1D4); //green
			state = 3;
			time1 = now_ms();
		}
		else if(state == 3){
			ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov);
			if(lbtn == 1){
				int react_time = now_ms() - time1;
				uart.disp("reaction time: ");
				uart.disp(react_time);
				uart.disp("ms\r\n");
//				displaySeg(&sseg, react_time);
				total_time += react_time;

				uart.disp("round: ");
				uart.disp(round);
				uart.disp("\r\n");
				round += 1;
				if(round == 5){
					int avg_time = total_time/5;
					uart.disp("average reaction time: ");
					uart.disp(avg_time);
					uart.disp("ms\r\n");
//					displaySeg(&sseg, react_time);
					screen_fill(&osd, 0x000); // black
					state = 4;
					//done = 1;
					sleep_ms(500);
					tap_sensor(&spi, &led);
					return 0;
				}
				else{
					state = 1;
				}
			}
		}
	}
}

void ghost_check_target_delete(SpriteCore *ghost_p) {
   int x, y;

   // slowly move mouse pointer
   ghost_p->bypass(0);
   ghost_p->wr_ctrl(0x1c);  //animation; blue ghost
   x = 0;
   y = 100;

   for (int i = 0; i < 156; i++) {
      ghost_p->move_xy(x, y);
      sleep_ms(100);
      x = x + 4;
      if (i == 80) {
         // change to red ghost half way
         ghost_p->wr_ctrl(0x04);
      }
   }
   sleep_ms(3000);
}

void sprite_target(SpriteCore *ghost_p, OsdCore *osd_p){
	int x, y;

	   //ghost_p->bypass(0);
	   ghost_p->wr_ctrl(0x1c);  //animation; blue ghost
	   x = 0;
	   y = 100;

	   //hit target counter
	   int hit_target = 0;

	   //target tiles
	   osd_p->clr_screen();
	   osd_p->wr_char(30, 6, 64, 1);
	   osd_p->wr_char(50, 6, 64, 1);
	   osd_p->wr_char(60, 6, 64, 1);
	   osd_p->set_color(0xF00, 0x000);

	   //80 x 29-30
	   //targets at 210 360 460

	   int step = 50; //controls speed

	   for (int i = 0; i < 480 * step; i++) {
	      ghost_p->move_xy(x, y);
	      //sleep_ms(10);
	      if( i%step == 0){
	    	  x = x + 1;
		      uart.disp("\r\n");
		      uart.disp(x);
	      }

	      if((tap_sensor(&spi, &led)>0.3)&& (1)){
	    	  hit_target += 1;
	    	  sleep_ms(10);
	      }
	   }

	   osd_p->wr_char(30, 13, 64, 1);
	   osd_p->wr_char(50, 13, 64, 1);
	   osd_p->wr_char(60, 13, 64, 1);

	   x = 0;
	   y = 200;
	   step = 15;

	   for (int i = 0; i < 480 * step; i++) {
	   	      ghost_p->move_xy(x, y);
	   	      //sleep_ms(10);
	   	      if( i%step == 0){
	   	    	  x = x + 1;
	   	      }


	   	      if(tap_sensor(&spi, &led)>0.01){
	   	    	  hit_target += 1;
	   	      }
	   	   }

	   osd_p->wr_char(30, 19, 64, 1);
	   osd_p->wr_char(50, 19, 64, 1);
	   osd_p->wr_char(60, 19, 64, 1);

	   x = 0;
	   y = 300;
	   step = 10;

	   for (int i = 0; i < 480 * step; i++) {
	   	      ghost_p->move_xy(x, y);
	   	      //sleep_ms(10);
	   	      if( i%step == 0){
	   	    	  x = x + 1;
	   	      }


	   	      if(tap_sensor(&spi, &led)>0.01){
	   	    	  hit_target += 1;
	   	      }
	   	   }

	   uart.disp("\r\nTargets Hit: ");
	   uart.disp(hit_target);
}

void mode(int sw0, int sw1, int sw2, Ps2Core *ps2_p){
//	int lbtn, rbtn, xmov, ymov;
//	ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov);
//	screen_fill(&osd, 0x000);
	if(sw0){
		mouse_react(&ps2);
	}
	else if(sw1 && (tap_sensor(&spi, &led)>0.01)){
		tap_react(&spi, &led);
//		tap_sensor(&spi, &led);
	}
	else if(sw2 && (tap_sensor(&spi, &led)>0.01)){
		sprite_target(&ghost, &osd);
	}
	else{
		screen_fill(&osd, 0x000);
	}
}

int main(){
	ghost_init(&ghost);
	mouse.bypass(1);
	screen_fill(&osd, 0x000);
//	displaySeg(&sseg, 0);
	while(1){
		mode(sw.read(0), sw.read(1), sw.read(2), &ps2);
//		ps2_check(&ps2, &ghost);
	}
}
