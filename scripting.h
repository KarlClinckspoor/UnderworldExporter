#ifndef scripting_h
	#define scripting_h
void buildScriptsUW(int game, tile LevelInfo[64][64], ObjectItem objList[1600], int LevelNo, char Script_Final_File[255]);
void EMAILScript(const char objName[80], ObjectItem currObj, int logChunk);
void scriptChainFunctionsUW(int game, ObjectItem objList[1600], ObjectItem currObj, int *conditionalCount, int *TriggerTargetX, int *TriggerTargetY, int *TriggerQuality, int *TriggerFlags, int *TriggerHomeX, int *TriggerHomeY, int scriptLevelNo);
void BuildScriptsShock(int game, tile LevelInfo[64][64], ObjectItem objList[1600], int LevelNo, char Script_Final_File[255]);
void addGlobal(int varIndex);
#endif scripting_h