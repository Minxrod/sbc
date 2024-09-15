#include "test_util.h"

#include "subsystem/graphics.h"
#include "system.h"

int test_graphics(void){
	// Simple construction test
	{
		struct graphics g = {0};
		init_graphics(&g);
		
		ASSERT(g.info[0].drawpage == 0, "[graphics] Screen 0 default draw page 0");
		ASSERT(g.info[0].displaypage == 0, "[graphics] Screen 0 default display page 0");
		ASSERT(g.info[0].prio == 3, "[graphics] Screen 0 default priorty 3");
		ASSERT(g.info[1].drawpage == 1, "[graphics] Screen 1 default draw page 1");
		ASSERT(g.info[1].displaypage == 1, "[graphics] Screen 1 default display page 1");
		ASSERT(g.info[1].prio == 3, "[graphics] Screen 1 default priorty 3");
	}
	
	// Simple GPAGE
	{
		struct ptc* p = run_code("GPAGE 1\r");
		
		ASSERT(p->graphics.screen == 1, "[graphics] Page set to 1");
		
		free_code(p);
	}
	
	// Simple GPSET
	{
		struct ptc* p = run_code("GPSET 0,0,1\rGPSET 8,8,2\rGPSET 16,16,3\r");
		
		ASSERT(p->res.grp[0][0] == 1, "[graphics] 0,0 pixel set to 1");
		ASSERT(p->res.grp[0][9*64] == 2, "[graphics] First pixel of small block 9 set to 2");
		ASSERT(p->res.grp[0][18*64] == 3, "[graphics] First pixel of small block 18 set to 3");
		
		free_code(p);
	}
	
	// GPSET with simple GPAGE
	{
		struct ptc* p = run_code("GPAGE 1\rGPSET 0,0,1\rGPSET 8,8,2\rGPAGE 0\rGPSET 16,16,3\r");
		
		ASSERT(p->res.grp[1][0] == 1, "[graphics] GRP1 0,0 pixel set to 1");
		ASSERT(p->res.grp[1][9*64] == 2, "[graphics] GRP1 pixel of small block 9 set to 2");
		ASSERT(p->res.grp[0][18*64] == 3, "[graphics] GRP0 pixel of small block 18 set to 3");
		
		free_code(p);
	}
	
	// GLINE tests
	{
		struct ptc* p = run_code("GLINE 0,0,7,0,1\rGLINE 0,0,7,7,2\rGLINE 0,0,0,7,3\r");
		
		const u8 match[64] = {
			3,1,1,1,1,1,1,1,
			3,2,0,0,0,0,0,0,
			3,0,2,0,0,0,0,0,
			3,0,0,2,0,0,0,0,
			3,0,0,0,2,0,0,0,
			3,0,0,0,0,2,0,0,
			3,0,0,0,0,0,2,0,
			3,0,0,0,0,0,0,2,
		};
		
		for (int y = 0; y < 8; ++y){
			for (int x = 0; x < 8; ++x){
				iprintf("%d ", p->res.grp[0][x+8*y]);
				ASSERT(p->res.grp[0][x+8*y] == match[x+8*y], "[graphics] GLINE test");
			}
			iprintf("\n");
		}
		
		free_code(p);
	}
	
	// GLINE tests
	{
		struct ptc* p = run_code("GLINE 3,7,0,6,1\rGLINE 1,2,3,4,2\rGLINE 0,7,6,3,3\r");
		
		const u8 match[64] = {
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,2,0,0,0,0,0,0,
			0,0,2,0,0,0,3,0,
			0,0,0,2,3,3,0,0,
			0,0,0,3,0,0,0,0,
			1,3,3,0,0,0,0,0,
			3,0,1,1,0,0,0,0,
		};
		
		for (int y = 0; y < 8; ++y){
			for (int x = 0; x < 8; ++x){
				iprintf("%d ", p->res.grp[0][x+8*y]);
				ASSERT(p->res.grp[0][x+8*y] == match[x+8*y], "[graphics] GLINE test");
			}
			iprintf("\n");
		}
		
		free_code(p);
	}
	
	// GSPOIT test
	{
		struct ptc* p = run_code(
			"GPSET 0,0,3\rA=GSPOIT(0,0)\r"
			"GPSET 46,72,81\rB=GSPOIT(46,72)\r"
			"GPSET 0,0,4\rC=GSPOIT(0,0)\r"
			"GPSET 256,192,72\rD=GSPOIT(256,192)\r"
			"E=GSPOIT(0,0,0)\r"
			"F=GSPOIT(1,0,0)\r"
		);
		
		CHECK_VAR_INT("A",3);
		CHECK_VAR_INT("B",81);
		CHECK_VAR_INT("C",4);
		CHECK_VAR_INT("D",-1);
		CHECK_VAR_INT("E",4);
		CHECK_VAR_INT("F",0); // different page

		free_code(p);
	}
	
	// GCOPY
#define GCOPY_TEST(dx,dy,msg) do {\
		struct ptc* p = run_code(\
			"FOR X=0 TO 3\rFOR Y=0 TO 3\rGPSET 1+X,1+Y,X+4*Y\rNEXT\rNEXT\r"\
			"GCOPY 0,1,1,4,4,"#dx","#dy",1\r"\
		);\
		for (int x = dx; x < dx+4; ++x){\
			for (int y = dy; y < dy+4; ++y){\
				ASSERT(p->res.grp[0][grp_index(x,y)] == (x-dx)+4*(y-dy), "[gcopy] "msg);\
			}\
		}\
		free_code(p);\
	} while(0)
	// GCOPY overlap tests
	{
		GCOPY_TEST(0,0,"up left");
		GCOPY_TEST(0,1,"up");
		GCOPY_TEST(0,2,"up right");
		GCOPY_TEST(1,0,"left");
		GCOPY_TEST(1,1,"self copy");
		GCOPY_TEST(1,2,"right");
		GCOPY_TEST(2,0,"down left");
		GCOPY_TEST(2,1,"down");
		GCOPY_TEST(2,2,"down right");
	}
	
	// GLINE tests
/*	{
		struct ptc* p = run_code(
			"FOR I=0 TO 3\r"
			" X1=I*46%256\r"
			" X2=I*64%256\r"
			" Y1=I*2%192\r"
			" Y2=(I*7+85)%192\r"
			" GLINE X1,Y1,X2,Y2,I\r"
			"NEXT\r"
			"A=0\r"
			"FOR I=0 TO 255\r"
			" FOR J=0 TO 191\r"
			"  A=(A+(I+J)*GSPOIT(I,J))%65536\r"
			" NEXT\r"
			"NEXT\r"
		);
		
		iprintf("A=%d\n", test_var(&p->vars, "A", VAR_NUMBER)->value.number >> FIXPOINT);
		CHECK_VAR_INT("A",52475);
		
		free_code(p);
	}*/
	
	// GPUTCHR palette update
	{
		struct ptc* p = run_code(
			"GPUTCHR 0,0,\"SPU1\",0,2,1\r"
			"COLREAD(\"SP\",36),A,B,C\r"
			"COLREAD(\"GRP\",36),D,E,F\r"
			"CHK=(A==D)+(B==C)+(E==F)\r"
		);

		CHECK_VAR_INT("CHK",3);

		free_code(p);
	}

	SUCCESS("test_graphics success");
}
