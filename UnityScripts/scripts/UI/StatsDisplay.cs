﻿using UnityEngine;
using System.Collections;

public class StatsDisplay : GuiBase {
	public UILabel CharName;
	public UILabel CharClass;
	public UILabel CharClassLevel;
	public UILabel CharStr;
	public UILabel CharDex;
	public UILabel CharInt;
	public UILabel CharVIT;
	public UILabel CharMana;
	public UILabel CharEXP;
	public UILabel CharSkills;
	public UILabel CharSkillLevels;

	public static int Offset;

	//public static UWCharacter playerUW;

	private string[] Skillnames = {"ATTACK","DEFENSE","UNARMED","SWORD","AXE","MACE","MISSILE",
									"MANA","LORE","CASTING","TRAPS","SEARCH","TRACK","SNEAK","REPAIR",
									"CHARM","PICKLOCK","ACROBAT","APPRAISE","SWIMMING"};
	public static bool UpdateNow=true;


	string tmpSkillNames;
	string tmpSkillValues;

	// Update is called once per frame
	void Update () {
		if (UpdateNow==true)
		{
			UpdateNow=false;
			CharName.text=playerUW.CharName;
			CharClass.text=playerUW.CharClass;
			CharClassLevel.text =playerUW.CharLevel.ToString();
			CharStr.text=playerUW.PlayerSkills.STR.ToString();
			CharDex.text=playerUW.PlayerSkills.DEX.ToString();
			CharInt.text=playerUW.PlayerSkills.INT.ToString();
			CharVIT.text = playerUW.CurVIT +"/"+playerUW.MaxVIT;
			CharMana.text = playerUW.PlayerMagic.CurMana +"/"+playerUW.PlayerMagic.MaxMana;
			CharEXP.text=playerUW.EXP.ToString ();
			tmpSkillNames="";
			tmpSkillValues="";
			if (Offset>15)
			{
				Offset=15;
			}
			if (Offset<0)
			{
				Offset=0;
			}
			for (int i = 0; i<=5;i++)
			{
				tmpSkillNames = tmpSkillNames + Skillnames[i+Offset];
				tmpSkillValues=tmpSkillValues+playerUW.PlayerSkills.GetSkill(i+Offset);
				if (i!=5)
				{
					tmpSkillNames = tmpSkillNames +"\n";
					tmpSkillValues = tmpSkillValues +"\n";
				}
			}
			CharSkills.text=tmpSkillNames;
			CharSkillLevels.text=tmpSkillValues;
		}
	}
}