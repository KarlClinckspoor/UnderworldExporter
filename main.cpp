#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "textures.h"
#include "gameobjects.h"
#include "tilemap.h"
#include "d3darkmod.h"
#include "SourceEngine.h"
#include "scripting.h"
#include "materials.h"
#include "main.h"
#include "asciimode.h"
#include "gamestrings.h"
#include "gamegraphics.h"
#include "Conversations.h"
//#include "fbxExport.h"
#include "Unity.h"
#include "hacking.h"


using namespace std;

texture *textureMasters;
objectMaster *objectMasters;
FILE *MAPFILE;

extern int levelNo;
extern int GAME;
void ExtractSkills();

int main()
	{
	int game = -1;
	int mode = -1;
	int gamefile = -1;
	int graphics_file_no = -1;
	int graphics_mode;
	int BitMapSize = 32;
	short panels = 0;//Panels.gr are a special case for extraction
	char Graphics_File[255];
	char Graphics_Pal[255];
	char FileToOpen[255];//Generic use file
	char FileToOpen2[255];//Generic use file
	char TempOutFileName[255];
	char auxPalPath[255];
	char path_target_platform[100];
	char OutFileName[255];
	char GameFilePath[255];
	char fileAssoc[255];
	char fileCrit[255];
	int critPal = 0;
	int bytark = 0;
	int BlocksToRepack = 80;//UW2 blocks to repack
	int useTGA = 0;
	FILE *f = NULL;


	/*
	if ((f = fopen("c:\\games\\uw2\\data\\lev.ark", "r")) == NULL)
		{
		printf("Could not open specified file\n");
		return 0;
		}

	long fileSize = getFileSize(f);
	unsigned char *tmp_ark = new unsigned char[fileSize];
	fread(tmp_ark, fileSize, 1, f);
	fclose(f);
	int datalen = 0;
	get_rwops_uw2dec(tmp_ark, 240, &datalen);

	return 0;*/

	if ((f = fopen("gamepaths.txt", "r")) == NULL)
		{
		printf("Could not open specified file\n");
		return 0;
		}

	fgets(path_uw0, 100, f);
	int ln = strlen(path_uw0) - 1;
	if (path_uw0[ln] == '\n')
		path_uw0[ln] = '\0';

	fgets(path_uw1, 100, f);
	ln = strlen(path_uw1) - 1;
	if (path_uw1[ln] == '\n')
		path_uw1[ln] = '\0';

	fgets(path_uw2, 100, f);
	ln = strlen(path_uw2) - 1;
	if (path_uw2[ln] == '\n')
		path_uw2[ln] = '\0';

	fgets(path_shock, 100, f);
	ln = strlen(path_shock) - 1;
	if (path_shock[ln] == '\n')
		path_shock[ln] = '\0';

	fgets(path_target_platform, 100, f);
	ln = strlen(path_target_platform) - 1;
	if (path_target_platform[ln] == '\n')
		path_target_platform[ln] = '\0';

	fclose(f);
	const char *uw_game_files[6];
	uw_game_files[0] = "Data\\LEV.ARK";
	//uw_game_files[1] = "Save0\\lev.ark";
	uw_game_files[1] = "Save1\\lev.ark";
	uw_game_files[2] = "Save2\\lev.ark";
	uw_game_files[3] = "Save3\\lev.ark";
	uw_game_files[4] = "Save4\\lev.ark";
	uw_game_files[5] = "data\\cnv.ark";	//UW2 ark files.

	const char *shock_game_files[9];
	shock_game_files[0] = "res\\Data\\archive.dat";
	shock_game_files[1] = "res\\Data\\SAVGAM00.dat";
	shock_game_files[2] = "res\\Data\\SAVGAM01.dat";
	shock_game_files[3] = "res\\Data\\SAVGAM02.dat";
	shock_game_files[4] = "res\\Data\\SAVGAM03.dat";
	shock_game_files[5] = "res\\Data\\SAVGAM04.dat";
	shock_game_files[6] = "res\\Data\\SAVGAM05.dat";
	shock_game_files[7] = "res\\Data\\SAVGAM06.dat";
	shock_game_files[8] = "res\\Data\\SAVGAM07.dat";


	const char *uw0_graphics_file[44];
	const char *uw1_graphics_file[45];
	const char *uw2_graphics_file[45];

	uw0_graphics_file[0] = "Data\\DF16.tr";// - Floor textures 16x16");
	uw0_graphics_file[1] = "Data\\DF32.tr";// - Floor textures 32x32\n");
	uw0_graphics_file[2] = "Data\\DW16.tr";// - Wall textures 16x16");
	uw0_graphics_file[3] = "Data\\DW64.tr";// - Wall textures 64x64\n");
	uw0_graphics_file[4] = "Data\\3DWIN.GR";
	uw0_graphics_file[5] = "Data\\ANIMO.GR";
	uw0_graphics_file[6] = "Data\\ARMOR_F.GR";
	uw0_graphics_file[7] = "Data\\ARMOR_M.GR";
	uw0_graphics_file[8] = "Data\\BODIES.GR";
	uw0_graphics_file[9] = "Data\\BUTTONS.GR";
	uw0_graphics_file[10] = "Data\\CHAINS.GR";
	uw0_graphics_file[11] = "Data\\CHARHEAD.GR";
	uw0_graphics_file[12] = "Data\\CHRBTNS.GR";
	uw0_graphics_file[13] = "Data\\COMPASS.GR";
	uw0_graphics_file[14] = "Data\\CONVERSE.GR";
	uw0_graphics_file[15] = "Data\\CURSORS.GR";
	uw0_graphics_file[16] = "Data\\DOORS.GR";
	uw0_graphics_file[17] = "Data\\DRAGONS.GR";
	uw0_graphics_file[18] = "Data\\EYES.GR";
	uw0_graphics_file[19] = "Data\\FLASKS.GR";
	uw0_graphics_file[20] = "Data\\GENHEAD.GR";
	uw0_graphics_file[21] = "Data\\HEADS.GR";
	uw0_graphics_file[22] = "Data\\INV.GR";
	uw0_graphics_file[23] = "Data\\LFTI.GR";
	uw0_graphics_file[24] = "Data\\OBJECTS.GR";
	uw0_graphics_file[25] = "Data\\OPBTN.GR";
	uw0_graphics_file[26] = "Data\\OPTB.GR";
	uw0_graphics_file[27] = "Data\\OPTBTNS.GR";
	uw0_graphics_file[28] = "Data\\PANELS.GR";
	uw0_graphics_file[29] = "Data\\POWER.GR";
	uw0_graphics_file[30] = "Data\\QUEST.GR";
	uw0_graphics_file[31] = "Data\\SCRLEDGE.GR";
	uw0_graphics_file[32] = "Data\\SPELLS.GR";
	uw0_graphics_file[33] = "Data\\TMFLAT.GR";
	uw0_graphics_file[34] = "Data\\TMOBJ.GR";
	uw0_graphics_file[35] = "Data\\WEAPONS.GR";
	uw0_graphics_file[36] = "Data\\BLNKMAP.BYT";
	uw0_graphics_file[37] = "Data\\CHARGEN.BYT";
	uw0_graphics_file[38] = "Data\\CONV.BYT";
	uw0_graphics_file[39] = "Data\\DMAIN.BYT";
	uw0_graphics_file[40] = "Data\\OPSCR.BYT";
	uw0_graphics_file[41] = "Data\\PRESD.BYT";
	uw0_graphics_file[42] = "Data\\WIN1.BYT";
	uw0_graphics_file[43] = "Data\\WIN2.BYT";


	uw1_graphics_file[0] = "Data\\F16.tr";// - Floor textures 16x16");
	uw1_graphics_file[1] = "Data\\F32.tr";// - Floor textures 32x32\n");
	uw1_graphics_file[2] = "Data\\W16.tr";// - Wall textures 16x16");
	uw1_graphics_file[3] = "Data\\W64.tr";// - Wall textures 64x64\n");
	uw1_graphics_file[4] = "Data\\3DWIN.GR";
	uw1_graphics_file[5] = "Data\\ANIMO.GR";
	uw1_graphics_file[6] = "Data\\ARMOR_F.GR";
	uw1_graphics_file[7] = "Data\\ARMOR_M.GR";
	uw1_graphics_file[8] = "Data\\BODIES.GR";
	uw1_graphics_file[9] = "Data\\BUTTONS.GR";
	uw1_graphics_file[10] = "Data\\CHAINS.GR";
	uw1_graphics_file[11] = "Data\\CHARHEAD.GR";
	uw1_graphics_file[12] = "Data\\CHRBTNS.GR";
	uw1_graphics_file[13] = "Data\\COMPASS.GR";
	uw1_graphics_file[14] = "Data\\CONVERSE.GR";
	uw1_graphics_file[15] = "Data\\CURSORS.GR";
	uw1_graphics_file[16] = "Data\\DOORS.GR";
	uw1_graphics_file[17] = "Data\\DRAGONS.GR";
	uw1_graphics_file[18] = "Data\\EYES.GR";
	uw1_graphics_file[19] = "Data\\FLASKS.GR";
	uw1_graphics_file[20] = "Data\\GENHEAD.GR";
	uw1_graphics_file[21] = "Data\\HEADS.GR";
	uw1_graphics_file[22] = "Data\\INV.GR";
	uw1_graphics_file[23] = "Data\\LFTI.GR";
	uw1_graphics_file[24] = "Data\\OBJECTS.GR";
	uw1_graphics_file[25] = "Data\\OPBTN.GR";//Pal #6. alpha off.
	uw1_graphics_file[26] = "Data\\OPTB.GR";
	uw1_graphics_file[27] = "Data\\OPTBTNS.GR";
	uw1_graphics_file[28] = "Data\\PANELS.GR";
	uw1_graphics_file[29] = "Data\\POWER.GR";
	uw1_graphics_file[30] = "Data\\QUEST.GR";
	uw1_graphics_file[31] = "Data\\SCRLEDGE.GR";
	uw1_graphics_file[32] = "Data\\SPELLS.GR";
	uw1_graphics_file[33] = "Data\\TMFLAT.GR";
	uw1_graphics_file[34] = "Data\\TMOBJ.GR";
	uw1_graphics_file[35] = "Data\\WEAPONS.GR";
	uw1_graphics_file[36] = "Data\\BLNKMAP.BYT";//3
	uw1_graphics_file[37] = "Data\\CHARGEN.BYT";//palette no 9
	uw1_graphics_file[38] = "Data\\CONV.BYT";
	uw1_graphics_file[39] = "Data\\MAIN.BYT";
	uw1_graphics_file[40] = "Data\\OPSCR.BYT";//palette no 6
	uw1_graphics_file[41] = "Data\\PRES1.BYT";//Palette no 15
	uw1_graphics_file[42] = "Data\\PRES2.BYT";//Palette no 21
	uw1_graphics_file[43] = "Data\\WIN1.BYT";
	uw1_graphics_file[44] = "Data\\WIN2.BYT";//22

	uw2_graphics_file[0] = "Data\\T64.tr";//uw2 textures
	uw2_graphics_file[1] = "Data\\3DWIN.GR";
	uw2_graphics_file[2] = "Data\\ANIMO.GR";
	uw2_graphics_file[3] = "Data\\ARMOR_F.GR";
	uw2_graphics_file[4] = "Data\\ARMOR_M.GR";
	uw2_graphics_file[5] = "Data\\BODIES.GR";
	uw2_graphics_file[6] = "Data\\BUTTONS.GR";
	uw2_graphics_file[7] = "Data\\CHAINS.GR";
	uw2_graphics_file[8] = "Data\\CHARHEAD.GR";
	uw2_graphics_file[9] = "Data\\CHRBTNS.GR";
	uw2_graphics_file[10] = "Data\\COMPASS.GR";
	uw2_graphics_file[11] = "Data\\CONVERSE.GR";
	uw2_graphics_file[12] = "Data\\CURSORS.GR";
	uw2_graphics_file[13] = "Data\\DOORS.GR";
	uw2_graphics_file[14] = "Data\\DRAGONS.GR";
	uw2_graphics_file[15] = "Data\\EYES.GR";
	uw2_graphics_file[16] = "Data\\FLASKS.GR";
	uw2_graphics_file[17] = "Data\\GEMPT.GR";
	uw2_graphics_file[18] = "Data\\GENHEAD.GR";
	uw2_graphics_file[19] = "Data\\GHED.GR";
	uw2_graphics_file[20] = "Data\\HEADS.GR";
	uw2_graphics_file[21] = "Data\\INV.GR";
	uw2_graphics_file[22] = "Data\\LFTI.GR";
	uw2_graphics_file[23] = "Data\\OBJECTS.GR";
	uw2_graphics_file[24] = "Data\\OPBTN.GR";
	uw2_graphics_file[25] = "Data\\OPTB.GR";
	uw2_graphics_file[26] = "Data\\OPTBTNS.GR";
	uw2_graphics_file[27] = "Data\\PANELS.GR";
	uw2_graphics_file[28] = "Data\\POWER.GR";
	uw2_graphics_file[29] = "Data\\QUESTION.GR";
	uw2_graphics_file[30] = "Data\\SCRLEDGE.GR";
	uw2_graphics_file[31] = "Data\\SPELLS.GR";
	uw2_graphics_file[32] = "Data\\TMFLAT.GR";
	uw2_graphics_file[33] = "Data\\TMOBJ.GR";
	uw2_graphics_file[34] = "Data\\VIEWS.GR";
	uw2_graphics_file[35] = "Data\\WEAP.GR";
	uw2_graphics_file[36] = "Data\\BYT.ARK";
	uw2_graphics_file[37] = "Cuts\\LBACK000.BYT";
	uw2_graphics_file[38] = "Cuts\\LBACK001.BYT";
	uw2_graphics_file[39] = "Cuts\\LBACK002.BYT";
	uw2_graphics_file[40] = "Cuts\\LBACK003.BYT";
	uw2_graphics_file[41] = "Cuts\\LBACK004.BYT";
	uw2_graphics_file[42] = "Cuts\\LBACK005.BYT";
	uw2_graphics_file[43] = "Cuts\\LBACK006.BYT";
	uw2_graphics_file[44] = "Cuts\\LBACK007.BYT";

	const char *uw1_cuts_file[47];
	uw1_cuts_file[0] = "cuts\\cs000.n01";//   black screen
	uw1_cuts_file[1] = "cuts\\cs000.n02";// * garamon in swirling air
	uw1_cuts_file[2] = "cuts\\cs000.n03";//   garamon talking
	uw1_cuts_file[3] = "cuts\\cs000.n04";// * garamon talking
	uw1_cuts_file[4] = "cuts\\cs000.n05";// * garamon talking
	uw1_cuts_file[5] = "cuts\\cs000.n06";// * garamon in swirling air
	uw1_cuts_file[6] = "cuts\\cs000.n07";//   garamon appearing
	uw1_cuts_file[7] = "cuts\\cs000.n10";//   intro w / tyball stealing princess, troll and guards etc.
	uw1_cuts_file[8] = "cuts\\cs000.n11";//   almric talking, on throne
	uw1_cuts_file[9] = "cuts\\cs000.n12";//   almric talking, closeup
	uw1_cuts_file[10] = "cuts\\cs000.n15";//   guard talking
	uw1_cuts_file[11] = "cuts\\cs000.n16";// * guard talking
	uw1_cuts_file[12] = "cuts\\cs000.n17";// * guard talking
	uw1_cuts_file[13] = "cuts\\cs000.n20";//   mountain scene, avatar taken to the abyss
	uw1_cuts_file[14] = "cuts\\cs000.n21";// * mountain scene
	uw1_cuts_file[15] = "cuts\\cs000.n22";//   abyss doors closed, from outside
	uw1_cuts_file[16] = "cuts\\cs000.n23";//   doors closed, from inside, w / avatar
	uw1_cuts_file[17] = "cuts\\cs000.n24";//   guard talking, with purple background
	uw1_cuts_file[18] = "cuts\\cs000.n25";// * guard talking, with purple background
	uw1_cuts_file[19] = "cuts\\cs001.n01";//   ship approaching, abyss collapsing
	uw1_cuts_file[20] = "cuts\\cs001.n02";//   ship taking avatar on board
	uw1_cuts_file[21] = "cuts\\cs001.n03";//  almric talking, on ship
	uw1_cuts_file[22] = "cuts\\cs001.n04";//   almric talking, on ship
	uw1_cuts_file[23] = "cuts\\cs001.n05";// * almric talking, on ship, birds in background
	uw1_cuts_file[24] = "cuts\\cs001.n06";//   arial talking
	uw1_cuts_file[25] = "cuts\\cs001.n07";//   arial talking
	uw1_cuts_file[26] = "cuts\\cs001.n10";//   abyss collapsing, ship sails away
	uw1_cuts_file[27] = "cuts\\cs002.n01";//   dying tyball talking
	uw1_cuts_file[28] = "cuts\\cs002.n02";// * dying tyball talking
	uw1_cuts_file[29] = "cuts\\cs002.n03";// * dying tyball talking
	uw1_cuts_file[30] = "cuts\\cs002.n04";// * dying tyball, dying
	uw1_cuts_file[31] = "cuts\\cs003.n01";//   arial talking
	uw1_cuts_file[32] = "cuts\\cs003.n02";// * arial talking
	uw1_cuts_file[33] = "cuts\\cs011.n01";//   "ultima underworld the stygian abyss" splash screen anim
	uw1_cuts_file[34] = "cuts\\cs012.n01";//   acknowledgements
	uw1_cuts_file[35] = "cuts\\cs013.n01";//   goblet with letters "in"
	uw1_cuts_file[36] = "cuts\\cs014.n01";//   goblet with letters "sa"
	uw1_cuts_file[37] = "cuts\\cs015.n01";//   goblet with letters "hn"
	uw1_cuts_file[38] = "cuts\\cs400.n01";//   "look" graphics for windows to abyss volcano core
	uw1_cuts_file[39] = "cuts\\cs401.n01";//   grave stones
	uw1_cuts_file[40] = "cuts\\cs402.n01";//   death skulls w / silver sapling
	uw1_cuts_file[41] = "cuts\\cs403.n01";//   death skulls animation
	uw1_cuts_file[42] = "cuts\\cs403.n02";//   death skull end anim
	uw1_cuts_file[43] = "cuts\\cs404.n01";//   anvil graphics
	uw1_cuts_file[44] = "cuts\\cs410.n01";//   map piece showing some traps
	uw1_cuts_file[45] = "cuts\\cs000.n13";
	uw1_cuts_file[46] = "cuts\\cs000.n14";

	const char *uw1_cuts_out_file[47];
	uw1_cuts_out_file[0] = "cs000_n01";//   black screen
	uw1_cuts_out_file[1] = "cs000_n02";// * garamon in swirling air
	uw1_cuts_out_file[2] = "cs000_n03";//   garamon talking
	uw1_cuts_out_file[3] = "cs000_n04";// * garamon talking
	uw1_cuts_out_file[4] = "cs000_n05";// * garamon talking
	uw1_cuts_out_file[5] = "cs000_n06";// * garamon in swirling air
	uw1_cuts_out_file[6] = "cs000_n07";//   garamon appearing
	uw1_cuts_out_file[7] = "cs000_n10";//   intro w / tyball stealing princess, troll and guards etc_
	uw1_cuts_out_file[8] = "cs000_n11";//   almric talking, on throne
	uw1_cuts_out_file[9] = "cs000_n12";//   almric talking, closeup
	uw1_cuts_out_file[10] = "cs000_n15";//   guard talking
	uw1_cuts_out_file[11] = "cs000_n16";// * guard talking
	uw1_cuts_out_file[12] = "cs000_n17";// * guard talking
	uw1_cuts_out_file[13] = "cs000_n20";//   mountain scene, avatar taken to the abyss
	uw1_cuts_out_file[14] = "cs000_n21";// * mountain scene
	uw1_cuts_out_file[15] = "cs000_n22";//   abyss doors closed, from outside
	uw1_cuts_out_file[16] = "cs000_n23";//   doors closed, from inside, w / avatar
	uw1_cuts_out_file[17] = "cs000_n24";//   guard talking, with purple background
	uw1_cuts_out_file[18] = "cs000_n25";// * guard talking, with purple background
	uw1_cuts_out_file[19] = "cs001_n01";//   ship approaching, abyss collapsing
	uw1_cuts_out_file[20] = "cs001_n02";//   ship taking avatar on board
	uw1_cuts_out_file[21] = "cs001_n03";//  almric talking, on ship
	uw1_cuts_out_file[22] = "cs001_n04";//   almric talking, on ship
	uw1_cuts_out_file[23] = "cs001_n05";// * almric talking, on ship, birds in background
	uw1_cuts_out_file[24] = "cs001_n06";//   arial talking
	uw1_cuts_out_file[25] = "cs001_n07";//   arial talking
	uw1_cuts_out_file[26] = "cs001_n10";//   abyss collapsing, ship sails away
	uw1_cuts_out_file[27] = "cs002_n01";//   dying tyball talking
	uw1_cuts_out_file[28] = "cs002_n02";// * dying tyball talking
	uw1_cuts_out_file[29] = "cs002_n03";// * dying tyball talking
	uw1_cuts_out_file[30] = "cs002_n04";// * dying tyball, dying
	uw1_cuts_out_file[31] = "cs003_n01";//   arial talking
	uw1_cuts_out_file[32] = "cs003_n02";// * arial talking
	uw1_cuts_out_file[33] = "cs011_n01";//   "ultima underworld the stygian abyss" splash screen anim
	uw1_cuts_out_file[34] = "cs012_n01";//   acknowledgements
	uw1_cuts_out_file[35] = "cs013_n01";//   goblet with letters "in"
	uw1_cuts_out_file[36] = "cs014_n01";//   goblet with letters "sa"
	uw1_cuts_out_file[37] = "cs015_n01";//   goblet with letters "hn"
	uw1_cuts_out_file[38] = "cs400_n01";//   "look" graphics for windows to abyss volcano core
	uw1_cuts_out_file[39] = "cs401_n01";//   grave stones
	uw1_cuts_out_file[40] = "cs402_n01";//   death skulls w / silver sapling
	uw1_cuts_out_file[41] = "cs403_n01";//   death skulls animation
	uw1_cuts_out_file[42] = "cs403_n02";//   death skull end anim
	uw1_cuts_out_file[43] = "cs404_n01";//   anvil graphics
	uw1_cuts_out_file[44] = "cs410_n01";//   map piece showing some traps
	uw1_cuts_out_file[45] = "cs000_n13";//  ?
	uw1_cuts_out_file[46] = "cs000_n14";//   ?

	int uw1_cuts_alpha[47];
	uw1_cuts_alpha[0] = 0;
	uw1_cuts_alpha[1] = 0;
	uw1_cuts_alpha[2] = 0;
	uw1_cuts_alpha[3] = 0;
	uw1_cuts_alpha[4] = 0;
	uw1_cuts_alpha[5] = 0;
	uw1_cuts_alpha[6] = 0;
	uw1_cuts_alpha[7] = 0;
	uw1_cuts_alpha[8] = 0;
	uw1_cuts_alpha[9] = 0;
	uw1_cuts_alpha[10] = 0;
	uw1_cuts_alpha[11] = 0;
	uw1_cuts_alpha[12] = 0;
	uw1_cuts_alpha[13] = 0;
	uw1_cuts_alpha[14] = 0;
	uw1_cuts_alpha[15] = 0;
	uw1_cuts_alpha[16] = 0;
	uw1_cuts_alpha[17] = 0;
	uw1_cuts_alpha[18] = 0;
	uw1_cuts_alpha[19] = 0;
	uw1_cuts_alpha[20] = 0;
	uw1_cuts_alpha[21] = 0;
	uw1_cuts_alpha[22] = 0;
	uw1_cuts_alpha[23] = 0;
	uw1_cuts_alpha[24] = 0;
	uw1_cuts_alpha[25] = 0;
	uw1_cuts_alpha[26] = 0;
	uw1_cuts_alpha[27] = 0;
	uw1_cuts_alpha[28] = 0;
	uw1_cuts_alpha[29] = 0;
	uw1_cuts_alpha[30] = 0;
	uw1_cuts_alpha[31] = 0;
	uw1_cuts_alpha[32] = 0;
	uw1_cuts_alpha[33] = 0;
	uw1_cuts_alpha[34] = 0;
	uw1_cuts_alpha[35] = 0;
	uw1_cuts_alpha[36] = 0;
	uw1_cuts_alpha[37] = 0;
	uw1_cuts_alpha[38] = 0;
	uw1_cuts_alpha[39] = 1;
	uw1_cuts_alpha[40] = 1;
	uw1_cuts_alpha[41] = 1;
	uw1_cuts_alpha[42] = 1;
	uw1_cuts_alpha[43] = 1;
	uw1_cuts_alpha[44] = 1;
	uw1_cuts_alpha[45] = 0;
	uw1_cuts_alpha[46] = 0;

	const char *uw2_cuts_file[57];
	uw2_cuts_file[0] = "cuts\\CS000.N00";
	uw2_cuts_file[1] = "cuts\\CS000.N01";
	uw2_cuts_file[2] = "cuts\\CS000.N02";
	uw2_cuts_file[3] = "cuts\\CS000.N03";
	uw2_cuts_file[4] = "cuts\\CS000.N04";
	uw2_cuts_file[5] = "cuts\\CS000.N05";
	uw2_cuts_file[6] = "cuts\\CS000.N06";
	uw2_cuts_file[7] = "cuts\\CS000.N07";
	uw2_cuts_file[8] = "cuts\\CS000.N10";
	uw2_cuts_file[9] = "cuts\\CS000.N11";
	uw2_cuts_file[10] = "cuts\\CS000.N12";
	uw2_cuts_file[11] = "cuts\\CS000.N13";
	uw2_cuts_file[12] = "cuts\\CS000.N14";
	uw2_cuts_file[13] = "cuts\\CS000.N15";
	uw2_cuts_file[14] = "cuts\\CS001.N00";
	uw2_cuts_file[15] = "cuts\\CS001.N01";
	uw2_cuts_file[16] = "cuts\\CS001.N02";
	uw2_cuts_file[17] = "cuts\\CS001.N03";
	uw2_cuts_file[18] = "cuts\\CS001.N04";
	uw2_cuts_file[19] = "cuts\\CS001.N05";
	uw2_cuts_file[20] = "cuts\\CS002.N00";
	uw2_cuts_file[21] = "cuts\\CS002.N01";
	uw2_cuts_file[22] = "cuts\\CS002.N02";
	uw2_cuts_file[23] = "cuts\\CS002.N03";
	uw2_cuts_file[24] = "cuts\\CS002.N04";
	uw2_cuts_file[25] = "cuts\\CS002.N05";
	uw2_cuts_file[26] = "cuts\\CS002.N06";
	uw2_cuts_file[27] = "cuts\\CS002.N07";
	uw2_cuts_file[28] = "cuts\\CS002.N10";
	uw2_cuts_file[29] = "cuts\\CS004.N00";
	uw2_cuts_file[30] = "cuts\\CS005.N00";
	uw2_cuts_file[31] = "cuts\\CS006.N00";
	uw2_cuts_file[32] = "cuts\\CS007.N00";
	uw2_cuts_file[33] = "cuts\\CS011.N00";
	uw2_cuts_file[34] = "cuts\\CS011.N01";
	uw2_cuts_file[35] = "cuts\\CS012.N00";
	uw2_cuts_file[36] = "cuts\\CS012.N01";
	uw2_cuts_file[37] = "cuts\\CS030.N00";
	uw2_cuts_file[38] = "cuts\\CS030.N01";
	uw2_cuts_file[39] = "cuts\\CS031.N00";
	uw2_cuts_file[40] = "cuts\\CS031.N01";
	uw2_cuts_file[41] = "cuts\\CS032.N00";
	uw2_cuts_file[42] = "cuts\\CS032.N01";
	uw2_cuts_file[43] = "cuts\\CS033.N00";
	uw2_cuts_file[44] = "cuts\\CS033.N01";
	uw2_cuts_file[45] = "cuts\\CS034.N00";
	uw2_cuts_file[46] = "cuts\\CS034.N01";
	uw2_cuts_file[47] = "cuts\\CS035.N00";
	uw2_cuts_file[48] = "cuts\\CS035.N01";
	uw2_cuts_file[49] = "cuts\\CS036.N00";
	uw2_cuts_file[50] = "cuts\\CS036.N01";
	uw2_cuts_file[51] = "cuts\\CS037.N01";
	uw2_cuts_file[52] = "cuts\\CS040.N00";
	uw2_cuts_file[53] = "cuts\\CS040.N01";
	uw2_cuts_file[54] = "cuts\\CS403.N00";
	uw2_cuts_file[55] = "cuts\\CS403.N01";
	uw2_cuts_file[56] = "cuts\\CS403.N02";

	int uw2_cuts_alpha[57];
	uw2_cuts_alpha[0] = 0;
	uw2_cuts_alpha[1] = 0;
	uw2_cuts_alpha[2] = 0;
	uw2_cuts_alpha[3] = 0;
	uw2_cuts_alpha[4] = 0;
	uw2_cuts_alpha[5] = 0;
	uw2_cuts_alpha[6] = 0;
	uw2_cuts_alpha[7] = 0;
	uw2_cuts_alpha[8] = 0;
	uw2_cuts_alpha[9] = 0;
	uw2_cuts_alpha[10] = 0;
	uw2_cuts_alpha[11] = 0;
	uw2_cuts_alpha[12] = 0;
	uw2_cuts_alpha[13] = 0;
	uw2_cuts_alpha[14] = 0;
	uw2_cuts_alpha[15] = 0;
	uw2_cuts_alpha[16] = 0;
	uw2_cuts_alpha[17] = 0;
	uw2_cuts_alpha[18] = 0;
	uw2_cuts_alpha[19] = 0;
	uw2_cuts_alpha[20] = 0;
	uw2_cuts_alpha[21] = 0;
	uw2_cuts_alpha[22] = 0;
	uw2_cuts_alpha[23] = 0;
	uw2_cuts_alpha[24] = 0;
	uw2_cuts_alpha[25] = 0;
	uw2_cuts_alpha[26] = 0;
	uw2_cuts_alpha[27] = 0;
	uw2_cuts_alpha[28] = 0;
	uw2_cuts_alpha[29] = 0;
	uw2_cuts_alpha[30] = 0;
	uw2_cuts_alpha[31] = 0;
	uw2_cuts_alpha[32] = 0;
	uw2_cuts_alpha[33] = 0;
	uw2_cuts_alpha[34] = 0;
	uw2_cuts_alpha[35] = 0;
	uw2_cuts_alpha[36] = 0;
	uw2_cuts_alpha[37] = 0;
	uw2_cuts_alpha[38] = 0;
	uw2_cuts_alpha[39] = 0;
	uw2_cuts_alpha[40] = 0;
	uw2_cuts_alpha[41] = 0;
	uw2_cuts_alpha[42] = 0;
	uw2_cuts_alpha[43] = 0;
	uw2_cuts_alpha[44] = 0;
	uw2_cuts_alpha[45] = 0;
	uw2_cuts_alpha[46] = 0;
	uw2_cuts_alpha[47] = 0;
	uw2_cuts_alpha[48] = 0;
	uw2_cuts_alpha[49] = 0;
	uw2_cuts_alpha[50] = 0;
	uw2_cuts_alpha[51] = 0;
	uw2_cuts_alpha[52] = 0;
	uw2_cuts_alpha[53] = 0;
	uw2_cuts_alpha[54] = 0;
	uw2_cuts_alpha[55] = 0;
	uw2_cuts_alpha[56] = 0;


	const char *uw2_cuts_out_file[57];
	uw2_cuts_out_file[0] = "CS000_N00";
	uw2_cuts_out_file[1] = "CS000_N01";
	uw2_cuts_out_file[2] = "CS000_N02";
	uw2_cuts_out_file[3] = "CS000_N03";
	uw2_cuts_out_file[4] = "CS000_N04";
	uw2_cuts_out_file[5] = "CS000_N05";
	uw2_cuts_out_file[6] = "CS000_N06";
	uw2_cuts_out_file[7] = "CS000_N07";
	uw2_cuts_out_file[8] = "CS000_N10";
	uw2_cuts_out_file[9] = "CS000_N11";
	uw2_cuts_out_file[10] = "CS000_N12";
	uw2_cuts_out_file[11] = "CS000_N13";
	uw2_cuts_out_file[12] = "CS000_N14";
	uw2_cuts_out_file[13] = "CS000_N15";
	uw2_cuts_out_file[14] = "CS001_N00";
	uw2_cuts_out_file[15] = "CS001_N01";
	uw2_cuts_out_file[16] = "CS001_N02";
	uw2_cuts_out_file[17] = "CS001_N03";
	uw2_cuts_out_file[18] = "CS001_N04";
	uw2_cuts_out_file[19] = "CS001_N05";
	uw2_cuts_out_file[20] = "CS002_N00";
	uw2_cuts_out_file[21] = "CS002_N01";
	uw2_cuts_out_file[22] = "CS002_N02";
	uw2_cuts_out_file[23] = "CS002_N03";
	uw2_cuts_out_file[24] = "CS002_N04";
	uw2_cuts_out_file[25] = "CS002_N05";
	uw2_cuts_out_file[26] = "CS002_N06";
	uw2_cuts_out_file[27] = "CS002_N07";
	uw2_cuts_out_file[28] = "CS002_N10";
	uw2_cuts_out_file[29] = "CS004_N00";
	uw2_cuts_out_file[30] = "CS005_N00";
	uw2_cuts_out_file[31] = "CS006_N00";
	uw2_cuts_out_file[32] = "CS007_N00";
	uw2_cuts_out_file[33] = "CS011_N00";
	uw2_cuts_out_file[34] = "CS011_N01";
	uw2_cuts_out_file[35] = "CS012_N00";
	uw2_cuts_out_file[36] = "CS012_N01";
	uw2_cuts_out_file[37] = "CS030_N00";
	uw2_cuts_out_file[38] = "CS030_N01";
	uw2_cuts_out_file[39] = "CS031_N00";
	uw2_cuts_out_file[40] = "CS031_N01";
	uw2_cuts_out_file[41] = "CS032_N00";
	uw2_cuts_out_file[42] = "CS032_N01";
	uw2_cuts_out_file[43] = "CS033_N00";
	uw2_cuts_out_file[44] = "CS033_N01";
	uw2_cuts_out_file[45] = "CS034_N00";
	uw2_cuts_out_file[46] = "CS034_N01";
	uw2_cuts_out_file[47] = "CS035_N00";
	uw2_cuts_out_file[48] = "CS035_N01";
	uw2_cuts_out_file[49] = "CS036_N00";
	uw2_cuts_out_file[50] = "CS036_N01";
	uw2_cuts_out_file[51] = "CS037_N01";
	uw2_cuts_out_file[52] = "CS040_N00";
	uw2_cuts_out_file[53] = "CS040_N01";
	uw2_cuts_out_file[54] = "CS403_N00";
	uw2_cuts_out_file[55] = "CS403_N01";
	uw2_cuts_out_file[56] = "CS403_N02";


	const char *shock_graphics_file[18];
	shock_graphics_file[0] = "res\\DATA\\OBJART.RES";
	shock_graphics_file[1] = "RES\\DATA\\OBJART2.RES";
	shock_graphics_file[2] = "RES\\DATA\\OBJART3.RES";
	shock_graphics_file[3] = "RES\\DATA\\CITMAT.RES";
	shock_graphics_file[4] = "RES\\DATA\\GAMESCR.RES";
	shock_graphics_file[5] = "RES\\DATA\\HANDART.RES";
	shock_graphics_file[6] = "RES\\DATA\\MFDART.RES";
	shock_graphics_file[7] = "RES\\DATA\\SIDEART.RES";
	shock_graphics_file[8] = "RES\\DATA\\START1.RES";
	shock_graphics_file[9] = "RES\\DATA\\SPLASH.RES";
	shock_graphics_file[10] = "RES\\DATA\\TEXTURE.RES";
	shock_graphics_file[11] = "RES\\DATA\\DEATH.RES";
	shock_graphics_file[12] = "RES\\DATA\\INTRO.RES";
	shock_graphics_file[13] = "RES\\DATA\\SVGADETH.RES";
	shock_graphics_file[14] = "RES\\DATA\\SVGAEND.RES";
	shock_graphics_file[15] = "RES\\DATA\\SVGAINTR.RES";
	shock_graphics_file[16] = "RES\\DATA\\vidmail.RES";
	shock_graphics_file[17] = "RES\\DATA\\WIN1.RES";

	const char *shock_pal_file[3];
	shock_pal_file[0] = "res\\DATA\\gamepal.res";
	shock_pal_file[1] = "RES\\DATA\\cutspal.res";
	shock_pal_file[2] = "RES\\DATA\\splshpal.res";

	const char *shock_cuts_file[3];
	shock_cuts_file[0] = "RES\\DATA\\START1.RES";
	shock_cuts_file[1] = "RES\\DATA\\DEATH.RES";
	shock_cuts_file[2] = "RES\\DATA\\WIN1.RES";

	//UW Object combination files
	const char *uw_objcomb_file;
	uw_objcomb_file = "DATA\\CMB.DAT";

	const char *uw2_objcomb_file;
	uw2_objcomb_file = "DATA\\CMB.DAT";

	//UW Common Object properties files
	const char *uw_comp_file;
	const char *uw_obj_prop_file;
	uw_comp_file = "DATA\\comobj.dat";
	uw_obj_prop_file = "DATA\\Objects.dat";

	const char *uw2_comp_file;
	const char *uw2_obj_prop_file;
	uw2_comp_file = "DATA\\comobj.dat";
	uw2_obj_prop_file = "DATA\\Objects.dat";

	levelNo = -1;
	if (true)
		{
		printf("Welcome to Underworld Exporter.\n");
		printf("\nAvailable games\n");
		printf("0) Ultima Underworld Demo (probably doesn't work!)\n");
		printf("1) Ultima Underworld 1: The Stygian Abyss\n");
		printf("2) Ultima Underworld 2: The Labyrinth of Worlds\n");
		printf("3) System Shock 1\n");
		printf("Please select a game.\n>");
		scanf("%d", &game);
		if ((game < UWDEMO) || (game > SHOCK))
			{
			printf("Invalid input. Bye.");
			return 0;
			}

		printf("Available Modes.\n");
		printf("%d) Ascii dump.\n", ASCII_MODE);
		printf("%d) IDTech/DarkMod export.\n", D3_MODE);
		printf("%d) String Extraction\n", STRINGS_EXTRACT_MODE);
		printf("%d) Bitmap Extraction\n", BITMAP_EXTRACT_MODE);
		printf("%d) Script build(Also runs as part of IDTech export)\n", SCRIPT_BUILD_MODE);
		printf("%d) Support Materials Builder\n", MATERIALS_BUILD_MODE);
		printf("%d) Conversation code dump (unfinished!)\n", CONVERSATION_MODE);
		printf("%d) Repacker mode (UW2 and Shock only. Use at own risk!)\n", REPACK_MODE);
		printf("%d) Source Engine export\n", SOURCE_MODE);
		printf("%d) Critter Art Extract\n", CRITTER_EXTRACT_MODE);
		printf("%d) Cutscene Art Extract\n", CUTSCENE_EXTRACT_MODE);
		printf("%d) FBX Export\n", FBX_MODE);
		printf("%d) Unity script generation\n", UNITY_MODE);
		printf("%d) Unity Tilemap info generation\n", UNITY_TILEMAP_MODE);
		printf("%d) SS1 Font Extraction\n", FONT_EXTRACT_MODE);
		printf("%d) Show UW Object Combinations\n", COMBINATION_EXTRACT_MODE);
		printf("%d) Show UW Common Object Properties\n", COMMON_PROPERTIES_EXTRACT_MODE);
		printf("%d) Decode UW1 Fonts (using UWAdventures code)\n", FONT_DECODE_UW1_MODE);
		printf("%d) Decode UW1 SKILLS File\n", SKILLS_EXTRACT_MODE);
		printf("Please select a mode.>");
		scanf("%d", &mode);
		if ((mode < 0) || (mode > 18))
			{
			printf("Invalid input. Bye.");
			return 0;
			}

		switch (mode)
			{
				case ASCII_MODE:
				case D3_MODE:
				case SOURCE_MODE:
				case SCRIPT_BUILD_MODE:
				case REPACK_MODE:
				case FBX_MODE:
				case UNITY_MODE:
				case UNITY_TILEMAP_MODE:
					{
					switch (game)
						{
							case UWDEMO:
								sprintf_s(GameFilePath, 255, "%s", path_uw0);
								break;
							case UW1:
							case UW2:
								printf("\nPick a level archive or save game to open\n");
								for (int i = 0; i < 5; i++)
									{
									printf("%d) %s\n", i, uw_game_files[i]);
									}
								if ((game == UW2) && (mode == REPACK_MODE))
									{
									printf("%d) %s\n", 6, uw_game_files[6]);
									}
								printf(">");
								scanf("%d", &gamefile);
								if ((gamefile < 0) || (gamefile >= 7))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								if (game == UW1)
									{
									sprintf_s(GameFilePath, 255, "%s\\%s", path_uw1, uw_game_files[gamefile]);
									}
								else
									{
									sprintf_s(GameFilePath, 255, "%s\\%s", path_uw2, uw_game_files[gamefile]);
									}
								break;
							case SHOCK:
								printf("\nPick a level archive or save game to open\n");
								for (int i = 0; i < 9; i++)
									{
									printf("%d) %s\n", i, shock_game_files[i]);
									}
								printf(">");
								scanf("%d", &gamefile);
								if ((gamefile < 0) || (gamefile >= 10))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								sprintf_s(GameFilePath, 255, "%s\\%s", path_shock, shock_game_files[gamefile]);
								break;
						}
					if (mode != REPACK_MODE)
						{
						switch (game)
							{
								case UWDEMO:
									levelNo = 0;	//only possible value
									break;
								case UW1://Print list of UW1 levels.
									printf("\nPick a level.\n");
									printf("0)Entrance level.\n");
									printf("1)Domain of the Mountainmen.\n");
									printf("2)The Swamp and Lizardmen.\n");
									printf("3)Trolls and Knights.\n");
									printf("4)Catacombs and banquet halls.\n");
									printf("5)The Seers.\n");
									printf("6)Tybals Lair.\n");
									printf("7)The Volcano.\n");
									printf("8)Ethereal Void.\n");
									printf(">");
									scanf("%d", &levelNo);
									if ((levelNo < 0) || (levelNo > 9))
										{
										printf("Invalid input. Bye.");
										return 0;
										}
									break;
								case UW2:
									printf("\nPick a level\n");
									printf("0 - 4 Britannia\n");
									printf("8 - 15 Prison Tower\n");
									printf("16 - 17 Killorn Keep\n");
									printf("24 - 25 Ice Cavern\n");
									printf("32 - 33 Talorus\n");
									printf("40 - 47 Academy\n");
									printf("48 - 51 Tomb\n");
									printf("56 - 58 Pits\n");
									printf("64 - 72 Void (65 is stickman level, 66 scint vault)\n");
									printf(">");
									scanf("%d", &levelNo);
									if (!(
										((levelNo >= 0) && (levelNo <= 4))
										|| ((levelNo >= 8) && (levelNo <= 15))
										|| ((levelNo >= 16) && (levelNo <= 17))
										|| ((levelNo >= 24) && (levelNo <= 25))
										|| ((levelNo >= 32) && (levelNo <= 33))
										|| ((levelNo >= 40) && (levelNo <= 47))
										|| ((levelNo >= 48) && (levelNo <= 51))
										|| ((levelNo >= 56) && (levelNo <= 58))
										|| ((levelNo >= 64) && (levelNo <= 72))
										))
										{
										printf("Invalid input. Bye.");
										return 0;
										}
									break;
								case SHOCK:
									printf("\nPick a level (need to dblchk these)\n");
									printf("0)Reactor\n");
									printf("1)Med SCI\n");
									printf("2)Research\n");
									printf("3)Maintenance\n");
									printf("4)Storage\n");
									printf("5)Flight Deck\n");
									printf("6)Executive\n");
									printf("7)System Engineering\n");
									printf("8)Security\n");
									printf("9)Bridge\n");
									printf("10)Cyberspace 1 (Shodan)\n");
									printf("11)Grove 1\n");
									printf("12)Grove 1\n");
									printf("13)Grove 1\n");
									printf("14)Cyberspace 2\n");
									printf("15)Cyberspace 3\n");
									printf(">");
									scanf("%d", &levelNo);
									if ((levelNo < 0) || (levelNo > 15))
										{
										printf("Invalid input. Bye.");
										return 0;
										}
									break;
							}
						printf("Enter a filename for output (%s\\[filename].map)\n>", path_target_platform);

						scanf("%s", TempOutFileName);
						sprintf_s(OutFileName, 255, "%s\\%s", path_target_platform, TempOutFileName);
						}
					else
						{
						switch (game)
							{
								case UW2:
									printf("Enter a filename for repacking into (%s\\data\\[filename].ark)\n>", path_uw2);
									scanf("%s", TempOutFileName);
									sprintf_s(OutFileName, 255, "%s\\data\\%s.ark", path_uw2, TempOutFileName);
									break;
								case SHOCK:
									printf("Enter a filename for repacking into (%s\\[filename].data)\n>", path_shock);
									scanf("%s", TempOutFileName);
									sprintf_s(OutFileName, 255, "%s\\res\\data\\%s.ark", path_shock, TempOutFileName);
									break;
								default:
									printf("\nInvalid game for repacking. Goodbye");
									return 0;
							}

						}

					break;
					}
				case BITMAP_EXTRACT_MODE:
				case FONT_EXTRACT_MODE:
					if (mode == FONT_EXTRACT_MODE){ game = SHOCK; }
					switch (game)
						{
							case UWDEMO:
								for (int i = 0; i < 44; i++)
									{
									printf("%d) %s", i, uw0_graphics_file[i]);
									if (i % 2 == 0)
										{
										printf("\t\t");
										}
									else
										{
										printf("\n");
										}
									}
								printf("\nPick a file\n>");
								scanf("%d", &graphics_file_no);
								if ((graphics_file_no < 0) || (graphics_file_no > 43))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								sprintf_s(Graphics_File, 255, "%s\\%s", path_uw0, uw0_graphics_file[graphics_file_no]);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw0);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw0, AUXILARY_PAL_FILE);
								if (graphics_file_no <= 3)
									{
									graphics_mode = UW_GRAPHICS_TEXTURES;
									switch (graphics_file_no)
										{
											case 0:
												BitMapSize = 16; break;
											case 1:
												BitMapSize = 32; break;
											case 2:
												BitMapSize = 16; break;
											case 3:
												BitMapSize = 64; break;
										}
									}
								else if (graphics_file_no <= 35)
									{
									if ((game==UW1) && (graphics_file_no == 28))//Panels
										{
										panels = 1;
										}
									if ((game == UW2) && (graphics_file_no == 27))//Panels
										{
										panels = 1;
										}
									graphics_mode = UW_GRAPHICS_GR;
									}
								else
									{
									graphics_mode = UW_GRAPHICS_BITMAPS;
									}
								break;
							case UW1:
								for (int i = 0; i < 45; i++)
									{
									printf("%d) %s", i, uw1_graphics_file[i]);
									if (i % 2 == 0)
										{
										printf("\t\t");
										}
									else
										{
										printf("\n");
										}
									}
								printf("\nPick a file\n>");
								scanf("%d", &graphics_file_no);
								if ((graphics_file_no < 0) || (graphics_file_no > 44))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								sprintf_s(Graphics_File, 255, "%s\\%s", path_uw1, uw1_graphics_file[graphics_file_no]);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw1);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw1, AUXILARY_PAL_FILE);
								if (graphics_file_no <= 3)
									{
									graphics_mode = UW_GRAPHICS_TEXTURES;
									switch (graphics_file_no)
										{
											case 0:
												BitMapSize = 16; break;
											case 1:
												BitMapSize = 32; break;
											case 2:
												BitMapSize = 16; break;
											case 3:
												BitMapSize = 64; break;
										}
									}
								else if (graphics_file_no <= 35)
									{
									if ( (game== UW1) && (graphics_file_no == 28))//Panels
										{
										panels = 1;
										}
									graphics_mode = UW_GRAPHICS_GR;
									}
								else
									{
									graphics_mode = UW_GRAPHICS_BITMAPS;
									}
								break;
							case UW2:
								for (int i = 0; i < 45; i++)
									{
									printf("%d) %s", i, uw2_graphics_file[i]);
									if (i % 2 == 0)
										{
										printf("\t\t");
										}
									else
										{
										printf("\n");
										}
									}
								printf("\nPick a file\n>");
								scanf("%d", &graphics_file_no);
								if ((graphics_file_no < 0) || (graphics_file_no > 45))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								if (graphics_file_no == 36)
									{//UW2 Byt ark
									bytark = 1;
									}
								sprintf_s(Graphics_File, 255, "%s\\%s", path_uw2, uw2_graphics_file[graphics_file_no]);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw2);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw2, AUXILARY_PAL_FILE);

								if (graphics_file_no <= 0)
									{//only on texture file in uw2
									graphics_mode = UW_GRAPHICS_TEXTURES;
									BitMapSize = 64;
									}
								else if (graphics_file_no >= 37)
									{//Byt files in cuts dir
									graphics_mode = UW_GRAPHICS_BITMAPS;
									}
								else
									{
									graphics_mode = UW_GRAPHICS_GR;
									if ((game == UW1) && (graphics_file_no == 28))//Panels
										{
										panels = 1;//special case due to file headers
										}

									}
								break;
							case SHOCK:
								for (int i = 0; i < 18; i++)
									{
									printf("%d) %s", i, shock_graphics_file[i]);
									if (i % 2 == 0)
										{
										printf("\t\t");
										}
									else
										{
										printf("\n");
										}
									}
								printf("\nPick a file\n>");
								scanf("%d", &graphics_file_no);
								if ((graphics_file_no < 0) || (graphics_file_no > 18))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								sprintf_s(Graphics_File, 255, "%s\\%s", path_shock, shock_graphics_file[graphics_file_no]);
								for (int i = 0; i < 3; i++)
									{
									printf("%d) %s\n", i, shock_pal_file[i]);
									}
								printf("\nPick a file\n>");
								scanf("%d", &graphics_file_no);
								if ((graphics_file_no < 0) || (graphics_file_no > 3))
									{
									printf("Invalid input. Bye.");
									return 0;
									}
								sprintf_s(Graphics_Pal, 255, "%s\\%s", path_shock, shock_pal_file[graphics_file_no]);
								//return 0;
						}
					printf("\nEnter a palette number to use.(Typically 0 to 7)\n>");
					scanf("%d", &critPal);
					printf("\nEnter a file format to use.\n0) BMP\n1) TGA\n>");
					scanf("%d", &useTGA);
					if (useTGA == 1)
						{
						printf("Enter a filename for output ([filename]_###.tga)\n>");
						}
					else
						{
						printf("Enter a filename for output ([filename]_###.bmp)\n>");
						}
					scanf("%s", OutFileName);
					break;
				case STRINGS_EXTRACT_MODE:
					switch (game)
						{
							case UWDEMO:
								sprintf_s(GameFilePath, 255, "%s\\%s", path_uw0, UW1_STRINGS_FILE);
								break;
							case UW1:
								sprintf_s(GameFilePath, 255, "%s\\%s", path_uw1, UW1_STRINGS_FILE);
								break;
							case UW2:
								sprintf_s(GameFilePath, 255, "%s\\%s", path_uw2, UW2_STRINGS_FILE);
								break;
							case SHOCK:
								sprintf_s(GameFilePath, 255, "%s\\%s", path_shock, SHOCK_STRINGS_FILE);
								break;
						}
					break;
				case MATERIALS_BUILD_MODE:
					break;
				case CRITTER_EXTRACT_MODE:
					if (game == SHOCK)
						{
						printf("\nI don't have a graphics extractor for System Shock as part of this program. You'll have to find one yourself.\n");
						return 0;
						}
					printf("\nType a critter filename for extraction from. gamepath\crit\\[filename]\n");
					scanf("%s", TempOutFileName);
					switch (game)
						{
							case UWDEMO:
								sprintf_s(fileCrit, 255, "%s\\crit\\%s", path_uw0, TempOutFileName);
								sprintf_s(fileCrit, 255, "%s\\crit\\", path_uw0);
								sprintf_s(fileAssoc, 255, "%s\\%s", path_uw0, UW0_CRITTER_ASSOC);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw1);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw0, AUXILARY_PAL_FILE);
								break;
							case UW1:
								//sprintf_s(fileCrit, 255, "%s\\crit\\%s", path_uw1, TempOutFileName);
								sprintf_s(fileCrit, 255, "%s\\crit\\", path_uw1);
								sprintf_s(fileAssoc, 255, "%s\\%s", path_uw1, UW1_CRITTER_ASSOC);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw1);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw1, AUXILARY_PAL_FILE);
								break;
							case UW2:
								//sprintf_s(fileCrit, 255, "%s\\crit\\%s", path_uw2, TempOutFileName);
								sprintf_s(fileCrit, 255, "%s\\crit\\", path_uw2);
								sprintf_s(fileAssoc, 255, "%s\\%s", path_uw2, UW2_CRITTER_ASSOC);
								sprintf_s(Graphics_Pal, 255, "%s\\data\\pals.dat", path_uw2);
								sprintf_s(auxPalPath, 255, "%s\\%s", path_uw2, AUXILARY_PAL_FILE);
								break;
						}
					printf("\nEnter a palette number to use.(0 to 3)\n>");
					scanf("%d", &critPal);
					printf("\nEnter a file format to use.\n0) BMP\n1) TGA\n>");
					scanf("%d", &useTGA);
					if (useTGA == 1)
						{
						printf("Enter a filename for output ([filename]_###.tga)\n>");
						}
					else
						{
						printf("Enter a filename for output ([filename]_###.bmp)\n>");
						}
					scanf("%s", OutFileName);
					break;
				case CUTSCENE_EXTRACT_MODE:
					if (game == SHOCK)
						{
						for (int i = 0; i < 3; i++)
							{
							printf("%d) %s", i, shock_cuts_file[i]);
							if (i % 2 == 0)
								{
								printf("\t\t");
								}
							else
								{
								printf("\n");
								}
							}
						printf("\nPick a file\n>");
						scanf("%d", &graphics_file_no);
						if ((graphics_file_no < 0) || (graphics_file_no > 3))
							{
							printf("Invalid input. Bye.");
							return 0;
							}
						sprintf_s(Graphics_File, 255, "%s\\%s", path_shock, shock_cuts_file[graphics_file_no]);
						for (int i = 0; i < 3; i++)
							{
							printf("%d) %s\n", i, shock_pal_file[i]);
							}
						printf("\nPick a file\n>");
						scanf("%d", &graphics_file_no);
						if ((graphics_file_no < 0) || (graphics_file_no > 3))
							{
							printf("Invalid input. Bye.");
							return 0;
							}
						sprintf_s(Graphics_Pal, 255, "%s\\%s", path_shock, shock_pal_file[graphics_file_no]);
						}
					else
						{
						
						printf("\nType a cutscene filename for extraction from. gamedata\\cuts\\[filename]\n");
						scanf("%s", TempOutFileName);
						switch (game)
							{
								case UWDEMO:
									sprintf_s(Graphics_File, 255, "%s\\cuts\\%s", path_uw0, TempOutFileName);
									break;
								case UW1:
									sprintf_s(Graphics_File, 255, "%s\\cuts\\%s", path_uw1, TempOutFileName);
									break;
								case UW2:
									sprintf_s(Graphics_File, 255, "%s\\cuts\\%s", path_uw2, TempOutFileName);
									break;
							}
							
						}
					//printf("\nEnter a file format to use.\n0) BMP\n1) TGA\n>");
					//scanf("%d", &useTGA);
					if (useTGA == 1)
						{
						printf("Enter a filename for output ([filename]_###.tga)\n>");
						}
					else
						{
						printf("Enter a filename for output ([filename]_###.bmp)\n>");
						}
					scanf("%s", OutFileName);
					break;
				case CONVERSATION_MODE:
					break;
				case SKILLS_EXTRACT_MODE:
					break;
			}
		}
	else
		{
		printf("commandline args processing eventually");
		}
	GAME = game;
	switch (game)
		{
			case UWDEMO:
			case UW1:
			case UW2:
				{BrushSizeX = 120; BrushSizeY = 120; BrushSizeZ = 15; break; }
			case SHOCK:
				{BrushSizeX = 120; BrushSizeY = 120; BrushSizeZ = 15; break; }//To ease on the steepness of some slopes that are impassible
		}

	LoadConfig(game, mode);

	char Logfile[255];
	strcpy_s(Logfile, LOGFILENAME);
	//sprintf_s(Logfile, 255, "%s", LOGFILENAME);
	if (fopen_s(&LOGFILE, Logfile, "w") != 0)
		{
		printf("Unable to create output file for logfile");
		return 0;
		}

	switch (mode)
		{
			case D3_MODE:
			case ASCII_MODE:
			case SCRIPT_BUILD_MODE:
			case SOURCE_MODE:
			case FBX_MODE:
			case UNITY_MODE:
			case UNITY_TILEMAP_MODE:
				printf("\n================Game %d==Level %d==Mode %d===================\n", game, levelNo, mode);
				exportMaps(game, mode, levelNo, OutFileName, GameFilePath);
				break;
			case STRINGS_EXTRACT_MODE:
				if (game == SHOCK)
					{
					unpackStringsShock(GameFilePath);
					}
				else
					{
					unpackStrings(game, GameFilePath);
					}
				break;
			case BITMAP_EXTRACT_MODE:
				if (game != SHOCK)
					{
					if (panels == 0)
						{
						if (bytark != 1)
							{
							//for (int f = 0; f < 8; f++)
							//	{
							//	sprintf(OutFileName, "%d",f);
							//extractTextureBitmap(-1, Graphics_File, Graphics_Pal, f, BitMapSize, graphics_mode, OutFileName, auxPalPath, useTGA);
							if  ((graphics_file_no == 35) && (1))//Weapons
								{
								char WeaponAnims[255];
								
								if (game==UW2)
									{
									sprintf_s(auxPalPath, 255, "%s\\data\\Weap.cm", path_uw2);
									sprintf_s(WeaponAnims, 255, "%s\\data\\Weap.dat", path_uw2);//TODO:Support other games properly!
									}
								else
									{
									sprintf_s(auxPalPath, 255, "%s\\data\\Weapons.cm", path_uw1);
									sprintf_s(WeaponAnims, 255, "%s\\data\\Weapons.dat", path_uw1);
									}
								
								ExtractWeaponAnimations(-1, Graphics_File, Graphics_Pal, critPal, BitMapSize, graphics_mode, OutFileName, auxPalPath, WeaponAnims, useTGA,game);
								}
							else
								{
								extractTextureBitmap(-1, Graphics_File, Graphics_Pal, critPal, BitMapSize, graphics_mode, OutFileName, auxPalPath, useTGA);
								}
							
							//	}

							}
						else
							{
							extractUW2Bitmaps(Graphics_File, Graphics_Pal, critPal, OutFileName, useTGA);
							}
						}
					else
						{
						extractPanels(-1, Graphics_File, Graphics_Pal, 0, BitMapSize, UW_GRAPHICS_GR, game, OutFileName, useTGA);
						}
					}
				else
					{
					//sprintf_s(Graphics_File, 255, testfile, path_uw2);
					ExtractShockGraphics(Graphics_File, Graphics_Pal, critPal, OutFileName, useTGA);
					}
				break;
			case FONT_EXTRACT_MODE:
				ExtractShockFont(Graphics_File, Graphics_Pal, critPal, OutFileName, useTGA);
				break;
			case CRITTER_EXTRACT_MODE:
				switch (game)
					{
					case UWDEMO:
						case UW1:extractAllCrittersUW1(fileAssoc, fileCrit, Graphics_Pal, game, useTGA); break;
						case UW2:extractAllCrittersUW2(fileAssoc, fileCrit, Graphics_Pal, game, useTGA); break;
					}
				//extractCritters(fileAssoc, fileCrit, Graphics_Pal, critPal, 64, UW_GRAPHICS_GR,game, 0, OutFileName,useTGA,1);
				break;
			case CUTSCENE_EXTRACT_MODE:
				switch (game)
					{
					case UW1://TODO: remove this hardcoding of cutscene selection.
						//for (int i = 17; i < 47; i++)
						for (int i = 38; i < 39; i++)
							{
							char inFile[255];
							sprintf(inFile, "%s\\%s",path_uw1, uw1_cuts_file[i] ,1);
							if (i == 17)
								{
								load_cuts_anim(inFile, uw1_cuts_out_file[i], useTGA,true,uw1_cuts_alpha[i]);
								}
							else
								{
								load_cuts_anim(inFile, uw1_cuts_out_file[i], useTGA, false, uw1_cuts_alpha[i]);
								}							
							}						
						break;
					case UW2:
						for (int i = 28; i < 57; i++)
							{
							char inFile[255];
							sprintf(inFile, "%s\\%s", path_uw2, uw2_cuts_file[i], 1);
							//if (i == 17)
							//	{
							//	load_cuts_anim(inFile, uw2_cuts_out_file[i], useTGA, true, uw2_cuts_alpha[i]);
							//	}
							//else
							//	{
								load_cuts_anim(inFile, uw2_cuts_out_file[i], useTGA, false, uw2_cuts_alpha[i]);
							//	}
							}
						break;
					case SHOCK:
						ExtractShockCutscenes(Graphics_File, Graphics_Pal, critPal, OutFileName, 0);//Bitmap only so far
						break;
					default:
						load_cuts_anim(Graphics_File, OutFileName, useTGA,false,0);
						break;
					}
							break;
			case MATERIALS_BUILD_MODE:
				//BuildSHOCKMtrFiles(0);
				//BuildXDataFile(game);
				//BuildSndShaderFiles();
				//BuildShockMtrFiles(3);
				//BuildUWMtrFiles(game,2);
				//BuildUWXData(game, 8);
				//BuildGuiFiles();
				//ExportModelFormat();
				//BuildWORDSXData(game);
				//BuildParticles(game);
				printf("Materials builder turned off at the moment.");
				break;
			case CONVERSATION_MODE:
				if (game == UW2)
					{
					ExtractConversationsUW2(game);
					}
				else
					{
					ExtractConversations(game);
					}
				
				break;
			case REPACK_MODE:
				if (game == UW2)
					{
					RepackUW2(GameFilePath, OutFileName, BlocksToRepack);
					}
				else
					{
					if (game == SHOCK)
						{//C:\Games\Terra Nova\CD\Terra_Nova\MAPS
						RepackShock(GameFilePath, OutFileName);
						//RepackShock("C:\\Games\\Terra Nova\\CD\\Terra_Nova\\MAPS\\FLAT.RES", OutFileName);
						}
					}
				break;
			case COMBINATION_EXTRACT_MODE:
				{
				switch (game)
					{
						case UW1:
							{
							sprintf_s(FileToOpen, 255, "%s\\%s", path_uw1, uw_objcomb_file);
							DumpObjectCombinations(FileToOpen, game);
							break;
							}
						case UW2:
							{
							sprintf_s(FileToOpen, 255, "%s\\%s", path_uw2, uw2_objcomb_file);
							DumpObjectCombinations(FileToOpen, game);
							break;
							}
					}
				break;
				}
			case COMMON_PROPERTIES_EXTRACT_MODE:
				{
				switch (game)
					{
						case UW1:
							{
							//sprintf_s(FileToOpen, 255, "%s\\%s", path_uw1, uw_comp_file);
							//sprintf_s(FileToOpen2, 255, "%s\\%s", path_uw1, uw_obj_prop_file);
							UWCommonObj(game);
							break;
							}
						case UW2:
							{
							//sprintf_s(FileToOpen, 255, "%s\\%s", path_uw2, uw2_comp_file);
							//sprintf_s(FileToOpen, 255, "%s\\%s", path_uw2, uw2_obj_prop_file);
							UWCommonObj(game);
							break;
							}
						case SHOCK:
							{
							shockCommonObject();
							break;
							}
					}
				break;
				}
			case FONT_DECODE_UW1_MODE:
				{
				DecodeUWFont();
				}
			case SKILLS_EXTRACT_MODE:
				{
				ExtractSkills();
				}
		}
	fclose(LOGFILE);
	}

	void LoadConfig(int game, int mode)
		{
		//Read in mile
		FILE *f = NULL;
		char filePathT[255];
		int i = 0;
		int texNo = 0;
		char texDesc[80];
		char texPath[80];

		int objNo = 0;
		char objDesc[80];
		char objPath[80];
		char objCat[10];
		char objSound[80];
		char objParticle[80];
		char objBase[80];
		char invIcon[80];
		char EquippedIconFemaleLowest[80];
		char EquippedIconMaleLowest[80];//and default
		char EquippedIconFemaleLow[80];
		char EquippedIconMaleLow[80];
		char EquippedIconFemaleMedium[80];
		char EquippedIconMaleMedium[80];
		char EquippedIconFemaleBest[80];
		char EquippedIconMaleBest[80];

		int objType;
		int hasPart;
		int hasSound;
		int isSolid; //or useable
		int isMoveable;
		int isInventory;
		int isAnimated;
		int useSprite;

		int ShouldSave;


		int objClass; int objSubClass; int objSubClassIndex;	//Shock object classes

		char line[500];
		textureMasters = new texture[340];
		objectMasters = new objectMaster[500];
		switch (game)
			{
				case UWDEMO:
				case UW1:
					if (mode != SOURCE_MODE)
						{
						strcpy_s(filePathT, UW1_TEXTURE_CONFIG_FILE);
						}
					else
						{
						strcpy_s(filePathT, UW1_TEXTURE_CONFIG_FILE_SOURCE);
						}
					break;
				case UW2:
					if (mode != SOURCE_MODE)
						{
						strcpy_s(filePathT, UW2_TEXTURE_CONFIG_FILE);
						}
					else
						{
						strcpy_s(filePathT, UW2_TEXTURE_CONFIG_FILE_SOURCE);
						}
					break;
				case SHOCK:
					if (mode != SOURCE_MODE)
						{
						strcpy_s(filePathT, SHOCK_TEXTURE_CONFIG_FILE);
						}
					else
						{
						strcpy_s(filePathT, SHOCK_TEXTURE_CONFIG_FILE_SOURCE);
						}
					break;
			}

		//f=fopen(filePathT,"r");
		if ((fopen_s(&f, filePathT, "r") == 0))
			{
			while (fgets(line, 255, f))
				{
				float align1_1 = 0;
				float align1_2 = 0;
				float align1_3 = 0;
				float align2_1 = 0;
				float align2_2 = 0;
				float align2_3 = 0;

				float floor_align1_1 = 0;
				float floor_align1_2 = 0;
				float floor_align1_3 = 0;
				float floor_align2_1 = 0;
				float floor_align2_2 = 0;
				float floor_align2_3 = 0;

				int water = 0;
				int lava = 0;
				sscanf(line, "%d %s %s %f %f %f %f %f %f %f %f %f %f %f %f %d %d",
					&texNo, &texDesc, &texPath,
					&align1_1, &align1_2, &align1_3, &align2_1, &align2_2, &align2_3,
					&floor_align1_1, &floor_align1_2, &floor_align1_3, &floor_align2_1, &floor_align2_2, &floor_align2_3,
					&water, &lava);


				textureMasters[texNo].textureNo = texNo;
				strcpy_s(textureMasters[texNo].desc, texDesc);
				strcpy_s(textureMasters[texNo].path, texPath);
				textureMasters[texNo].align1_1 = align1_1;
				textureMasters[texNo].align1_2 = align1_2;
				textureMasters[texNo].align1_3 = align1_3;
				textureMasters[texNo].align2_1 = align2_1;
				textureMasters[texNo].align2_2 = align2_2;
				textureMasters[texNo].align2_3 = align2_3;

				textureMasters[texNo].floor_align1_1 = floor_align1_1;
				textureMasters[texNo].floor_align1_2 = floor_align1_2;
				textureMasters[texNo].floor_align1_3 = floor_align1_3;
				textureMasters[texNo].floor_align2_1 = floor_align2_1;
				textureMasters[texNo].floor_align2_2 = floor_align2_2;
				textureMasters[texNo].floor_align2_3 = floor_align2_3;


				textureMasters[texNo].water = water;
				textureMasters[texNo].lava = lava;
				i++;
				}
			fclose(f);
			}
		char filePathO[255];
		switch (game)
			{
				case UWDEMO:
				case UW1:
					strcpy_s(filePathO, UW1_OBJECT_CONFIG_FILE); break;
				case UW2:
					strcpy_s(filePathO, UW2_OBJECT_CONFIG_FILE); break;
				case SHOCK:
					strcpy_s(filePathO, SHOCK_OBJECT_CONFIG_FILE); break;
			}

		if ((fopen_s(&f, filePathO, "r") == 0))
			{
			switch (game)
				{
					case UWDEMO:
					case UW1:
					case UW2:
					case SHOCK:
						{
						while (fgets(line, 500, f))
							{                                                             //v
							sscanf(line, "%d %d %d %d %s %d %s %s %d %s %d %s %s %d %d %d %s %s %s %s %s %s %s %s %s %d %d %d",
								&objNo, &objClass, &objSubClass, &objSubClassIndex, &objCat, &objType,
								&objDesc, &objPath, &hasPart, &objParticle, &hasSound, &objSound, &objBase, &isSolid, &isMoveable, &isInventory, &invIcon,
								&EquippedIconFemaleLowest, &EquippedIconFemaleLow, &EquippedIconFemaleMedium, &EquippedIconFemaleBest,
								&EquippedIconMaleLowest, &EquippedIconMaleLow, &EquippedIconMaleMedium, &EquippedIconMaleBest,
								&isAnimated, &useSprite, &ShouldSave);
							objectMasters[objNo].index = objNo;
							objectMasters[objNo].isSet = 1;
							objectMasters[objNo].objClass = objClass;
							objectMasters[objNo].objSubClass = objSubClass;
							objectMasters[objNo].objSubClassIndex = objSubClassIndex;


							strcpy_s(objectMasters[objNo].desc, objDesc);
							strcpy_s(objectMasters[objNo].path, objPath);
							strcpy_s(objectMasters[objNo].particle, objParticle);
							strcpy_s(objectMasters[objNo].sound, objSound);
							strcpy_s(objectMasters[objNo].base, objBase);

							if (strcmp(objCat, "model") == 0)
								{
								objectMasters[objNo].isEntity = 0;
								}
							else if (strcmp(objCat, "entity") == 0)
								{
								objectMasters[objNo].isEntity = 1;
								}
							else
								{
								objectMasters[objNo].isEntity = -1;
								}

							objectMasters[objNo].type = objType;
							objectMasters[objNo].DeathWatch = 0;
							objectMasters[objNo].hasParticle = hasPart;
							objectMasters[objNo].hasSound = hasSound;
							objectMasters[objNo].isSolid = isSolid;
							objectMasters[objNo].isMoveable = isMoveable;
							objectMasters[objNo].isInventory = isInventory;

							if ((game == UW1) || (game == UWDEMO))
								{
								strcpy_s(objectMasters[objNo].EquippedIconFemaleLowest, EquippedIconFemaleLowest);
								strcpy_s(objectMasters[objNo].EquippedIconMaleLowest, EquippedIconMaleLowest);
								strcpy_s(objectMasters[objNo].EquippedIconFemaleLow, EquippedIconFemaleLow);
								strcpy_s(objectMasters[objNo].EquippedIconMaleLow, EquippedIconMaleLow);
								strcpy_s(objectMasters[objNo].EquippedIconFemaleMedium, EquippedIconFemaleMedium);
								strcpy_s(objectMasters[objNo].EquippedIconMaleMedium, EquippedIconMaleMedium);
								strcpy_s(objectMasters[objNo].EquippedIconFemaleBest, EquippedIconFemaleBest);
								strcpy_s(objectMasters[objNo].EquippedIconMaleBest, EquippedIconMaleBest);
								objectMasters[objNo].isAnimated = isAnimated;
								objectMasters[objNo].useSprite = useSprite;
								strcpy_s(objectMasters[objNo].InvIcon, invIcon);
								objectMasters[objNo].ShouldSave = ShouldSave;
								}





							//printf("uw1_object_%03d.prt\n",objNo);
							i++;
							}
						fclose(f);
						break;
						}
				}
			}
		}

void exportMaps(int game, int mode, int LevelNo, char OutFileName[255], char filePath[255])
	{
	ObjectItem objList[1600];
	//shockObjectItem shockobjList[1600];
	long texture_map[256];
	long texture_map_shock[272];
	int roomIndex = 1;
	char Map_Output_File[255];
	char Script_Output_File[255];
	//char *filePath;

	if ((mode == D3_MODE))
		{
		sprintf_s(Map_Output_File, 255, "%s.map", OutFileName);
		if (fopen_s(&MAPFILE, Map_Output_File, "w") != 0)
			{
			printf("Unable to create output file for map");
			return;
			}
		}
	if ((mode == SOURCE_MODE))
		{
		sprintf_s(Map_Output_File, 255, "%s.vmf", OutFileName);
		if (fopen_s(&MAPFILE, Map_Output_File, "w") != 0)
			{
			printf("Unable to create output file for map");
			return;
			}
		}
	switch (game)
		{
			case UWDEMO:		//Underworld Demo
				{
				//filePath = UW0_LEVEL_PATH ;	//"C:\\Games\\Ultima\\UWDemo\\DATA\\level13.st";
				BuildTileMapUW(LevelInfo, objList, texture_map, filePath, game, LevelNo);
				setTileNeighbourCount(LevelInfo);
				BuildObjectListUW(LevelInfo, objList, texture_map, filePath, game, LevelNo);
				setObjectTileXY(game, LevelInfo, objList);
				SetExitBits(LevelInfo, objList);
				setDoorBits(LevelInfo, objList);
				//setCorridors(LevelInfo, &roomIndex);
				setRooms(LevelInfo, &roomIndex);
				setPatchBits(LevelInfo, objList);
				setElevatorBits(LevelInfo, objList);
				setTerrainChangeBits(LevelInfo, objList);
				setKeyCount(game, LevelInfo, objList);
				PrintUWObjects(objList);	//Since I can't get full debug info until I have TileX/Y set.
				MergeWaterRegions(LevelInfo);
				//			
				break;
				}
			case UW1:		//Underworld 1
				{
				//filePath = UW1_LEVEL_PATH ;	// "C:\\Games\\Ultima\\UW1\\DATA\\lev.ark";
				char GravePath[255];
				sprintf_s(GravePath, 255, "%s\\%s", path_uw1, UW1_GRAVES);
				UWCommonObj(game);				
				UW1_Graves(GravePath);
				BuildTileMapUW(LevelInfo, objList, texture_map, filePath, game, LevelNo);
				setTileNeighbourCount(LevelInfo);
				BuildObjectListUW(LevelInfo, objList, texture_map, filePath, game, LevelNo);
				setObjectTileXY(game, LevelInfo, objList);
				SetExitBits(LevelInfo, objList);
				setDoorBits(LevelInfo, objList);
				setBridgeBits(LevelInfo, objList);
				//setCorridors(LevelInfo, &roomIndex);
				setRooms(LevelInfo, &roomIndex);
				setPatchBits(LevelInfo, objList);
				setElevatorBits(LevelInfo, objList);
				setTerrainChangeBits(LevelInfo, objList);
				SetBullFrog(LevelInfo, objList, LevelNo);
				setKeyCount(game, LevelInfo, objList);
				PrintUWObjects(objList);	//Since I can't get full debug info until I have TileX/Y set.
				//CleanUp(LevelInfo,game); //Get rid of unneeded tiles.
				MergeWaterRegions(LevelInfo);
				MergeLavaRegions(LevelInfo);
				break;
				}
			case UW2:		//Underworld 2
				{
				//filePath = UW2_LEVEL_PATH;	//"C:\\Games\\Ultima\\UW2\\DATA\\lev.ark";
				if (BuildTileMapUW(LevelInfo, objList, texture_map, filePath, game, LevelNo) == -1) { return; };
				setTileNeighbourCount(LevelInfo);
				BuildObjectListUW(LevelInfo, objList, texture_map, filePath, game, LevelNo);
				setObjectTileXY(game, LevelInfo, objList);
				setDoorBits(LevelInfo, objList);
				//setCorridors(LevelInfo, &roomIndex);
				setRooms(LevelInfo, &roomIndex);
				setPatchBits(LevelInfo, objList);
				setElevatorBits(LevelInfo, objList);
				setTerrainChangeBits(LevelInfo, objList);
				setKeyCount(game, LevelInfo, objList);
				PrintUWObjects(objList);	//Since I can't get full debug info until I have TileX/Y set.
				//CleanUp(LevelInfo,game); //Get rid of unneeded tiles.
				MergeWaterRegions(LevelInfo);
				break;
				}
			case SHOCK:		//system shock
				{
				//filePath = SHOCK_LEVEL_PATH;	//"C:\\Games\\SystemShock\\Res\\DATA\\archive.dat";
				BuildTileMapShock(LevelInfo, objList, texture_map_shock, filePath, game, LevelNo);
				BuildObjectListShock(LevelInfo, objList, texture_map_shock, filePath, game, LevelNo);
				SetDeathWatch(objList);
				setTileNeighbourCount(LevelInfo);
				setDoorBits(LevelInfo, objList);
				//setCorridors(LevelInfo, &roomIndex);
				setRooms(LevelInfo, &roomIndex);
				setKeyCount(game, LevelInfo, objList);
				//CleanUp(LevelInfo,game); //Get rid of unneeded tiles.
				break;
				}
			default:
				{printf("Invalid Game!"); return; }
		}


	switch (mode)
		{
			case ASCII_MODE:		//ascii + other data maps
				{
				CleanUp(LevelInfo, game);
				DumpAscii(game, LevelInfo, objList, LevelNo, 0);	//1 for maps only. 0 for extra printouts like objects, textures + heightmaps.

				break;
				}
			case D3_MODE:		//D3/Dark Mod
				{
				RenderDarkModLevel(LevelInfo, objList, game);
				fclose(MAPFILE);
				//break;
				}
			case SOURCE_MODE:	//Source engine
				{
				CleanUp(LevelInfo, game); //Get rid of unneeded tiles.
				RenderSourceEnginelLevel(LevelInfo, objList, game);
				fclose(MAPFILE);
				}
			case SCRIPT_BUILD_MODE:
				{
				sprintf_s(Script_Output_File, 255, "%s.script", OutFileName);
				if (game != SHOCK)
					{
					buildScriptsUW(game, LevelInfo, objList, LevelNo, Script_Output_File);
					}
				else
					{
					BuildScriptsShock(game, LevelInfo, objList, LevelNo, Script_Output_File);
					}
				break;
				}
			case FBX_MODE:
				CleanUp(LevelInfo, game); //Get rid of unneeded tiles.
				//printf("%d", LevelInfo[3][5].Render);
				//TODO: restore this for FBX RenderFBXLevel(LevelInfo, objList, game);
				break;
			case UNITY_MODE:
				RenderUnityObjectList(game, LevelNo, LevelInfo, objList);
				break;
			case UNITY_TILEMAP_MODE:
				PrintUnityTileMap(game, LevelNo, LevelInfo);
				break;
		}
}

void PrintCharClass(int classType)
	{
	switch (classType)
		{
		case 0:
			fprintf(LOGFILE, "FIGHTER");break;
		case 1:
			fprintf(LOGFILE, "MAGE"); break;
		case 2:
			fprintf(LOGFILE, "BARD"); break;
		case 3:
			fprintf(LOGFILE, "TINKER"); break;
		case 4:
			fprintf(LOGFILE, "DRUID"); break;
		case 5:
			fprintf(LOGFILE, "PALADIN"); break;
		case 6:
			fprintf(LOGFILE, "RANGER"); break;
		case 7:
			fprintf(LOGFILE, "SHEPARD"); break;
		default:
			fprintf(LOGFILE, "UNKNOWN!"); break;
		}

	}
void PrintSkill(int SkillType)
	{
	switch (SkillType)
		{
		case 0: //Attack
			fprintf(LOGFILE, "%d //Attack", SkillType+31); break;
		case 1: //Defense
			fprintf(LOGFILE, "%d //Defense", SkillType+31); break;
		case 2: //unarmed
			fprintf(LOGFILE, "%d //Unarmed", SkillType+31); break;
		case 3: //Sword
			fprintf(LOGFILE, "%d //Sword", SkillType+31); break;
		case 4: //Axe
			fprintf(LOGFILE, "%d //Axe", SkillType+31); break;
		case 5: //Mace
			fprintf(LOGFILE, "%d //Mace", SkillType+31); break;
		case 6: //Missile
			fprintf(LOGFILE, "%d //Missile", SkillType+31); break;
		case 7: //Mana
			fprintf(LOGFILE, "%d //Mana", SkillType+31); break;
		case 8: //Lore
			fprintf(LOGFILE, "%d //Lore", SkillType+31); break;
		case 9: //Casting
			fprintf(LOGFILE, "%d //Casting", SkillType+31); break;
		case 10: //Traps
			fprintf(LOGFILE, "%d //Traps", SkillType+31); break;
		case 11: //Search
			fprintf(LOGFILE, "%d //Search", SkillType+31); break;
		case 12: //Track
			fprintf(LOGFILE, "%d //Track", SkillType + 31); break;
		case 13: //Sneak
			fprintf(LOGFILE, "%d //Sneak", SkillType + 31); break;
		case 14: //Repair
			fprintf(LOGFILE, "%d //Repair", SkillType + 31); break;
		case 15: //Charm
			fprintf(LOGFILE, "%d //Charm", SkillType + 31); break;
		case 16: //Pick Lock
			fprintf(LOGFILE, "%d //Picklock", SkillType + 31); break;
		case 17: //Acrobat
			fprintf(LOGFILE, "%d //Acrobat", SkillType + 31); break;
		case 18: //Appraise
			fprintf(LOGFILE, "%d //Appraise", SkillType + 31); break;
		case 19: //Swimming
			fprintf(LOGFILE, "%d //Swimming", SkillType + 31); break;
		default: //Unknown
			fprintf(LOGFILE, "%d //UNKNOWN!", SkillType); break;
		}
	}

	void ExtractSkills()
		{
		char filePath[255];
		unsigned char *Buffer;
		sprintf_s(filePath, 255, "%s\\%s", path_uw1, UW1_SKILLS);
		FILE *file = NULL;      // File pointer
		if (fopen_s(&file, filePath, "rb") != 0)
			{
			printf("Could not open specified file\n");
			return;
			}
		else
			{
			// Get the size of the file in bytes
			long fileSize = getFileSize(file);
			// Allocate space in the buffer for the whole file
			Buffer = new unsigned char[fileSize];
			fread(Buffer, fileSize, 1, file);
			fclose(file);
			int add_ptr=0x20; //Starts further out the file.
			for (int CharClass=0; CharClass<8;CharClass++)//For each character type.
				{//Five sets of skills
				fprintf(LOGFILE, "\nChar Class ");
				PrintCharClass(CharClass);
				fprintf(LOGFILE, "//(%d)\n", CharClass);
				fprintf(LOGFILE, "\t//Base Str %d\n", getValAtAddress(Buffer, CharClass * 4 + 0, 8));
				fprintf(LOGFILE, "\t//Base Dex %d\n", getValAtAddress(Buffer, CharClass * 4 + 1, 8));
				fprintf(LOGFILE, "\t//Base Int %d\n", getValAtAddress(Buffer, CharClass * 4 + 2, 8));
				fprintf(LOGFILE, "\t//Unk (seed?) %d\n", getValAtAddress(Buffer, CharClass * 4 + 3, 8));
				for (int SkillChoice = 0; SkillChoice<5;SkillChoice++)
					{
					fprintf(LOGFILE, "\tSkillChoices %d\n", SkillChoice);
					int NoOfEntries = getValAtAddress(Buffer,add_ptr++,8);
					for (int i = 0; i < NoOfEntries; i++)
						{
						fprintf(LOGFILE, "\t\tSkill : ");
						PrintSkill(getValAtAddress(Buffer, add_ptr++, 8));
						fprintf(LOGFILE, "\n");
						}
					}
				}
			}
	
	}
